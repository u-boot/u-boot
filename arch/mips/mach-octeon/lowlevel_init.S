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

LEAF(mips_mach_early_init)

	move    s0, ra

	bal	__dummy
	 nop

__dummy:
	/* Get the actual address that we are running at */
	PTR_LA	a7, __dummy
	dsubu	t3, ra, a7	/* t3 now has reloc offset */

	PTR_LA	t1, _start
	daddu	t0, t1, t3	/* t0 now has actual address of _start */

	/* Calculate end address of copy loop */
	PTR_LA	t2, _end
	daddiu	t2, t2, 0x4000	/* Increase size to include appended DTB */
	daddiu	t2, t2, 127
	ins	t2, zero, 0, 7	/* Round up to cache line for memcpy */

	/* Copy ourself to the L2 cache from flash, 32 bytes at a time */
1:
	ld	a0, 0(t0)
	ld	a1, 8(t0)
	ld	a2, 16(t0)
	ld	a3, 24(t0)
	sd	a0, 0(t1)
	sd	a1, 8(t1)
	sd	a2, 16(t1)
	sd	a3, 24(t1)
	addiu	t0, 32
	addiu	t1, 32
	bne	t1, t2, 1b
	 nop

	sync

	/*
	 * Return to start.S now running from TEXT_BASE, which points
	 * to DRAM address space, which effectively is L2 cache now.
	 * This speeds up the init process extremely, especially the
	 * DDR init code.
	 */
	dsubu	s0, s0, t3	/* Fixup return address with reloc offset */
	jr.hb	s0		/* Jump back with hazard barrier */
	 nop

	END(mips_mach_early_init)
