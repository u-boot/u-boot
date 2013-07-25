/*
 * Copyright (C) 2004-2006 Atmel Corporation
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */
#ifndef __ASM_GLOBAL_DATA_H__
#define __ASM_GLOBAL_DATA_H__

/* Architecture-specific global data */
struct arch_global_data {
	unsigned long stack_end;	/* highest stack address */
	unsigned long cpu_hz;		/* cpu core clock frequency */
};

#include <asm-generic/global_data.h>

#define DECLARE_GLOBAL_DATA_PTR register gd_t *gd asm("r5")

#endif /* __ASM_GLOBAL_DATA_H__ */
