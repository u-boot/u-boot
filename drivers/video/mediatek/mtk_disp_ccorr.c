// SPDX-License-Identifier: GPL-2.0-only
/*
 * MediaTek Video Color Correction Engine support
 *
 * Copyright (c) 2026 BayLibre, SAS.
 * Author: Julien Masson <jmasson@baylibre.com>
 *         Julien Stephan <jstephan@baylibre.com>
 */

#include <dm/device.h>
#include <fdtdec.h>
#include <linux/bitfield.h>
#include <linux/bitops.h>

#include "mtk_disp_ccorr.h"
#include "mtk_disp_comp.h"

#define DISP_CCORR_EN		0x00
#define CCORR_EN			BIT(0)

#define DISP_CCORR_CFG		0x20
#define CCORR_ENGINE_EN			BIT(1)

#define DISP_CCORR_SIZE		0x30
#define DISP_CCORR_SIZE_HSIZE		GENMASK(28, 16)
#define DISP_CCORR_SIZE_VSIZE		GENMASK(12, 0)

void mtk_ccorr_config(struct udevice *dev, const struct display_timing *timing)
{
	u32 sz;

	sz = FIELD_PREP(DISP_CCORR_SIZE_HSIZE, timing->hactive.typ) |
	     FIELD_PREP(DISP_CCORR_SIZE_VSIZE, timing->vactive.typ);

	mtk_disp_comp_write(dev, DISP_CCORR_SIZE, sz);
	mtk_disp_comp_write(dev, DISP_CCORR_CFG, CCORR_ENGINE_EN);
}

void mtk_ccorr_start(struct udevice *dev)
{
	mtk_disp_comp_write(dev, DISP_CCORR_EN, CCORR_EN);
}

static const struct udevice_id mtk_disp_ccorr_ids[] = {
	{ .compatible = "mediatek,mt8183-disp-ccorr" },
	{ }
};

U_BOOT_DRIVER(mtk_disp_ccorr) = {
	.name	   = "mtk_disp_ccorr",
	.id	   = UCLASS_MISC,
	.of_match  = mtk_disp_ccorr_ids,
	.probe	   = mtk_disp_comp_probe,
	.priv_auto = sizeof(struct mtk_disp_comp_priv),
};
