// SPDX-License-Identifier: GPL-2.0-only
/*
 * MediaTek Video MDP RDMA Support
 *
 * Copyright (c) 2026 BayLibre, SAS.
 * Author: Julien Stephan <jstephan@baylibre.com>
 */

#include <dm.h>
#include <dm/device_compat.h>
#include <errno.h>
#include <fdtdec.h>
#include <linux/bitfield.h>
#include <linux/bitops.h>
#include <video.h>

#include "mtk_disp_comp.h"
#include "mtk_mdp_rdma.h"

/* MDP RDMA registers (see Linux drivers/gpu/drm/mediatek/mtk_mdp_rdma.c) */
#define MDP_RDMA_EN			0x000
#define FLD_ROT_ENABLE				BIT(0)
#define MDP_RDMA_CON			0x020
#define FLD_SIMPLE_MODE				BIT(4)
#define MDP_RDMA_GMCIF_CON		0x028
#define FLD_COMMAND_DIV				BIT(0)
#define FLD_EXT_PREULTRA_EN			BIT(3)
#define FLD_RD_REQ_TYPE				GENMASK(7, 4)
#define VAL_RD_REQ_TYPE_BURST_8_ACCESS		7
#define FLD_ULTRA_EN				GENMASK(13, 12)
#define VAL_ULTRA_EN_ENABLE			1
#define FLD_PRE_ULTRA_EN			GENMASK(17, 16)
#define VAL_PRE_ULTRA_EN_ENABLE			1
#define FLD_EXT_ULTRA_EN			BIT(18)
#define MDP_RDMA_SRC_CON		0x030
#define FLD_UNIFORM_CONFIG			BIT(17)
#define FLD_SRC_FORMAT				GENMASK(3, 0)
#define MDP_RDMA_COMP_CON		0x038
#define MDP_RDMA_MF_BKGD_SIZE_IN_BYTE	0x060
#define FLD_MF_BKGD_WB				GENMASK(22, 0)
#define MDP_RDMA_MF_SRC_SIZE		0x070
#define FLD_MF_SRC_H				GENMASK(30, 16)
#define FLD_MF_SRC_W				GENMASK(14, 0)
#define MDP_RDMA_MF_CLIP_SIZE		0x078
#define FLD_MF_CLIP_H				GENMASK(30, 16)
#define FLD_MF_CLIP_W				GENMASK(14, 0)
#define MDP_RDMA_SRC_OFFSET_0		0x118
#define MDP_RDMA_TRANSFORM_0		0x200
#define MDP_RDMA_SRC_BASE_0		0xf00

/*
 * Source format 2 is named after its byte order in memory, B, G, R, A, as
 * the datasheet does. That is the same encoding as DRM_FORMAT_XRGB8888,
 * which Linux' rdma_fmt_convert() maps to RDMA_INPUT_FORMAT_RGBA8888: its
 * name only records that the alpha byte comes last, the R/B order being
 * selected separately by FLD_SWAP (left clear here).
 */
#define MDP_RDMA_SRC_FORMAT_BGRA8888		2

/*
 * Fields used by this driver but not present in the upstream driver, named
 * after the datasheet:
 * - MDP_RDMA_EN[23:4] INTERNAL_DCM_EN: dynamic clock gating of the RDMA
 *   internal blocks, all enabled out of reset (0x00fffff0)
 * - MDP_RDMA_TRANSFORM_0[15] BITEXTEND_WITH_ZERO: extend the LSBs with zeros
 *   when converting 8-bit data to 10-bit
 */
#define FLD_INTERNAL_DCM_EN			GENMASK(23, 4)
#define FLD_BITEXTEND_WITH_ZERO			BIT(15)
#define FB_BYTES_PER_PIXEL			4

/*
 * DRAM request settings, as programmed by Linux'
 * mtk_mdp_rdma_fifo_config(): 8-beat burst reads plus the ultra/pre-ultra
 * high priority requests that keep the RDMA from underrunning at high pixel
 * clocks.
 */
#define MDP_RDMA_GMCIF_CON_QOS \
	(FLD_COMMAND_DIV | FLD_EXT_PREULTRA_EN | FLD_EXT_ULTRA_EN | \
	 FIELD_PREP(FLD_RD_REQ_TYPE, VAL_RD_REQ_TYPE_BURST_8_ACCESS) | \
	 FIELD_PREP(FLD_ULTRA_EN, VAL_ULTRA_EN_ENABLE) | \
	 FIELD_PREP(FLD_PRE_ULTRA_EN, VAL_PRE_ULTRA_EN_ENABLE))

