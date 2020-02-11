// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2017-2018 STMicroelectronics - All Rights Reserved
 * Author(s): Philippe Cornu <philippe.cornu@st.com> for STMicroelectronics.
 *	      Yannick Fertre <yannick.fertre@st.com> for STMicroelectronics.
 */

#include <common.h>
#include <clk.h>
#include <display.h>
#include <dm.h>
#include <panel.h>
#include <reset.h>
#include <video.h>
#include <video_bridge.h>
#include <asm/io.h>
#include <asm/arch/gpio.h>
#include <dm/device-internal.h>
#include <dm/device_compat.h>

struct stm32_ltdc_priv {
	void __iomem *regs;
	enum video_log2_bpp l2bpp;
	u32 bg_col_argb;
	u32 crop_x, crop_y, crop_w, crop_h;
	u32 alpha;
};

/* LTDC main registers */
#define LTDC_IDR	0x00	/* IDentification */
#define LTDC_LCR	0x04	/* Layer Count */
#define LTDC_SSCR	0x08	/* Synchronization Size Configuration */
#define LTDC_BPCR	0x0C	/* Back Porch Configuration */
#define LTDC_AWCR	0x10	/* Active Width Configuration */
#define LTDC_TWCR	0x14	/* Total Width Configuration */
#define LTDC_GCR	0x18	/* Global Control */
#define LTDC_GC1R	0x1C	/* Global Configuration 1 */
#define LTDC_GC2R	0x20	/* Global Configuration 2 */
#define LTDC_SRCR	0x24	/* Shadow Reload Configuration */
#define LTDC_GACR	0x28	/* GAmma Correction */
#define LTDC_BCCR	0x2C	/* Background Color Configuration */
#define LTDC_IER	0x34	/* Interrupt Enable */
#define LTDC_ISR	0x38	/* Interrupt Status */
#define LTDC_ICR	0x3C	/* Interrupt Clear */
#define LTDC_LIPCR	0x40	/* Line Interrupt Position Conf. */
#define LTDC_CPSR	0x44	/* Current Position Status */
#define LTDC_CDSR	0x48	/* Current Display Status */

/* LTDC layer 1 registers */
#define LTDC_L1LC1R	0x80	/* L1 Layer Configuration 1 */
#define LTDC_L1LC2R	0x84	/* L1 Layer Configuration 2 */
#define LTDC_L1CR	0x84	/* L1 Control */
#define LTDC_L1WHPCR	0x88	/* L1 Window Hor Position Config */
#define LTDC_L1WVPCR	0x8C	/* L1 Window Vert Position Config */
#define LTDC_L1CKCR	0x90	/* L1 Color Keying Configuration */
#define LTDC_L1PFCR	0x94	/* L1 Pixel Format Configuration */
#define LTDC_L1CACR	0x98	/* L1 Constant Alpha Config */
#define LTDC_L1DCCR	0x9C	/* L1 Default Color Configuration */
#define LTDC_L1BFCR	0xA0	/* L1 Blend Factors Configuration */
#define LTDC_L1FBBCR	0xA4	/* L1 FrameBuffer Bus Control */
#define LTDC_L1AFBCR	0xA8	/* L1 AuxFB Control */
#define LTDC_L1CFBAR	0xAC	/* L1 Color FrameBuffer Address */
#define LTDC_L1CFBLR	0xB0	/* L1 Color FrameBuffer Length */
#define LTDC_L1CFBLNR	0xB4	/* L1 Color FrameBuffer Line Nb */
#define LTDC_L1AFBAR	0xB8	/* L1 AuxFB Address */
#define LTDC_L1AFBLR	0xBC	/* L1 AuxFB Length */
#define LTDC_L1AFBLNR	0xC0	/* L1 AuxFB Line Number */
#define LTDC_L1CLUTWR	0xC4	/* L1 CLUT Write */

/* Bit definitions */
#define SSCR_VSH	GENMASK(10, 0)	/* Vertical Synchronization Height */
#define SSCR_HSW	GENMASK(27, 16)	/* Horizontal Synchronization Width */

#define BPCR_AVBP	GENMASK(10, 0)	/* Accumulated Vertical Back Porch */
#define BPCR_AHBP	GENMASK(27, 16)	/* Accumulated Horizontal Back Porch */

#define AWCR_AAH	GENMASK(10, 0)	/* Accumulated Active Height */
#define AWCR_AAW	GENMASK(27, 16)	/* Accumulated Active Width */

