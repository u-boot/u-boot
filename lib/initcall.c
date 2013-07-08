/*
 * Copyright (c) 2013 The Chromium OS Authors.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <initcall.h>

int initcall_run_list(init_fnc_t init_sequence[])
{
	init_fnc_t *init_fnc_ptr;

	for (init_fnc_ptr = init_sequence; *init_fnc_ptr; ++init_fnc_ptr) {
		debug("initcall: %p\n", *init_fnc_ptr);
		if ((*init_fnc_ptr)()) {
			debug("initcall sequence %p failed at call %p\n",
			      init_sequence, *init_fnc_ptr);
			return -1;
		}
	}
	return 0;
}
