// SPDX-License-Identifier: GPL-2.0-only
/*
 * MediaTek Video Disp Mutex Support
 *
 * Copyright (c) 2026 BayLibre, SAS.
 * Author: Julien Stephan <jstephan@baylibre.com>
 */

#include <dm/device.h>
#include <dm/device_compat.h>
#include <dm/read.h>
#include <errno.h>
#include <linux/bitops.h>

#include "mtk_disp_comp.h"
#include "mtk_disp_mutex.h"

/*
 * Register layout of one mutex instance @id (as in the Linux mtk-mutex
 * driver): the block packs the per-mutex registers 0x20 apart. SOF holds
 * the start/end-of-frame source, MOD0/MOD1 the attached modules 0-31 and
 * 32-63. The offsets are index-based so the HDMI and DSI helpers below can
 * drive a different instance/controller each.
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

/*
 * The MT8365 and MT8366 DSI pipelines, and the MT8189 DSI and DVO ones, use
 * instance 0 of the mmsys mutex
 */
#define MUTEX_DISP_ID		0

/* the MT8365 shifts the end-of-frame selector by 6 instead of 7 */
#define MT8365_MUTEX_SOF_DSI0		1
#define MT8365_MUTEX_EOF_DSI0		(MT8365_MUTEX_SOF_DSI0 << 6)

#define MT8365_MUTEX_MOD_DISP_OVL0	7
#define MT8365_MUTEX_MOD_DISP_RDMA0	9
#define MT8365_MUTEX_MOD_DISP_COLOR0	12
#define MT8365_MUTEX_MOD_DISP_CCORR	13
#define MT8365_MUTEX_MOD_DISP_AAL	14
#define MT8365_MUTEX_MOD_DISP_GAMMA	15
#define MT8365_MUTEX_MOD_DISP_DITHER	16

/* the MT8366 drives the DSI straight out of the rdma, without the PQ blocks */
#define MT8366_MUTEX_SOF_DSI0		1
#define MT8366_MUTEX_EOF_DSI0		(MT8366_MUTEX_SOF_DSI0 << 7)

#define MT8366_MUTEX_MOD_DISP_OVL0	0
#define MT8366_MUTEX_MOD_DISP_RDMA0	4
#define MT8366_MUTEX_MOD_DISP_DSI0	13

/* the MT8189 matches the MT8366, except for where the DSI sits in MOD0 */
#define MT8189_MUTEX_SOF_DSI0		1
#define MT8189_MUTEX_EOF_DSI0		(MT8189_MUTEX_SOF_DSI0 << 7)

#define MT8189_MUTEX_MOD_DISP_OVL0	0
#define MT8189_MUTEX_MOD_DISP_RDMA0	4
#define MT8189_MUTEX_MOD_DISP_DSI0	22

/* the DVO output stage replaces the DSI on the eDP path of the same mmsys */
#define MT8189_MUTEX_SOF_DVO0		5
#define MT8189_MUTEX_EOF_DVO0		(MT8189_MUTEX_SOF_DVO0 << 7)

#define MT8189_MUTEX_MOD_DISP_DVO0	21

/**
 * struct mtk_disp_mutex_data - per-SoC mutex data
 * @dsi_mod: MOD0 bits of the components of the OVL -> DSI pipeline
 * @dsi_sof: start and end-of-frame selectors of the DSI
 * @dvo_mod: MOD0 bits of the components of the OVL -> DVO pipeline, 0 on
 *	     the SoCs without a DVO output stage
 * @dvo_sof: start and end-of-frame selectors of the DVO
 * @no_clk: the mutex has no clock of its own
 */
struct mtk_disp_mutex_data {
	u32 dsi_mod;
	u32 dsi_sof;
	u32 dvo_mod;
	u32 dvo_sof;
	bool no_clk;
};

/**
 * struct mtk_disp_mutex_priv - private data of a mutex
 * @comp: common component state, must be the first member
 * @data: per-SoC data, NULL on the SoCs without a DSI pipeline
 */
struct mtk_disp_mutex_priv {
	struct mtk_disp_comp_priv comp;
	const struct mtk_disp_mutex_data *data;
};

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

static const struct mtk_disp_mutex_data mt8365_mutex_data = {
	.dsi_mod = MUTEX_MOD_BIT(MT8365_MUTEX_MOD_DISP_OVL0) |
		   MUTEX_MOD_BIT(MT8365_MUTEX_MOD_DISP_RDMA0) |
		   MUTEX_MOD_BIT(MT8365_MUTEX_MOD_DISP_COLOR0) |
		   MUTEX_MOD_BIT(MT8365_MUTEX_MOD_DISP_CCORR) |
		   MUTEX_MOD_BIT(MT8365_MUTEX_MOD_DISP_AAL) |
		   MUTEX_MOD_BIT(MT8365_MUTEX_MOD_DISP_GAMMA) |
		   MUTEX_MOD_BIT(MT8365_MUTEX_MOD_DISP_DITHER),
	.dsi_sof = MT8365_MUTEX_EOF_DSI0 | MT8365_MUTEX_SOF_DSI0,
	.no_clk = true,
};

