// SPDX-License-Identifier: GPL-2.0+
/*
 * NEORV32 UART driver
 */

#include <dm.h>
#include <fdtdec.h>
#include <serial.h>
#include <asm/io.h>
#include <linux/errno.h>
#include <linux/bitops.h>

#include <neorv32/neorv32.h>
#include <neorv32/neorv32_sysinfo.h>
#include <neorv32/neorv32_uart.h>

struct neorv32_uart_plat {
	void __iomem *base;
	u32 clock;
};

static void neorv32_uart_setbrg_hw(void __iomem *base, u32 clock, int baudrate)
{
	u32 prsc_sel = 0;
	u32 baud_div;
	u32 ctrl;

	if (!clock || !baudrate)
		return;

	baud_div = clock / (2 * baudrate);
	while (baud_div >= 0x3ffU) {
		if (prsc_sel == 2 || prsc_sel == 4)
			baud_div >>= 3;
		else
			baud_div >>= 1;
		prsc_sel++;
	}

	ctrl = 0;
	ctrl |= BIT(UART_CTRL_EN);
	ctrl |= (prsc_sel & 0x7) << UART_CTRL_PRSC_LSB;
	ctrl |= ((baud_div - 1) & 0x3ff) << UART_CTRL_BAUD_LSB;
	writel(ctrl, base + NEORV32_UART_CTRL);
}

static int neorv32_serial_setbrg(struct udevice *dev, int baudrate)
{
	struct neorv32_uart_plat *plat = dev_get_plat(dev);

	if (!plat->clock)
		plat->clock = neorv32_sysinfo_get_clk();

	neorv32_uart_setbrg_hw(plat->base, plat->clock, baudrate);

	return 0;
}

static int neorv32_serial_putc(struct udevice *dev, const char ch)
{
	struct neorv32_uart_plat *plat = dev_get_plat(dev);
	void __iomem *base = plat->base;
	u32 ctrl;

	if (ch == '\n')
		neorv32_serial_putc(dev, '\r');

	do {
		ctrl = readl(base + NEORV32_UART_CTRL);
	} while (!(ctrl & BIT(UART_CTRL_TX_NFULL)));

	writel((u32)ch, base + NEORV32_UART_DATA);

	return 0;
}

static int neorv32_serial_getc(struct udevice *dev)
{
	struct neorv32_uart_plat *plat = dev_get_plat(dev);
	void __iomem *base = plat->base;
	u32 ctrl;

	do {
		ctrl = readl(base + NEORV32_UART_CTRL);
	} while (!(ctrl & BIT(UART_CTRL_RX_NEMPTY)));

	return (readl(base + NEORV32_UART_DATA) >> UART_DATA_RTX_LSB) &
		GENMASK(UART_DATA_RTX_MSB, UART_DATA_RTX_LSB);
}

static int neorv32_serial_pending(struct udevice *dev, bool input)
{
	struct neorv32_uart_plat *plat = dev_get_plat(dev);
	u32 ctrl = readl(plat->base + NEORV32_UART_CTRL);

	if (input)
		return !!(ctrl & BIT(UART_CTRL_RX_NEMPTY));

	return !(ctrl & BIT(UART_CTRL_TX_NFULL));
}

static int neorv32_serial_of_to_plat(struct udevice *dev)
{
	struct neorv32_uart_plat *plat = dev_get_plat(dev);
	fdt_addr_t addr;

	addr = dev_read_addr(dev);
	if (addr == FDT_ADDR_T_NONE)
		return -EINVAL;

	plat->base = (void __iomem *)(uintptr_t)addr;

	dev_read_u32(dev, "clock-frequency", &plat->clock);

	return 0;
}

static const struct dm_serial_ops neorv32_serial_ops = {
	.putc = neorv32_serial_putc,
	.getc = neorv32_serial_getc,
	.pending = neorv32_serial_pending,
	.setbrg = neorv32_serial_setbrg,
};

static const struct udevice_id neorv32_serial_ids[] = {
	{ .compatible = "neorv32,uart" },
	{ }
};

U_BOOT_DRIVER(neorv32_serial) = {
	.name		= "neorv32_serial",
	.id		= UCLASS_SERIAL,
	.of_match	= neorv32_serial_ids,
	.of_to_plat	= neorv32_serial_of_to_plat,
	.plat_auto	= sizeof(struct neorv32_uart_plat),
	.ops		= &neorv32_serial_ops,
};
