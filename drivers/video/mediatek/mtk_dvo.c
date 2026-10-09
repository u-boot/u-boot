// SPDX-License-Identifier: GPL-2.0-only
/*
 * MediaTek MT8189 Digital Video Output (DVO) support
 *
 * DVO is the display controller's output stage on MT8189, replacing the
 * DPI/DP_INTF block used by earlier SoCs. It takes the pixel stream from
 * the display data path and drives the eDP transmitter with video timing.
 *
 * Copyright (c) 2014 MediaTek Inc.
 * Copyright (c) 2026 Collabora Ltd.
 *                    AngeloGioacchino Del Regno <angelogioacchino.delregno@collabora.com>
 * Copyright (c) 2026 BayLibre, SAS.
 *                    David Lechner <dlechner@baylibre.com>
 */

#include <clk.h>
#include <dm.h>
#include <dm/device_compat.h>
#include <fdtdec.h>
#include <linux/bitfield.h>
#include <linux/bitops.h>
#include <linux/build_bug.h>
#include <linux/err.h>
#include <linux/kernel.h>
#include <linux/math64.h>
#include <linux/stddef.h>

#include "mtk_disp_comp.h"
#include "mtk_dvo.h"

#define DVO_EN				0x00
#define EN					BIT(0)

#define DVO_RET				0x04
#define SWRST					BIT(0)

#define DVO_OUTPUT_SET			0x18
#define OUT_NP_SEL				GENMASK(1, 0)
#define CH_SWAP					GENMASK(7, 5)
#define SWAP_RGB				0
#define HS_INV					BIT(20)
#define VS_INV					BIT(21)

#define DVO_SRC_SIZE			0x20
#define SRC_HSIZE				GENMASK(15, 0)
#define SRC_VSIZE				GENMASK(31, 16)

#define DVO_PIC_SIZE			0x24
#define PIC_HSIZE				GENMASK(15, 0)
#define PIC_VSIZE				GENMASK(31, 16)

#define DVO_TGEN_H0			0x50
#define HFP					GENMASK(15, 0)
#define HSYNC					GENMASK(31, 16)

#define DVO_TGEN_H1			0x54
#define HSYNC2ACT				GENMASK(15, 0)
#define HACT					GENMASK(31, 16)

#define DVO_TGEN_V0			0x58
#define VFP					GENMASK(15, 0)
#define VSYNC					GENMASK(31, 16)

#define DVO_TGEN_V1			0x5c
#define VSYNC2ACT				GENMASK(15, 0)
#define VACT					GENMASK(31, 16)

#define DVO_TGEN_V_LAST_TRAILING_BLANK	0x6c
#define V_LAST_TRAILING_BLANK			GENMASK(15, 0)

#define DVO_TGEN_OUTPUT_DELAY_LINE	0x7c
#define EXT_TG_DLY_LINE				GENMASK(15, 0)

#define DVO_TGEN_INFOQ_LATENCY		0x80
#define INFOQ_START_LATENCY			GENMASK(15, 0)
#define INFOQ_END_LATENCY			GENMASK(31, 16)

#define DVO_SHADOW_CTRL			0x190
#define FORCE_COMMIT				BIT(0)
#define BYPASS_SHADOW				BIT(1)

#define DVO_BUF_CON0			0x220
#define DISP_BUF_EN				BIT(0)
#define FIFO_UNDERFLOW_DONE_BLOCK		BIT(4)

#define DVO_BUF_SODI_HIGH		0x230
#define DVO_BUF_SODI_LOW		0x234
#define DVO_DISP_BUF				GENMASK(31, 0)

/* Output one iteration of four pixels; OUT_NP_SEL encodes 1T1P/1T2P/1T4P. */
#define MTK_DVO_PIXELS_PER_ITER		4
#define MTK_DVO_OUT_PIXITER_1T4P	2

/* The display data path feeds DVO two pixels per iteration. */
#define MTK_DVO_INPUT_MODE		2
#define MTK_DVO_OUTPUT_MODE		4

/* 1 unit = 2 group data = 2 * 1T4P = 8 pixels */
#define MTK_DISP_BUF_SRAM_UNIT_SIZE	8
#define MTK_DISP_LINE_BUF_DVO_US	40
#define MTK_DVO_EDP_MAX_CLK		297
/* Fallback when the engine clock cannot be read, in MHz */
#define MTK_DVO_DEFAULT_MMSYS_MHZ	273
#define MTK_DVO_MAX_HACTIVE		3840
#define MTK_DVO_LINE_BUFFER_SIZE	(MTK_DVO_MAX_HACTIVE / MTK_DISP_BUF_SRAM_UNIT_SIZE)
#define MTK_DVO_BITS_PER_CYCLE		(8 * 32 * 1000000)

