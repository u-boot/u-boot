/*
 * (C) Copyright 2012 Stephen Warren
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <lcd.h>
#include <memalign.h>
#include <phys2bus.h>
#include <asm/arch/mbox.h>
#include <asm/arch/msg.h>
#include <asm/global_data.h>

DECLARE_GLOBAL_DATA_PTR;

/* Global variables that lcd.c expects to exist */
vidinfo_t panel_info;

static int bcm2835_pitch;

void lcd_ctrl_init(void *lcdbase)
{
	int ret;
	int w, h;
	ulong fb_base, fb_size, fb_start, fb_end;

	debug("bcm2835: Query resolution...\n");
	ret = bcm2835_get_video_size(&w, &h);
	if (ret) {
		/* FIXME: How to disable the LCD to prevent errors? hang()? */
		return;
	}

	debug("bcm2835: Setting up display for %d x %d\n", w, h);
	ret = bcm2835_set_video_params(&w, &h, 32, BCM2835_MBOX_PIXEL_ORDER_RGB,
				       BCM2835_MBOX_ALPHA_MODE_IGNORED,
				       &fb_base, &fb_size, &bcm2835_pitch);

	debug("bcm2835: Final resolution is %d x %d\n", w, h);

	panel_info.vl_col = w;
	panel_info.vl_row = h;
	panel_info.vl_bpix = LCD_COLOR32;

	gd->fb_base = fb_base;

	/* Enable dcache for the frame buffer */
	fb_start = fb_base & ~(MMU_SECTION_SIZE - 1);
	fb_end = fb_base + fb_size;
	fb_end = ALIGN(fb_end, 1 << MMU_SECTION_SHIFT);
	mmu_set_region_dcache_behaviour(fb_start, fb_end - fb_start,
					DCACHE_WRITEBACK);
	lcd_set_flush_dcache(1);
}

void lcd_enable(void)
{
}

int lcd_get_size(int *line_length)
{
	*line_length = bcm2835_pitch;

	return *line_length * panel_info.vl_row;
}
