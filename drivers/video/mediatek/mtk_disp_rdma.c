// SPDX-License-Identifier: GPL-2.0-only
/*
 * MediaTek Video RDMA support
 *
 * Copyright (c) 2026 BayLibre, SAS.
 * Author: Julien Masson <jmasson@baylibre.com>
 *         Julien Stephan <jstephan@baylibre.com>
 */

#include <dm/device.h>
#include <dm/device_compat.h>
#include <dm/read.h>
#include <errno.h>
#include <fdtdec.h>
#include <linux/bitfield.h>
#include <linux/bitops.h>
#include <linux/sizes.h>

#include "mtk_disp_comp.h"
#include "mtk_disp_rdma.h"

#define DISP_REG_RDMA_GLOBAL_CON	0x010
#define RDMA_ENGINE_EN				BIT(0)

#define DISP_REG_RDMA_SIZE_CON_0	0x014
#define RDMA_SIZE_CON_0_WIDTH			GENMASK(11, 0)

#define DISP_REG_RDMA_SIZE_CON_1	0x018
#define RDMA_SIZE_CON_1_HEIGHT			GENMASK(19, 0)

#define DISP_REG_RDMA_FIFO_CON		0x040
#define RDMA_FIFO_UNDERFLOW_EN			BIT(31)
#define RDMA_FIFO_PSEUDO_SIZE			GENMASK(27, 16)
#define RDMA_OUTPUT_VALID_FIFO_THRESHOLD	GENMASK(11, 0)

/* the FIFO registers count in 16-byte words */
#define RDMA_FIFO_WORD(bytes)		((bytes) / 16)

/**
 * struct mtk_disp_rdma_data - per-SoC rdma data
 * @fifo_size: default FIFO size, in bytes
 */
struct mtk_disp_rdma_data {
	u32 fifo_size;
};

/**
 * struct mtk_disp_rdma_priv - private data of an rdma
 * @comp: common component state, must be the first member
 * @fifo_size: FIFO size, in bytes, from "mediatek,rdma-fifo-size" or the
 *	       per-SoC default
 */
struct mtk_disp_rdma_priv {
	struct mtk_disp_comp_priv comp;
	u32 fifo_size;
};

void mtk_rdma_config(struct udevice *dev, const struct display_timing *timing)
{
	struct mtk_disp_rdma_priv *rdma = dev_get_priv(dev);
	u32 threshold = rdma->fifo_size * 7 / 10;

	mtk_disp_comp_update(dev, DISP_REG_RDMA_SIZE_CON_0,
			     FIELD_PREP(RDMA_SIZE_CON_0_WIDTH, timing->hactive.typ),
			     RDMA_SIZE_CON_0_WIDTH);
	mtk_disp_comp_update(dev, DISP_REG_RDMA_SIZE_CON_1,
			     FIELD_PREP(RDMA_SIZE_CON_1_HEIGHT, timing->vactive.typ),
			     RDMA_SIZE_CON_1_HEIGHT);

	/*
	 * Enable FIFO underflow since the DSI and the DPI can't be blocked,
	 * and set the output threshold to 70% of the FIFO size so that it
	 * never overflows.
	 */
	mtk_disp_comp_write(dev, DISP_REG_RDMA_FIFO_CON,
			    RDMA_FIFO_UNDERFLOW_EN |
			    FIELD_PREP(RDMA_FIFO_PSEUDO_SIZE,
				       RDMA_FIFO_WORD(rdma->fifo_size)) |
			    FIELD_PREP(RDMA_OUTPUT_VALID_FIFO_THRESHOLD,
				       RDMA_FIFO_WORD(threshold)));
}

void mtk_rdma_start(struct udevice *dev)
{
	mtk_disp_comp_update(dev, DISP_REG_RDMA_GLOBAL_CON, RDMA_ENGINE_EN,
			     RDMA_ENGINE_EN);
}

static int mtk_disp_rdma_probe(struct udevice *dev)
{
	struct mtk_disp_rdma_priv *rdma = dev_get_priv(dev);
	const struct mtk_disp_rdma_data *data;
	int ret;

	data = (const struct mtk_disp_rdma_data *)dev_get_driver_data(dev);
	if (!data)
		return -EINVAL;

	ret = mtk_disp_comp_probe(dev);
	if (ret)
		return ret;

	/*
	 * The rdma sits behind an SMI larb, which has to be ungated with it
	 * even on the paths where the overlay is the one doing the DMA.
	 */
	ret = mtk_disp_comp_get_larb(dev);
	if (ret)
		return ret;

	/*
	 * The instances of a same SoC may have different FIFO depths, so the
	 * per-compatible size is only the default: the MT8365 and the MT8183
	 * describe both of their RDMAs in the device tree, the MT8186 none.
	 */
	ret = dev_read_u32(dev, "mediatek,rdma-fifo-size", &rdma->fifo_size);
	if (ret && ret != -EINVAL) {
		dev_err(dev, "failed to get the rdma fifo size: %d\n", ret);
		return ret;
	}

	if (ret)
		rdma->fifo_size = data->fifo_size;

	return 0;
}

static const struct mtk_disp_rdma_data mt8183_rdma_driver_data = {
	.fifo_size = 5 * SZ_1K,
};

static const struct mtk_disp_rdma_data mt8195_rdma_driver_data = {
	.fifo_size = 1920,
};

static const struct udevice_id mtk_disp_rdma_ids[] = {
	{
		.compatible = "mediatek,mt8183-disp-rdma",
		.data = (ulong)&mt8183_rdma_driver_data,
	},
	{
		.compatible = "mediatek,mt8195-disp-rdma",
		.data = (ulong)&mt8195_rdma_driver_data,
	},
	{ }
};

U_BOOT_DRIVER(mtk_disp_rdma) = {
	.name	   = "mtk_disp_rdma",
	.id	   = UCLASS_MISC,
	.of_match  = mtk_disp_rdma_ids,
	.probe	   = mtk_disp_rdma_probe,
	.priv_auto = sizeof(struct mtk_disp_rdma_priv),
};
