/*
 * Copyright (c) 2013 Google, Inc
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <fdtdec.h>
#include <lcd.h>
#include <malloc.h>
#include <asm/sdl.h>
#include <asm/u-boot-sandbox.h>

DECLARE_GLOBAL_DATA_PTR;

enum {
	/* Maximum LCD size we support */
	LCD_MAX_WIDTH		= 1366,
	LCD_MAX_HEIGHT		= 768,
	LCD_MAX_LOG2_BPP	= 4,		/* 2^4 = 16 bpp */
};

vidinfo_t panel_info;

void lcd_setcolreg(ushort regno, ushort red, ushort green, ushort blue)
{
}

void lcd_ctrl_init(void *lcdbase)
{
	/*
	 * Allocate memory to keep BMP color conversion map. This is required
	 * for 8 bit BMPs only (hence 256 colors). If malloc fails - keep
	 * going, it is not even clear if displyaing the bitmap will be
	 * required on the way up.
	 */
	panel_info.cmap = malloc(256 * NBITS(panel_info.vl_bpix) / 8);
}

void lcd_enable(void)
{
	if (sandbox_sdl_init_display(panel_info.vl_col, panel_info.vl_row,
				     panel_info.vl_bpix))
		puts("LCD init failed\n");
}

int sandbox_lcd_sdl_early_init(void)
{
	const void *blob = gd->fdt_blob;
	int xres = LCD_MAX_WIDTH, yres = LCD_MAX_HEIGHT;
	int node;
	int ret = 0;

	/*
	 * The code in common/lcd.c does not cope with not being able to
	 * set up a frame buffer. It will just happily keep writing to
	 * invalid memory. So here we make sure that at least some buffer
	 * is available even if it actually won't be displayed.
	 */
	node = fdtdec_next_compatible(blob, 0, COMPAT_SANDBOX_LCD_SDL);
	if (node >= 0) {
		xres = fdtdec_get_int(blob, node, "xres", LCD_MAX_WIDTH);
		yres = fdtdec_get_int(blob, node, "yres", LCD_MAX_HEIGHT);
		if (xres < 0 || xres > LCD_MAX_WIDTH) {
			xres = LCD_MAX_WIDTH;
			ret = -EINVAL;
		}
		if (yres < 0 || yres > LCD_MAX_HEIGHT) {
			yres = LCD_MAX_HEIGHT;
			ret = -EINVAL;
		}
	}

	panel_info.vl_col = xres;
	panel_info.vl_row = yres;
	panel_info.vl_bpix = LCD_COLOR16;

	return ret;
}
