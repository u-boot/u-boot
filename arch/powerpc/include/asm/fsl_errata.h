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
	    ((SVR_SOC_VER(svr) == SVR_T4160) && SVR_MAJ(svr) <= 1) ||
	    ((SVR_SOC_VER(svr) == SVR_T4080) && SVR_MAJ(svr) <= 1) ||
	    ((SVR_SOC_VER(svr) == SVR_B4860) && SVR_MAJ(svr) <= 2) ||
	    ((SVR_SOC_VER(svr) == SVR_B4420) && SVR_MAJ(svr) <= 2) ||
	    ((SVR_SOC_VER(svr) == SVR_T2080) && SVR_MAJ(svr) <= 1) ||
	    ((SVR_SOC_VER(svr) == SVR_T2081) && SVR_MAJ(svr) <= 1))
		return true;

	return false;
}
#endif

#ifdef CONFIG_SYS_FSL_ERRATUM_A006261
static inline bool has_erratum_a006261(void)
{
	u32 svr = get_svr();
	u32 soc = SVR_SOC_VER(svr);

	switch (soc) {
	case SVR_P1010:
		return IS_SVR_REV(svr, 1, 0) || IS_SVR_REV(svr, 2, 0);
	case SVR_P2041:
	case SVR_P2040:
		return IS_SVR_REV(svr, 1, 0) ||
			IS_SVR_REV(svr, 1, 1) || IS_SVR_REV(svr, 2, 1);
	case SVR_P3041:
		return IS_SVR_REV(svr, 1, 0) ||
			IS_SVR_REV(svr, 1, 1) ||
			IS_SVR_REV(svr, 2, 0) || IS_SVR_REV(svr, 2, 1);
	case SVR_P5010:
	case SVR_P5020:
	case SVR_P5021:
		return IS_SVR_REV(svr, 1, 0) || IS_SVR_REV(svr, 2, 0);
	case SVR_T4240:
	case SVR_T4160:
	case SVR_T4080:
		return IS_SVR_REV(svr, 1, 0) || IS_SVR_REV(svr, 2, 0);
	case SVR_T1040:
		return IS_SVR_REV(svr, 1, 0);
	case SVR_T2080:
	case SVR_T2081:
		return IS_SVR_REV(svr, 1, 0);
	case SVR_P5040:
		return IS_SVR_REV(svr, 1, 0);
	}

	return false;
}
#endif

static inline bool has_erratum_a007075(void)
{
	u32 svr = get_svr();
	u32 soc = SVR_SOC_VER(svr);

	switch (soc) {
	case SVR_B4860:
	case SVR_B4420:
		return IS_SVR_REV(svr, 1, 0) || IS_SVR_REV(svr, 2, 0);
	case SVR_P1010:
		return IS_SVR_REV(svr, 1, 0);
	case SVR_P4080:
		return IS_SVR_REV(svr, 2, 0) || IS_SVR_REV(svr, 3, 0);
	}
	return false;
}
#endif
