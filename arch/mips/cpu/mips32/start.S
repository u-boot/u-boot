/*
 *  Startup Code for MIPS32 CPU-core
 *
 *  Copyright (c) 2003	Wolfgang Denk <wd@denx.de>
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <asm-offsets.h>
#include <config.h>
#include <asm/regdef.h>
#include <asm/mipsregs.h>

#ifndef CONFIG_SYS_MIPS_CACHE_MODE
#define CONFIG_SYS_MIPS_CACHE_MODE CONF_CM_CACHABLE_NONCOHERENT
#endif

	/*
	 * For the moment disable interrupts, mark the kernel mode and
	 * set ST0_KX so that the CPU does not spit fire when using
	 * 64-bit addresses.
	 */
	.macro	setup_c0_status set clr
	.set	push
	mfc0	t0, CP0_STATUS
	or	t0, ST0_CU0 | \set | 0x1f | \clr
	xor	t0, 0x1f | \clr
	mtc0	t0, CP0_STATUS
	.set	noreorder
	sll	zero, 3				# ehb
	.set	pop
	.endm

	.macro	setup_c0_status_reset
#ifdef CONFIG_64BIT
	setup_c0_status ST0_KX 0
#else
	setup_c0_status 0 0
#endif
	.endm

#define RVECENT(f,n) \
   b f; nop
#define XVECENT(f,bev) \
   b f     ;           \
   li k0,bev

	.set noreorder

	.globl _start
	.text
_start:
	RVECENT(reset,0)			# U-boot entry point
	RVECENT(reset,1)			# software reboot
#ifdef CONFIG_SYS_XWAY_EBU_BOOTCFG
	/*
	 * Almost all Lantiq XWAY SoC devices have an external bus unit (EBU) to
	 * access external NOR flashes. If the board boots from NOR flash the
	 * internal BootROM does a blind read at address 0xB0000010 to read the
	 * initial configuration for that EBU in order to access the flash
	 * device with correct parameters. This config option is board-specific.
	 */
	.word CONFIG_SYS_XWAY_EBU_BOOTCFG
	.word 0x00000000
#else
	RVECENT(romReserved,2)
