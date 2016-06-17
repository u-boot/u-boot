/*
 *  Startup Code for MIPS32 CPU-core
 *
 *  Copyright (c) 2003	Wolfgang Denk <wd@denx.de>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <asm-offsets.h>
#include <config.h>
#include <asm/asm.h>
#include <asm/regdef.h>
#include <asm/mipsregs.h>

#ifndef CONFIG_SYS_MIPS_CACHE_MODE
#define CONFIG_SYS_MIPS_CACHE_MODE CONF_CM_CACHABLE_NONCOHERENT
#endif

#ifndef CONFIG_SYS_INIT_SP_ADDR
#define CONFIG_SYS_INIT_SP_ADDR	(CONFIG_SYS_SDRAM_BASE + \
				CONFIG_SYS_INIT_SP_OFFSET)
#endif

#ifdef CONFIG_32BIT
# define MIPS_RELOC	3
# define STATUS_SET	0
#endif

#ifdef CONFIG_64BIT
# ifdef CONFIG_SYS_LITTLE_ENDIAN
#  define MIPS64_R_INFO(ssym, r_type3, r_type2, r_type) \
	(((r_type) << 24) | ((r_type2) << 16) | ((r_type3) << 8) | (ssym))
# else
#  define MIPS64_R_INFO(ssym, r_type3, r_type2, r_type) \
	((r_type) | ((r_type2) << 8) | ((r_type3) << 16) | (ssym) << 24)
# endif
# define MIPS_RELOC	MIPS64_R_INFO(0x00, 0x00, 0x12, 0x03)
# define STATUS_SET	ST0_KX
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

	.set noreorder

ENTRY(_start)
	/* U-Boot entry point */
	b	reset
	 nop

	.org 0x10
#if defined(CONFIG_SYS_XWAY_EBU_BOOTCFG)
	/*
	 * Almost all Lantiq XWAY SoC devices have an external bus unit (EBU) to
	 * access external NOR flashes. If the board boots from NOR flash the
	 * internal BootROM does a blind read at address 0xB0000010 to read the
	 * initial configuration for that EBU in order to access the flash
	 * device with correct parameters. This config option is board-specific.
	 */
	.word CONFIG_SYS_XWAY_EBU_BOOTCFG
	.word 0x0
#elif defined(CONFIG_MALTA)
	/*
	 * Linux expects the Board ID here.
	 */
	.word 0x00000420	# 0x420 (Malta Board with CoreLV)
	.word 0x00000000
#endif

	.org 0x200
	/* TLB refill, 32 bit task */
1:	b	1b
	 nop

	.org 0x280
	/* XTLB refill, 64 bit task */
1:	b	1b
	 nop

	.org 0x300
	/* Cache error exception */
1:	b	1b
	 nop

	.org 0x380
	/* General exception */
1:	b	1b
	 nop

	.org 0x400
	/* Catch interrupt exceptions */
1:	b	1b
	 nop

	.org 0x480
	/* EJTAG debug exception */
1:	b	1b
	 nop

	.align 4
reset:

	/* Clear watch registers */
	MTC0	zero, CP0_WATCHLO
	mtc0	zero, CP0_WATCHHI

	/* WP(Watch Pending), SW0/1 should be cleared */
	mtc0	zero, CP0_CAUSE

	setup_c0_status STATUS_SET 0

	/* Init Timer */
	mtc0	zero, CP0_COUNT
	mtc0	zero, CP0_COMPARE

#ifndef CONFIG_SKIP_LOWLEVEL_INIT
	/* CONFIG0 register */
	li	t0, CONF_CM_UNCACHED
	mtc0	t0, CP0_CONFIG
#endif

	/*
	 * Initialize $gp, force pointer sized alignment of bal instruction to
	 * forbid the compiler to put nop's between bal and _gp. This is
	 * required to keep _gp and ra aligned to 8 byte.
	 */
	.align	PTRLOG
	bal	1f
	 nop
	PTR	_gp
1:
	PTR_L	gp, 0(ra)

#ifndef CONFIG_SKIP_LOWLEVEL_INIT
	/* Initialize any external memory */
	PTR_LA	t9, lowlevel_init
	jalr	t9
	 nop

	/* Initialize caches... */
	PTR_LA	t9, mips_cache_reset
	jalr	t9
	 nop

	/* ... and enable them */
	li	t0, CONFIG_SYS_MIPS_CACHE_MODE
	mtc0	t0, CP0_CONFIG
#endif

	/* Set up temporary stack */
	li	t0, -16
	PTR_LI	t1, CONFIG_SYS_INIT_SP_ADDR
	and	sp, t1, t0		# force 16 byte alignment
	PTR_SUB	sp, sp, GD_SIZE		# reserve space for gd
	and	sp, sp, t0		# force 16 byte alignment
	move	k0, sp			# save gd pointer
