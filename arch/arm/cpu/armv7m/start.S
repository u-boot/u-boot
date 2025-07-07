/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2015
 * Kamil Lulko, <kamil.lulko@gmail.com>
 */

#include <linux/linkage.h>
#include <asm/assembler.h>

/*
 * Startup code (reset vector)
 */
ENTRY(reset)
	W(b)	_main	@ Jump to _main (C runtime crt0.S)
ENDPROC(reset)

/*
 * Setup CPU for C runtime
 */
ENTRY(c_runtime_cpu_setup)
	mov pc, lr			@ Jump back to caller
ENDPROC(c_runtime_cpu_setup)
