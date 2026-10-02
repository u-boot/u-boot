// SPDX-License-Identifier: GPL-2.0-only
/*
 * MediaTek Video Adaptive Ambient Light support
 *
 * Copyright (c) 2026 BayLibre, SAS.
 * Author: Julien Masson <jmasson@baylibre.com>
 *         Julien Stephan <jstephan@baylibre.com>
 */

#include <dm/device.h>
#include <fdtdec.h>
#include <linux/bitfield.h>
#include <linux/bitops.h>

#include "mtk_disp_aal.h"
#include "mtk_disp_comp.h"

#define DISP_AAL_EN		0x00
#define AAL_EN				BIT(0)

#define DISP_AAL_SIZE		0x30
#define DISP_AAL_SIZE_HSIZE		GENMASK(28, 16)
#define DISP_AAL_SIZE_VSIZE		GENMASK(12, 0)

#define DISP_AAL_OUTPUT_SIZE	0x4d8

void mtk_aal_config(struct udevice *dev, const struct display_timing *timing)
{
	u32 sz;

	sz = FIELD_PREP(DISP_AAL_SIZE_HSIZE, timing->hactive.typ) |
	     FIELD_PREP(DISP_AAL_SIZE_VSIZE, timing->vactive.typ);

	mtk_disp_comp_write(dev, DISP_AAL_SIZE, sz);
	mtk_disp_comp_write(dev, DISP_AAL_OUTPUT_SIZE, sz);
}

void mtk_aal_start(struct udevice *dev)
{
	mtk_disp_comp_write(dev, DISP_AAL_EN, AAL_EN);
}

static const struct udevice_id mtk_disp_aal_ids[] = {
	{ .compatible = "mediatek,mt8183-disp-aal" },
	{ }
};

U_BOOT_DRIVER(mtk_disp_aal) = {
	.name	   = "mtk_disp_aal",
	.id	   = UCLASS_MISC,
	.of_match  = mtk_disp_aal_ids,
	.probe	   = mtk_disp_comp_probe,
	.priv_auto = sizeof(struct mtk_disp_comp_priv),
};
