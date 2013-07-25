/*
 * Driver for AT91/AT32 MULTI LAYER LCD Controller
 *
 * Copyright (C) 2012 Atmel Corporation
 *
 * SPDX-License-Identifier:	GPL-2.0+ 
 */

#include <common.h>
#include <asm/io.h>
#include <asm/arch/gpio.h>
#include <asm/arch/clk.h>
#include <lcd.h>
#include <atmel_hlcdc.h>

/* configurable parameters */
#define ATMEL_LCDC_CVAL_DEFAULT		0xc8
#define ATMEL_LCDC_DMA_BURST_LEN	8
#ifndef ATMEL_LCDC_GUARD_TIME
#define ATMEL_LCDC_GUARD_TIME		1
#endif

#define ATMEL_LCDC_FIFO_SIZE		512

#define lcdc_readl(reg)		__raw_readl((reg))
#define lcdc_writel(reg, val)	__raw_writel((val), (reg))

/*
 * the CLUT register map as following
 * RCLUT(24 ~ 16), GCLUT(15 ~ 8), BCLUT(7 ~ 0)
 */
void lcd_setcolreg(ushort regno, ushort red, ushort green, ushort blue)
{
	lcdc_writel(((red << LCDC_BASECLUT_RCLUT_Pos) & LCDC_BASECLUT_RCLUT_Msk)
		| ((green << LCDC_BASECLUT_GCLUT_Pos) & LCDC_BASECLUT_GCLUT_Msk)
		| ((blue << LCDC_BASECLUT_BCLUT_Pos) & LCDC_BASECLUT_BCLUT_Msk),
		panel_info.mmio + ATMEL_LCDC_LUT(regno));
}

