// SPDX-License-Identifier: GPL-2.0-only
/*
 * MediaTek Video GAMMA support
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
#include "mtk_disp_gamma.h"

#define DISP_GAMMA_EN		0x00
#define GAMMA_EN			BIT(0)

#define DISP_GAMMA_SIZE		0x30
#define DISP_GAMMA_SIZE_HSIZE		GENMASK(28, 16)
#define DISP_GAMMA_SIZE_VSIZE		GENMASK(12, 0)

void mtk_gamma_config(struct udevice *dev, const struct display_timing *timing)
{
	u32 sz;

	sz = FIELD_PREP(DISP_GAMMA_SIZE_HSIZE, timing->hactive.typ) |
	     FIELD_PREP(DISP_GAMMA_SIZE_VSIZE, timing->vactive.typ);

	mtk_disp_comp_write(dev, DISP_GAMMA_SIZE, sz);
}

void mtk_gamma_start(struct udevice *dev)
{
	mtk_disp_comp_write(dev, DISP_GAMMA_EN, GAMMA_EN);
}

static const struct udevice_id mtk_disp_gamma_ids[] = {
	{ .compatible = "mediatek,mt8183-disp-gamma" },
	{ }
};

U_BOOT_DRIVER(mtk_disp_gamma) = {
	.name	   = "mtk_disp_gamma",
	.id	   = UCLASS_MISC,
	.of_match  = mtk_disp_gamma_ids,
	.probe	   = mtk_disp_comp_probe,
	.priv_auto = sizeof(struct mtk_disp_comp_priv),
};
