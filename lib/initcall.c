// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2013 The Chromium OS Authors.
 */

#include <common.h>
#include <efi.h>
#include <initcall.h>
#include <log.h>
#include <asm/global_data.h>

DECLARE_GLOBAL_DATA_PTR;

static ulong calc_reloc_ofs(void)
{
#ifdef CONFIG_EFI_APP
	return (ulong)image_base;
#endif
	/*
	 * Sandbox is relocated by the OS, so symbols always appear at
	 * the relocated address.
	 */
	if (IS_ENABLED(CONFIG_SANDBOX) || (gd->flags & GD_FLG_RELOC))
		return gd->reloc_off;

	return 0;
}
/*
 * To enable debugging. add #define DEBUG at the top of the including file.
 *
 * To find a symbol, use grep on u-boot.map
 */
int initcall_run_list(const init_fnc_t init_sequence[])
{
	ulong reloc_ofs = calc_reloc_ofs();
	const init_fnc_t *init_fnc_ptr;

	for (init_fnc_ptr = init_sequence; *init_fnc_ptr; ++init_fnc_ptr) {
		int ret;

		if (reloc_ofs)
			debug("initcall: %p (relocated to %p)\n",
			      (char *)*init_fnc_ptr - reloc_ofs,
			      (char *)*init_fnc_ptr);
		else
			debug("initcall: %p\n", (char *)*init_fnc_ptr - reloc_ofs);

		ret = (*init_fnc_ptr)();
		if (ret) {
			printf("initcall sequence %p failed at call %p (err=%d)\n",
			       init_sequence,
			       (char *)*init_fnc_ptr - reloc_ofs, ret);
			return -1;
		}
	}
	return 0;
}