void lcd_ctrl_init(void *lcdbase)
{
	unsigned long value;
	struct lcd_dma_desc *desc;
	struct atmel_hlcd_regs *regs;

	if (!has_lcdc())
		return;     /* No lcdc */

	regs = (struct atmel_hlcd_regs *)panel_info.mmio;

	/* Disable DISP signal */
	lcdc_writel(&regs->lcdc_lcddis, LCDC_LCDDIS_DISPDIS);
	while ((lcdc_readl(&regs->lcdc_lcdsr) & LCDC_LCDSR_DISPSTS))
		udelay(1);
	/* Disable synchronization */
	lcdc_writel(&regs->lcdc_lcddis, LCDC_LCDDIS_SYNCDIS);
	while ((lcdc_readl(&regs->lcdc_lcdsr) & LCDC_LCDSR_LCDSTS))
		udelay(1);
	/* Disable pixel clock */
	lcdc_writel(&regs->lcdc_lcddis, LCDC_LCDDIS_CLKDIS);
	while ((lcdc_readl(&regs->lcdc_lcdsr) & LCDC_LCDSR_CLKSTS))
		udelay(1);
	/* Disable PWM */
	lcdc_writel(&regs->lcdc_lcddis, LCDC_LCDDIS_PWMDIS);
	while ((lcdc_readl(&regs->lcdc_lcdsr) & LCDC_LCDSR_PWMSTS))
		udelay(1);

	/* Set pixel clock */
	value = get_lcdc_clk_rate(0) / panel_info.vl_clk;
	if (get_lcdc_clk_rate(0) % panel_info.vl_clk)
		value++;

	if (value < 1) {
		/* Using system clock as pixel clock */
		lcdc_writel(&regs->lcdc_lcdcfg0,
					LCDC_LCDCFG0_CLKDIV(0)
					| LCDC_LCDCFG0_CGDISHCR
					| LCDC_LCDCFG0_CGDISHEO
					| LCDC_LCDCFG0_CGDISOVR1
					| LCDC_LCDCFG0_CGDISBASE
					| panel_info.vl_clk_pol
					| LCDC_LCDCFG0_CLKSEL);

	} else {
		lcdc_writel(&regs->lcdc_lcdcfg0,
				LCDC_LCDCFG0_CLKDIV(value - 2)
				| LCDC_LCDCFG0_CGDISHCR
				| LCDC_LCDCFG0_CGDISHEO
				| LCDC_LCDCFG0_CGDISOVR1
				| LCDC_LCDCFG0_CGDISBASE
				| panel_info.vl_clk_pol);
	}

	/* Initialize control register 5 */
	value = 0;

	value |= panel_info.vl_sync;

#ifndef LCD_OUTPUT_BPP
	/* Output is 24bpp */
	value |= LCDC_LCDCFG5_MODE_OUTPUT_24BPP;
#else
	switch (LCD_OUTPUT_BPP) {
	case 12:
		value |= LCDC_LCDCFG5_MODE_OUTPUT_12BPP;
		break;
	case 16:
		value |= LCDC_LCDCFG5_MODE_OUTPUT_16BPP;
		break;
	case 18:
		value |= LCDC_LCDCFG5_MODE_OUTPUT_18BPP;
		break;
	case 24:
		value |= LCDC_LCDCFG5_MODE_OUTPUT_24BPP;
		break;
	default:
		BUG();
		break;
	}
#endif

	value |= LCDC_LCDCFG5_GUARDTIME(ATMEL_LCDC_GUARD_TIME);
	value |= (LCDC_LCDCFG5_DISPDLY | LCDC_LCDCFG5_VSPDLYS);
	lcdc_writel(&regs->lcdc_lcdcfg5, value);

	/* Vertical & Horizontal Timing */
	value = LCDC_LCDCFG1_VSPW(panel_info.vl_vsync_len - 1);
	value |= LCDC_LCDCFG1_HSPW(panel_info.vl_hsync_len - 1);
	lcdc_writel(&regs->lcdc_lcdcfg1, value);

	value = LCDC_LCDCFG2_VBPW(panel_info.vl_lower_margin);
	value |= LCDC_LCDCFG2_VFPW(panel_info.vl_upper_margin - 1);
	lcdc_writel(&regs->lcdc_lcdcfg2, value);

	value = LCDC_LCDCFG3_HBPW(panel_info.vl_right_margin - 1);
	value |= LCDC_LCDCFG3_HFPW(panel_info.vl_left_margin - 1);
	lcdc_writel(&regs->lcdc_lcdcfg3, value);

	/* Display size */
	value = LCDC_LCDCFG4_RPF(panel_info.vl_row - 1);
	value |= LCDC_LCDCFG4_PPL(panel_info.vl_col - 1);
	lcdc_writel(&regs->lcdc_lcdcfg4, value);

	lcdc_writel(&regs->lcdc_basecfg0,
			LCDC_BASECFG0_BLEN_AHB_INCR4 | LCDC_BASECFG0_DLBO);

	switch (NBITS(panel_info.vl_bpix)) {
	case 16:
		lcdc_writel(&regs->lcdc_basecfg1,
			LCDC_BASECFG1_RGBMODE_16BPP_RGB_565);
		break;
	default:
		BUG();
		break;
	}

	lcdc_writel(&regs->lcdc_basecfg2, LCDC_BASECFG2_XSTRIDE(0));
	lcdc_writel(&regs->lcdc_basecfg3, 0);
	lcdc_writel(&regs->lcdc_basecfg4, LCDC_BASECFG4_DMA);

	/* Disable all interrupts */
	lcdc_writel(&regs->lcdc_lcdidr, ~0UL);
	lcdc_writel(&regs->lcdc_baseidr, ~0UL);

	/* Setup the DMA descriptor, this descriptor will loop to itself */
	desc = (struct lcd_dma_desc *)(lcdbase - 16);

	desc->address = (u32)lcdbase;
	/* Disable DMA transfer interrupt & descriptor loaded interrupt. */
	desc->control = LCDC_BASECTRL_ADDIEN | LCDC_BASECTRL_DSCRIEN
			| LCDC_BASECTRL_DMAIEN | LCDC_BASECTRL_DFETCH;
	desc->next = (u32)desc;

	lcdc_writel(&regs->lcdc_baseaddr, desc->address);
	lcdc_writel(&regs->lcdc_basectrl, desc->control);
	lcdc_writel(&regs->lcdc_basenext, desc->next);
	lcdc_writel(&regs->lcdc_basecher, LCDC_BASECHER_CHEN |
					  LCDC_BASECHER_UPDATEEN);

	/* Enable LCD */
	value = lcdc_readl(&regs->lcdc_lcden);
	lcdc_writel(&regs->lcdc_lcden, value | LCDC_LCDEN_CLKEN);
	while (!(lcdc_readl(&regs->lcdc_lcdsr) & LCDC_LCDSR_CLKSTS))
		udelay(1);
	value = lcdc_readl(&regs->lcdc_lcden);
	lcdc_writel(&regs->lcdc_lcden, value | LCDC_LCDEN_SYNCEN);
	while (!(lcdc_readl(&regs->lcdc_lcdsr) & LCDC_LCDSR_LCDSTS))
		udelay(1);
	value = lcdc_readl(&regs->lcdc_lcden);
	lcdc_writel(&regs->lcdc_lcden, value | LCDC_LCDEN_DISPEN);
	while (!(lcdc_readl(&regs->lcdc_lcdsr) & LCDC_LCDSR_DISPSTS))
		udelay(1);
	value = lcdc_readl(&regs->lcdc_lcden);
	lcdc_writel(&regs->lcdc_lcden, value | LCDC_LCDEN_PWMEN);
	while (!(lcdc_readl(&regs->lcdc_lcdsr) & LCDC_LCDSR_PWMSTS))
		udelay(1);
}
