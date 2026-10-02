// SPDX-License-Identifier: GPL-2.0-only
/*
 * MediaTek DPI support
 *
 * Copyright (c) 2026 BayLibre, SAS.
 * Author: Julien Stephan <jstephan@baylibre.com>
 */

#include <dm.h>
#include <fdtdec.h>
#include <linux/bitops.h>

#include "mtk_disp_comp.h"
#include "mtk_dpi.h"
#include "mtk_dpi_regs.h"

enum mtk_dpi_out_bit_num {
	MTK_DPI_OUT_BIT_NUM_8BITS,
	MTK_DPI_OUT_BIT_NUM_10BITS,
	MTK_DPI_OUT_BIT_NUM_12BITS,
	MTK_DPI_OUT_BIT_NUM_16BITS
};

enum mtk_dpi_out_yc_map {
	MTK_DPI_OUT_YC_MAP_RGB,
	MTK_DPI_OUT_YC_MAP_CYCY,
	MTK_DPI_OUT_YC_MAP_YCYC,
	MTK_DPI_OUT_YC_MAP_CY,
	MTK_DPI_OUT_YC_MAP_YC
};

enum mtk_dpi_out_channel_swap {
	MTK_DPI_OUT_CHANNEL_SWAP_RGB,
	MTK_DPI_OUT_CHANNEL_SWAP_GBR,
	MTK_DPI_OUT_CHANNEL_SWAP_BRG,
	MTK_DPI_OUT_CHANNEL_SWAP_RBG,
	MTK_DPI_OUT_CHANNEL_SWAP_GRB,
	MTK_DPI_OUT_CHANNEL_SWAP_BGR
};

enum mtk_dpi_polarity {
	MTK_DPI_POLARITY_RISING,
	MTK_DPI_POLARITY_FALLING,
};

struct mtk_dpi_polarities {
	enum mtk_dpi_polarity de_pol;
	enum mtk_dpi_polarity ck_pol;
	enum mtk_dpi_polarity hsync_pol;
	enum mtk_dpi_polarity vsync_pol;
};

struct mtk_dpi_sync_param {
	u32 sync_width;
	u32 front_porch;
	u32 back_porch;
	bool shift_half_line;
};

struct mtk_dpi_yc_limit {
	u16 y_top;
	u16 y_bottom;
	u16 c_top;
	u16 c_bottom;
};

static void mtk_dpi_sw_reset(struct udevice *dev, bool reset)
{
	mtk_disp_comp_update(dev, DPI_RET, reset ? RST : 0, RST);
}

static void mtk_dpi_config_hsync(struct udevice *dev,
				 const struct mtk_dpi_sync_param *sync)
{
	mtk_disp_comp_update(dev, DPI_TGEN_HWIDTH, sync->sync_width << HPW,
			     HPW_MASK);
	mtk_disp_comp_update(dev, DPI_TGEN_HPORCH, sync->back_porch << HBP,
			     HBP_MASK);
	mtk_disp_comp_update(dev, DPI_TGEN_HPORCH, sync->front_porch << HFP,
			     HFP_MASK);
}

static void mtk_dpi_config_vsync(struct udevice *dev,
				 const struct mtk_dpi_sync_param *sync,
				 u32 width_addr, u32 porch_addr)
{
	mtk_disp_comp_update(dev, width_addr,
			     sync->sync_width << VSYNC_WIDTH_SHIFT,
			     VSYNC_WIDTH_MASK);
	mtk_disp_comp_update(dev, width_addr,
			     sync->shift_half_line << VSYNC_HALF_LINE_SHIFT,
			     VSYNC_HALF_LINE_MASK);
	mtk_disp_comp_update(dev, porch_addr,
			     sync->back_porch << VSYNC_BACK_PORCH_SHIFT,
			     VSYNC_BACK_PORCH_MASK);
	mtk_disp_comp_update(dev, porch_addr,
			     sync->front_porch << VSYNC_FRONT_PORCH_SHIFT,
			     VSYNC_FRONT_PORCH_MASK);
}

