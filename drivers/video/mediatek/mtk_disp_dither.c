// SPDX-License-Identifier: GPL-2.0-only
/*
 * MediaTek Video Dither support
 *
 * Copyright (c) 2026 BayLibre, SAS.
 * Author: Julien Masson <jmasson@baylibre.com>
 *         Julien Stephan <jstephan@baylibre.com>
 */

#include <dm/device.h>
#include <fdtdec.h>
#include <linux/bitfield.h>
#include <linux/bitops.h>

#include "mtk_disp_comp.h"
#include "mtk_disp_dither.h"

#define DISP_REG_DITHER_EN	0x00
#define DITHER_EN			BIT(0)

#define DISP_REG_DITHER_CFG	0x20
#define DITHER_RELAY_MODE		BIT(0)
#define DITHER_ENGINE_EN		BIT(1)

#define DISP_REG_DITHER_SIZE	0x30
#define DISP_REG_DITHER_SIZE_HSIZE	GENMASK(28, 16)
#define DISP_REG_DITHER_SIZE_VSIZE	GENMASK(12, 0)

#define DISP_REG_DITHER_5	0x114
#define DISP_REG_DITHER_7	0x11c

#define DISP_REG_DITHER_15	0x13c
#define DITHER_LSB_ERR_SHIFT_R		GENMASK(30, 28)
#define DITHER_ADD_LSHIFT_R		GENMASK(22, 20)
#define DITHER_NEW_BIT_MODE		BIT(0)

#define DISP_REG_DITHER_16	0x140
#define DITHER_LSB_ERR_SHIFT_B		GENMASK(30, 28)
#define DITHER_ADD_LSHIFT_B		GENMASK(22, 20)
#define DITHER_LSB_ERR_SHIFT_G		GENMASK(14, 12)
#define DITHER_ADD_LSHIFT_G		GENMASK(6, 4)

/* the pipeline carries 10 bits per component, the panel takes 8 */
#define MTK_MAX_BPC			10
#define MTK_DITHER_BPC			8
#define MTK_DITHER_SHIFT		(MTK_MAX_BPC - MTK_DITHER_BPC)

void mtk_dither_config(struct udevice *dev, const struct display_timing *timing)
{
	u32 sz;

	sz = FIELD_PREP(DISP_REG_DITHER_SIZE_HSIZE, timing->hactive.typ) |
	     FIELD_PREP(DISP_REG_DITHER_SIZE_VSIZE, timing->vactive.typ);

	mtk_disp_comp_write(dev, DISP_REG_DITHER_SIZE, sz);
	mtk_disp_comp_write(dev, DISP_REG_DITHER_CFG, DITHER_RELAY_MODE);

	/* as mtk_dither_set_common() in Linux */
	mtk_disp_comp_write(dev, DISP_REG_DITHER_5, 0);
	mtk_disp_comp_write(dev, DISP_REG_DITHER_7, 0);
	mtk_disp_comp_write(dev, DISP_REG_DITHER_15,
			    FIELD_PREP(DITHER_LSB_ERR_SHIFT_R, MTK_DITHER_SHIFT) |
			    FIELD_PREP(DITHER_ADD_LSHIFT_R, MTK_DITHER_SHIFT) |
			    DITHER_NEW_BIT_MODE);
	mtk_disp_comp_write(dev, DISP_REG_DITHER_16,
			    FIELD_PREP(DITHER_LSB_ERR_SHIFT_B, MTK_DITHER_SHIFT) |
			    FIELD_PREP(DITHER_ADD_LSHIFT_B, MTK_DITHER_SHIFT) |
			    FIELD_PREP(DITHER_LSB_ERR_SHIFT_G, MTK_DITHER_SHIFT) |
			    FIELD_PREP(DITHER_ADD_LSHIFT_G, MTK_DITHER_SHIFT));
	mtk_disp_comp_write(dev, DISP_REG_DITHER_CFG, DITHER_ENGINE_EN);
}

void mtk_dither_start(struct udevice *dev)
{
	mtk_disp_comp_write(dev, DISP_REG_DITHER_EN, DITHER_EN);
}

static const struct udevice_id mtk_disp_dither_ids[] = {
	{ .compatible = "mediatek,mt8183-disp-dither" },
	{ }
};

U_BOOT_DRIVER(mtk_disp_dither) = {
	.name	   = "mtk_disp_dither",
	.id	   = UCLASS_MISC,
	.of_match  = mtk_disp_dither_ids,
	.probe	   = mtk_disp_comp_probe,
	.priv_auto = sizeof(struct mtk_disp_comp_priv),
};
