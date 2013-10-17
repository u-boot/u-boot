/*
 * Copyright (c) 2011 The Chromium OS Authors.
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <fdtdec.h>
#include <lcd.h>

#include <asm/system.h>
#include <asm/gpio.h>

#include <asm/arch/clock.h>
#include <asm/arch/funcmux.h>
#include <asm/arch/pinmux.h>
#include <asm/arch/pwm.h>
#include <asm/arch/display.h>
#include <asm/arch-tegra/timer.h>

DECLARE_GLOBAL_DATA_PTR;

/* These are the stages we go throuh in enabling the LCD */
enum stage_t {
	STAGE_START,
	STAGE_PANEL_VDD,
	STAGE_LVDS,
	STAGE_BACKLIGHT_VDD,
	STAGE_PWM,
	STAGE_BACKLIGHT_EN,
	STAGE_DONE,
};

static enum stage_t stage;	/* Current stage we are at */
static unsigned long timer_next; /* Time we can move onto next stage */

/* Our LCD config, set up in handle_stage() */
static struct fdt_panel_config config;
struct fdt_disp_config *disp_config;	/* Display controller config */

enum {
	/* Maximum LCD size we support */
	LCD_MAX_WIDTH		= 1366,
	LCD_MAX_HEIGHT		= 768,
	LCD_MAX_LOG2_BPP	= 4,		/* 2^4 = 16 bpp */
};

vidinfo_t panel_info = {
	/* Insert a value here so that we don't end up in the BSS */
	.vl_col = -1,
};

#ifndef CONFIG_OF_CONTROL
#error "You must enable CONFIG_OF_CONTROL to get Tegra LCD support"
#endif

static void update_panel_size(struct fdt_disp_config *config)
{
	panel_info.vl_col = config->width;
	panel_info.vl_row = config->height;
	panel_info.vl_bpix = config->log2_bpp;
}

/*
 *  Main init function called by lcd driver.
 *  Inits and then prints test pattern if required.
 */

void lcd_ctrl_init(void *lcdbase)
{
	int type = DCACHE_OFF;
	int size;

	assert(disp_config);

	/* Make sure that we can acommodate the selected LCD */
	assert(disp_config->width <= LCD_MAX_WIDTH);
	assert(disp_config->height <= LCD_MAX_HEIGHT);
	assert(disp_config->log2_bpp <= LCD_MAX_LOG2_BPP);
	if (disp_config->width <= LCD_MAX_WIDTH
			&& disp_config->height <= LCD_MAX_HEIGHT
			&& disp_config->log2_bpp <= LCD_MAX_LOG2_BPP)
		update_panel_size(disp_config);
	size = lcd_get_size(&lcd_line_length);

	/* Set up the LCD caching as requested */
	if (config.cache_type & FDT_LCD_CACHE_WRITE_THROUGH)
		type = DCACHE_WRITETHROUGH;
	else if (config.cache_type & FDT_LCD_CACHE_WRITE_BACK)
		type = DCACHE_WRITEBACK;
	mmu_set_region_dcache_behaviour(disp_config->frame_buffer, size, type);

	/* Enable flushing after LCD writes if requested */
	lcd_set_flush_dcache(config.cache_type & FDT_LCD_CACHE_FLUSH);

	debug("LCD frame buffer at %08X\n", disp_config->frame_buffer);
}

ulong calc_fbsize(void)
{
	return (panel_info.vl_col * panel_info.vl_row *
		NBITS(panel_info.vl_bpix)) / 8;
}

