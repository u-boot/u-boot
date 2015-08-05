/*
 * Copyright 2014 Google Inc.
 *
 * SPDX-License-Identifier:     GPL-2.0+
 *
 */

#include <common.h>
#include <errno.h>
#include <fdtdec.h>
#include <lcd.h>
#include <asm/gpio.h>
#include <asm/arch-tegra/clk_rst.h>
#include <asm/arch/clock.h>
#include <asm/arch-tegra/dc.h>
#include <asm/io.h>

DECLARE_GLOBAL_DATA_PTR;

enum {
	/* Maximum LCD size we support */
	LCD_MAX_WIDTH		= 1920,
	LCD_MAX_HEIGHT		= 1200,
	LCD_MAX_LOG2_BPP	= 4,		/* 2^4 = 16 bpp */
};

vidinfo_t panel_info = {
	/* Insert a value here so that we don't end up in the BSS */
	.vl_col = -1,
};

int tegra_lcd_check_next_stage(const void *blob, int wait)
{
	return 0;
}

void tegra_lcd_early_init(const void *blob)
{
	/*
	 * Go with the maximum size for now. We will fix this up after
	 * relocation. These values are only used for memory alocation.
	 */
	panel_info.vl_col = LCD_MAX_WIDTH;
	panel_info.vl_row = LCD_MAX_HEIGHT;
	panel_info.vl_bpix = LCD_MAX_LOG2_BPP;
}

static int tegra124_lcd_init(void *lcdbase)
{
	struct display_timing timing;
	int ret;

	clock_set_up_plldp();
	clock_start_periph_pll(PERIPH_ID_HOST1X, CLOCK_ID_PERIPH, 408000000);

	clock_enable(PERIPH_ID_HOST1X);
	clock_enable(PERIPH_ID_DISP1);
	clock_enable(PERIPH_ID_PWM);
	clock_enable(PERIPH_ID_DPAUX);
	clock_enable(PERIPH_ID_SOR0);
	udelay(2);

	reset_set_enable(PERIPH_ID_HOST1X, 0);
	reset_set_enable(PERIPH_ID_DISP1, 0);
	reset_set_enable(PERIPH_ID_PWM, 0);
	reset_set_enable(PERIPH_ID_DPAUX, 0);
	reset_set_enable(PERIPH_ID_SOR0, 0);

	ret = display_init(lcdbase, 1 << LCD_BPP, &timing);
	if (ret)
		return ret;

	panel_info.vl_col = roundup(timing.hactive.typ, 16);
	panel_info.vl_row = timing.vactive.typ;

	lcd_set_flush_dcache(1);

	return 0;
}

void lcd_ctrl_init(void *lcdbase)
{
	ulong start;
	int ret;

	start = get_timer(0);
	ret = tegra124_lcd_init(lcdbase);
	debug("LCD init took %lu ms\n", get_timer(start));
	if (ret)
		printf("%s: Error %d\n", __func__, ret);
}

void lcd_enable(void)
{
}
