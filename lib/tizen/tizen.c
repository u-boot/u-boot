/*
 * (C) Copyright 2012 Samsung Electronics
 * Donghwa Lee <dh09.lee@samsung.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <lcd.h>
#include <libtizen.h>

#include "tizen_hd_logo.h"
#include "tizen_hd_logo_data.h"

void get_tizen_logo_info(vidinfo_t *vid)
{
	switch (vid->resolution) {
	case HD_RESOLUTION:
		vid->logo_width = TIZEN_HD_LOGO_WIDTH;
		vid->logo_height = TIZEN_HD_LOGO_HEIGHT;
		vid->logo_addr = (ulong)tizen_hd_logo;
		break;
	default:
		break;
	}
}
