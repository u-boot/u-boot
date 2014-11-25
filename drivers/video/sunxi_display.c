/*
 * Display driver for Allwinner SoCs.
 *
 * (C) Copyright 2013-2014 Luc Verhaegen <libv@skynet.be>
 * (C) Copyright 2014 Hans de Goede <hdegoede@redhat.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>

#include <asm/arch/clock.h>
#include <asm/arch/display.h>
#include <asm/global_data.h>
#include <asm/io.h>
#include <fdtdec.h>
#include <fdt_support.h>
#include <linux/fb.h>
#include <video_fb.h>

DECLARE_GLOBAL_DATA_PTR;

struct sunxi_display {
	GraphicDevice graphic_device;
	bool enabled;
} sunxi_display;

static int sunxi_hdmi_hpd_detect(void)
{
	struct sunxi_ccm_reg * const ccm =
		(struct sunxi_ccm_reg *)SUNXI_CCM_BASE;
	struct sunxi_hdmi_reg * const hdmi =
		(struct sunxi_hdmi_reg *)SUNXI_HDMI_BASE;

	/* Set pll3 to 300MHz */
	clock_set_pll3(300000000);

	/* Set hdmi parent to pll3 */
	clrsetbits_le32(&ccm->hdmi_clk_cfg, CCM_HDMI_CTRL_PLL_MASK,
			CCM_HDMI_CTRL_PLL3);

	/* Set ahb gating to pass */
#ifdef CONFIG_MACH_SUN6I
	setbits_le32(&ccm->ahb_reset1_cfg, 1 << AHB_RESET_OFFSET_HDMI);
#endif
	setbits_le32(&ccm->ahb_gate1, 1 << AHB_GATE_OFFSET_HDMI);

	/* Clock on */
	setbits_le32(&ccm->hdmi_clk_cfg, CCM_HDMI_CTRL_GATE);

	writel(SUNXI_HDMI_CTRL_ENABLE, &hdmi->ctrl);
	writel(SUNXI_HDMI_PAD_CTRL0_HDP, &hdmi->pad_ctrl0);

	udelay(1000);

	if (readl(&hdmi->hpd) & SUNXI_HDMI_HPD_DETECT)
		return 1;

	/* No need to keep these running */
	clrbits_le32(&hdmi->ctrl, SUNXI_HDMI_CTRL_ENABLE);
	clrbits_le32(&ccm->hdmi_clk_cfg, CCM_HDMI_CTRL_GATE);
	clrbits_le32(&ccm->ahb_gate1, 1 << AHB_GATE_OFFSET_HDMI);
#ifdef CONFIG_MACH_SUN6I
	clrbits_le32(&ccm->ahb_reset1_cfg, 1 << AHB_RESET_OFFSET_HDMI);
#endif
	clock_set_pll3(0);

	return 0;
}

/*
 * This is the entity that mixes and matches the different layers and inputs.
 * Allwinner calls it the back-end, but i like composer better.
 */
static void sunxi_composer_init(void)
{
	struct sunxi_ccm_reg * const ccm =
		(struct sunxi_ccm_reg *)SUNXI_CCM_BASE;
	struct sunxi_de_be_reg * const de_be =
		(struct sunxi_de_be_reg *)SUNXI_DE_BE0_BASE;
	int i;

#ifdef CONFIG_MACH_SUN6I
	/* Reset off */
	setbits_le32(&ccm->ahb_reset1_cfg, 1 << AHB_RESET_OFFSET_DE_BE0);
#endif

	/* Clocks on */
	setbits_le32(&ccm->ahb_gate1, 1 << AHB_GATE_OFFSET_DE_BE0);
	setbits_le32(&ccm->dram_clk_gate, 1 << CCM_DRAM_GATE_OFFSET_DE_BE0);
	clock_set_de_mod_clock(&ccm->be0_clk_cfg, 300000000);

	/* Engine bug, clear registers after reset */
	for (i = 0x0800; i < 0x1000; i += 4)
		writel(0, SUNXI_DE_BE0_BASE + i);

	setbits_le32(&de_be->mode, SUNXI_DE_BE_MODE_ENABLE);
}

