/*
 * -*- mode:c -*-
 *
 * (C) Copyright 2000
 * Marius Groeger <mgroeger@sysgo.de>
 * Sysgo Real-Time Solutions, GmbH <www.elinos.com>
 *
 * void ull_write(unsigned long long volatile *address,
 *                unsigned long long volatile *data)
 * r3 = address
 * r4 = data
 *
 * void ull_read(unsigned long long volatile *address,
 *                unsigned long long volatile *data)
 * r3 = address
 * r4 = data
 *
 * Uses the floating point unit to read and write 64 bit wide
 * data (unsigned long long) on the 60x bus. This is necessary
 * because all 4 flash chips use the /WE line from byte lane 0
 *
 * IMPORTANT: data should always be 8-aligned, otherwise an exception will
 * occur.
 */

#include <ppc_asm.tmpl>
#include <ppc_defs.h>

	.globl	ull_write
ull_write:
	lfd		0,0(r4)
	stfd	0,0(r3)
	blr

	.globl	ull_read
ull_read:
	lfd		0, 0(r3)
	stfd	0, 0(r4)
	blr