static const struct mtk_disp_mutex_data mt8366_mutex_data = {
	.dsi_mod = MUTEX_MOD_BIT(MT8366_MUTEX_MOD_DISP_OVL0) |
		   MUTEX_MOD_BIT(MT8366_MUTEX_MOD_DISP_RDMA0) |
		   MUTEX_MOD_BIT(MT8366_MUTEX_MOD_DISP_DSI0),
	.dsi_sof = MT8366_MUTEX_EOF_DSI0 | MT8366_MUTEX_SOF_DSI0,
};

static const struct mtk_disp_mutex_data mt8189_mutex_data = {
	.dsi_mod = MUTEX_MOD_BIT(MT8189_MUTEX_MOD_DISP_OVL0) |
		   MUTEX_MOD_BIT(MT8189_MUTEX_MOD_DISP_RDMA0) |
		   MUTEX_MOD_BIT(MT8189_MUTEX_MOD_DISP_DSI0),
	.dsi_sof = MT8189_MUTEX_EOF_DSI0 | MT8189_MUTEX_SOF_DSI0,
	.dvo_mod = MUTEX_MOD_BIT(MT8189_MUTEX_MOD_DISP_OVL0) |
		   MUTEX_MOD_BIT(MT8189_MUTEX_MOD_DISP_RDMA0) |
		   MUTEX_MOD_BIT(MT8189_MUTEX_MOD_DISP_DVO0),
	.dvo_sof = MT8189_MUTEX_EOF_DVO0 | MT8189_MUTEX_SOF_DVO0,
};

static void mtk_disp_mutex_enable(struct udevice *dev, u32 mod, u32 sof)
{
	mtk_disp_comp_write(dev, DISP_MUTEX_MOD0(MUTEX_DISP_ID), mod);
	mtk_disp_comp_write(dev, DISP_MUTEX_SOF(MUTEX_DISP_ID), sof);
	mtk_disp_comp_write(dev, DISP_MUTEX_EN(MUTEX_DISP_ID), MUTEX_EN);
}

int mtk_disp_mutex_ovl_dsi_enable(struct udevice *dev)
{
	struct mtk_disp_mutex_priv *mutex = dev_get_priv(dev);

	if (!mutex->data || !mutex->data->dsi_mod) {
		dev_err(dev, "no DSI pipeline on this mutex\n");
		return -EOPNOTSUPP;
	}

	mtk_disp_mutex_enable(dev, mutex->data->dsi_mod, mutex->data->dsi_sof);

	return 0;
}

int mtk_disp_mutex_ovl_dvo_enable(struct udevice *dev)
{
	struct mtk_disp_mutex_priv *mutex = dev_get_priv(dev);

	if (!mutex->data || !mutex->data->dvo_mod) {
		dev_err(dev, "no DVO pipeline on this mutex\n");
		return -EOPNOTSUPP;
	}

	mtk_disp_mutex_enable(dev, mutex->data->dvo_mod, mutex->data->dvo_sof);

	return 0;
}

static int mtk_disp_mutex_probe(struct udevice *dev)
{
	struct mtk_disp_mutex_priv *mutex = dev_get_priv(dev);

	/* the HDMI pipeline needs no per-SoC data */
	mutex->data = (const struct mtk_disp_mutex_data *)dev_get_driver_data(dev);

	if (mutex->data && mutex->data->no_clk) {
		mutex->comp.base = dev_remap_addr(dev);

		return mutex->comp.base ? 0 : -EINVAL;
	}

	return mtk_disp_comp_probe(dev);
}

static const struct udevice_id mtk_disp_mutex_ids[] = {
	{
		.compatible = "mediatek,mt8188-disp-mutex",
	},
	{
		.compatible = "mediatek,mt8189-disp-mutex",
		.data = (ulong)&mt8189_mutex_data,
	},
	{
		.compatible = "mediatek,mt8365-disp-mutex",
		.data = (ulong)&mt8365_mutex_data,
	},
	{
		.compatible = "mediatek,mt8366-disp-mutex",
		.data = (ulong)&mt8366_mutex_data,
	},
	{ }
};

U_BOOT_DRIVER(mtk_disp_mutex) = {
	.name	   = "mtk_disp_mutex",
	.id	   = UCLASS_MISC,
	.of_match  = mtk_disp_mutex_ids,
	.probe	   = mtk_disp_mutex_probe,
	.priv_auto = sizeof(struct mtk_disp_mutex_priv),
};
