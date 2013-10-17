/*
 * Driver for AT91/AT32 LCD Controller
 *
 * Copyright (C) 2007 Atmel Corporation
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <asm/io.h>
#include <asm/arch/gpio.h>
#include <asm/arch/clk.h>
#include <lcd.h>
#include <atmel_lcdc.h>

/* configurable parameters */
#define ATMEL_LCDC_CVAL_DEFAULT		0xc8
#define ATMEL_LCDC_DMA_BURST_LEN	8
#ifndef ATMEL_LCDC_GUARD_TIME
#define ATMEL_LCDC_GUARD_TIME		1
#endif

#if defined(CONFIG_AT91SAM9263) || defined(CONFIG_AT91CAP9)
#define ATMEL_LCDC_FIFO_SIZE		2048
#else
#define ATMEL_LCDC_FIFO_SIZE		512
#endif

#define lcdc_readl(mmio, reg)		__raw_readl((mmio)+(reg))
#define lcdc_writel(mmio, reg, val)	__raw_writel((val), (mmio)+(reg))

void lcd_setcolreg(ushort regno, ushort red, ushort green, ushort blue)
{
#if defined(CONFIG_ATMEL_LCD_BGR555)
	lcdc_writel(panel_info.mmio, ATMEL_LCDC_LUT(regno),
		    (red >> 3) | ((green & 0xf8) << 2) | ((blue & 0xf8) << 7));
#else
	lcdc_writel(panel_info.mmio, ATMEL_LCDC_LUT(regno),
		    (blue >> 3) | ((green & 0xfc) << 3) | ((red & 0xf8) << 8));
#endif
}

void lcd_ctrl_init(void *lcdbase)
{
	unsigned long value;

	/* Turn off the LCD controller and the DMA controller */
	lcdc_writel(panel_info.mmio, ATMEL_LCDC_PWRCON,
		    ATMEL_LCDC_GUARD_TIME << ATMEL_LCDC_GUARDT_OFFSET);

	/* Wait for the LCDC core to become idle */
	while (lcdc_readl(panel_info.mmio, ATMEL_LCDC_PWRCON) & ATMEL_LCDC_BUSY)
		udelay(10);

	lcdc_writel(panel_info.mmio, ATMEL_LCDC_DMACON, 0);

	/* Reset LCDC DMA */
	lcdc_writel(panel_info.mmio, ATMEL_LCDC_DMACON, ATMEL_LCDC_DMARST);

	/* ...set frame size and burst length = 8 words (?) */
	value = (panel_info.vl_col * panel_info.vl_row *
		 NBITS(panel_info.vl_bpix)) / 32;
	value |= ((ATMEL_LCDC_DMA_BURST_LEN - 1) << ATMEL_LCDC_BLENGTH_OFFSET);
	lcdc_writel(panel_info.mmio, ATMEL_LCDC_DMAFRMCFG, value);

	/* Set pixel clock */
	value = get_lcdc_clk_rate(0) / panel_info.vl_clk;
	if (get_lcdc_clk_rate(0) % panel_info.vl_clk)
		value++;
	value = (value / 2) - 1;

	if (!value) {
		lcdc_writel(panel_info.mmio, ATMEL_LCDC_LCDCON1, ATMEL_LCDC_BYPASS);
	} else
		lcdc_writel(panel_info.mmio, ATMEL_LCDC_LCDCON1,
			    value << ATMEL_LCDC_CLKVAL_OFFSET);

	/* Initialize control register 2 */
#ifdef CONFIG_AVR32
	value = ATMEL_LCDC_MEMOR_BIG | ATMEL_LCDC_CLKMOD_ALWAYSACTIVE;
#else
	value = ATMEL_LCDC_MEMOR_LITTLE | ATMEL_LCDC_CLKMOD_ALWAYSACTIVE;
#endif
	if (panel_info.vl_tft)
		value |= ATMEL_LCDC_DISTYPE_TFT;

	value |= panel_info.vl_sync;
	value |= (panel_info.vl_bpix << 5);
	lcdc_writel(panel_info.mmio, ATMEL_LCDC_LCDCON2, value);

	/* Vertical timing */
	value = (panel_info.vl_vsync_len - 1) << ATMEL_LCDC_VPW_OFFSET;
	value |= panel_info.vl_upper_margin << ATMEL_LCDC_VBP_OFFSET;
	value |= panel_info.vl_lower_margin;
	lcdc_writel(panel_info.mmio, ATMEL_LCDC_TIM1, value);

	/* Horizontal timing */
	value = (panel_info.vl_right_margin - 1) << ATMEL_LCDC_HFP_OFFSET;
	value |= (panel_info.vl_hsync_len - 1) << ATMEL_LCDC_HPW_OFFSET;
	value |= (panel_info.vl_left_margin - 1);
	lcdc_writel(panel_info.mmio, ATMEL_LCDC_TIM2, value);

	/* Display size */
	value = (panel_info.vl_col - 1) << ATMEL_LCDC_HOZVAL_OFFSET;
	value |= panel_info.vl_row - 1;
	lcdc_writel(panel_info.mmio, ATMEL_LCDC_LCDFRMCFG, value);

	/* FIFO Threshold: Use formula from data sheet */
	value = ATMEL_LCDC_FIFO_SIZE - (2 * ATMEL_LCDC_DMA_BURST_LEN + 3);
	lcdc_writel(panel_info.mmio, ATMEL_LCDC_FIFO, value);

	/* Toggle LCD_MODE every frame */
	lcdc_writel(panel_info.mmio, ATMEL_LCDC_MVAL, 0);

	/* Disable all interrupts */
	lcdc_writel(panel_info.mmio, ATMEL_LCDC_IDR, ~0UL);

	/* Set contrast */
	value = ATMEL_LCDC_PS_DIV8 |
		ATMEL_LCDC_ENA_PWMENABLE;
	if (!panel_info.vl_cont_pol_low)
		value |= ATMEL_LCDC_POL_POSITIVE;
	lcdc_writel(panel_info.mmio, ATMEL_LCDC_CONTRAST_CTR, value);
	lcdc_writel(panel_info.mmio, ATMEL_LCDC_CONTRAST_VAL, ATMEL_LCDC_CVAL_DEFAULT);

	/* Set framebuffer DMA base address and pixel offset */
	lcdc_writel(panel_info.mmio, ATMEL_LCDC_DMABADDR1, (u_long)lcdbase);

	lcdc_writel(panel_info.mmio, ATMEL_LCDC_DMACON, ATMEL_LCDC_DMAEN);
	lcdc_writel(panel_info.mmio, ATMEL_LCDC_PWRCON,
		    (ATMEL_LCDC_GUARD_TIME << ATMEL_LCDC_GUARDT_OFFSET) | ATMEL_LCDC_PWR);
}

ulong calc_fbsize(void)
{
	return ((panel_info.vl_col * panel_info.vl_row *
		NBITS(panel_info.vl_bpix)) / 8) + PAGE_SIZE;
}
