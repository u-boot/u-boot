// SPDX-License-Identifier: GPL-2.0-only
/*
 * MediaTek Video Color support
 *
 * Copyright (c) 2026 BayLibre, SAS.
 * Author: Julien Masson <jmasson@baylibre.com>
 *         Julien Stephan <jstephan@baylibre.com>
 */

#include <dm/device.h>
#include <fdtdec.h>
#include <linux/bitops.h>

#include "mtk_disp_color.h"
#include "mtk_disp_comp.h"

#define DISP_COLOR_CFG_MAIN	0x400
#define COLOR_BYPASS_ALL		BIT(7)
#define COLOR_SEQ_SEL			BIT(13)

#define DISP_COLOR_START	0xc00

#define DISP_COLOR_WIDTH	0xc50
#define DISP_COLOR_HEIGHT	0xc54

void mtk_color_config(struct udevice *dev, const struct display_timing *timing)
{
	mtk_disp_comp_write(dev, DISP_COLOR_WIDTH, timing->hactive.typ);
	mtk_disp_comp_write(dev, DISP_COLOR_HEIGHT, timing->vactive.typ);
}

void mtk_color_start(struct udevice *dev)
{
	mtk_disp_comp_write(dev, DISP_COLOR_CFG_MAIN,
			    COLOR_BYPASS_ALL | COLOR_SEQ_SEL);
	mtk_disp_comp_write(dev, DISP_COLOR_START, 1);
}

static const struct udevice_id mtk_disp_color_ids[] = {
	{ .compatible = "mediatek,mt8173-disp-color" },
	{ }
};

U_BOOT_DRIVER(mtk_disp_color) = {
	.name	   = "mtk_disp_color",
	.id	   = UCLASS_MISC,
	.of_match  = mtk_disp_color_ids,
	.probe	   = mtk_disp_comp_probe,
	.priv_auto = sizeof(struct mtk_disp_comp_priv),
};