#define TWCR_TOTALH	GENMASK(10, 0)	/* TOTAL Height */
#define TWCR_TOTALW	GENMASK(27, 16)	/* TOTAL Width */

#define GCR_LTDCEN	BIT(0)		/* LTDC ENable */
#define GCR_DEN		BIT(16)		/* Dither ENable */
#define GCR_PCPOL	BIT(28)		/* Pixel Clock POLarity-Inverted */
#define GCR_DEPOL	BIT(29)		/* Data Enable POLarity-High */
#define GCR_VSPOL	BIT(30)		/* Vertical Synchro POLarity-High */
#define GCR_HSPOL	BIT(31)		/* Horizontal Synchro POLarity-High */

#define GC1R_WBCH	GENMASK(3, 0)	/* Width of Blue CHannel output */
#define GC1R_WGCH	GENMASK(7, 4)	/* Width of Green Channel output */
#define GC1R_WRCH	GENMASK(11, 8)	/* Width of Red Channel output */
#define GC1R_PBEN	BIT(12)		/* Precise Blending ENable */
#define GC1R_DT		GENMASK(15, 14)	/* Dithering Technique */
#define GC1R_GCT	GENMASK(19, 17)	/* Gamma Correction Technique */
#define GC1R_SHREN	BIT(21)		/* SHadow Registers ENabled */
#define GC1R_BCP	BIT(22)		/* Background Colour Programmable */
#define GC1R_BBEN	BIT(23)		/* Background Blending ENabled */
#define GC1R_LNIP	BIT(24)		/* Line Number IRQ Position */
#define GC1R_TP		BIT(25)		/* Timing Programmable */
#define GC1R_IPP	BIT(26)		/* IRQ Polarity Programmable */
#define GC1R_SPP	BIT(27)		/* Sync Polarity Programmable */
#define GC1R_DWP	BIT(28)		/* Dither Width Programmable */
#define GC1R_STREN	BIT(29)		/* STatus Registers ENabled */
#define GC1R_BMEN	BIT(31)		/* Blind Mode ENabled */

#define GC2R_EDCA	BIT(0)		/* External Display Control Ability  */
#define GC2R_STSAEN	BIT(1)		/* Slave Timing Sync Ability ENabled */
#define GC2R_DVAEN	BIT(2)		/* Dual-View Ability ENabled */
#define GC2R_DPAEN	BIT(3)		/* Dual-Port Ability ENabled */
#define GC2R_BW		GENMASK(6, 4)	/* Bus Width (log2 of nb of bytes) */
#define GC2R_EDCEN	BIT(7)		/* External Display Control ENabled */

#define SRCR_IMR	BIT(0)		/* IMmediate Reload */
#define SRCR_VBR	BIT(1)		/* Vertical Blanking Reload */

#define LXCR_LEN	BIT(0)		/* Layer ENable */
#define LXCR_COLKEN	BIT(1)		/* Color Keying Enable */
#define LXCR_CLUTEN	BIT(4)		/* Color Look-Up Table ENable */

#define LXWHPCR_WHSTPOS	GENMASK(11, 0)	/* Window Horizontal StarT POSition */
#define LXWHPCR_WHSPPOS	GENMASK(27, 16)	/* Window Horizontal StoP POSition */

#define LXWVPCR_WVSTPOS	GENMASK(10, 0)	/* Window Vertical StarT POSition */
#define LXWVPCR_WVSPPOS	GENMASK(26, 16)	/* Window Vertical StoP POSition */

#define LXPFCR_PF	GENMASK(2, 0)	/* Pixel Format */

#define LXCACR_CONSTA	GENMASK(7, 0)	/* CONSTant Alpha */

#define LXBFCR_BF2	GENMASK(2, 0)	/* Blending Factor 2 */
#define LXBFCR_BF1	GENMASK(10, 8)	/* Blending Factor 1 */

#define LXCFBLR_CFBLL	GENMASK(12, 0)	/* Color Frame Buffer Line Length */
#define LXCFBLR_CFBP	GENMASK(28, 16)	/* Color Frame Buffer Pitch in bytes */

#define LXCFBLNR_CFBLN	GENMASK(10, 0)	/* Color Frame Buffer Line Number */

