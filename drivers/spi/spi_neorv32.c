// SPDX-License-Identifier: GPL-2.0+
/*
 * NEORV32 SPI controller driver
 */

#include <dm.h>
#include <fdtdec.h>
#include <spi.h>
#include <asm/io.h>
#include <linux/bitops.h>
#include <linux/errno.h>

#include <neorv32/neorv32.h>
#include <neorv32/neorv32_sysinfo.h>
#include <neorv32/neorv32_spi.h>

struct neorv32_spi_priv {
	void __iomem *base;
	u32 clock;
	u32 speed_hz;
	u8 mode;
	u8 prsc_sel;
	u8 cdiv;
};

static const u16 neorv32_prsc_lut[8] = { 2, 4, 8, 64, 128, 1024, 2048, 4096 };

static void neorv32_spi_update_ctrl(struct neorv32_spi_priv *priv)
{
	u32 ctrl = 0;

	ctrl |= BIT(SPI_CTRL_EN);
	ctrl |= ((priv->mode & SPI_CPHA) ? 1 : 0) << SPI_CTRL_CPHA;
	ctrl |= ((priv->mode & SPI_CPOL) ? 1 : 0) << SPI_CTRL_CPOL;
	ctrl |= (priv->prsc_sel & 0x7) << SPI_CTRL_PRSC0;
	ctrl |= (priv->cdiv & 0xf) << SPI_CTRL_CDIV0;

	writel(ctrl, priv->base + NEORV32_SPI_CTRL);
}

static int neorv32_spi_set_speed(struct udevice *dev, uint speed)
{
	struct neorv32_spi_priv *priv = dev_get_priv(dev);
	u32 best_hz = 0;
	u8 best_prsc = 7;
	u8 best_cdiv = 15;
	int prsc, cdiv;

	if (!speed)
		return -EINVAL;

	if (!priv->clock)
		priv->clock = neorv32_sysinfo_get_clk();

	for (prsc = 0; prsc < 8; prsc++) {
		for (cdiv = 0; cdiv < 16; cdiv++) {
			u32 denom = 2 * neorv32_prsc_lut[prsc] * (1 + cdiv);
			u32 hz = priv->clock / denom;

			if (hz <= speed && hz > best_hz) {
				best_hz = hz;
				best_prsc = prsc;
				best_cdiv = cdiv;
			}
		}
	}

	priv->speed_hz = best_hz ? best_hz : (priv->clock / (2 *
			neorv32_prsc_lut[best_prsc] * (1 + best_cdiv)));
	priv->prsc_sel = best_prsc;
	priv->cdiv = best_cdiv;

	neorv32_spi_update_ctrl(priv);

	return 0;
}

static int neorv32_spi_set_mode(struct udevice *dev, uint mode)
{
	struct neorv32_spi_priv *priv = dev_get_priv(dev);

	priv->mode = mode;
	neorv32_spi_update_ctrl(priv);

	return 0;
}

static void neorv32_spi_cs(struct neorv32_spi_priv *priv, int cs, bool enable)
{
	u32 cmd;

	if (enable)
		cmd = BIT(SPI_DATA_CMD) | BIT(SPI_DATA_CSEN) | (cs & 0x7);
	else
		cmd = BIT(SPI_DATA_CMD);

	writel(cmd, priv->base + NEORV32_SPI_DATA);
}

static int neorv32_spi_xfer(struct udevice *dev, unsigned int bitlen,
			    const void *dout, void *din, unsigned long flags)
{
	struct udevice *bus = dev->parent;
	struct neorv32_spi_priv *priv = dev_get_priv(bus);
	struct dm_spi_slave_plat *slave_plat = dev_get_parent_plat(dev);
	const u8 *tx = dout;
	u8 *rx = din;
	u32 ctrl;
	unsigned int len;
	unsigned int i;
	u8 byte;

	if (bitlen % 8)
		return -EINVAL;

	len = bitlen / 8;

	if (flags & SPI_XFER_BEGIN)
		neorv32_spi_cs(priv, slave_plat->cs[0], true);

	for (i = 0; i < len; i++) {
		byte = tx ? tx[i] : 0xFF;

		do {
			ctrl = readl(priv->base + NEORV32_SPI_CTRL);
		} while (ctrl & BIT(SPI_CTRL_TX_FULL));

		writel((u32)byte, priv->base + NEORV32_SPI_DATA);

		do {
			ctrl = readl(priv->base + NEORV32_SPI_CTRL);
		} while (ctrl & BIT(SPI_CTRL_BUSY));

		if (rx)
			rx[i] = readl(priv->base + NEORV32_SPI_DATA) & 0xff;
	}

	if (flags & SPI_XFER_END)
		neorv32_spi_cs(priv, slave_plat->cs[0], false);

	return 0;
}

static int neorv32_spi_of_to_plat(struct udevice *dev)
{
	struct neorv32_spi_priv *priv = dev_get_priv(dev);
	fdt_addr_t addr;

	addr = dev_read_addr(dev);
	if (addr == FDT_ADDR_T_NONE)
		return -EINVAL;

	priv->base = (void __iomem *)(uintptr_t)addr;

	dev_read_u32(dev, "clock-frequency", &priv->clock);

	return 0;
}

static const struct dm_spi_ops neorv32_spi_ops = {
	.xfer		= neorv32_spi_xfer,
	.set_speed	= neorv32_spi_set_speed,
	.set_mode	= neorv32_spi_set_mode,
};

static const struct udevice_id neorv32_spi_ids[] = {
	{ .compatible = "neorv32,spi" },
	{ }
};

U_BOOT_DRIVER(neorv32_spi) = {
	.name		= "neorv32_spi",
	.id		= UCLASS_SPI,
	.of_match	= neorv32_spi_ids,
	.of_to_plat	= neorv32_spi_of_to_plat,
	.priv_auto	= sizeof(struct neorv32_spi_priv),
	.ops		= &neorv32_spi_ops,
};
