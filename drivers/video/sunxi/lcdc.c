/*
 * Timing controller driver for Allwinner SoCs.
 *
 * (C) Copyright 2013-2014 Luc Verhaegen <libv@skynet.be>
 * (C) Copyright 2014-2015 Hans de Goede <hdegoede@redhat.com>
 * (C) Copyright 2017 Jernej Skrabec <jernej.skrabec@siol.net>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>

#include <asm/arch/lcdc.h>
#include <asm/io.h>

static int lcdc_get_clk_delay(const struct display_timing *mode, int tcon)
{
	int delay;

	delay = mode->vfront_porch.typ + mode->vsync_len.typ +
		mode->vback_porch.typ;
	if (mode->flags & DISPLAY_FLAGS_INTERLACED)
		delay /= 2;
	if (tcon == 1)
		delay -= 2;

	return (delay > 30) ? 30 : delay;
}

void lcdc_init(struct sunxi_lcdc_reg * const lcdc)
{
	/* Init lcdc */
	writel(0, &lcdc->ctrl); /* Disable tcon */
	writel(0, &lcdc->int0); /* Disable all interrupts */

	/* Disable tcon0 dot clock */
	clrbits_le32(&lcdc->tcon0_dclk, SUNXI_LCDC_TCON0_DCLK_ENABLE);

	/* Set all io lines to tristate */
	writel(0xffffffff, &lcdc->tcon0_io_tristate);
	writel(0xffffffff, &lcdc->tcon1_io_tristate);
}

void lcdc_enable(struct sunxi_lcdc_reg * const lcdc, int depth)
{
	setbits_le32(&lcdc->ctrl, SUNXI_LCDC_CTRL_TCON_ENABLE);
#ifdef CONFIG_VIDEO_LCD_IF_LVDS
	setbits_le32(&lcdc->tcon0_lvds_intf, SUNXI_LCDC_TCON0_LVDS_INTF_ENABLE);
	setbits_le32(&lcdc->lvds_ana0, SUNXI_LCDC_LVDS_ANA0);
#ifdef CONFIG_SUNXI_GEN_SUN6I
	udelay(2); /* delay at least 1200 ns */
	setbits_le32(&lcdc->lvds_ana0, SUNXI_LCDC_LVDS_ANA0_EN_MB);
	udelay(2); /* delay at least 1200 ns */
	setbits_le32(&lcdc->lvds_ana0, SUNXI_LCDC_LVDS_ANA0_DRVC);
	if (depth == 18)
		setbits_le32(&lcdc->lvds_ana0, SUNXI_LCDC_LVDS_ANA0_DRVD(0x7));
	else
		setbits_le32(&lcdc->lvds_ana0, SUNXI_LCDC_LVDS_ANA0_DRVD(0xf));
#else
	setbits_le32(&lcdc->lvds_ana0, SUNXI_LCDC_LVDS_ANA0_UPDATE);
	udelay(2); /* delay at least 1200 ns */
	setbits_le32(&lcdc->lvds_ana1, SUNXI_LCDC_LVDS_ANA1_INIT1);
	udelay(1); /* delay at least 120 ns */
	setbits_le32(&lcdc->lvds_ana1, SUNXI_LCDC_LVDS_ANA1_INIT2);
	setbits_le32(&lcdc->lvds_ana0, SUNXI_LCDC_LVDS_ANA0_UPDATE);
#endif
#endif
}