#define BF1_PAXCA	0x600		/* Pixel Alpha x Constant Alpha */
#define BF1_CA		0x400		/* Constant Alpha */
#define BF2_1PAXCA	0x007		/* 1 - (Pixel Alpha x Constant Alpha) */
#define BF2_1CA		0x005		/* 1 - Constant Alpha */

enum stm32_ltdc_pix_fmt {
	PF_ARGB8888 = 0,
	PF_RGB888,
	PF_RGB565,
	PF_ARGB1555,
	PF_ARGB4444,
	PF_L8,
	PF_AL44,
	PF_AL88
};

/* TODO add more color format support */
static u32 stm32_ltdc_get_pixel_format(enum video_log2_bpp l2bpp)
{
	enum stm32_ltdc_pix_fmt pf;

	switch (l2bpp) {
	case VIDEO_BPP16:
		pf = PF_RGB565;
		break;

	case VIDEO_BPP32:
		pf = PF_ARGB8888;
		break;

	case VIDEO_BPP8:
		pf = PF_L8;
		break;

	case VIDEO_BPP1:
	case VIDEO_BPP2:
	case VIDEO_BPP4:
	default:
		pr_warn("%s: warning %dbpp not supported yet, %dbpp instead\n",
			__func__, VNBITS(l2bpp), VNBITS(VIDEO_BPP16));
		pf = PF_RGB565;
		break;
	}

	debug("%s: %d bpp -> ltdc pf %d\n", __func__, VNBITS(l2bpp), pf);

	return (u32)pf;
}

static bool has_alpha(u32 fmt)
{
	switch (fmt) {
	case PF_ARGB8888:
	case PF_ARGB1555:
	case PF_ARGB4444:
	case PF_AL44:
	case PF_AL88:
		return true;
	case PF_RGB888:
	case PF_RGB565:
	case PF_L8:
	default:
		return false;
	}
}

static void stm32_ltdc_enable(struct stm32_ltdc_priv *priv)
{
	/* Reload configuration immediately & enable LTDC */
	setbits_le32(priv->regs + LTDC_SRCR, SRCR_IMR);
	setbits_le32(priv->regs + LTDC_GCR, GCR_LTDCEN);
}

static void stm32_ltdc_set_mode(struct stm32_ltdc_priv *priv,
				struct display_timing *timings)
{
	void __iomem *regs = priv->regs;
	u32 hsync, vsync, acc_hbp, acc_vbp, acc_act_w, acc_act_h;
	u32 total_w, total_h;
	u32 val;

	/* Convert video timings to ltdc timings */
	hsync = timings->hsync_len.typ - 1;
	vsync = timings->vsync_len.typ - 1;
	acc_hbp = hsync + timings->hback_porch.typ;
	acc_vbp = vsync + timings->vback_porch.typ;
	acc_act_w = acc_hbp + timings->hactive.typ;
	acc_act_h = acc_vbp + timings->vactive.typ;
	total_w = acc_act_w + timings->hfront_porch.typ;
	total_h = acc_act_h + timings->vfront_porch.typ;

	/* Synchronization sizes */
	val = (hsync << 16) | vsync;
	clrsetbits_le32(regs + LTDC_SSCR, SSCR_VSH | SSCR_HSW, val);

	/* Accumulated back porch */
	val = (acc_hbp << 16) | acc_vbp;
	clrsetbits_le32(regs + LTDC_BPCR, BPCR_AVBP | BPCR_AHBP, val);

	/* Accumulated active width */
	val = (acc_act_w << 16) | acc_act_h;
	clrsetbits_le32(regs + LTDC_AWCR, AWCR_AAW | AWCR_AAH, val);

	/* Total width & height */
	val = (total_w << 16) | total_h;
	clrsetbits_le32(regs + LTDC_TWCR, TWCR_TOTALH | TWCR_TOTALW, val);

	setbits_le32(regs + LTDC_LIPCR, acc_act_h + 1);

	/* Signal polarities */
	val = 0;
	debug("%s: timing->flags 0x%08x\n", __func__, timings->flags);
	if (timings->flags & DISPLAY_FLAGS_HSYNC_HIGH)
		val |= GCR_HSPOL;
	if (timings->flags & DISPLAY_FLAGS_VSYNC_HIGH)
		val |= GCR_VSPOL;
	if (timings->flags & DISPLAY_FLAGS_DE_HIGH)
		val |= GCR_DEPOL;
	if (timings->flags & DISPLAY_FLAGS_PIXDATA_NEGEDGE)
		val |= GCR_PCPOL;
	clrsetbits_le32(regs + LTDC_GCR,
			GCR_HSPOL | GCR_VSPOL | GCR_DEPOL | GCR_PCPOL, val);