/* SODI watermark thresholds, in microseconds of buffered data */
#define TWAKE_UP			5
#define ULTRA_LOW_US			23

struct mtk_dvo_priv {
	/*
	 * Must stay first: mtk_disp_comp_*() are handed this device and cast
	 * dev_get_priv() straight to struct mtk_disp_comp_priv, and the eDP
	 * driver calls them on the DVO across the driver boundary.
	 */
	struct mtk_disp_comp_priv comp;
	struct clk *engine_clk;
	struct clk *pixel_clk;
	struct clk *pll_clk;
};

/*
 * Keep the PLL in the range it is happy at by running it at a multiple of
 * the pixel rate for the slower modes. Thresholds are in kHz.
 */
static u32 mtk_dvo_calculate_factor(u32 clock_khz)
{
	if (clock_khz < 70000)
		return 4;
	if (clock_khz < 200000)
		return 2;

	return 1;
}

static void mtk_dvo_sw_reset(struct udevice *dev, bool reset)
{
	mtk_disp_comp_update(dev, DVO_RET, reset ? SWRST : 0, SWRST);
}

void mtk_dvo_hw_enable(struct udevice *dev)
{
	mtk_disp_comp_update(dev, DVO_EN, EN, EN);
}

/*
 * Program the display buffer watermarks from the rate the buffer fills at
 * (the display data path writing into it) against the rate DVO drains it at
 * (the pixel stream leaving for the eDP transmitter), so that the memory
 * subsystem is asked to step up before the FIFO can run dry.
 */
static void mtk_dvo_sodi_setting(struct udevice *dev,
				 const struct display_timing *timing)
{
	const u64 sodi_total = (u64)MTK_DVO_BITS_PER_CYCLE *
			       MTK_DISP_BUF_SRAM_UNIT_SIZE * 32;
	struct mtk_dvo_priv *priv = dev_get_priv(dev);
	u64 sodi_high, sodi_low, sodi_high_rem, sodi_low_rem, tmp;
	u64 dvo_fifo_size, fifo_size, total_bit;
	u64 consume_rate, consume_rate_rem;
	u64 fill_rate, fill_rate_rem;
	u64 data_rate;
	u32 mmsys_clk;
	ulong rate;

	/*
	 * The fill rate below is how fast the display data path writes into
	 * the DVO FIFO, so it is paced by the engine clock rather than by the
	 * pixel rate the FIFO is drained at.
	 */
	rate = clk_get_rate(priv->engine_clk);
	if (IS_ERR_VALUE(rate) || !rate) {
		dev_warn(dev, "no engine clock rate, assuming %u MHz\n",
			 MTK_DVO_DEFAULT_MMSYS_MHZ);
		mmsys_clk = MTK_DVO_DEFAULT_MMSYS_MHZ;
	} else {
		mmsys_clk = rate / 1000000;
	}

	fill_rate = div64_u64_rem((u64)mmsys_clk * MTK_DVO_INPUT_MODE * 30,
				  32ULL * MTK_DISP_BUF_SRAM_UNIT_SIZE,
				  &fill_rate_rem);

	data_rate = div64_u64((u64)timing->hactive.typ * timing->vactive.typ *
			      timing->pixelclock.typ,
			      (u64)(timing->hactive.typ + timing->hfront_porch.typ +
				    timing->hsync_len.typ + timing->hback_porch.typ) *
			      (timing->vactive.typ + timing->vfront_porch.typ +
			       timing->vsync_len.typ + timing->vback_porch.typ));

	consume_rate = div64_u64_rem(data_rate * 30 * 5 / 4,
				     MTK_DVO_BITS_PER_CYCLE, &consume_rate_rem);

	total_bit = (u64)MTK_DVO_EDP_MAX_CLK * MTK_DVO_OUTPUT_MODE * 30 *
		    MTK_DISP_LINE_BUF_DVO_US;
	fifo_size = total_bit / (MTK_DISP_BUF_SRAM_UNIT_SIZE * 30);

	/* DVO supports MSO mode, so calculate with three line buffers */
	dvo_fifo_size = fifo_size + 3 * MTK_DVO_LINE_BUFFER_SIZE;

	sodi_high_rem = fill_rate_rem * MTK_DVO_BITS_PER_CYCLE;
	tmp = consume_rate_rem * (32 * MTK_DISP_BUF_SRAM_UNIT_SIZE);

	if (sodi_high_rem < tmp) {
		fill_rate -= 1;
		sodi_high_rem += sodi_total;
	}
	sodi_high_rem -= tmp;
	sodi_high_rem *= 32 * 6;
	sodi_high_rem = div64_u64(sodi_high_rem, sodi_total);

	sodi_high = (dvo_fifo_size * 30 * 5 - 32 * 6 * (fill_rate - consume_rate) -
		     sodi_high_rem + (5 * 32 - 1)) / (5 * 32);

	sodi_low_rem = DIV_ROUND_UP_ULL(consume_rate_rem * (ULTRA_LOW_US + TWAKE_UP),
					MTK_DVO_BITS_PER_CYCLE);
	sodi_low = consume_rate * (ULTRA_LOW_US + TWAKE_UP) + sodi_low_rem;

	mtk_disp_comp_update(dev, DVO_BUF_SODI_HIGH, sodi_high, DVO_DISP_BUF);
	mtk_disp_comp_update(dev, DVO_BUF_SODI_LOW, sodi_low, DVO_DISP_BUF);
}

