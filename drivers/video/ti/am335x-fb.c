// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2013-2018 Hannes Schmelzer <oe5hpm@oevsv.at>
 * B&R Industrial Automation GmbH - http://www.br-automation.com
 * Copyright (C) 2020 Dario Binacchi <dariobin@libero.it>
 *
 * minimal framebuffer driver for TI's AM335x SoC to be compatible with
 * Wolfgang Denk's LCD-Framework (CONFIG_LCD, common/lcd.c)
 *
 * - supporting 16/24/32bit RGB/TFT raster Mode (not using palette)
 * - sets up LCD controller as in 'am335x_lcdpanel' struct given
 * - starts output DMA from gd->fb_base buffer
 */
#include <common.h>
#include <lcd.h>
#include <log.h>
#include <asm/arch/clock.h>
#include <asm/arch/hardware.h>
#include <asm/arch/omap.h>
#include <asm/arch/sys_proto.h>
#include <asm/global_data.h>
#include <asm/io.h>
#include <linux/delay.h>
#include <linux/err.h>
#include "am335x-fb.h"

#define LCDC_FMAX				200000000

/* LCD Control Register */
#define LCDC_CTRL_CLK_DIVISOR_MASK		GENMASK(15, 8)
#define LCDC_CTRL_RASTER_MODE			BIT(0)
#define LCDC_CTRL_CLK_DIVISOR(x)		(((x) & GENMASK(7, 0)) << 8)
/* LCD Clock Enable Register */
#define LCDC_CLKC_ENABLE_CORECLKEN		BIT(0)
#define LCDC_CLKC_ENABLE_LIDDCLKEN		BIT(1)
#define LCDC_CLKC_ENABLE_DMACLKEN		BIT(2)
/* LCD DMA Control Register */
#define LCDC_DMA_CTRL_BURST_SIZE(x)		(((x) & GENMASK(2, 0)) << 4)
#define LCDC_DMA_CTRL_BURST_1			0x0
#define LCDC_DMA_CTRL_BURST_2			0x1
#define LCDC_DMA_CTRL_BURST_4			0x2
#define LCDC_DMA_CTRL_BURST_8			0x3
#define LCDC_DMA_CTRL_BURST_16			0x4
#define LCDC_DMA_CTRL_FIFO_TH(x)		(((x) & GENMASK(2, 0)) << 8)
/* LCD Timing_0 Register */
#define LCDC_RASTER_TIMING_0_HORMSB(x)	((((x) - 1) & BIT(10)) >> 7)
#define LCDC_RASTER_TIMING_0_HORLSB(x) (((((x) >> 4) - 1) & GENMASK(5, 0)) << 4)
#define LCDC_RASTER_TIMING_0_HSWLSB(x)	((((x) - 1) & GENMASK(5, 0)) << 10)
#define LCDC_RASTER_TIMING_0_HFPLSB(x)	((((x) - 1) & GENMASK(7, 0)) << 16)
#define LCDC_RASTER_TIMING_0_HBPLSB(x)	((((x) - 1) & GENMASK(7, 0)) << 24)
/* LCD Timing_1 Register */
#define LCDC_RASTER_TIMING_1_VERLSB(x)		(((x) - 1) & GENMASK(9, 0))
#define LCDC_RASTER_TIMING_1_VSW(x)	((((x) - 1) & GENMASK(5, 0)) << 10)
#define LCDC_RASTER_TIMING_1_VFP(x)		(((x) & GENMASK(7, 0)) << 16)
#define LCDC_RASTER_TIMING_1_VBP(x)		(((x) & GENMASK(7, 0)) << 24)
/* LCD Timing_2 Register */
#define LCDC_RASTER_TIMING_2_HFPMSB(x)	((((x) - 1) & GENMASK(9, 8)) >> 8)
#define LCDC_RASTER_TIMING_2_HBPMSB(x)	((((x) - 1) & GENMASK(9, 8)) >> 4)
#define LCDC_RASTER_TIMING_2_ACB(x)		(((x) & GENMASK(7, 0)) << 8)
#define LCDC_RASTER_TIMING_2_ACBI(x)		(((x) & GENMASK(3, 0)) << 16)
#define LCDC_RASTER_TIMING_2_VSYNC_INVERT	BIT(20)
#define LCDC_RASTER_TIMING_2_HSYNC_INVERT	BIT(21)
#define LCDC_RASTER_TIMING_2_PXCLK_INVERT	BIT(22)
#define LCDC_RASTER_TIMING_2_DE_INVERT		BIT(23)
#define LCDC_RASTER_TIMING_2_HSVS_RISEFALL	BIT(24)
#define LCDC_RASTER_TIMING_2_HSVS_CONTROL	BIT(25)
#define LCDC_RASTER_TIMING_2_VERMSB(x)		((((x) - 1) & BIT(10)) << 16)
#define LCDC_RASTER_TIMING_2_HSWMSB(x)	((((x) - 1) & GENMASK(9, 6)) << 21)
/* LCD Raster Ctrl Register */
#define LCDC_RASTER_CTRL_ENABLE			BIT(0)
#define LCDC_RASTER_CTRL_TFT_MODE		BIT(7)
#define LCDC_RASTER_CTRL_DATA_ORDER		BIT(8)
#define LCDC_RASTER_CTRL_REQDLY(x)		(((x) & GENMASK(7, 0)) << 12)
#define LCDC_RASTER_CTRL_PALMODE_RAWDATA	(0x02 << 20)
#define LCDC_RASTER_CTRL_TFT_ALT_ENABLE		BIT(23)
#define LCDC_RASTER_CTRL_TFT_24BPP_MODE		BIT(25)
#define LCDC_RASTER_CTRL_TFT_24BPP_UNPACK	BIT(26)

