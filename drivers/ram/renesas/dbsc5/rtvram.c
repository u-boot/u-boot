// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2024 Renesas Electronics Corp.
 */

#include <asm/io.h>
#include <dm.h>
#include <errno.h>
#include <ram.h>
#include <linux/sizes.h>

/* RT-VRAM register base address */
#define RTVRAM_VBUF_CFG			0x6504
#define RTVRAM_VBUF_CFG_CACHE_MODE_8WAY	(1 << 8)
#define RTVRAM_VBUF_CFG_VBUF_SIZE_28M	(6 << 0)
#define RTVRAM_VBUF_CFG_VBUF_SIZE_32M	(7 << 0)
#define RTVRAM_EXT_MODE			0x8500
#define RTVRAM_EXT_MODE_EXT		BIT(0)
#define RTVRAM_VBUF_BADDR		0xC580

#define RTVRAM_VBUF_AREA_SIZE		SZ_4M

struct renesas_dbsc5_rtvram_priv {
	void __iomem		*regs;
};

struct renesas_dbsc5_rtvram_data {
	u8	sdram_40bit_addr_top;	/* Left-shift by 32 */
	u8	vbuf_num;
	u8	size;
	u8	size_bit;
};

static int renesas_dbsc5_rtvram_probe(struct udevice *dev)
{
	struct renesas_dbsc5_rtvram_priv *priv = dev_get_priv(dev);
	struct renesas_dbsc5_rtvram_data *data =
		(struct renesas_dbsc5_rtvram_data *)dev_get_driver_data(dev);
	u64 addr;
	int i;

	/* Set each 4MB from the top of SDRAM as the buffer area of RT-VRAM. */
	for (i = 0; i < data->vbuf_num + 1; i++) {
		addr = (((u64)(data->sdram_40bit_addr_top) << 32ULL) + (RTVRAM_VBUF_AREA_SIZE * i)) >> 16;
		writel(lower_32_bits(addr), priv->regs + (RTVRAM_VBUF_BADDR + (4 * i)));
	}

	/* Cache Mode: 8-way, VBF size: 28M */
	setbits_le32(priv->regs + RTVRAM_VBUF_CFG,
		     RTVRAM_VBUF_CFG_CACHE_MODE_8WAY | data->size_bit);

	/* Change from Compatible Mode to Extended Mode */
	writel(RTVRAM_EXT_MODE_EXT, priv->regs + RTVRAM_EXT_MODE);

	dsb();

	return 0;
}

static int renesas_dbsc5_rtvram_of_to_plat(struct udevice *dev)
{
	struct renesas_dbsc5_rtvram_priv *priv = dev_get_priv(dev);

	priv->regs = dev_read_addr_ptr(dev);
	if (!priv->regs)
		return -EINVAL;

	return 0;
}

static int renesas_dbsc5_rtvram_get_info(struct udevice *dev,
					 struct ram_info *info)
{
	struct renesas_dbsc5_rtvram_priv *priv = dev_get_priv(dev);
	struct renesas_dbsc5_rtvram_data *data =
		(struct renesas_dbsc5_rtvram_data *)dev_get_driver_data(dev);

	info->base = (phys_addr_t)priv->regs;
	info->size = data->size * SZ_1M;

	return 0;
}

static const struct ram_ops renesas_dbsc5_rtvram_ops = {
	.get_info = renesas_dbsc5_rtvram_get_info,
};

struct renesas_dbsc5_rtvram_data gen4_rtvram_data = {
	.sdram_40bit_addr_top = 0x4,
	.vbuf_num = 7,
	.size = 28,
	.size_bit = RTVRAM_VBUF_CFG_VBUF_SIZE_28M,
};

struct renesas_dbsc5_rtvram_data gen5_rtvram_data = {
	.sdram_40bit_addr_top = 0x12,
	.vbuf_num = 8,
	.size = 32,
	.size_bit = RTVRAM_VBUF_CFG_VBUF_SIZE_32M,
};

static const struct udevice_id renesas_dbsc5_rtvram_ids[] = {
	{ .compatible = "renesas,r8a779g0-rtvram", .data = (ulong)&gen4_rtvram_data },
	{ .compatible = "renesas,r8a779h0-rtvram", .data = (ulong)&gen4_rtvram_data },
	{ .compatible = "renesas,r8a78000-rtvram", .data = (ulong)&gen5_rtvram_data },
	{ /* sentinel */ }
};

U_BOOT_DRIVER(renesas_dbsc5_rtvram) = {
	.name		= "dbsc5_rtvram",
	.id		= UCLASS_RAM,
	.of_match	= renesas_dbsc5_rtvram_ids,
	.of_to_plat	= renesas_dbsc5_rtvram_of_to_plat,
	.ops		= &renesas_dbsc5_rtvram_ops,
	.probe		= renesas_dbsc5_rtvram_probe,
	.priv_auto	= sizeof(struct renesas_dbsc5_rtvram_priv),
};
