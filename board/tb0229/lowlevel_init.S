/*
 *  Memory sub-system initialization code for TANBAC Evaluation board TB0229.
 *
 *  Copyright (c) 2003  Masami Komiya <mkomiya@sonare.it>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2, or (at
 * your option) any later version.
 */

#include <config.h>
#include <asm/regdef.h>


	.globl  lowlevel_init
lowlevel_init:

	/* BCUCNTREG1 = 0x0040 */
	la	t0, 0xaf000000
	li	t1, 0x0040
	sh	t1, 0(t0)

	/* ROMSIZEREG = 0x3333 */
	la	t0, 0xaf000004
	li	t1, 0x3333
	sh	t1, 0(t0)

	/* ROMSPEEDREG = 0x3003 */
	la	t0, 0xaf000006
	li	t1, 0x3003
	sh	t1, 0(t0)

	/* BCUCNTREG3 = 0 */
	la	t0, 0xaf000016
	li	t1, 0x0000
	sh	t1, 0(t0)

	/* CMUCLKMSK */
	la	t0, 0xaf000060
	li	t1, 0x39a2
	sh	t1, 0(t0)

	/* PMUCNTREG */
	la	t0, 0xaf0000c2
	li	t1, 0x0006
	sh	t1, 0(t0)

	/* SDRAMMODEREG = 0x8029 */
	la	t0, 0xaf000400
	li	t1, 0x8029
	sh	t1, 0(t0)

	/* SDRAMCNTREG = 0x2322 */
	la	t0, 0xaf000402
	li	t1, 0x2322
	sh	t1, 0(t0)

	/* BCURFCNTREG = 0x0106 */
	la	t0, 0xaf000404
	li	t1, 0x0106
	sh	t1, 0(t0)

	/* RAMSZEREG = 0x5555 (64MB Bank) */
	la	t0, 0xaf000408
	li	t1, 0x5555
	sh	t1, 0(t0)

	j       ra
	nop