struct am335x_lcdhw {
	unsigned int		pid;			/* 0x00 */
	unsigned int		ctrl;			/* 0x04 */
	unsigned int		gap0;			/* 0x08 */
	unsigned int		lidd_ctrl;		/* 0x0C */
	unsigned int		lidd_cs0_conf;		/* 0x10 */
	unsigned int		lidd_cs0_addr;		/* 0x14 */
	unsigned int		lidd_cs0_data;		/* 0x18 */
	unsigned int		lidd_cs1_conf;		/* 0x1C */
	unsigned int		lidd_cs1_addr;		/* 0x20 */
	unsigned int		lidd_cs1_data;		/* 0x24 */
	unsigned int		raster_ctrl;		/* 0x28 */
	unsigned int		raster_timing0;		/* 0x2C */
	unsigned int		raster_timing1;		/* 0x30 */
	unsigned int		raster_timing2;		/* 0x34 */
	unsigned int		raster_subpanel;	/* 0x38 */
	unsigned int		raster_subpanel2;	/* 0x3C */
	unsigned int		lcddma_ctrl;		/* 0x40 */
	unsigned int		lcddma_fb0_base;	/* 0x44 */
	unsigned int		lcddma_fb0_ceiling;	/* 0x48 */
	unsigned int		lcddma_fb1_base;	/* 0x4C */
	unsigned int		lcddma_fb1_ceiling;	/* 0x50 */
	unsigned int		sysconfig;		/* 0x54 */
	unsigned int		irqstatus_raw;		/* 0x58 */
	unsigned int		irqstatus;		/* 0x5C */
	unsigned int		irqenable_set;		/* 0x60 */
	unsigned int		irqenable_clear;	/* 0x64 */
	unsigned int		gap1;			/* 0x68 */
	unsigned int		clkc_enable;		/* 0x6C */
	unsigned int		clkc_reset;		/* 0x70 */
};

DECLARE_GLOBAL_DATA_PTR;

#if !defined(LCD_CNTL_BASE)
#error "hw-base address of LCD-Controller (LCD_CNTL_BASE) not defined!"
#endif

/* Macro definitions */
#define FBSIZE(x)	(((x)->hactive * (x)->vactive * (x)->bpp) >> 3)

#define LCDC_RASTER_TIMING_2_INVMASK(x)		((x) & GENMASK(25, 20))

static struct am335x_lcdhw *lcdhw = (void *)LCD_CNTL_BASE;

