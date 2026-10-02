// SPDX-License-Identifier: GPL-2.0-only
/*
 * MediaTek Video ETHDR support
 *
 * Copyright (c) 2026 BayLibre, SAS.
 * Author: Julien Stephan <jstephan@baylibre.com>
 */

#include <asm/io.h>
#include <dm.h>
#include <errno.h>
#include <linux/bitops.h>

#include "mtk_disp_comp.h"
#include "mtk_ethdr.h"

#define MIX_EN				0xc
#define MIX_ROI_SIZE			0x18
#define MIX_DATAPATH_CON		0x1c
#define OUTPUT_NO_RND				BIT(3)
#define SOURCE_RGB_SEL				BIT(7)
#define BACKGROUND_RELAY			(4 << 9)
#define MIX_ROI_BGCLR			0x20
#define BGCLR_BLACK				0xff000000
#define MIX_SRC_CON			0x24
#define MIX_L_SRC_CON(n)		(0x28 + 0x18 * (n))
#define NON_PREMULTI_SOURCE			(2 << 12)
#define MIX_L_SRC_SIZE(n)		(0x30 + 0x18 * (n))
#define MIX_L_SRC_OFFSET(n)		(0x34 + 0x18 * (n))
#define MIX_FUNC_DCM0			0x120
#define MIX_FUNC_DCM1			0x124
#define MIX_FUNC_DCM_ENABLE			0xffffffff

/* HDR video back-end registers, in the "vdo_be" register window */
#define HDR_VDO_BE_0204_VDO_DM_BE	0x204
#define HDR_VDO_BE_0204_BYPASS_ALL		0x7e

/* Fields used by this driver but not present in the upstream driver */
#define MIX_SRC_L2_EN				BIT(2)
#define L2_SRC_SEL				(2 << 20)
#define L2_OUT_SEL				(2 << 22)

void mtk_ethdr_config(struct udevice *dev, u16 width, u16 height)
{
	struct mtk_ethdr_priv *priv = dev_get_priv(dev);

	/*
	 * The HDR video back-end sits between the mixer output and the final
	 * merge, so put it in bypass: this pipeline does no tone mapping.
	 */
	writel(HDR_VDO_BE_0204_BYPASS_ALL,
	       priv->vdo_be + HDR_VDO_BE_0204_VDO_DM_BE);

	/* enable internal clocks */
	mtk_disp_comp_write(dev, MIX_FUNC_DCM0, MIX_FUNC_DCM_ENABLE);
	mtk_disp_comp_write(dev, MIX_FUNC_DCM1, MIX_FUNC_DCM_ENABLE);

	mtk_disp_comp_write(dev, MIX_ROI_SIZE, (height << 16) | width);
	mtk_disp_comp_write(dev, MIX_DATAPATH_CON,
			    OUTPUT_NO_RND | SOURCE_RGB_SEL | BACKGROUND_RELAY);
	mtk_disp_comp_write(dev, MIX_L_SRC_SIZE(2), (height << 16) | width);
	mtk_disp_comp_write(dev, MIX_L_SRC_OFFSET(2), 0);
	mtk_disp_comp_write(dev, MIX_ROI_BGCLR, BGCLR_BLACK);

	/* only configure L2 since only rdma4 and rdma5 are used */
	mtk_disp_comp_write(dev, MIX_L_SRC_CON(2), NON_PREMULTI_SOURCE);
	mtk_disp_comp_write(dev, MIX_SRC_CON,
			    MIX_SRC_L2_EN | L2_SRC_SEL | L2_OUT_SEL);

	/* enable the mixer once its geometry is set, as Linux does */
	mtk_disp_comp_write(dev, MIX_EN, 0x1);
}

static int mtk_ethdr_probe(struct udevice *dev)
{
	struct mtk_ethdr_priv *priv = dev_get_priv(dev);
	int ret;

	ret = mtk_disp_comp_probe(dev);
	if (ret)
		return ret;

	priv->vdo_be = dev_remap_addr_name(dev, "vdo_be");
	if (!priv->vdo_be)
		return -EINVAL;

	return 0;
}

static const struct udevice_id mtk_ethdr_ids[] = {
	{ .compatible = "mediatek,mt8195-disp-ethdr" },
	{ }
};

/*
 * The mixer is the first register window ("mixer" in reg-names) of the
 * ethdr block, so binding the ethdr node with default register index 0
 * gives the mixer registers.
 */
U_BOOT_DRIVER(mtk_ethdr) = {
	.name	   = "mtk_ethdr",
	.id	   = UCLASS_MISC,
	.of_match  = mtk_ethdr_ids,
	.probe	   = mtk_ethdr_probe,
	.priv_auto = sizeof(struct mtk_ethdr_priv),
};