void lcdc_tcon0_mode_set(struct sunxi_lcdc_reg * const lcdc,
			 const struct display_timing *mode,
			 int clk_div, bool for_ext_vga_dac,
			 int depth, int dclk_phase)
{
	int bp, clk_delay, total, val;

#ifndef CONFIG_SUNXI_DE2
	/* Use tcon0 */
	clrsetbits_le32(&lcdc->ctrl, SUNXI_LCDC_CTRL_IO_MAP_MASK,
			SUNXI_LCDC_CTRL_IO_MAP_TCON0);
#endif

	clk_delay = lcdc_get_clk_delay(mode, 0);
	writel(SUNXI_LCDC_TCON0_CTRL_ENABLE |
	       SUNXI_LCDC_TCON0_CTRL_CLK_DELAY(clk_delay), &lcdc->tcon0_ctrl);

	writel(SUNXI_LCDC_TCON0_DCLK_ENABLE |
	       SUNXI_LCDC_TCON0_DCLK_DIV(clk_div), &lcdc->tcon0_dclk);

	writel(SUNXI_LCDC_X(mode->hactive.typ) |
	       SUNXI_LCDC_Y(mode->vactive.typ), &lcdc->tcon0_timing_active);

	bp = mode->hsync_len.typ + mode->hback_porch.typ;
	total = mode->hactive.typ + mode->hfront_porch.typ + bp;
	writel(SUNXI_LCDC_TCON0_TIMING_H_TOTAL(total) |
	       SUNXI_LCDC_TCON0_TIMING_H_BP(bp), &lcdc->tcon0_timing_h);

	bp = mode->vsync_len.typ + mode->vback_porch.typ;
	total = mode->vactive.typ + mode->vfront_porch.typ + bp;
	writel(SUNXI_LCDC_TCON0_TIMING_V_TOTAL(total) |
	       SUNXI_LCDC_TCON0_TIMING_V_BP(bp), &lcdc->tcon0_timing_v);

#ifdef CONFIG_VIDEO_LCD_IF_PARALLEL
	writel(SUNXI_LCDC_X(mode->hsync_len.typ) |
	       SUNXI_LCDC_Y(mode->vsync_len.typ), &lcdc->tcon0_timing_sync);

	writel(0, &lcdc->tcon0_hv_intf);
	writel(0, &lcdc->tcon0_cpu_intf);
#endif
#ifdef CONFIG_VIDEO_LCD_IF_LVDS
	val = (depth == 18) ? 1 : 0;
	writel(SUNXI_LCDC_TCON0_LVDS_INTF_BITWIDTH(val) |
	       SUNXI_LCDC_TCON0_LVDS_CLK_SEL_TCON0, &lcdc->tcon0_lvds_intf);
#endif

	if (depth == 18 || depth == 16) {
		writel(SUNXI_LCDC_TCON0_FRM_SEED, &lcdc->tcon0_frm_seed[0]);
		writel(SUNXI_LCDC_TCON0_FRM_SEED, &lcdc->tcon0_frm_seed[1]);
		writel(SUNXI_LCDC_TCON0_FRM_SEED, &lcdc->tcon0_frm_seed[2]);
		writel(SUNXI_LCDC_TCON0_FRM_SEED, &lcdc->tcon0_frm_seed[3]);
		writel(SUNXI_LCDC_TCON0_FRM_SEED, &lcdc->tcon0_frm_seed[4]);
		writel(SUNXI_LCDC_TCON0_FRM_SEED, &lcdc->tcon0_frm_seed[5]);
		writel(SUNXI_LCDC_TCON0_FRM_TAB0, &lcdc->tcon0_frm_table[0]);
		writel(SUNXI_LCDC_TCON0_FRM_TAB1, &lcdc->tcon0_frm_table[1]);
		writel(SUNXI_LCDC_TCON0_FRM_TAB2, &lcdc->tcon0_frm_table[2]);
		writel(SUNXI_LCDC_TCON0_FRM_TAB3, &lcdc->tcon0_frm_table[3]);
		writel(((depth == 18) ?
			SUNXI_LCDC_TCON0_FRM_CTRL_RGB666 :
			SUNXI_LCDC_TCON0_FRM_CTRL_RGB565),
		       &lcdc->tcon0_frm_ctrl);
	}

	val = SUNXI_LCDC_TCON0_IO_POL_DCLK_PHASE(dclk_phase);
	if (mode->flags & DISPLAY_FLAGS_HSYNC_LOW)
		val |= SUNXI_LCDC_TCON_HSYNC_MASK;
	if (mode->flags & DISPLAY_FLAGS_VSYNC_LOW)
		val |= SUNXI_LCDC_TCON_VSYNC_MASK;

#ifdef CONFIG_VIDEO_VGA_VIA_LCD_FORCE_SYNC_ACTIVE_HIGH
	if (for_ext_vga_dac)
		val = 0;
#endif
	writel(val, &lcdc->tcon0_io_polarity);

	writel(0, &lcdc->tcon0_io_tristate);
}

