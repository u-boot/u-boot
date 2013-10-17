/*
 * Copyright (c) 2009 Wind River Systems, Inc.
 * Tom Rix <Tom.Rix@windriver.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */
#include <common.h>
#include <asm/arch/cpu.h>
#include <asm/io.h>
#include <asm/arch/mux.h>
#include <asm/gpio.h>

#define DEBUG_BOARD_CONNECTED		1
#define DEBUG_BOARD_NOT_CONNECTED	0

static int debug_board_connected = DEBUG_BOARD_CONNECTED;

static void zoom2_debug_board_detect (void)
{
	int val = 0;

	if (!gpio_request(158, "")) {
		/*
		 * GPIO to query for debug board
		 * 158 db board query
		 */
		gpio_direction_input(158);
		val = gpio_get_value(158);
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