static void sunxi_composer_mode_set(struct fb_videomode *mode,
				    unsigned int address)
{
	struct sunxi_de_be_reg * const de_be =
		(struct sunxi_de_be_reg *)SUNXI_DE_BE0_BASE;

	writel(SUNXI_DE_BE_HEIGHT(mode->yres) | SUNXI_DE_BE_WIDTH(mode->xres),
	       &de_be->disp_size);
	writel(SUNXI_DE_BE_HEIGHT(mode->yres) | SUNXI_DE_BE_WIDTH(mode->xres),
	       &de_be->layer0_size);
	writel(SUNXI_DE_BE_LAYER_STRIDE(mode->xres), &de_be->layer0_stride);
	writel(address << 3, &de_be->layer0_addr_low32b);
	writel(address >> 29, &de_be->layer0_addr_high4b);
	writel(SUNXI_DE_BE_LAYER_ATTR1_FMT_XRGB8888, &de_be->layer0_attr1_ctrl);

	setbits_le32(&de_be->mode, SUNXI_DE_BE_MODE_LAYER0_ENABLE);
}

/*
 * LCDC, what allwinner calls a CRTC, so timing controller and serializer.
 */
static void sunxi_lcdc_pll_set(int dotclock, int *clk_div, int *clk_double)
{
	struct sunxi_ccm_reg * const ccm =
		(struct sunxi_ccm_reg *)SUNXI_CCM_BASE;
	int value, n, m, diff;
	int best_n = 0, best_m = 0, best_diff = 0x0FFFFFFF;
	int best_double = 0;

	/*
	 * Find the lowest divider resulting in a matching clock, if there
	 * is no match, pick the closest lower clock, as monitors tend to
	 * not sync to higher frequencies.
	 */
	for (m = 15; m > 0; m--) {
		n = (m * dotclock) / 3000;

		if ((n >= 9) && (n <= 127)) {
			value = (3000 * n) / m;
			diff = dotclock - value;
			if (diff < best_diff) {
				best_diff = diff;
				best_m = m;
				best_n = n;
				best_double = 0;
			}
		}

		/* These are just duplicates */
		if (!(m & 1))
			continue;

		n = (m * dotclock) / 6000;
		if ((n >= 9) && (n <= 127)) {
			value = (6000 * n) / m;
			diff = dotclock - value;
			if (diff < best_diff) {
				best_diff = diff;
				best_m = m;
				best_n = n;
				best_double = 1;
			}
		}
	}

	debug("dotclock: %dkHz = %dkHz: (%d * 3MHz * %d) / %d\n",
	      dotclock, (best_double + 1) * 3000 * best_n / best_m,
	      best_double + 1, best_n, best_m);

	clock_set_pll3(best_n * 3000000);

	writel(CCM_LCD_CH1_CTRL_GATE |
	    (best_double ? CCM_LCD_CH1_CTRL_PLL3_2X : CCM_LCD_CH1_CTRL_PLL3) |
	    CCM_LCD_CH1_CTRL_M(best_m), &ccm->lcd0_ch1_clk_cfg);

	*clk_div = best_m;
	*clk_double = best_double;
}

