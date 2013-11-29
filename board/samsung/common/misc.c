/*
 * Copyright (C) 2013 Samsung Electronics
 * Przemyslaw Marczak <p.marczak@samsung.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <lcd.h>
#include <libtizen.h>
#include <samsung/misc.h>

#ifdef CONFIG_CMD_BMP
void draw_logo(void)
{
	int x, y;
	ulong addr;

	addr = panel_info.logo_addr;
	if (!addr) {
		error("There is no logo data.");
		return;
	}

	if (panel_info.vl_width >= panel_info.logo_width) {
		x = ((panel_info.vl_width - panel_info.logo_width) >> 1);
		x += panel_info.logo_x_offset; /* For X center align */
	} else {
		x = 0;
		printf("Warning: image width is bigger than display width\n");
	}

	if (panel_info.vl_height >= panel_info.logo_height) {
		y = ((panel_info.vl_height - panel_info.logo_height) >> 1);
		y += panel_info.logo_y_offset; /* For Y center align */
	} else {
		y = 0;
		printf("Warning: image height is bigger than display height\n");
	}

	bmp_display(addr, x, y);
}
#endif /* CONFIG_CMD_BMP */