	/* Overall background color */
	writel(priv->bg_col_argb, priv->regs + LTDC_BCCR);
}

static void stm32_ltdc_set_layer1(struct stm32_ltdc_priv *priv, ulong fb_addr)
{
	void __iomem *regs = priv->regs;
	u32 x0, x1, y0, y1;
	u32 pitch_in_bytes;
	u32 line_length;
	u32 bus_width;
	u32 val, tmp, bpp;
	u32 format;

	x0 = priv->crop_x;
	x1 = priv->crop_x + priv->crop_w - 1;
	y0 = priv->crop_y;
	y1 = priv->crop_y + priv->crop_h - 1;

	/* Horizontal start and stop position */
	tmp = (readl(regs + LTDC_BPCR) & BPCR_AHBP) >> 16;
	val = ((x1 + 1 + tmp) << 16) + (x0 + 1 + tmp);
	clrsetbits_le32(regs + LTDC_L1WHPCR, LXWHPCR_WHSTPOS | LXWHPCR_WHSPPOS,
			val);

	/* Vertical start & stop position */
	tmp = readl(regs + LTDC_BPCR) & BPCR_AVBP;
	val = ((y1 + 1 + tmp) << 16) + (y0 + 1 + tmp);
	clrsetbits_le32(regs + LTDC_L1WVPCR, LXWVPCR_WVSTPOS | LXWVPCR_WVSPPOS,
			val);

	/* Layer background color */
	writel(priv->bg_col_argb, regs + LTDC_L1DCCR);

	/* Color frame buffer pitch in bytes & line length */
	bpp = VNBITS(priv->l2bpp);
	pitch_in_bytes = priv->crop_w * (bpp >> 3);
	bus_width = 8 << ((readl(regs + LTDC_GC2R) & GC2R_BW) >> 4);
	line_length = ((bpp >> 3) * priv->crop_w) + (bus_width >> 3) - 1;
	val = (pitch_in_bytes << 16) | line_length;
	clrsetbits_le32(regs + LTDC_L1CFBLR, LXCFBLR_CFBLL | LXCFBLR_CFBP, val);

	/* Pixel format */
	format = stm32_ltdc_get_pixel_format(priv->l2bpp);
	clrsetbits_le32(regs + LTDC_L1PFCR, LXPFCR_PF, format);

	/* Constant alpha value */
	clrsetbits_le32(regs + LTDC_L1CACR, LXCACR_CONSTA, priv->alpha);

	/* Specifies the blending factors : with or without pixel alpha */
	/* Manage hw-specific capabilities */
	val = has_alpha(format) ? BF1_PAXCA | BF2_1PAXCA : BF1_CA | BF2_1CA;

	/* Blending factors */
	clrsetbits_le32(regs + LTDC_L1BFCR, LXBFCR_BF2 | LXBFCR_BF1, val);

	/* Frame buffer line number */
	clrsetbits_le32(regs + LTDC_L1CFBLNR, LXCFBLNR_CFBLN, priv->crop_h);

	/* Frame buffer address */
	writel(fb_addr, regs + LTDC_L1CFBAR);

	/* Enable layer 1 */
	setbits_le32(priv->regs + LTDC_L1CR, LXCR_LEN);
}