static void sunxi_lcdc_init(void)
{
	struct sunxi_ccm_reg * const ccm =
		(struct sunxi_ccm_reg *)SUNXI_CCM_BASE;
	struct sunxi_lcdc_reg * const lcdc =
		(struct sunxi_lcdc_reg *)SUNXI_LCD0_BASE;

	/* Reset off */
#ifdef CONFIG_MACH_SUN6I
	setbits_le32(&ccm->ahb_reset1_cfg, 1 << AHB_RESET_OFFSET_LCD0);
#else
	setbits_le32(&ccm->lcd0_ch0_clk_cfg, CCM_LCD_CH0_CTRL_RST);
#endif

	/* Clock on */
	setbits_le32(&ccm->ahb_gate1, 1 << AHB_GATE_OFFSET_LCD0);

	/* Init lcdc */
	writel(0, &lcdc->ctrl); /* Disable tcon */
	writel(0, &lcdc->int0); /* Disable all interrupts */

	/* Disable tcon0 dot clock */
	clrbits_le32(&lcdc->tcon0_dclk, SUNXI_LCDC_TCON0_DCLK_ENABLE);

	/* Set all io lines to tristate */
	writel(0xffffffff, &lcdc->tcon0_io_tristate);
	writel(0xffffffff, &lcdc->tcon1_io_tristate);
}

static void sunxi_lcdc_mode_set(struct fb_videomode *mode,
				int *clk_div, int *clk_double)
{
	struct sunxi_lcdc_reg * const lcdc =
		(struct sunxi_lcdc_reg *)SUNXI_LCD0_BASE;
	int bp, total;

	/* Use tcon1 */
	clrsetbits_le32(&lcdc->ctrl, SUNXI_LCDC_CTRL_IO_MAP_MASK,
			SUNXI_LCDC_CTRL_IO_MAP_TCON1);

	/* Enabled, 0x1e start delay */
	writel(SUNXI_LCDC_TCON1_CTRL_ENABLE |
	       SUNXI_LCDC_TCON1_CTRL_CLK_DELAY(0x1e), &lcdc->tcon1_ctrl);

	writel(SUNXI_LCDC_X(mode->xres) | SUNXI_LCDC_Y(mode->yres),
	       &lcdc->tcon1_timing_source);
	writel(SUNXI_LCDC_X(mode->xres) | SUNXI_LCDC_Y(mode->yres),
	       &lcdc->tcon1_timing_scale);
	writel(SUNXI_LCDC_X(mode->xres) | SUNXI_LCDC_Y(mode->yres),
	       &lcdc->tcon1_timing_out);

	bp = mode->hsync_len + mode->left_margin;
	total = mode->xres + mode->right_margin + bp;
	writel(SUNXI_LCDC_TCON1_TIMING_H_TOTAL(total) |
	       SUNXI_LCDC_TCON1_TIMING_H_BP(bp), &lcdc->tcon1_timing_h);

	bp = mode->vsync_len + mode->upper_margin;
	total = mode->yres + mode->lower_margin + bp;
	writel(SUNXI_LCDC_TCON1_TIMING_V_TOTAL(total) |
	       SUNXI_LCDC_TCON1_TIMING_V_BP(bp), &lcdc->tcon1_timing_v);

	writel(SUNXI_LCDC_X(mode->hsync_len) | SUNXI_LCDC_Y(mode->vsync_len),
	       &lcdc->tcon1_timing_sync);

	sunxi_lcdc_pll_set(mode->pixclock, clk_div, clk_double);
}

#ifdef CONFIG_MACH_SUN6I
static void sunxi_drc_init(void)
{
	struct sunxi_ccm_reg * const ccm =
		(struct sunxi_ccm_reg *)SUNXI_CCM_BASE;

	/* On sun6i the drc must be clocked even when in pass-through mode */
	setbits_le32(&ccm->ahb_reset1_cfg, 1 << AHB_RESET_OFFSET_DRC0);
	clock_set_de_mod_clock(&ccm->iep_drc0_clk_cfg, 300000000);
}
#endif

