/*
 * Code shared between SPL and U-Boot proper
 *
 * Copyright (c) 2015 Google, Inc
 * Written by Simon Glass <sjg@chromium.org>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>

DECLARE_GLOBAL_DATA_PTR;

/* Unfortunately x86 can't compile this code as gd cannot be assigned */
#ifndef CONFIG_X86
__weak void arch_setup_gd(struct global_data *gd_ptr)
{
	gd = gd_ptr;
}
#endif /* !CONFIG_X86 */

ulong board_init_f_mem(ulong top)
{
	struct global_data *gd_ptr;

	/* Leave space for the stack we are running with now */
	top -= 0x40;

	top -= sizeof(struct global_data);
	top = ALIGN(top, 16);
	gd_ptr = (struct global_data *)top;
	memset(gd_ptr, '\0', sizeof(*gd));
	arch_setup_gd(gd_ptr);

#if defined(CONFIG_SYS_MALLOC_F)
	top -= CONFIG_SYS_MALLOC_F_LEN;
	gd->malloc_base = top;
#endif

	return top;
}