static int stm32_ltdc_probe(struct udevice *dev)
{
	struct video_uc_platdata *uc_plat = dev_get_uclass_platdata(dev);
	struct video_priv *uc_priv = dev_get_uclass_priv(dev);
	struct stm32_ltdc_priv *priv = dev_get_priv(dev);
	struct udevice *bridge = NULL;
	struct udevice *panel = NULL;
	struct display_timing timings;
	struct clk pclk;
	struct reset_ctl rst;
	int ret;

	priv->regs = (void *)dev_read_addr(dev);
	if ((fdt_addr_t)priv->regs == FDT_ADDR_T_NONE) {
		dev_err(dev, "ltdc dt register address error\n");
		return -EINVAL;
	}

	ret = clk_get_by_index(dev, 0, &pclk);
	if (ret) {
		dev_err(dev, "peripheral clock get error %d\n", ret);
		return ret;
	}

	ret = clk_enable(&pclk);
	if (ret) {
		dev_err(dev, "peripheral clock enable error %d\n", ret);
		return ret;
	}

	ret = uclass_first_device_err(UCLASS_PANEL, &panel);
	if (ret) {
		if (ret != -ENODEV)
			dev_err(dev, "panel device error %d\n", ret);
		return ret;
	}

	ret = panel_get_display_timing(panel, &timings);
	if (ret) {
		ret = fdtdec_decode_display_timing(gd->fdt_blob,
						   dev_of_offset(panel),
						   0, &timings);
		if (ret) {
			dev_err(dev, "decode display timing error %d\n", ret);
			return ret;
		}
	}

	ret = clk_set_rate(&pclk, timings.pixelclock.typ);
	if (ret)
		dev_warn(dev, "fail to set pixel clock %d hz\n",
			 timings.pixelclock.typ);

	debug("%s: Set pixel clock req %d hz get %ld hz\n", __func__,
	      timings.pixelclock.typ, clk_get_rate(&pclk));

	ret = reset_get_by_index(dev, 0, &rst);
	if (ret) {
		dev_err(dev, "missing ltdc hardware reset\n");
		return ret;
	}

	/* Reset */
	reset_deassert(&rst);

	if (IS_ENABLED(CONFIG_VIDEO_BRIDGE)) {
		ret = uclass_get_device(UCLASS_VIDEO_BRIDGE, 0, &bridge);
		if (ret)
			debug("No video bridge, or no backlight on bridge\n");

		if (bridge) {
			ret = video_bridge_attach(bridge);
			if (ret) {
				dev_err(dev, "fail to attach bridge\n");
				return ret;
			}
		}
	}

	/* TODO Below parameters are hard-coded for the moment... */
	priv->l2bpp = VIDEO_BPP16;
	priv->bg_col_argb = 0xFFFFFFFF; /* white no transparency */
	priv->crop_x = 0;
	priv->crop_y = 0;
	priv->crop_w = timings.hactive.typ;
	priv->crop_h = timings.vactive.typ;
	priv->alpha = 0xFF;

	debug("%s: %dx%d %dbpp frame buffer at 0x%lx\n", __func__,
	      timings.hactive.typ, timings.vactive.typ,
	      VNBITS(priv->l2bpp), uc_plat->base);
	debug("%s: crop %d,%d %dx%d bg 0x%08x alpha %d\n", __func__,
	      priv->crop_x, priv->crop_y, priv->crop_w, priv->crop_h,
	      priv->bg_col_argb, priv->alpha);

	/* Configure & start LTDC */
	stm32_ltdc_set_mode(priv, &timings);
	stm32_ltdc_set_layer1(priv, uc_plat->base);
	stm32_ltdc_enable(priv);

	uc_priv->xsize = timings.hactive.typ;
	uc_priv->ysize = timings.vactive.typ;
	uc_priv->bpix = priv->l2bpp;

	if (!bridge) {
		ret = panel_enable_backlight(panel);
		if (ret) {
			dev_err(dev, "panel %s enable backlight error %d\n",
				panel->name, ret);
			return ret;
		}
	} else if (IS_ENABLED(CONFIG_VIDEO_BRIDGE)) {
		ret = video_bridge_set_backlight(bridge, 80);
		if (ret) {
			dev_err(dev, "fail to set backlight\n");
			return ret;
		}
	}

	video_set_flush_dcache(dev, true);

	return 0;
}

static int stm32_ltdc_bind(struct udevice *dev)
{
	struct video_uc_platdata *uc_plat = dev_get_uclass_platdata(dev);

	uc_plat->size = CONFIG_VIDEO_STM32_MAX_XRES *
			CONFIG_VIDEO_STM32_MAX_YRES *
			(CONFIG_VIDEO_STM32_MAX_BPP >> 3);
	debug("%s: frame buffer max size %d bytes\n", __func__, uc_plat->size);

	return 0;
}

static const struct udevice_id stm32_ltdc_ids[] = {
	{ .compatible = "st,stm32-ltdc" },
	{ }
};

U_BOOT_DRIVER(stm32_ltdc) = {
	.name			= "stm32_display",
	.id			= UCLASS_VIDEO,
	.of_match		= stm32_ltdc_ids,
	.probe			= stm32_ltdc_probe,
	.bind			= stm32_ltdc_bind,
	.priv_auto_alloc_size	= sizeof(struct stm32_ltdc_priv),
};
