/*
 * Copyright 2013 Freescale Semiconductor, Inc.
 *
 * SPDX-License-Identifier:    GPL-2.0+
 */

#ifndef _ASM_FSL_ERRATA_H
#define _ASM_FSL_ERRATA_H

#include <common.h>
#include <asm/processor.h>

#ifdef CONFIG_SYS_FSL_ERRATUM_A006379
static inline bool has_erratum_a006379(void)
{
	u32 svr = get_svr();
	if (((SVR_SOC_VER(svr) == SVR_T4240) && SVR_MAJ(svr) <= 1) ||
	    ((SVR_SOC_VER(svr) == SVR_B4860) && SVR_MAJ(svr) <= 2))
		return true;

	return false;
}
#endif

#endif