#endif
	RVECENT(romReserved,3)
	RVECENT(romReserved,4)
	RVECENT(romReserved,5)
	RVECENT(romReserved,6)
	RVECENT(romReserved,7)
	RVECENT(romReserved,8)
	RVECENT(romReserved,9)
	RVECENT(romReserved,10)
	RVECENT(romReserved,11)
	RVECENT(romReserved,12)
	RVECENT(romReserved,13)
	RVECENT(romReserved,14)
	RVECENT(romReserved,15)
	RVECENT(romReserved,16)
	RVECENT(romReserved,17)
	RVECENT(romReserved,18)
	RVECENT(romReserved,19)
	RVECENT(romReserved,20)
	RVECENT(romReserved,21)
	RVECENT(romReserved,22)
	RVECENT(romReserved,23)
	RVECENT(romReserved,24)
	RVECENT(romReserved,25)
	RVECENT(romReserved,26)
	RVECENT(romReserved,27)
	RVECENT(romReserved,28)
	RVECENT(romReserved,29)
	RVECENT(romReserved,30)
	RVECENT(romReserved,31)
	RVECENT(romReserved,32)
	RVECENT(romReserved,33)
	RVECENT(romReserved,34)
	RVECENT(romReserved,35)
	RVECENT(romReserved,36)
	RVECENT(romReserved,37)
	RVECENT(romReserved,38)
	RVECENT(romReserved,39)
	RVECENT(romReserved,40)
	RVECENT(romReserved,41)
	RVECENT(romReserved,42)
	RVECENT(romReserved,43)
	RVECENT(romReserved,44)
	RVECENT(romReserved,45)
	RVECENT(romReserved,46)
	RVECENT(romReserved,47)
	RVECENT(romReserved,48)
	RVECENT(romReserved,49)
	RVECENT(romReserved,50)
	RVECENT(romReserved,51)
	RVECENT(romReserved,52)
	RVECENT(romReserved,53)
	RVECENT(romReserved,54)
	RVECENT(romReserved,55)
	RVECENT(romReserved,56)
	RVECENT(romReserved,57)
	RVECENT(romReserved,58)
	RVECENT(romReserved,59)
	RVECENT(romReserved,60)
	RVECENT(romReserved,61)
	RVECENT(romReserved,62)
	RVECENT(romReserved,63)
	XVECENT(romExcHandle,0x200)	# bfc00200: R4000 tlbmiss vector
	RVECENT(romReserved,65)
	RVECENT(romReserved,66)
	RVECENT(romReserved,67)
	RVECENT(romReserved,68)
	RVECENT(romReserved,69)
	RVECENT(romReserved,70)
	RVECENT(romReserved,71)
	RVECENT(romReserved,72)
	RVECENT(romReserved,73)
	RVECENT(romReserved,74)
	RVECENT(romReserved,75)
	RVECENT(romReserved,76)
	RVECENT(romReserved,77)
	RVECENT(romReserved,78)
	RVECENT(romReserved,79)
	XVECENT(romExcHandle,0x280)	# bfc00280: R4000 xtlbmiss vector
	RVECENT(romReserved,81)
	RVECENT(romReserved,82)
	RVECENT(romReserved,83)
	RVECENT(romReserved,84)
	RVECENT(romReserved,85)
	RVECENT(romReserved,86)
	RVECENT(romReserved,87)
	RVECENT(romReserved,88)
	RVECENT(romReserved,89)
	RVECENT(romReserved,90)
	RVECENT(romReserved,91)
	RVECENT(romReserved,92)
	RVECENT(romReserved,93)
	RVECENT(romReserved,94)
	RVECENT(romReserved,95)
	XVECENT(romExcHandle,0x300)	# bfc00300: R4000 cache vector
	RVECENT(romReserved,97)
	RVECENT(romReserved,98)
	RVECENT(romReserved,99)
	RVECENT(romReserved,100)
	RVECENT(romReserved,101)
	RVECENT(romReserved,102)
	RVECENT(romReserved,103)
	RVECENT(romReserved,104)
	RVECENT(romReserved,105)
	RVECENT(romReserved,106)
	RVECENT(romReserved,107)
	RVECENT(romReserved,108)
	RVECENT(romReserved,109)
	RVECENT(romReserved,110)
	RVECENT(romReserved,111)
	XVECENT(romExcHandle,0x380)	# bfc00380: R4000 general vector
	RVECENT(romReserved,113)
	RVECENT(romReserved,114)
	RVECENT(romReserved,115)
	RVECENT(romReserved,116)
	RVECENT(romReserved,116)
	RVECENT(romReserved,118)
	RVECENT(romReserved,119)
	RVECENT(romReserved,120)
	RVECENT(romReserved,121)
	RVECENT(romReserved,122)
	RVECENT(romReserved,123)
	RVECENT(romReserved,124)
	RVECENT(romReserved,125)
	RVECENT(romReserved,126)
	RVECENT(romReserved,127)

	/*
	 * We hope there are no more reserved vectors!
	 * 128 * 8 == 1024 == 0x400
	 * so this is address R_VEC+0x400 == 0xbfc00400
	 */
	.align 4
reset:

	/* Clear watch registers */
	mtc0	zero, CP0_WATCHLO
	mtc0	zero, CP0_WATCHHI

	/* WP(Watch Pending), SW0/1 should be cleared */
	mtc0	zero, CP0_CAUSE

	setup_c0_status_reset

	/* Init Timer */
	mtc0	zero, CP0_COUNT
	mtc0	zero, CP0_COMPARE

#ifndef CONFIG_SKIP_LOWLEVEL_INIT
	/* CONFIG0 register */
	li	t0, CONF_CM_UNCACHED
	mtc0	t0, CP0_CONFIG
#endif

	/* Initialize $gp */
	bal	1f
	 nop
	.word	_gp
1:
	lw	gp, 0(ra)

