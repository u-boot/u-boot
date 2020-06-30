/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2020 Stefan Roese <sr@denx.de>
 */

#include <config.h>
#include <asm-offsets.h>
#include <asm/cacheops.h>
#include <asm/regdef.h>
#include <asm/mipsregs.h>
#include <asm/addrspace.h>
#include <asm/asm.h>

	.set noreorder

LEAF(lowlevel_init)
	jr	ra
	 nop
	END(lowlevel_init)
