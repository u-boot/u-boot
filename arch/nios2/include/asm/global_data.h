/*
 * (C) Copyright 2004, Psyent Corporation <www.psyent.com>
 * Scott McNutt <smcnutt@psyent.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */
#ifndef	__ASM_NIOS2_GLOBALDATA_H_
#define __ASM_NIOS2_GLOBALDATA_H_

/* Architecture-specific global data */
struct arch_global_data {
};

#include <asm-generic/global_data.h>

#define DECLARE_GLOBAL_DATA_PTR     register gd_t *gd asm ("gp")

#endif /* __ASM_NIOS2_GLOBALDATA_H_ */
