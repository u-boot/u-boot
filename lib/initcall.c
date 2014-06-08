/*
 * Copyright (c) 2013 The Chromium OS Authors.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <initcall.h>

DECLARE_GLOBAL_DATA_PTR;

int initcall_run_list(const init_fnc_t init_sequence[])
{
	const init_fnc_t *init_fnc_ptr;

	for (init_fnc_ptr = init_sequence; *init_fnc_ptr; ++init_fnc_ptr) {
		unsigned long reloc_ofs = 0;

		if (gd->flags & GD_FLG_RELOC)
			reloc_ofs = gd->reloc_off;
		debug("initcall: %p\n", (char *)*init_fnc_ptr - reloc_ofs);
		if ((*init_fnc_ptr)()) {
			printf("initcall sequence %p failed at call %p\n",
			       init_sequence,
			       (char *)*init_fnc_ptr - reloc_ofs);
			return -1;
		}
	}
	return 0;
}