int mtk_dvo_config(struct udevice *dev, const struct display_timing *timing)
{
	struct mtk_dvo_priv *priv = dev_get_priv(dev);
	u32 hsync_len, hfront_porch, hback_porch, hactive;
	u32 vsync_len = timing->vsync_len.typ;
	u32 vback_porch = timing->vback_porch.typ;
	u32 vactive = timing->vactive.typ;
	u32 dvo_clk;
	u32 factor;
	ulong rate;
	u32 pol = 0;

	/*
	 * The watermark maths divides by the frame's totals and the PLL
	 * request scales the pixel clock, so a degenerate mode has to be
	 * rejected here rather than producing a silent divide by zero.
	 */
	if (!timing->pixelclock.typ || !timing->hactive.typ ||
	    !timing->vactive.typ) {
		dev_err(dev, "invalid timing\n");
		return -EINVAL;
	}

	if (timing->flags & DISPLAY_FLAGS_INTERLACED) {
		dev_err(dev, "interlaced modes are not supported\n");
		return -EINVAL;
	}

	/*
	 * Retune this output's own PLL first: the EDP_SEL mux downstream only
	 * selects between its inputs, so the rate has to exist before we can
	 * ask for it.
	 */
	factor = mtk_dvo_calculate_factor(timing->pixelclock.typ / 1000);

	rate = clk_set_rate(priv->pll_clk, (ulong)timing->pixelclock.typ * factor);
	if (IS_ERR_VALUE(rate)) {
		dev_err(dev, "failed to set pll: %ld\n", (long)rate);
		return (int)rate;
	}

	/* clocked once per iteration, see the horizontal timings below */
	rate = clk_get_rate(priv->pll_clk);
	if (IS_ERR_VALUE(rate) || !rate) {
		dev_err(dev, "cannot read back the pll rate: %ld\n", (long)rate);
		return IS_ERR_VALUE(rate) ? (int)rate : -EINVAL;
	}

	dvo_clk = rate / factor / MTK_DVO_PIXELS_PER_ITER;

	rate = clk_set_rate(priv->pixel_clk, dvo_clk);
	if (IS_ERR_VALUE(rate)) {
		dev_err(dev, "failed to set pixel clock: %ld\n", (long)rate);
		return (int)rate;
	}

	/*
	 * DVO moves MTK_DVO_PIXELS_PER_ITER pixels per iteration, so the
	 * horizontal timings are counted in iterations rather than pixels.
	 */
	hsync_len = timing->hsync_len.typ / MTK_DVO_PIXELS_PER_ITER;
	hfront_porch = timing->hfront_porch.typ / MTK_DVO_PIXELS_PER_ITER;
	hback_porch = timing->hback_porch.typ / MTK_DVO_PIXELS_PER_ITER;
	hactive = DIV_ROUND_UP(timing->hactive.typ, MTK_DVO_PIXELS_PER_ITER);

	/* the HS/VS invert bits are set for an active-high sync */
	if (timing->flags & DISPLAY_FLAGS_HSYNC_HIGH)
		pol |= HS_INV;
	if (timing->flags & DISPLAY_FLAGS_VSYNC_HIGH)
		pol |= VS_INV;

	mtk_dvo_sw_reset(dev, true);

	mtk_disp_comp_update(dev, DVO_OUTPUT_SET, pol, HS_INV | VS_INV);

	mtk_disp_comp_update(dev, DVO_TGEN_H0,
			     FIELD_PREP(HSYNC, hsync_len) |
			     FIELD_PREP(HFP, hfront_porch), HSYNC | HFP);
	mtk_disp_comp_update(dev, DVO_TGEN_H1,
			     FIELD_PREP(HSYNC2ACT, hback_porch + hsync_len),
			     HSYNC2ACT);

	mtk_disp_comp_update(dev, DVO_TGEN_V0,
			     FIELD_PREP(VSYNC, vsync_len) |
			     FIELD_PREP(VFP, timing->vfront_porch.typ),
			     VSYNC | VFP);
	mtk_disp_comp_update(dev, DVO_TGEN_V1,
			     FIELD_PREP(VSYNC2ACT, vback_porch + vsync_len),
			     VSYNC2ACT);

	mtk_disp_comp_update(dev, DVO_SRC_SIZE,
			     FIELD_PREP(SRC_HSIZE, timing->hactive.typ) |
			     FIELD_PREP(SRC_VSIZE, vactive),
			     SRC_HSIZE | SRC_VSIZE);
	mtk_disp_comp_update(dev, DVO_PIC_SIZE,
			     FIELD_PREP(PIC_HSIZE, timing->hactive.typ) |
			     FIELD_PREP(PIC_VSIZE, vactive),
			     PIC_HSIZE | PIC_VSIZE);
	mtk_disp_comp_update(dev, DVO_TGEN_H1, FIELD_PREP(HACT, hactive), HACT);
	mtk_disp_comp_update(dev, DVO_TGEN_V1, FIELD_PREP(VACT, vactive), VACT);

	/* RGB output, so no channel swap */
	mtk_disp_comp_update(dev, DVO_OUTPUT_SET,
			     FIELD_PREP(CH_SWAP, SWAP_RGB), CH_SWAP);

	/* start the info queue as soon as each frame does */
	mtk_disp_comp_update(dev, DVO_TGEN_INFOQ_LATENCY, 0,
			     INFOQ_START_LATENCY | INFOQ_END_LATENCY);

	mtk_disp_comp_update(dev, DVO_BUF_CON0,
			     DISP_BUF_EN | FIFO_UNDERFLOW_DONE_BLOCK,
			     DISP_BUF_EN | FIFO_UNDERFLOW_DONE_BLOCK);

	mtk_disp_comp_update(dev, DVO_TGEN_V_LAST_TRAILING_BLANK,
			     FIELD_PREP(V_LAST_TRAILING_BLANK, 0x20),
			     V_LAST_TRAILING_BLANK);
	mtk_disp_comp_update(dev, DVO_TGEN_OUTPUT_DELAY_LINE,
			     FIELD_PREP(EXT_TG_DLY_LINE, 0x20),
			     EXT_TG_DLY_LINE);

	mtk_dvo_sodi_setting(dev, timing);

	/*
	 * More than one pixel per iteration needs the shadow registers, so
	 * that a whole set of timings is latched at once.
	 */
	mtk_disp_comp_update(dev, DVO_SHADOW_CTRL, FORCE_COMMIT,
			     FORCE_COMMIT | BYPASS_SHADOW);
	mtk_disp_comp_update(dev, DVO_OUTPUT_SET,
			     FIELD_PREP(OUT_NP_SEL, MTK_DVO_OUT_PIXITER_1T4P),
			     OUT_NP_SEL);

	mtk_dvo_sw_reset(dev, false);

	return 0;
}