static void sunxi_hdmi_mode_set(struct fb_videomode *mode,
				int clk_div, int clk_double)
{
	struct sunxi_hdmi_reg * const hdmi =
		(struct sunxi_hdmi_reg *)SUNXI_HDMI_BASE;
	int x, y;

	/* Write clear interrupt status bits */
	writel(SUNXI_HDMI_IRQ_STATUS_BITS, &hdmi->irq);

	/* Init various registers, select pll3 as clock source */
	writel(SUNXI_HDMI_VIDEO_POL_TX_CLK, &hdmi->video_polarity);
	writel(SUNXI_HDMI_PAD_CTRL0_RUN, &hdmi->pad_ctrl0);
	writel(SUNXI_HDMI_PAD_CTRL1, &hdmi->pad_ctrl1);
	writel(SUNXI_HDMI_PLL_CTRL, &hdmi->pll_ctrl);
	writel(SUNXI_HDMI_PLL_DBG0_PLL3, &hdmi->pll_dbg0);

	/* Setup clk div and doubler */
	clrsetbits_le32(&hdmi->pll_ctrl, SUNXI_HDMI_PLL_CTRL_DIV_MASK,
			SUNXI_HDMI_PLL_CTRL_DIV(clk_div));
	if (!clk_double)
		setbits_le32(&hdmi->pad_ctrl1, SUNXI_HDMI_PAD_CTRL1_HALVE);

	/* Setup timing registers */
	writel(SUNXI_HDMI_Y(mode->yres) | SUNXI_HDMI_X(mode->xres),
	       &hdmi->video_size);

	x = mode->hsync_len + mode->left_margin;
	y = mode->vsync_len + mode->upper_margin;
	writel(SUNXI_HDMI_Y(y) | SUNXI_HDMI_X(x), &hdmi->video_bp);

	x = mode->right_margin;
	y = mode->lower_margin;
	writel(SUNXI_HDMI_Y(y) | SUNXI_HDMI_X(x), &hdmi->video_fp);

	x = mode->hsync_len;
	y = mode->vsync_len;
	writel(SUNXI_HDMI_Y(y) | SUNXI_HDMI_X(x), &hdmi->video_spw);

	if (mode->sync & FB_SYNC_HOR_HIGH_ACT)
		setbits_le32(&hdmi->video_polarity, SUNXI_HDMI_VIDEO_POL_HOR);

	if (mode->sync & FB_SYNC_VERT_HIGH_ACT)
		setbits_le32(&hdmi->video_polarity, SUNXI_HDMI_VIDEO_POL_VER);
}

static void sunxi_engines_init(void)
{
	sunxi_composer_init();
	sunxi_lcdc_init();
#ifdef CONFIG_MACH_SUN6I
	sunxi_drc_init();
#endif
}

static void sunxi_mode_set(struct fb_videomode *mode, unsigned int address)
{
	struct sunxi_de_be_reg * const de_be =
		(struct sunxi_de_be_reg *)SUNXI_DE_BE0_BASE;
	struct sunxi_lcdc_reg * const lcdc =
		(struct sunxi_lcdc_reg *)SUNXI_LCD0_BASE;
	struct sunxi_hdmi_reg * const hdmi =
		(struct sunxi_hdmi_reg *)SUNXI_HDMI_BASE;
	int clk_div, clk_double;
	int retries = 3;

retry:
	clrbits_le32(&hdmi->video_ctrl, SUNXI_HDMI_VIDEO_CTRL_ENABLE);
	clrbits_le32(&lcdc->ctrl, SUNXI_LCDC_CTRL_TCON_ENABLE);
	clrbits_le32(&de_be->mode, SUNXI_DE_BE_MODE_START);

	sunxi_composer_mode_set(mode, address);
	sunxi_lcdc_mode_set(mode, &clk_div, &clk_double);
	sunxi_hdmi_mode_set(mode, clk_div, clk_double);

	setbits_le32(&de_be->reg_ctrl, SUNXI_DE_BE_REG_CTRL_LOAD_REGS);
	setbits_le32(&de_be->mode, SUNXI_DE_BE_MODE_START);

	udelay(1000000 / mode->refresh + 500);

	setbits_le32(&lcdc->ctrl, SUNXI_LCDC_CTRL_TCON_ENABLE);

	udelay(1000000 / mode->refresh + 500);

	setbits_le32(&hdmi->video_ctrl, SUNXI_HDMI_VIDEO_CTRL_ENABLE);

	udelay(1000000 / mode->refresh + 500);

	/*
	 * Sometimes the display pipeline does not sync up properly, if
	 * this happens the hdmi fifo underrun or overrun bits are set.
	 */
	if (readl(&hdmi->irq) &
	    (SUNXI_HDMI_IRQ_STATUS_FIFO_UF | SUNXI_HDMI_IRQ_STATUS_FIFO_OF)) {
		if (retries--)
			goto retry;
		printf("HDMI fifo under or overrun\n");
	}
}