static void mtk_dpi_config_vsync_lodd(struct udevice *dev,
				      const struct mtk_dpi_sync_param *sync)
{
	mtk_dpi_config_vsync(dev, sync, DPI_TGEN_VWIDTH, DPI_TGEN_VPORCH);
}

static void mtk_dpi_config_vsync_leven(struct udevice *dev,
				       const struct mtk_dpi_sync_param *sync)
{
	mtk_dpi_config_vsync(dev, sync, DPI_TGEN_VWIDTH_LEVEN,
			     DPI_TGEN_VPORCH_LEVEN);
}

static void mtk_dpi_config_vsync_rodd(struct udevice *dev,
				      const struct mtk_dpi_sync_param *sync)
{
	mtk_dpi_config_vsync(dev, sync, DPI_TGEN_VWIDTH_RODD,
			     DPI_TGEN_VPORCH_RODD);
}

static void mtk_dpi_config_vsync_reven(struct udevice *dev,
				       const struct mtk_dpi_sync_param *sync)
{
	mtk_dpi_config_vsync(dev, sync, DPI_TGEN_VWIDTH_REVEN,
			     DPI_TGEN_VPORCH_REVEN);
}

static void mtk_dpi_config_pol(struct udevice *dev,
			       const struct mtk_dpi_polarities *dpi_pol)
{
	unsigned int pol;

	pol = (dpi_pol->ck_pol == MTK_DPI_POLARITY_RISING ? 0 : CK_POL) |
	      (dpi_pol->de_pol == MTK_DPI_POLARITY_RISING ? 0 : DE_POL) |
	      (dpi_pol->hsync_pol == MTK_DPI_POLARITY_RISING ? 0 : HSYNC_POL) |
	      (dpi_pol->vsync_pol == MTK_DPI_POLARITY_RISING ? 0 : VSYNC_POL);
	mtk_disp_comp_update(dev, DPI_OUTPUT_SETTING, pol,
			     CK_POL | DE_POL | HSYNC_POL | VSYNC_POL);
}

static void mtk_dpi_config_interface(struct udevice *dev, bool inter)
{
	mtk_disp_comp_update(dev, DPI_CON, inter ? INTL_EN : 0, INTL_EN);
}

static void mtk_dpi_config_input_2p(struct udevice *dev, bool input_2p)
{
	mtk_disp_comp_update(dev, DPI_CON, input_2p ? DPI_INPUT_2P_EN : 0,
			     DPI_INPUT_2P_EN);
}

static void mtk_dpi_config_output_1t1p(struct udevice *dev, bool output_1t1p)
{
	mtk_disp_comp_update(dev, DPI_CON, output_1t1p ? DPI_OUTPUT_1T1P_EN : 0,
			     DPI_OUTPUT_1T1P_EN);
}

static void mtk_dpi_config_fb_size(struct udevice *dev, u32 width, u32 height)
{
	mtk_disp_comp_update(dev, DPI_SIZE, width << HSIZE, HSIZE_MASK);
	mtk_disp_comp_update(dev, DPI_SIZE, height << VSIZE, VSIZE_MASK);
}

static void mtk_dpi_config_channel_limit(struct udevice *dev,
					 const struct mtk_dpi_yc_limit *limit)
{
	mtk_disp_comp_update(dev, DPI_Y_LIMIT, limit->y_bottom << Y_LIMINT_BOT,
			     Y_LIMINT_BOT_MASK);
	mtk_disp_comp_update(dev, DPI_Y_LIMIT, limit->y_top << Y_LIMINT_TOP,
			     Y_LIMINT_TOP_MASK);
	mtk_disp_comp_update(dev, DPI_C_LIMIT, limit->c_bottom << C_LIMIT_BOT,
			     C_LIMIT_BOT_MASK);
	mtk_disp_comp_update(dev, DPI_C_LIMIT, limit->c_top << C_LIMIT_TOP,
			     C_LIMIT_TOP_MASK);
}

