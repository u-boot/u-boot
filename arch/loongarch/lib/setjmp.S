/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2024 Jiaxun Yang <jiaxun.yang@flygoat.com>
 */

#include <config.h>
#include <asm/asm.h>
#include <linux/linkage.h>

.pushsection .text.setjmp, "ax"
ENTRY(setjmp)
	LONG_S		s0, a0, 0
	LONG_S 		s1, a0, (1 * LONGSIZE)
	LONG_S		s2, a0, (2 * LONGSIZE)
	LONG_S		s3, a0, (3 * LONGSIZE)
	LONG_S		s4, a0, (4 * LONGSIZE)
	LONG_S		s5, a0, (5 * LONGSIZE)
	LONG_S		s6, a0, (6 * LONGSIZE)
	LONG_S		s7, a0, (7 * LONGSIZE)
	LONG_S		s8, a0, (8 * LONGSIZE)
	LONG_S		fp, a0, (9 * LONGSIZE)
	LONG_S		sp, a0, (10 * LONGSIZE)
	LONG_S		ra, a0, (11 * LONGSIZE)

	move		a0, zero
	ret
ENDPROC(setjmp)
.popsection

.pushsection .text.longjmp, "ax"
ENTRY(longjmp)
	LONG_L		s0, a0, 0
	LONG_L 		s1, a0, (1 * LONGSIZE)
	LONG_L		s2, a0, (2 * LONGSIZE)
	LONG_L		s3, a0, (3 * LONGSIZE)
	LONG_L		s4, a0, (4 * LONGSIZE)
	LONG_L		s5, a0, (5 * LONGSIZE)
	LONG_L		s6, a0, (6 * LONGSIZE)
	LONG_L		s7, a0, (7 * LONGSIZE)
	LONG_L		s8, a0, (8 * LONGSIZE)
	LONG_L		fp, a0, (9 * LONGSIZE)
	LONG_L		sp, a0, (10 * LONGSIZE)
	LONG_L		ra, a0, (11 * LONGSIZE)

	/* Move the return value in place, but return 1 if passed 0. */
	li.w		a0, 1
	beqz		a1, 1f
	move		a0, a1
1:
	jr		ra
ENDPROC(longjmp)
.popsection