void *video_hw_init(void)
{
	static GraphicDevice *graphic_device = &sunxi_display.graphic_device;
	/*
	 * Vesa standard 1024x768@60
	 * 65.0  1024 1048 1184 1344  768 771 777 806  -hsync -vsync
	 */
	struct fb_videomode mode = {
		.name = "1024x768",
		.refresh = 60,
		.xres = 1024,
		.yres = 768,
		.pixclock = 65000,
		.left_margin = 160,
		.right_margin = 24,
		.upper_margin = 29,
		.lower_margin = 3,
		.hsync_len = 136,
		.vsync_len = 6,
		.sync = 0,
		.vmode = 0,
		.flag = 0,
	};
	int ret;

	memset(&sunxi_display, 0, sizeof(struct sunxi_display));

	printf("Reserved %dkB of RAM for Framebuffer.\n",
	       CONFIG_SUNXI_FB_SIZE >> 10);
	gd->fb_base = gd->ram_top;

	ret = sunxi_hdmi_hpd_detect();
	if (!ret)
		return NULL;

	printf("HDMI connected.\n");
	sunxi_display.enabled = true;

	printf("Setting up a %s console.\n", mode.name);
	sunxi_engines_init();
	sunxi_mode_set(&mode, gd->fb_base - CONFIG_SYS_SDRAM_BASE);

	/*
	 * These are the only members of this structure that are used. All the
	 * others are driver specific. There is nothing to decribe pitch or
	 * stride, but we are lucky with our hw.
	 */
	graphic_device->frameAdrs = gd->fb_base;
	graphic_device->gdfIndex = GDF_32BIT_X888RGB;
	graphic_device->gdfBytesPP = 4;
	graphic_device->winSizeX = mode.xres;
	graphic_device->winSizeY = mode.yres;

	return graphic_device;
}

/*
 * Simplefb support.
 */
#if defined(CONFIG_OF_BOARD_SETUP) && defined(CONFIG_VIDEO_DT_SIMPLEFB)
int sunxi_simplefb_setup(void *blob)
{
	static GraphicDevice *graphic_device = &sunxi_display.graphic_device;
	int offset, ret;

	if (!sunxi_display.enabled)
		return 0;

	/* Find a framebuffer node, with pipeline == "de_be0-lcd0-hdmi" */
	offset = fdt_node_offset_by_compatible(blob, -1,
					       "allwinner,simple-framebuffer");
	while (offset >= 0) {
		ret = fdt_find_string(blob, offset, "allwinner,pipeline",
				      "de_be0-lcd0-hdmi");
		if (ret == 0)
			break;
		offset = fdt_node_offset_by_compatible(blob, offset,
					       "allwinner,simple-framebuffer");
	}
	if (offset < 0) {
		eprintf("Cannot setup simplefb: node not found\n");
		return 0; /* Keep older kernels working */
	}

	ret = fdt_setup_simplefb_node(blob, offset, gd->fb_base,
			graphic_device->winSizeX, graphic_device->winSizeY,
			graphic_device->winSizeX * graphic_device->gdfBytesPP,
			"x8r8g8b8");
	if (ret)
		eprintf("Cannot setup simplefb: Error setting properties\n");

	return ret;
}
#endif /* CONFIG_OF_BOARD_SETUP && CONFIG_VIDEO_DT_SIMPLEFB */