static void mtk_dpi_config_bit_num(struct udevice *dev,
				   enum mtk_dpi_out_bit_num num)
{
	u32 val;

	switch (num) {
	case MTK_DPI_OUT_BIT_NUM_8BITS:
		val = OUT_BIT_8;
		break;
	case MTK_DPI_OUT_BIT_NUM_10BITS:
		val = OUT_BIT_10;
		break;
	case MTK_DPI_OUT_BIT_NUM_12BITS:
		val = OUT_BIT_12;
		break;
	case MTK_DPI_OUT_BIT_NUM_16BITS:
		val = OUT_BIT_16;
		break;
	default:
		val = OUT_BIT_8;
		break;
	}

	mtk_disp_comp_update(dev, DPI_OUTPUT_SETTING, val << OUT_BIT,
			     OUT_BIT_MASK);
}

static void mtk_dpi_config_yc_map(struct udevice *dev,
				  enum mtk_dpi_out_yc_map map)
{
	u32 val;

	switch (map) {
	case MTK_DPI_OUT_YC_MAP_RGB:
		val = YC_MAP_RGB;
		break;
	case MTK_DPI_OUT_YC_MAP_CYCY:
		val = YC_MAP_CYCY;
		break;
	case MTK_DPI_OUT_YC_MAP_YCYC:
		val = YC_MAP_YCYC;
		break;
	case MTK_DPI_OUT_YC_MAP_CY:
		val = YC_MAP_CY;
		break;
	case MTK_DPI_OUT_YC_MAP_YC:
		val = YC_MAP_YC;
		break;
	default:
		val = YC_MAP_RGB;
		break;
	}

	mtk_disp_comp_update(dev, DPI_OUTPUT_SETTING, val << YC_MAP,
			     YC_MAP_MASK);
}

static void mtk_dpi_config_channel_swap(struct udevice *dev,
					enum mtk_dpi_out_channel_swap swap)
{
	u32 val;

	switch (swap) {
	case MTK_DPI_OUT_CHANNEL_SWAP_RGB:
		val = SWAP_RGB;
		break;
	case MTK_DPI_OUT_CHANNEL_SWAP_GBR:
		val = SWAP_GBR;
		break;
	case MTK_DPI_OUT_CHANNEL_SWAP_BRG:
		val = SWAP_BRG;
		break;
	case MTK_DPI_OUT_CHANNEL_SWAP_RBG:
		val = SWAP_RBG;
		break;
	case MTK_DPI_OUT_CHANNEL_SWAP_GRB:
		val = SWAP_GRB;
		break;
	case MTK_DPI_OUT_CHANNEL_SWAP_BGR:
		val = SWAP_BGR;
		break;
	default:
		val = SWAP_RGB;
		break;
	}

	mtk_disp_comp_update(dev, DPI_OUTPUT_SETTING, val << CH_SWAP,
			     CH_SWAP_MASK);
}

static void mtk_dpi_config_swap_input(struct udevice *dev, bool enable)
{
	mtk_disp_comp_update(dev, DPI_CON, enable ? IN_RB_SWAP : 0, IN_RB_SWAP);
}

static void mtk_dpi_config_yuv422_enable(struct udevice *dev, bool enable)
{
	mtk_disp_comp_update(dev, DPI_CON, enable ? YUV422_EN : 0, YUV422_EN);
}

static void mtk_dpi_config_csc_enable(struct udevice *dev, bool enable)
{
	mtk_disp_comp_update(dev, DPI_CON, enable ? CSC_ENABLE : 0, CSC_ENABLE);
}

void mtk_dpi_hw_enable(struct udevice *dev)
{
	mtk_disp_comp_update(dev, DPI_EN, EN, EN);
}

void mtk_dpi_hw_disable(struct udevice *dev)
{
	mtk_disp_comp_update(dev, DPI_EN, 0, EN);
}

