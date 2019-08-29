// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2011
 * Graeme Russ, <graeme.russ@gmail.com>
 */

#include <common.h>
#include <linux/errno.h>
#include <asm/mtrr.h>

DECLARE_GLOBAL_DATA_PTR;

int init_cache_f_r(void)
{
#if CONFIG_IS_ENABLED(X86_32BIT_INIT) && !defined(CONFIG_HAVE_FSP) && \
		!defined(CONFIG_SYS_SLIMBOOTLOADER)
	int ret;

	ret = mtrr_commit(false);
	/* If MTRR MSR is not implemented by the processor, just ignore it */
	if (ret && ret != -ENOSYS)
		return ret;
#endif
	/* Initialise the CPU cache(s) */
	return init_cache();
}
