/*
 * (C) Copyright 2015
 * Kamil Lulko, <rev13@wp.pl>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

.globl	reset
.type reset, %function
reset:
	b	_main

.globl	c_runtime_cpu_setup
c_runtime_cpu_setup:
	mov	pc, lr