int lcd_get_size(int *line_length)
{
	*line_length = (panel_info.vl_col * NBITS(panel_info.vl_bpix)) / 8;
	return *line_length * panel_info.vl_row + 0x20;
}

struct dpll_data {
	unsigned long rounded_rate;
	u16 rounded_m;
	u8 rounded_n;
	u8 rounded_div;
};

/**
 * am335x_dpll_round_rate() - Round a target rate for an OMAP DPLL
 *
 * @dpll_data: struct dpll_data pointer for the DPLL
 * @rate:      New DPLL clock rate
 * @return rounded rate and the computed m, n and div values in the dpll_data
 *         structure, or -ve error code.
 */
static ulong am335x_dpll_round_rate(struct dpll_data *dd, ulong rate)
{
	unsigned int m, n, d;
	unsigned long rounded_rate;
	int err, err_r;

	dd->rounded_rate = -EFAULT;
	err = rate;
	err_r = err;

	for (d = 2; err && d < 255; d++) {
		for (m = 2; m < 2047; m++) {
			if ((V_OSCK * m) < (rate * d))
				continue;

			n = (V_OSCK * m) / (rate * d);
			if (n > 127)
				break;

			if (((V_OSCK * m) / n) > LCDC_FMAX)
				break;

			rounded_rate = (V_OSCK * m) / n / d;
			err = abs(rounded_rate - rate);
			if (err < err_r) {
				err_r = err;
				dd->rounded_rate = rounded_rate;
				dd->rounded_m = m;
				dd->rounded_n = n;
				dd->rounded_div = d;
				if (err == 0)
					break;
			}
		}
	}

	debug("DPLL display: best error %d Hz (M %d, N %d, DIV %d)\n",
	      err_r, dd->rounded_m, dd->rounded_n, dd->rounded_div);

	return dd->rounded_rate;
}

/**
 * am335x_fb_set_pixel_clk_rate() - Set pixel clock rate.
 *
 * @am335x_lcdhw: Base address of the LCD controller registers.
 * @rate:         New clock rate in Hz.
 * @return new rate, or -ve error code.
 */
static ulong am335x_fb_set_pixel_clk_rate(struct am335x_lcdhw *regs, ulong rate)
{
	struct dpll_params dpll_disp = { 1, 0, 1, -1, -1, -1, -1 };
	struct dpll_data dd;
	ulong round_rate;
	u32 reg;

	round_rate = am335x_dpll_round_rate(&dd, rate);
	if (IS_ERR_VALUE(round_rate))
		return round_rate;

	dpll_disp.m = dd.rounded_m;
	dpll_disp.n = dd.rounded_n;
	do_setup_dpll(&dpll_disp_regs, &dpll_disp);

	reg = readl(&regs->ctrl) & ~LCDC_CTRL_CLK_DIVISOR_MASK;
	reg |= LCDC_CTRL_CLK_DIVISOR(dd.rounded_div);
	writel(reg, &regs->ctrl);
	return round_rate;
}

