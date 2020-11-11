// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2011
 * Graeme Russ, <graeme.russ@gmail.com>
 */

#include <common.h>
#include <init.h>
#include <linux/errno.h>
#include <asm/mtrr.h>

DECLARE_GLOBAL_DATA_PTR;

int init_cache_f_r(void)
{
	bool do_mtrr = CONFIG_IS_ENABLED(X86_32BIT_INIT) ||
		 IS_ENABLED(CONFIG_FSP_VERSION2);
	int ret;

	do_mtrr &= !IS_ENABLED(CONFIG_FSP_VERSION1) &&
		!IS_ENABLED(CONFIG_SYS_SLIMBOOTLOADER);

	if (do_mtrr) {
		ret = mtrr_commit(false);
		/*
		 * If MTRR MSR is not implemented by the processor, just ignore
		 * it
		 */
		if (ret && ret != -ENOSYS)
			return ret;
	}

	if (!ll_boot_init())
		return 0;

	/* Initialise the CPU cache(s) */
	return init_cache();
}