#ifdef CONFIG_SYS_MALLOC_F_LEN
	li	t2, CONFIG_SYS_MALLOC_F_LEN
	PTR_SUB	sp, sp, t2		# reserve space for early malloc
	and	sp, sp, t0		# force 16 byte alignment
#endif
	move	fp, sp

	/* Clear gd */
	move	t0, k0
1:
	PTR_S	zero, 0(t0)
	blt	t0, t1, 1b
	 PTR_ADDI t0, PTRSIZE

#ifdef CONFIG_SYS_MALLOC_F_LEN
	PTR_S	sp, GD_MALLOC_BASE(k0)	# gd->malloc_base offset
#endif

	move	a0, zero		# a0 <-- boot_flags = 0
	PTR_LA	t9, board_init_f
	jr	t9
	 move	ra, zero

	END(_start)

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
ENTRY(relocate_code)
	move	sp, a0			# set new stack pointer
	move	fp, sp

	move	s0, a1			# save gd in s0
	move	s2, a2			# save destination address in s2

	PTR_LI	t0, CONFIG_SYS_MONITOR_BASE
	PTR_SUB	s1, s2, t0		# s1 <-- relocation offset

	PTR_LA	t3, in_ram
	PTR_L	t2, -(3 * PTRSIZE)(t3)	# t2 <-- __image_copy_end
	move	t1, a2

	PTR_ADD	gp, s1			# adjust gp

	/*
	 * t0 = source address
	 * t1 = target address
	 * t2 = source end address
	 */
1:
	PTR_L	t3, 0(t0)
	PTR_S	t3, 0(t1)
	PTR_ADDU t0, PTRSIZE
	blt	t0, t2, 1b
	 PTR_ADDU t1, PTRSIZE

	/* If caches were enabled, we would have to flush them here. */
	PTR_SUB	a1, t1, s2		# a1 <-- size
	PTR_LA	t9, flush_cache
	jalr	t9
	 move	a0, s2			# a0 <-- destination address

	/* Jump to where we've relocated ourselves */
	PTR_ADDI t0, s2, in_ram - _start
	jr	t0
	 nop

	PTR	__rel_dyn_end
	PTR	__rel_dyn_start
	PTR	__image_copy_end
	PTR	_GLOBAL_OFFSET_TABLE_
	PTR	num_got_entries

in_ram:
	/*
	 * Now we want to update GOT.
	 *
	 * GOT[0] is reserved. GOT[1] is also reserved for the dynamic object
	 * generated by GNU ld. Skip these reserved entries from relocation.
	 */
	PTR_L	t3, -(1 * PTRSIZE)(t0)	# t3 <-- num_got_entries
	PTR_L	t8, -(2 * PTRSIZE)(t0)	# t8 <-- _GLOBAL_OFFSET_TABLE_
	PTR_ADD	t8, s1			# t8 now holds relocated _G_O_T_
	PTR_ADDI t8, t8, 2 * PTRSIZE	# skipping first two entries
	PTR_LI	t2, 2
1:
	PTR_L	t1, 0(t8)
	beqz	t1, 2f
	 PTR_ADD t1, s1
	PTR_S	t1, 0(t8)
2:
	PTR_ADDI t2, 1
	blt	t2, t3, 1b
	 PTR_ADDI t8, PTRSIZE

	/* Update dynamic relocations */
	PTR_L	t1, -(4 * PTRSIZE)(t0)	# t1 <-- __rel_dyn_start
	PTR_L	t2, -(5 * PTRSIZE)(t0)	# t2 <-- __rel_dyn_end

	b	2f			# skip first reserved entry
	 PTR_ADDI t1, 2 * PTRSIZE

1:
	lw	t8, -4(t1)		# t8 <-- relocation info

	PTR_LI	t3, MIPS_RELOC
	bne	t8, t3, 2f		# skip non-MIPS_RELOC entries
	 nop

	PTR_L	t3, -(2 * PTRSIZE)(t1)	# t3 <-- location to fix up in FLASH

	PTR_L	t8, 0(t3)		# t8 <-- original pointer
	PTR_ADD	t8, s1			# t8 <-- adjusted pointer

	PTR_ADD	t3, s1			# t3 <-- location to fix up in RAM
	PTR_S	t8, 0(t3)

2:
	blt	t1, t2, 1b
	 PTR_ADDI t1, 2 * PTRSIZE	# each rel.dyn entry is 2*PTRSIZE bytes

	/*
	 * Clear BSS
	 *
	 * GOT is now relocated. Thus __bss_start and __bss_end can be
	 * accessed directly via $gp.
	 */
	PTR_LA	t1, __bss_start		# t1 <-- __bss_start
	PTR_LA	t2, __bss_end		# t2 <-- __bss_end

1:
	PTR_S	zero, 0(t1)
	blt	t1, t2, 1b
	 PTR_ADDI t1, PTRSIZE

	move	a0, s0			# a0 <-- gd
	move	a1, s2
	PTR_LA	t9, board_init_r
	jr	t9
	 move	ra, zero

	END(relocate_code)
