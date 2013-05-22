/*
 * Copyright (C) 2013 Gabor Juhos <juhosg@openwrt.org>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 as published
 * by the Free Software Foundation.
 */

#include <asm/regdef.h>

	.text
	.set noreorder
	.set mips32

	.globl	lowlevel_init
lowlevel_init:

	jr	ra
	 nop
