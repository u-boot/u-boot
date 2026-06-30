/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2018 Intel Corporation. All rights reserved
 * Copyright (C) 2026 Altera Corporation <www.altera.com>
 */

#define SMC_ARG0		0
#define SMC_ARG1		(SMC_ARG0 + 1)
#define SMC_ARG2		(SMC_ARG1 + 1)
#define SMC_ARG3		(SMC_ARG2 + 1)
#define SMC_RETURN_ARGS_MAX	(SMC_ARG3 + 1)

/*
 * Macro functions for allocation and read/write of
 * variables to be assigned to registers
 */
/* Allocate memory for variable */
#define SMC_ALLOC_REG_MEM(var) unsigned long var[SMC_RETURN_ARGS_MAX]
/* Clear variable */
#define SMC_INIT_REG_MEM(var) \
	do { \
		int i; \
		for (i = 0; i < SMC_RETURN_ARGS_MAX; i++) \
			var[i] = 0; \
	} while (0)
/* Read variable */
#define SMC_GET_REG_MEM(var, i) var[i]
/* Write Variable */
#define SMC_ASSIGN_REG_MEM(var, i, val) \
	do { \
		var[i] = (val); \
	} while (0)
/* Assign variables back to registers */
#define SMC_RET_REG_MEM(var) \
	do { \
		asm volatile("ldr x0, %0\n" \
			     "ldr x1, %1\n" \
			     "ldr x2, %2\n" \
			     "ldr x3, %3\n" \
			     : : "m" (var[0]), "m" (var[1]), \
				 "m" (var[2]), "m" (var[3]) : ); \
	} while (0)