void mtk_dpi_config(struct udevice *dev, const struct display_timing *timing)
{
	struct mtk_dpi_yc_limit limit;
	struct mtk_dpi_polarities dpi_pol;
	struct mtk_dpi_sync_param hsync;
	struct mtk_dpi_sync_param vsync_lodd = { };
	struct mtk_dpi_sync_param vsync_leven = { };
	struct mtk_dpi_sync_param vsync_rodd = { };
	struct mtk_dpi_sync_param vsync_reven = { };
	bool interlaced = timing->flags & DISPLAY_FLAGS_INTERLACED;

	limit.c_bottom = 0x0010;
	limit.c_top = 0x0FE0;
	limit.y_bottom = 0x0010;
	limit.y_top = 0x0FE0;

	dpi_pol.ck_pol = MTK_DPI_POLARITY_FALLING;
	dpi_pol.de_pol = MTK_DPI_POLARITY_RISING;
	dpi_pol.hsync_pol = timing->flags & DISPLAY_FLAGS_HSYNC_HIGH ?
			    MTK_DPI_POLARITY_FALLING :
			    MTK_DPI_POLARITY_RISING;
	dpi_pol.vsync_pol = timing->flags & DISPLAY_FLAGS_VSYNC_HIGH ?
			    MTK_DPI_POLARITY_FALLING :
			    MTK_DPI_POLARITY_RISING;

	hsync.sync_width = timing->hsync_len.typ;
	hsync.back_porch = timing->hback_porch.typ;
	hsync.front_porch = timing->hfront_porch.typ;
	hsync.shift_half_line = false;

	vsync_lodd.sync_width = timing->vsync_len.typ;
	vsync_lodd.back_porch = timing->vback_porch.typ;
	vsync_lodd.front_porch = timing->vfront_porch.typ;
	vsync_lodd.shift_half_line = false;

	if (interlaced) {
		vsync_leven = vsync_lodd;
		vsync_leven.shift_half_line = true;
	}

	mtk_dpi_sw_reset(dev, true);
	mtk_dpi_config_pol(dev, &dpi_pol);

	mtk_dpi_config_hsync(dev, &hsync);
	mtk_dpi_config_vsync_lodd(dev, &vsync_lodd);
	mtk_dpi_config_vsync_rodd(dev, &vsync_rodd);
	mtk_dpi_config_vsync_leven(dev, &vsync_leven);
	mtk_dpi_config_vsync_reven(dev, &vsync_reven);

	mtk_dpi_config_interface(dev, interlaced);
	if (interlaced)
		mtk_dpi_config_fb_size(dev, timing->hactive.typ,
				       timing->vactive.typ / 2);
	else
		mtk_dpi_config_fb_size(dev, timing->hactive.typ,
				       timing->vactive.typ);

	mtk_dpi_config_input_2p(dev, true);
	mtk_dpi_config_output_1t1p(dev, true);
	mtk_dpi_config_channel_limit(dev, &limit);
	mtk_dpi_config_bit_num(dev, MTK_DPI_OUT_BIT_NUM_8BITS);
	mtk_dpi_config_channel_swap(dev, MTK_DPI_OUT_CHANNEL_SWAP_RGB);
	mtk_dpi_config_yc_map(dev, MTK_DPI_OUT_YC_MAP_RGB);
	mtk_dpi_config_yuv422_enable(dev, false);
	mtk_dpi_config_csc_enable(dev, false);
	mtk_dpi_config_swap_input(dev, false);
	mtk_dpi_sw_reset(dev, false);
}

static const struct udevice_id mtk_dpi_ids[] = {
	{ .compatible = "mediatek,mt8195-dpi" },
	{ }
};

U_BOOT_DRIVER(mtk_dpi) = {
	.name	   = "mtk_dpi",
	.id	   = UCLASS_MISC,
	.of_match  = mtk_dpi_ids,
	.probe	   = mtk_disp_comp_probe,
	.priv_auto = sizeof(struct mtk_disp_comp_priv),
};