void lcd_setcolreg(ushort regno, ushort red, ushort green, ushort blue)
{
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

/**
 * Decode the panel information from the fdt.
 *
 * @param blob		fdt blob
 * @param config	structure to store fdt config into
 * @return 0 if ok, -ve on error
 */
static int fdt_decode_lcd(const void *blob, struct fdt_panel_config *config)
{
	int display_node;

	disp_config = tegra_display_get_config();
	if (!disp_config) {
		debug("%s: Display controller is not configured\n", __func__);
		return -1;
	}
	display_node = disp_config->panel_node;
	if (display_node < 0) {
		debug("%s: No panel configuration available\n", __func__);
		return -1;
	}

	config->pwm_channel = pwm_request(blob, display_node, "nvidia,pwm");
	if (config->pwm_channel < 0) {
		debug("%s: Unable to request PWM channel\n", __func__);
		return -1;
	}

	config->cache_type = fdtdec_get_int(blob, display_node,
					    "nvidia,cache-type",
					    FDT_LCD_CACHE_WRITE_BACK_FLUSH);

	/* These GPIOs are all optional */
	fdtdec_decode_gpio(blob, display_node, "nvidia,backlight-enable-gpios",
			    &config->backlight_en);
	fdtdec_decode_gpio(blob, display_node, "nvidia,lvds-shutdown-gpios",
			   &config->lvds_shutdown);
	fdtdec_decode_gpio(blob, display_node, "nvidia,backlight-vdd-gpios",
			   &config->backlight_vdd);
	fdtdec_decode_gpio(blob, display_node, "nvidia,panel-vdd-gpios",
			   &config->panel_vdd);

	return fdtdec_get_int_array(blob, display_node, "nvidia,panel-timings",
			config->panel_timings, FDT_LCD_TIMINGS);
}

/**
 * Handle the next stage of device init
 */
static int handle_stage(const void *blob)
{
	debug("%s: stage %d\n", __func__, stage);

	/* do the things for this stage */
	switch (stage) {
	case STAGE_START:
		/* Initialize the Tegra display controller */
		if (tegra_display_probe(gd->fdt_blob, (void *)gd->fb_base)) {
			printf("%s: Failed to probe display driver\n",
			__func__);
			return -1;
		}

		/* get panel details */
		if (fdt_decode_lcd(blob, &config)) {
			printf("No valid LCD information in device tree\n");
			return -1;
		}

		/*
		 * It is possible that the FDT has requested that the LCD be
		 * disabled. We currently don't support this. It would require
		 * changes to U-Boot LCD subsystem to have LCD support
		 * compiled in but not used. An easier option might be to
		 * still have a frame buffer, but leave the backlight off and
		 * remove all mention of lcd in the stdout environment
		 * variable.
		 */

		funcmux_select(PERIPH_ID_DISP1, FUNCMUX_DEFAULT);

		fdtdec_setup_gpio(&config.panel_vdd);
		fdtdec_setup_gpio(&config.lvds_shutdown);
		fdtdec_setup_gpio(&config.backlight_vdd);
		fdtdec_setup_gpio(&config.backlight_en);

		/*
		 * TODO: If fdt includes output flag we can omit this code
		 * since fdtdec_setup_gpio will do it for us.
		 */
		if (fdt_gpio_isvalid(&config.panel_vdd))
			gpio_direction_output(config.panel_vdd.gpio, 0);
		if (fdt_gpio_isvalid(&config.lvds_shutdown))
			gpio_direction_output(config.lvds_shutdown.gpio, 0);
		if (fdt_gpio_isvalid(&config.backlight_vdd))
			gpio_direction_output(config.backlight_vdd.gpio, 0);
		if (fdt_gpio_isvalid(&config.backlight_en))
			gpio_direction_output(config.backlight_en.gpio, 0);
		break;
	case STAGE_PANEL_VDD:
		if (fdt_gpio_isvalid(&config.panel_vdd))
			gpio_direction_output(config.panel_vdd.gpio, 1);
		break;
	case STAGE_LVDS:
		if (fdt_gpio_isvalid(&config.lvds_shutdown))
			gpio_set_value(config.lvds_shutdown.gpio, 1);
		break;
	case STAGE_BACKLIGHT_VDD:
		if (fdt_gpio_isvalid(&config.backlight_vdd))
			gpio_set_value(config.backlight_vdd.gpio, 1);
		break;
	case STAGE_PWM:
		/* Enable PWM at 15/16 high, 32768 Hz with divider 1 */
		pinmux_set_func(PINGRP_GPU, PMUX_FUNC_PWM);
		pinmux_tristate_disable(PINGRP_GPU);

		pwm_enable(config.pwm_channel, 32768, 0xdf, 1);
		break;
	case STAGE_BACKLIGHT_EN:
		if (fdt_gpio_isvalid(&config.backlight_en))
			gpio_set_value(config.backlight_en.gpio, 1);
		break;
	case STAGE_DONE:
		break;
	}

	/* set up timer for next stage */
	timer_next = timer_get_us();
	if (stage < FDT_LCD_TIMINGS)
		timer_next += config.panel_timings[stage] * 1000;

	/* move to next stage */
	stage++;
	return 0;
}

int tegra_lcd_check_next_stage(const void *blob, int wait)
{
	if (stage == STAGE_DONE)
		return 0;

	do {
		/* wait if we need to */
		debug("%s: stage %d\n", __func__, stage);
		if (stage != STAGE_START) {
			int delay = timer_next - timer_get_us();

			if (delay > 0) {
				if (wait)
					udelay(delay);
				else
					return 0;
			}
		}

		if (handle_stage(blob))
			return -1;
	} while (wait && stage != STAGE_DONE);
	if (stage == STAGE_DONE)
		debug("%s: LCD init complete\n", __func__);

	return 0;
}

void lcd_enable(void)
{
	/*
	 * Backlight and power init will be done separately in
	 * tegra_lcd_check_next_stage(), which should be called in
	 * board_late_init().
	 *
	 * U-Boot code supports only colour depth, selected at compile time.
	 * The device tree setting should match this. Otherwise the display
	 * will not look right, and U-Boot may crash.
	 */
	if (disp_config->log2_bpp != LCD_BPP) {
		printf("%s: Error: LCD depth configured in FDT (%d = %dbpp)"
			" must match setting of LCD_BPP (%d)\n", __func__,
		       disp_config->log2_bpp, disp_config->bpp, LCD_BPP);
	}
}
