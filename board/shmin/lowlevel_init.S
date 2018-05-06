/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2008, 2010 Nobuhiro Iwamatsu <iwamatsu@nigauri.org>
 */

#include <config.h>

#include <asm/processor.h>
#include <asm/macro.h>


	.global	lowlevel_init

	.text
	.align	2

lowlevel_init:
	/* Use setting of original bootloader */
	rts
	nop
	.align 2
