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
#define RTVRAM_EXT_MODE			0x8500
#define RTVRAM_EXT_MODE_EXT		BIT(0)
#define RTVRAM_VBUF_BADDR		0xC580

#define RTVRAM_VBUF_NUM			7

#define SDRAM_40BIT_ADDR_TOP		0x0400000000ULL
#define RTVRAM_VBUF_AREA_SIZE		SZ_4M

struct renesas_dbsc5_rtvram_priv {
	void __iomem		*regs;
};

static int renesas_dbsc5_rtvram_probe(struct udevice *dev)
{
	struct renesas_dbsc5_rtvram_priv *priv = dev_get_priv(dev);
	u64 addr;
	int i;

	/* Set each 4MB from the top of SDRAM as the buffer area of RT-VRAM. */
	for (i = 0; i < RTVRAM_VBUF_NUM; i++) {
		addr = (SDRAM_40BIT_ADDR_TOP + (RTVRAM_VBUF_AREA_SIZE * i)) >> 16;
		writel(lower_32_bits(addr), priv->regs + (RTVRAM_VBUF_BADDR + (4 * i)));
	}

	/* Cache Mode: 8-way, VBF size: 28M */
	setbits_le32(priv->regs + RTVRAM_VBUF_CFG,
		     RTVRAM_VBUF_CFG_CACHE_MODE_8WAY | RTVRAM_VBUF_CFG_VBUF_SIZE_28M);

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

	info->base = (phys_addr_t)priv->regs;
	info->size = 28 * SZ_1M;

	return 0;
}

static const struct ram_ops renesas_dbsc5_rtvram_ops = {
	.get_info = renesas_dbsc5_rtvram_get_info,
};

static const struct udevice_id renesas_dbsc5_rtvram_ids[] = {
	{ .compatible = "renesas,r8a779g0-rtvram" },
	{ .compatible = "renesas,r8a779h0-rtvram" },
	{ /* sentinel */ }
};

U_BOOT_DRIVER(renesas_dbsc5_rtvram) = {
	.name		= "rtvram",
	.id		= UCLASS_RAM,
	.of_match	= renesas_dbsc5_rtvram_ids,
	.of_to_plat	= renesas_dbsc5_rtvram_of_to_plat,
	.ops		= &renesas_dbsc5_rtvram_ops,
	.probe		= renesas_dbsc5_rtvram_probe,
	.priv_auto	= sizeof(struct renesas_dbsc5_rtvram_priv),
};