int am335xfb_init(struct am335x_lcdpanel *panel)
{
	u32 raster_ctrl = 0;
	struct cm_dpll *const cmdpll = (struct cm_dpll *)CM_DPLL;
	ulong rate;
	u32 reg;

	if (gd->fb_base == 0) {
		printf("ERROR: no valid fb_base stored in GLOBAL_DATA_PTR!\n");
		return -1;
	}
	if (panel == NULL) {
		printf("ERROR: missing ptr to am335x_lcdpanel!\n");
		return -1;
	}

	/* We can already set the bits for the raster_ctrl in this check */
	switch (panel->bpp) {
	case 16:
		break;
	case 32:
		raster_ctrl |= LCDC_RASTER_CTRL_TFT_24BPP_UNPACK;
		/* fallthrough */
	case 24:
		raster_ctrl |= LCDC_RASTER_CTRL_TFT_24BPP_MODE;
		break;
	default:
		pr_err("am335x-fb: invalid bpp value: %d\n", panel->bpp);
		return -1;
	}

	/* check given clock-frequency */
	if (panel->pxl_clk > (LCDC_FMAX / 2)) {
		pr_err("am335x-fb: requested pxl-clk: %d not supported!\n",
		       panel->pxl_clk);
		return -1;
	}

	debug("setting up LCD-Controller for %dx%dx%d (hfp=%d,hbp=%d,hsw=%d / ",
	      panel->hactive, panel->vactive, panel->bpp,
	      panel->hfp, panel->hbp, panel->hsw);
	debug("vfp=%d,vbp=%d,vsw=%d / clk=%d)\n",
	      panel->vfp, panel->vfp, panel->vsw, panel->pxl_clk);
	debug("using frambuffer at 0x%08x with size %d.\n",
	      (unsigned int)gd->fb_base, FBSIZE(panel));

	rate = am335x_fb_set_pixel_clk_rate(lcdhw, panel->pxl_clk);
	if (IS_ERR_VALUE(rate))
		return rate;

	/* clock source for LCDC from dispPLL M2 */
	writel(0x0, &cmdpll->clklcdcpixelclk);

	/* palette default entry */
	memset((void *)gd->fb_base, 0, 0x20);
	*(unsigned int *)gd->fb_base = 0x4000;
	/* point fb behind palette */
	gd->fb_base += 0x20;

	/* turn ON display through powercontrol function if accessible */
	if (panel->panel_power_ctrl != NULL)
		panel->panel_power_ctrl(1);

	debug("am335x-fb: wait for stable power ...\n");
	mdelay(panel->pup_delay);
	lcdhw->clkc_enable = LCDC_CLKC_ENABLE_CORECLKEN |
		LCDC_CLKC_ENABLE_LIDDCLKEN | LCDC_CLKC_ENABLE_DMACLKEN;
	lcdhw->raster_ctrl = 0;

	reg = lcdhw->ctrl & LCDC_CTRL_CLK_DIVISOR_MASK;
	reg |= LCDC_CTRL_RASTER_MODE;
	lcdhw->ctrl = reg;

	lcdhw->lcddma_fb0_base = gd->fb_base;
	lcdhw->lcddma_fb0_ceiling = gd->fb_base + FBSIZE(panel);
	lcdhw->lcddma_fb1_base = gd->fb_base;
	lcdhw->lcddma_fb1_ceiling = gd->fb_base + FBSIZE(panel);
	lcdhw->lcddma_ctrl = LCDC_DMA_CTRL_BURST_SIZE(LCDC_DMA_CTRL_BURST_16);

	lcdhw->raster_timing0 = LCDC_RASTER_TIMING_0_HORLSB(panel->hactive) |
				LCDC_RASTER_TIMING_0_HORMSB(panel->hactive) |
				LCDC_RASTER_TIMING_0_HFPLSB(panel->hfp) |
				LCDC_RASTER_TIMING_0_HBPLSB(panel->hbp) |
				LCDC_RASTER_TIMING_0_HSWLSB(panel->hsw);
	lcdhw->raster_timing1 = LCDC_RASTER_TIMING_1_VBP(panel->vbp) |
				LCDC_RASTER_TIMING_1_VFP(panel->vfp) |
				LCDC_RASTER_TIMING_1_VSW(panel->vsw) |
				LCDC_RASTER_TIMING_1_VERLSB(panel->vactive);
	lcdhw->raster_timing2 = LCDC_RASTER_TIMING_2_HSWMSB(panel->hsw) |
				LCDC_RASTER_TIMING_2_VERMSB(panel->vactive) |
				LCDC_RASTER_TIMING_2_INVMASK(panel->pol) |
				LCDC_RASTER_TIMING_2_HBPMSB(panel->hbp) |
				LCDC_RASTER_TIMING_2_HFPMSB(panel->hfp) |
				0x0000FF00;	/* clk cycles for ac-bias */
	lcdhw->raster_ctrl =	raster_ctrl |
				LCDC_RASTER_CTRL_PALMODE_RAWDATA |
				LCDC_RASTER_CTRL_TFT_MODE |
				LCDC_RASTER_CTRL_ENABLE;

	debug("am335x-fb: waiting picture to be stable.\n.");
	mdelay(panel->pon_delay);

	return 0;
}