void lcdc_tcon1_mode_set(struct sunxi_lcdc_reg * const lcdc,
			 const struct display_timing *mode,
			 bool ext_hvsync, bool is_composite)
{
	int bp, clk_delay, total, val, yres;

#ifndef CONFIG_SUNXI_DE2
	/* Use tcon1 */
	clrsetbits_le32(&lcdc->ctrl, SUNXI_LCDC_CTRL_IO_MAP_MASK,
			SUNXI_LCDC_CTRL_IO_MAP_TCON1);
#endif

	clk_delay = lcdc_get_clk_delay(mode, 1);
	writel(SUNXI_LCDC_TCON1_CTRL_ENABLE |
	       ((mode->flags & DISPLAY_FLAGS_INTERLACED) ?
			SUNXI_LCDC_TCON1_CTRL_INTERLACE_ENABLE : 0) |
	       SUNXI_LCDC_TCON1_CTRL_CLK_DELAY(clk_delay), &lcdc->tcon1_ctrl);

	yres = mode->vactive.typ;
	if (mode->flags & DISPLAY_FLAGS_INTERLACED)
		yres /= 2;
	writel(SUNXI_LCDC_X(mode->hactive.typ) | SUNXI_LCDC_Y(yres),
	       &lcdc->tcon1_timing_source);
	writel(SUNXI_LCDC_X(mode->hactive.typ) | SUNXI_LCDC_Y(yres),
	       &lcdc->tcon1_timing_scale);
	writel(SUNXI_LCDC_X(mode->hactive.typ) | SUNXI_LCDC_Y(yres),
	       &lcdc->tcon1_timing_out);

	bp = mode->hsync_len.typ + mode->hback_porch.typ;
	total = mode->hactive.typ + mode->hfront_porch.typ + bp;
	writel(SUNXI_LCDC_TCON1_TIMING_H_TOTAL(total) |
	       SUNXI_LCDC_TCON1_TIMING_H_BP(bp), &lcdc->tcon1_timing_h);

	bp = mode->vsync_len.typ + mode->vback_porch.typ;
	total = mode->vactive.typ + mode->vfront_porch.typ + bp;
	if (!(mode->flags & DISPLAY_FLAGS_INTERLACED))
		total *= 2;
	writel(SUNXI_LCDC_TCON1_TIMING_V_TOTAL(total) |
	       SUNXI_LCDC_TCON1_TIMING_V_BP(bp), &lcdc->tcon1_timing_v);

	writel(SUNXI_LCDC_X(mode->hsync_len.typ) |
	       SUNXI_LCDC_Y(mode->vsync_len.typ), &lcdc->tcon1_timing_sync);

	if (ext_hvsync) {
		val = 0;
		if (mode->flags & DISPLAY_FLAGS_HSYNC_HIGH)
			val |= SUNXI_LCDC_TCON_HSYNC_MASK;
		if (mode->flags & DISPLAY_FLAGS_VSYNC_HIGH)
			val |= SUNXI_LCDC_TCON_VSYNC_MASK;
		writel(val, &lcdc->tcon1_io_polarity);

		clrbits_le32(&lcdc->tcon1_io_tristate,
			     SUNXI_LCDC_TCON_VSYNC_MASK |
			     SUNXI_LCDC_TCON_HSYNC_MASK);
	}

#ifdef CONFIG_MACH_SUN5I
	if (is_composite)
		clrsetbits_le32(&lcdc->mux_ctrl, SUNXI_LCDC_MUX_CTRL_SRC0_MASK,
				SUNXI_LCDC_MUX_CTRL_SRC0(1));
#endif
}
