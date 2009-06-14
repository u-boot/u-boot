/*
 * Copyright (c) 2009 Wind River Systems, Inc.
 * Tom Rix <Tom.Rix@windriver.com>
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
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 *
 */
#include <common.h>
#include <asm/arch/cpu.h>
#include <asm/io.h>
#include <asm/arch/mux.h>
#include <asm/arch/gpio.h>

#define DEBUG_BOARD_CONNECTED		1
#define DEBUG_BOARD_NOT_CONNECTED	0

static int debug_board_connected = DEBUG_BOARD_CONNECTED;

static void zoom2_debug_board_detect (void)
{
	int val = 0;

	if (!omap_request_gpio(158)) {
		/*
		 * GPIO to query for debug board
		 * 158 db board query
		 */
		omap_set_gpio_direction(158, 1);
		val = omap_get_gpio_datain(158);
		omap_free_gpio(158);
	}

	if (!val)
		debug_board_connected = DEBUG_BOARD_NOT_CONNECTED;
}

int zoom2_debug_board_connected (void)
{
	static int first_time = 1;

	if (first_time) {
		zoom2_debug_board_detect ();
		first_time = 0;
	}
	return debug_board_connected;
}