int mtk_mdp_rdma_config(struct udevice *dev, struct video_uc_plat *plat,
			const struct display_timing *timing,
			unsigned int pitch, bool apply_offset)
{
	u32 width = timing->hactive.typ;
	u32 height = timing->vactive.typ;
	u32 offset = apply_offset ? ((width / 2) * FB_BYTES_PER_PIXEL) : 0;
	u32 fb;

	/* the RDMA source address register is 32-bit only */
	if (plat->base != (u32)plat->base) {
		dev_err(dev, "framebuffer base %#lx is above 4GB\n", plat->base);
		return -EINVAL;
	}
	fb = plat->base;

	/* the stride register only holds FLD_MF_BKGD_WB bits */
	if (pitch > FLD_MF_BKGD_WB) {
		dev_err(dev, "framebuffer pitch %u is too large\n", pitch);
		return -EINVAL;
	}

	mtk_disp_comp_write(dev, MDP_RDMA_EN, FLD_INTERNAL_DCM_EN);

	/* DRAM request type and ultra thresholds */
	mtk_disp_comp_write(dev, MDP_RDMA_GMCIF_CON, MDP_RDMA_GMCIF_CON_QOS);

	/*
	 * Set the source format. U-Boot's 32bpp framebuffer has no alpha
	 * channel and the mixer blends this layer with alpha disabled, so
	 * FLD_OUTPUT_ARGB is left clear as Linux does for such a source.
	 */
	mtk_disp_comp_write(dev, MDP_RDMA_SRC_CON,
			    FIELD_PREP(FLD_SRC_FORMAT,
				       MDP_RDMA_SRC_FORMAT_BGRA8888) |
			    FLD_UNIFORM_CONFIG);

	/* no framebuffer compression */
	mtk_disp_comp_write(dev, MDP_RDMA_COMP_CON, 0x0);

	/* set base address */
	mtk_disp_comp_write(dev, MDP_RDMA_SRC_BASE_0, fb);

	/* stride comes from the framebuffer line length */
	mtk_disp_comp_write(dev, MDP_RDMA_MF_BKGD_SIZE_IN_BYTE,
			    FIELD_PREP(FLD_MF_BKGD_WB, pitch));

	/*
	 * each of the two RDMAs reads one half of the frame: size and
	 * clip cover half the width, and the second RDMA (apply_offset)
	 * starts at the middle of the line
	 */
	mtk_disp_comp_write(dev, MDP_RDMA_MF_SRC_SIZE,
			    FIELD_PREP(FLD_MF_SRC_H, height) |
			    FIELD_PREP(FLD_MF_SRC_W, width / 2));
	mtk_disp_comp_write(dev, MDP_RDMA_MF_CLIP_SIZE,
			    FIELD_PREP(FLD_MF_CLIP_H, height) |
			    FIELD_PREP(FLD_MF_CLIP_W, width / 2));
	mtk_disp_comp_write(dev, MDP_RDMA_SRC_OFFSET_0, offset);

	/*
	 * Simple mode, plus the downstream way of feeding the 10-bit
	 * pipeline from 8-bit data: BITEXTEND_WITH_ZERO zero-extends each
	 * component, instead of MDP_RDMA_CON's FLD_OUTPUT_10B used by Linux.
	 * Only one of the two is needed; do not switch to FLD_OUTPUT_10B
	 * when re-syncing with Linux.
	 */
	mtk_disp_comp_write(dev, MDP_RDMA_CON, FLD_SIMPLE_MODE);
	mtk_disp_comp_write(dev, MDP_RDMA_TRANSFORM_0,
			    FLD_BITEXTEND_WITH_ZERO);

	/* enable engine */
	mtk_disp_comp_write(dev, MDP_RDMA_EN,
			    FLD_INTERNAL_DCM_EN | FLD_ROT_ENABLE);

	return 0;
}

static int mtk_mdp_rdma_probe(struct udevice *dev)
{
	int ret;

	ret = mtk_disp_comp_probe(dev);
	if (ret)
		return ret;

	/* the RDMA is a DMA master: it feeds from an SMI larb */
	return mtk_disp_comp_get_larb(dev);
}

static const struct udevice_id mtk_mdp_rdma_ids[] = {
	{ .compatible = "mediatek,mt8195-vdo1-rdma" },
	{ }
};

U_BOOT_DRIVER(mtk_mdp_rdma) = {
	.name	   = "mtk_mdp_rdma",
	.id	   = UCLASS_MISC,
	.of_match  = mtk_mdp_rdma_ids,
	.probe	   = mtk_mdp_rdma_probe,
	.priv_auto = sizeof(struct mtk_disp_comp_priv),
};
