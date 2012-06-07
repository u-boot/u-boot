/*
 * (C) Copyright 2012 Samsung Electronics
 * Donghwa Lee <dh09.lee@samsung.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * aint with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 *
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