#ifndef CONFIG_SKIP_LOWLEVEL_INIT
	/* Initialize any external memory */
	la	t9, lowlevel_init
	jalr	t9
	 nop

	/* Initialize caches... */
	la	t9, mips_cache_reset
	jalr	t9
	 nop

	/* ... and enable them */
	li	t0, CONFIG_SYS_MIPS_CACHE_MODE
	mtc0	t0, CP0_CONFIG
#endif

	/* Set up temporary stack */
	li	t0, CONFIG_SYS_SDRAM_BASE + CONFIG_SYS_INIT_SP_OFFSET
	la	sp, 0(t0)

	la	t9, board_init_f
	jr	t9
	 nop

/*
 * void relocate_code (addr_sp, gd, addr_moni)
 *
 * This "function" does not return, instead it continues in RAM
 * after relocating the monitor code.
 *
 * a0 = addr_sp
 * a1 = gd
 * a2 = destination address
 */
	.globl	relocate_code
	.ent	relocate_code
relocate_code:
	move	sp, a0			# set new stack pointer

	li	t0, CONFIG_SYS_MONITOR_BASE
	la	t3, in_ram
	lw	t2, -12(t3)		# t2 <-- uboot_end_data
	move	t1, a2
	move	s2, a2			# s2 <-- destination address

	/*
	 * Fix $gp:
	 *
	 * New $gp = (Old $gp - CONFIG_SYS_MONITOR_BASE) + Destination Address
	 */
	move	t6, gp
	sub	gp, CONFIG_SYS_MONITOR_BASE
	add	gp, a2			# gp now adjusted
	sub	s1, gp, t6		# s1 <-- relocation offset

	/*
	 * t0 = source address
	 * t1 = target address
	 * t2 = source end address
	 */

	/*
	 * Save destination address and size for later usage in flush_cache()
	 */
	move	s0, a1			# save gd in s0
	move	a0, t1			# a0 <-- destination addr
	sub	a1, t2, t0		# a1 <-- size

1:
	lw	t3, 0(t0)
	sw	t3, 0(t1)
	addu	t0, 4
	ble	t0, t2, 1b
	 addu	t1, 4

	/* If caches were enabled, we would have to flush them here. */

	/* a0 & a1 are already set up for flush_cache(start, size) */
	la	t9, flush_cache
	jalr	t9
	 nop

	/* Jump to where we've relocated ourselves */
	addi	t0, s2, in_ram - _start
	jr	t0
	 nop

	.word	_gp
	.word	_GLOBAL_OFFSET_TABLE_
	.word	uboot_end_data
	.word	uboot_end
	.word	num_got_entries

in_ram:
	/*
	 * Now we want to update GOT.
	 *
	 * GOT[0] is reserved. GOT[1] is also reserved for the dynamic object
	 * generated by GNU ld. Skip these reserved entries from relocation.
	 */
	lw	t3, -4(t0)		# t3 <-- num_got_entries
	lw	t4, -16(t0)		# t4 <-- _GLOBAL_OFFSET_TABLE_
	lw	t5, -20(t0)		# t5 <-- _gp
	sub	t4, t5			# compute offset
	add	t4, t4, gp		# t4 now holds relocated _G_O_T_
	addi	t4, t4, 8		# skipping first two entries
	li	t2, 2
1:
	lw	t1, 0(t4)
	beqz	t1, 2f
	 add	t1, s1
	sw	t1, 0(t4)
2:
	addi	t2, 1
	blt	t2, t3, 1b
	 addi	t4, 4

	/* Clear BSS */
	lw	t1, -12(t0)		# t1 <-- uboot_end_data
	lw	t2, -8(t0)		# t2 <-- uboot_end
	add	t1, s1			# adjust pointers
	add	t2, s1

	sub	t1, 4
1:
	addi	t1, 4
	bltl	t1, t2, 1b
	 sw	zero, 0(t1)

	move	a0, s0			# a0 <-- gd
	la	t9, board_init_r
	jr	t9
	 move	a1, s2

	.end	relocate_code

	/* Exception handlers */
romReserved:
	b	romReserved

romExcHandle:
	b	romExcHandle
