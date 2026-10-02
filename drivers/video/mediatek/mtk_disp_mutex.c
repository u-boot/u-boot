// SPDX-License-Identifier: GPL-2.0-only
/*
 * MediaTek Video Disp Mutex Support
 *
 * Copyright (c) 2026 BayLibre, SAS.
 * Author: Julien Stephan <jstephan@baylibre.com>
 */

#include <dm.h>
#include <linux/bitops.h>

#include "mtk_disp_comp.h"
#include "mtk_disp_mutex.h"

/*
 * Register layout of one mutex instance @id (as in the Linux mtk-mutex
 * driver): the block packs the per-mutex registers 0x20 apart. SOF holds
 * the start/end-of-frame source, MOD0/MOD1 the attached modules 0-31 and
 * 32-63. The offsets are index-based so a future DSI helper can drive a
 * different instance/controller.
 */
#define DISP_MUTEX_EN(id)	(0x20 + 0x20 * (id))
#define MUTEX_EN			BIT(0)

#define DISP_MUTEX_SOF(id)	(0x2c + 0x20 * (id))
#define DISP_MUTEX_MOD0(id)	(0x30 + 0x20 * (id))
#define DISP_MUTEX_MOD1(id)	(0x34 + 0x20 * (id))

/* the modules are numbered across MOD0 and MOD1, as in Linux */
#define MUTEX_MOD_BIT(mod)	BIT((mod) % 32)

/*
 * The DPI1 -> HDMI pipeline uses mutex instance 1 of the vdosys1 mutex
 * controller. The SOF register packs the start-of-frame selector in the low
 * bits and the matching end-of-frame selector shifted by 7. The selectors
 * and the module indices are the ones of the Linux mtk-mutex driver.
 */
#define MUTEX_HDMI_ID				1
#define MT8188_MUTEX_SOF_DPI1			5
#define MT8188_MUTEX_EOF_DPI1			(MT8188_MUTEX_SOF_DPI1 << 7)

#define MT8188_MUTEX_MOD_DISP1_MDP_RDMA4	4
#define MT8188_MUTEX_MOD_DISP1_MDP_RDMA5	5
#define MT8188_MUTEX_MOD_DISP1_PADDING4		12
#define MT8188_MUTEX_MOD_DISP1_PADDING5		13
#define MT8188_MUTEX_MOD_DISP1_VPP_MERGE2	22
#define MT8188_MUTEX_MOD_DISP1_VPP_MERGE4	24
#define MT8188_MUTEX_MOD_DISP1_DISP_MIXER	30
#define MT8188_MUTEX_MOD_DISP1_DPI1		38

void mt8188_mutex_config_hdmi(struct udevice *dev)
{
	mtk_disp_comp_write(dev, DISP_MUTEX_SOF(MUTEX_HDMI_ID),
			    MT8188_MUTEX_EOF_DPI1 | MT8188_MUTEX_SOF_DPI1);
	mtk_disp_comp_write(dev, DISP_MUTEX_MOD0(MUTEX_HDMI_ID),
			    MUTEX_MOD_BIT(MT8188_MUTEX_MOD_DISP1_MDP_RDMA4) |
			    MUTEX_MOD_BIT(MT8188_MUTEX_MOD_DISP1_MDP_RDMA5) |
			    MUTEX_MOD_BIT(MT8188_MUTEX_MOD_DISP1_PADDING4) |
			    MUTEX_MOD_BIT(MT8188_MUTEX_MOD_DISP1_PADDING5) |
			    MUTEX_MOD_BIT(MT8188_MUTEX_MOD_DISP1_VPP_MERGE2) |
			    MUTEX_MOD_BIT(MT8188_MUTEX_MOD_DISP1_VPP_MERGE4) |
			    MUTEX_MOD_BIT(MT8188_MUTEX_MOD_DISP1_DISP_MIXER));
	mtk_disp_comp_write(dev, DISP_MUTEX_MOD1(MUTEX_HDMI_ID),
			    MUTEX_MOD_BIT(MT8188_MUTEX_MOD_DISP1_DPI1));
	mtk_disp_comp_write(dev, DISP_MUTEX_EN(MUTEX_HDMI_ID), MUTEX_EN);
}

static const struct udevice_id mtk_disp_mutex_ids[] = {
	{ .compatible = "mediatek,mt8188-disp-mutex" },
	{ }
};

U_BOOT_DRIVER(mtk_disp_mutex) = {
	.name	   = "mtk_disp_mutex",
	.id	   = UCLASS_MISC,
	.of_match  = mtk_disp_mutex_ids,
	.probe	   = mtk_disp_comp_probe,
	.priv_auto = sizeof(struct mtk_disp_comp_priv),
};
