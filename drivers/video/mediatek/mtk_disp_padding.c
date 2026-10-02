// SPDX-License-Identifier: GPL-2.0-only
/*
 * MediaTek Video Disp Padding Support
 *
 * Copyright (c) 2026 BayLibre, SAS.
 * Author: Julien Stephan <jstephan@baylibre.com>
 */

#include <dm.h>
#include <linux/bitops.h>

#include "mtk_disp_comp.h"
#include "mtk_disp_padding.h"

#define PADDING_CONTROL_REG	0x00
#define PADDING_BYPASS			BIT(0)
#define PADDING_ENABLE			BIT(1)
#define PADDING_PIC_SIZE_REG	0x04
#define PADDING_H_REG		0x08 /* horizontal */
#define PADDING_V_REG		0x0c /* vertical */
#define PADDING_COLOR_REG	0x10

void mtk_disp_padding_config(struct udevice *dev)
{
	mtk_disp_comp_write(dev, PADDING_CONTROL_REG,
			    PADDING_ENABLE | PADDING_BYPASS);

	/*
	 * Even in bypass mode all the settings must be cleared to 0 or
	 * undefined behaviours could happen.
	 */
	mtk_disp_comp_write(dev, PADDING_PIC_SIZE_REG, 0x0);
	mtk_disp_comp_write(dev, PADDING_H_REG, 0x0);
	mtk_disp_comp_write(dev, PADDING_V_REG, 0x0);
	mtk_disp_comp_write(dev, PADDING_COLOR_REG, 0x0);
}

static const struct udevice_id mtk_disp_padding_ids[] = {
	{ .compatible = "mediatek,mt8188-disp-padding" },
	{ }
};

U_BOOT_DRIVER(mtk_disp_padding) = {
	.name	   = "mtk_disp_padding",
	.id	   = UCLASS_MISC,
	.of_match  = mtk_disp_padding_ids,
	.probe	   = mtk_disp_comp_probe,
	.priv_auto = sizeof(struct mtk_disp_comp_priv),
};