static int mtk_dvo_probe(struct udevice *dev)
{
	struct mtk_dvo_priv *priv = dev_get_priv(dev);
	int ret;

	ret = mtk_disp_comp_probe(dev);
	if (ret)
		return ret;

	priv->engine_clk = devm_clk_get(dev, "engine");
	if (IS_ERR(priv->engine_clk)) {
		dev_err(dev, "cannot get the engine clock: %ld\n",
			PTR_ERR(priv->engine_clk));
		return PTR_ERR(priv->engine_clk);
	}

	priv->pixel_clk = devm_clk_get(dev, "pixel");
	if (IS_ERR(priv->pixel_clk)) {
		dev_err(dev, "cannot get the pixel clock: %ld\n",
			PTR_ERR(priv->pixel_clk));
		return PTR_ERR(priv->pixel_clk);
	}

	priv->pll_clk = devm_clk_get(dev, "pll");
	if (IS_ERR(priv->pll_clk)) {
		dev_err(dev, "cannot get the PLL clock: %ld\n",
			PTR_ERR(priv->pll_clk));
		return PTR_ERR(priv->pll_clk);
	}

	return 0;
}

static_assert(offsetof(struct mtk_dvo_priv, comp) == 0,
	      "mtk_disp_comp_priv has to stay the first member");

static const struct udevice_id mtk_dvo_ids[] = {
	{ .compatible = "mediatek,mt8189-edp-dvo" },
	{ }
};

/*
 * DM_FLAG_OS_PREPARE has DM remove this device before the OS starts, which
 * powers the display domain off and resets the whole vdosys0 front end.
 * Handing OVL, RDMA, the mutex and an SMI larb that is still bypassing the
 * IOMMU to the kernel leaves it unable to bring the display up, and it does
 * not reset a block it finds already running.
 */
U_BOOT_DRIVER(mtk_dvo) = {
	.name = "mtk_dvo",
	.id = UCLASS_MISC,
	.of_match = mtk_dvo_ids,
	.probe = mtk_dvo_probe,
	.priv_auto = sizeof(struct mtk_dvo_priv),
	.flags = DM_FLAG_OS_PREPARE,
};
