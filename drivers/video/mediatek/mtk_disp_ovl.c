// SPDX-License-Identifier: GPL-2.0-only
/*
 * MediaTek Video Overlay support
 *
 * Copyright (c) 2026 BayLibre, SAS.
 * Author: Julien Masson <jmasson@baylibre.com>
 *         Julien Stephan <jstephan@baylibre.com>
 */

#include <dm/device.h>
#include <dm/device_compat.h>
#include <errno.h>
#include <fdtdec.h>
#include <linux/bitfield.h>
#include <linux/bitops.h>
#include <linux/kernel.h>
#include <mtk_smi.h>
#include <video.h>

#include "mtk_disp_comp.h"
#include "mtk_disp_ovl.h"

#define DISP_REG_OVL_EN		0x00c
#define OVL_EN				BIT(0)

#define DISP_REG_OVL_RST	0x014

#define DISP_REG_OVL_ROI_SIZE	0x020
#define OVL_ROI_SIZE_WIDTH		GENMASK(15, 0)
#define OVL_ROI_SIZE_HEIGHT		GENMASK(31, 16)

#define DISP_REG_OVL_DATAPATH_CON 0x024
#define OVL_LAYER_SMI_ID_EN		BIT(0)

#define DISP_REG_OVL_ROI_BGCLR	0x028
#define OVL_COLOR_ALPHA			GENMASK(31, 24)

#define DISP_REG_OVL_SRC_CON	0x02c
#define L0_EN				BIT(0)

#define DISP_REG_OVL_CON	0x030
#define OVL_CON_ALPHA			GENMASK(7, 0)
#define OVL_CON_AEN			BIT(8)
#define OVL_CON_CLRFMT			GENMASK(14, 12)
#define OVL_CON_CLRFMT_ARGB8888		2

#define DISP_REG_OVL_SRC_SIZE	0x038
#define OVL_SRC_SIZE_WIDTH		GENMASK(15, 0)
#define OVL_SRC_SIZE_HEIGHT		GENMASK(31, 16)

#define DISP_REG_OVL_OFFSET	0x03c

#define DISP_REG_OVL_PITCH	0x044
#define OVL_PITCH_LSB			GENMASK(15, 0)
#define OVL_CONST_BLEND			BIT(28)

#define DISP_REG_OVL_RDMA0_CTRL	0x0c0
#define RDMA0_EN			BIT(0)

#define DISP_REG_OVL_RDMA_GMC	0x0c8
#define GMC_THRESHOLD_BITS	16
#define GMC_THRESHOLD_HIGH	((1 << GMC_THRESHOLD_BITS) / 4)
#define GMC_THRESHOLD_LOW	((1 << GMC_THRESHOLD_BITS) / 8)

#define DISP_REG_OVL_CLRFMT_EXT	0x2d0
#define OVL_CON_CLRFMT_BIT_DEPTH_MASK(n)	(GENMASK(1, 0) << (4 * (n)))
#define OVL_CON_CLRFMT_BIT_DEPTH(depth, n)	((depth) << (4 * (n)))
#define OVL_CON_CLRFMT_8_BIT			0

#define DISP_REG_OVL_ADDR	0xf40

/**
 * struct mtk_disp_ovl_data - per-SoC overlay data
 * @smi_id_en: the overlay tags its DRAM requests with an SMI id
 * @supports_clrfmt_ext: the overlay has the CLRFMT_EXT bit depth register
 * @gmc_bits: width of the GMC thresholds, in bits
 */
struct mtk_disp_ovl_data {
	bool smi_id_en;
	bool supports_clrfmt_ext;
	unsigned int gmc_bits;
};

/**
 * struct mtk_disp_ovl_priv - private data of an overlay
 * @comp: common component state, must be the first member
 * @data: per-SoC overlay data
 */
struct mtk_disp_ovl_priv {
	struct mtk_disp_comp_priv comp;
	const struct mtk_disp_ovl_data *data;
};

void mtk_ovl_config(struct udevice *dev, const struct display_timing *timing)
{
	u32 width = timing->hactive.typ;
	u32 height = timing->vactive.typ;

	if (width != 0 && height != 0)
		mtk_disp_comp_write(dev, DISP_REG_OVL_ROI_SIZE,
				    FIELD_PREP(OVL_ROI_SIZE_WIDTH, width) |
				    FIELD_PREP(OVL_ROI_SIZE_HEIGHT, height));

	/* the background must be opaque black, or alpha blending has no effect */
	mtk_disp_comp_write(dev, DISP_REG_OVL_ROI_BGCLR, OVL_COLOR_ALPHA);

	mtk_disp_comp_write(dev, DISP_REG_OVL_RST, 1);
	mtk_disp_comp_write(dev, DISP_REG_OVL_RST, 0);
}

/*
 * Layer 0 is always fed our fixed 8-bit-per-channel XRGB8888 framebuffer, so
 * the 10-bit format check Linux does here never applies to us.
 */
static void mtk_ovl_set_bit_depth(struct udevice *dev)
{
	struct mtk_disp_ovl_priv *ovl = dev_get_priv(dev);

	if (!ovl->data->supports_clrfmt_ext)
		return;

	mtk_disp_comp_update(dev, DISP_REG_OVL_CLRFMT_EXT,
			     OVL_CON_CLRFMT_BIT_DEPTH(OVL_CON_CLRFMT_8_BIT, 0),
			     OVL_CON_CLRFMT_BIT_DEPTH_MASK(0));
}

static void mtk_ovl_layer_on(struct udevice *dev)
{
	struct mtk_disp_ovl_priv *ovl = dev_get_priv(dev);
	u32 gmc_thrshd_l, gmc_thrshd_h, gmc;

	mtk_disp_comp_write(dev, DISP_REG_OVL_RDMA0_CTRL, RDMA0_EN);

	gmc_thrshd_l = GMC_THRESHOLD_LOW >> (GMC_THRESHOLD_BITS - ovl->data->gmc_bits);
	gmc_thrshd_h = GMC_THRESHOLD_HIGH >> (GMC_THRESHOLD_BITS - ovl->data->gmc_bits);
	if (ovl->data->gmc_bits == 10)
		gmc = gmc_thrshd_h | gmc_thrshd_h << 16;
	else
		gmc = gmc_thrshd_l | gmc_thrshd_l << 8 |
		      gmc_thrshd_h << 16 | gmc_thrshd_h << 24;
	mtk_disp_comp_write(dev, DISP_REG_OVL_RDMA_GMC, gmc);

	mtk_disp_comp_update(dev, DISP_REG_OVL_SRC_CON, L0_EN, L0_EN);
}

int mtk_ovl_layer(struct udevice *dev, const struct video_uc_plat *plat,
		  const struct display_timing *timing)
{
	struct mtk_disp_ovl_priv *ovl = dev_get_priv(dev);
	u32 width = timing->hactive.typ;
	u32 height = timing->vactive.typ;
	u32 pitch = width * VNBYTES(VIDEO_BPP32);

	/* DISP_REG_OVL_ADDR is 32-bit, the framebuffer has to fit below 4GB */
	if (upper_32_bits(plat->base)) {
		dev_err(dev, "framebuffer at 0x%lx is above 4GB\n", plat->base);
		return -ERANGE;
	}

	/* opaque layer with alpha blending on, as Linux sets it up */
	mtk_disp_comp_write(dev, DISP_REG_OVL_CON,
			    FIELD_PREP(OVL_CON_CLRFMT, OVL_CON_CLRFMT_ARGB8888) |
			    FIELD_PREP(OVL_CON_ALPHA, 0xff) | OVL_CON_AEN);

	/*
	 * The framebuffer has no real per-pixel alpha channel, so without
	 * OVL_CONST_BLEND the hardware reads the (zero) alpha byte from
	 * memory and blends the whole layer as fully transparent.
	 */
	mtk_disp_comp_write(dev, DISP_REG_OVL_PITCH,
			    FIELD_PREP(OVL_PITCH_LSB, pitch) | OVL_CONST_BLEND);

	mtk_disp_comp_write(dev, DISP_REG_OVL_SRC_SIZE,
			    FIELD_PREP(OVL_SRC_SIZE_WIDTH, width) |
			    FIELD_PREP(OVL_SRC_SIZE_HEIGHT, height));

	/* the layer covers the whole region of interest */
	mtk_disp_comp_write(dev, DISP_REG_OVL_OFFSET, 0);

	/*
	 * Without an IOMMU driver the overlay reads the framebuffer with a
	 * physical address, so take its larb port out of the translated
	 * address space now that the larb is ungated.
	 */
	mtk_smi_larb_port_set_pa(ovl->comp.larb, ovl->comp.larb_port);

	mtk_disp_comp_write(dev, DISP_REG_OVL_ADDR, plat->base);

	mtk_ovl_set_bit_depth(dev);

	mtk_ovl_layer_on(dev);

	return 0;
}

void mtk_ovl_start(struct udevice *dev)
{
	struct mtk_disp_ovl_priv *ovl = dev_get_priv(dev);

	if (ovl->data->smi_id_en)
		mtk_disp_comp_update(dev, DISP_REG_OVL_DATAPATH_CON,
				     OVL_LAYER_SMI_ID_EN, OVL_LAYER_SMI_ID_EN);

	mtk_disp_comp_write(dev, DISP_REG_OVL_EN, OVL_EN);
}

static int mtk_disp_ovl_probe(struct udevice *dev)
{
	struct mtk_disp_ovl_priv *ovl = dev_get_priv(dev);
	int ret;

	ret = mtk_disp_comp_probe(dev);
	if (ret)
		return ret;

	ovl->data = (const struct mtk_disp_ovl_data *)dev_get_driver_data(dev);
	if (!ovl->data)
		return -EINVAL;

	/* the overlay is a DMA master: it feeds from an SMI larb */
	return mtk_disp_comp_get_larb(dev);
}

static const struct mtk_disp_ovl_data mt8192_ovl_driver_data = {
	.smi_id_en = true,
	.gmc_bits = 10,
};

static const struct mtk_disp_ovl_data mt8366_ovl_driver_data = {
	.smi_id_en = true,
	.supports_clrfmt_ext = true,
	.gmc_bits = 10,
};

static const struct udevice_id mtk_disp_ovl_ids[] = {
	{
		/* the MT8189 OVL is the MT8366 one */
		.compatible = "mediatek,mt8189-disp-ovl",
		.data = (ulong)&mt8366_ovl_driver_data,
	},
	{
		.compatible = "mediatek,mt8192-disp-ovl",
		.data = (ulong)&mt8192_ovl_driver_data,
	},
	{
		.compatible = "mediatek,mt8366-disp-ovl",
		.data = (ulong)&mt8366_ovl_driver_data,
	},
	{ }
};

U_BOOT_DRIVER(mtk_disp_ovl) = {
	.name	   = "mtk_disp_ovl",
	.id	   = UCLASS_MISC,
	.of_match  = mtk_disp_ovl_ids,
	.probe	   = mtk_disp_ovl_probe,
	.priv_auto = sizeof(struct mtk_disp_ovl_priv),
};
