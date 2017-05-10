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

	.set noreorder

	.macro init_wr sel
	MTC0	zero, CP0_WATCHLO,\sel
	mtc0	t1, CP0_WATCHHI,\sel
	mfc0	t0, CP0_WATCHHI,\sel
	bgez	t0, wr_done
	 nop
	.endm

	.macro uhi_mips_exception
	move	k0, t9		# preserve t9 in k0
	move	k1, a0		# preserve a0 in k1
	li	t9, 15		# UHI exception operation
	li	a0, 0		# Use hard register context
	sdbbp	1		# Invoke UHI operation
	.endm

	.macro setup_stack_gd
	li	t0, -16
	PTR_LI	t1, CONFIG_SYS_INIT_SP_ADDR
	and	sp, t1, t0		# force 16 byte alignment
	PTR_SUBU \
		sp, sp, GD_SIZE		# reserve space for gd
	and	sp, sp, t0		# force 16 byte alignment
	move	k0, sp			# save gd pointer
#ifdef CONFIG_SYS_MALLOC_F_LEN
	li	t2, CONFIG_SYS_MALLOC_F_LEN
	PTR_SUBU \
		sp, sp, t2		# reserve space for early malloc
	and	sp, sp, t0		# force 16 byte alignment
#endif
	move	fp, sp

	/* Clear gd */
	move	t0, k0
1:
	PTR_S	zero, 0(t0)
	blt	t0, t1, 1b
	 PTR_ADDIU t0, PTRSIZE

#ifdef CONFIG_SYS_MALLOC_F_LEN
	PTR_S	sp, GD_MALLOC_BASE(k0)	# gd->malloc_base offset
#endif
	.endm

ENTRY(_start)
	/* U-Boot entry point */
	b	reset
	 mtc0	zero, CP0_COUNT	# clear cp0 count for most accurate boot timing

#if defined(CONFIG_SYS_XWAY_EBU_BOOTCFG)
	/*
	 * Almost all Lantiq XWAY SoC devices have an external bus unit (EBU) to
	 * access external NOR flashes. If the board boots from NOR flash the
	 * internal BootROM does a blind read at address 0xB0000010 to read the
	 * initial configuration for that EBU in order to access the flash
	 * device with correct parameters. This config option is board-specific.
	 */
	.org 0x10
	.word CONFIG_SYS_XWAY_EBU_BOOTCFG
	.word 0x0
#endif
#if defined(CONFIG_MALTA)
	/*
	 * Linux expects the Board ID here.
	 */
	.org 0x10
	.word 0x00000420	# 0x420 (Malta Board with CoreLV)
	.word 0x00000000
#endif

#if defined(CONFIG_ROM_EXCEPTION_VECTORS)
	/*
	 * Exception vector entry points. When running from ROM, an exception
	 * cannot be handled. Halt execution and transfer control to debugger,
	 * if one is attached.
	 */
	.org 0x200
	/* TLB refill, 32 bit task */
	uhi_mips_exception

	.org 0x280
	/* XTLB refill, 64 bit task */
	uhi_mips_exception

	.org 0x300
	/* Cache error exception */
	uhi_mips_exception

	.org 0x380
	/* General exception */
	uhi_mips_exception

	.org 0x400
	/* Catch interrupt exceptions */
	uhi_mips_exception

	.org 0x480
	/* EJTAG debug exception */
1:	b	1b
	 nop

	.org 0x500
#endif

reset:
#if __mips_isa_rev >= 6
	mfc0	t0, CP0_CONFIG, 5
	and	t0, t0, MIPS_CONF5_VP
	beqz	t0, 1f
	 nop

	b	2f
	 mfc0	t0, CP0_GLOBALNUMBER
#endif

#ifdef CONFIG_ARCH_BMIPS
1:	mfc0	t0, CP0_DIAGNOSTIC, 3
	and	t0, t0, (1 << 31)
#else
1:	mfc0	t0, CP0_EBASE
	and	t0, t0, EBASE_CPUNUM
#endif

	/* Hang if this isn't the first CPU in the system */
2:	beqz	t0, 4f
	 nop
3:	wait
	b	3b
	 nop

	/* Init CP0 Status */
4:	mfc0	t0, CP0_STATUS
	and	t0, ST0_IMPL
	or	t0, ST0_BEV | ST0_ERL | STATUS_SET
	mtc0	t0, CP0_STATUS

	/*
	 * Check whether CP0 Config1 is implemented. If not continue
	 * with legacy Watch register initialization.
	 */
	mfc0	t0, CP0_CONFIG
	bgez	t0, wr_legacy
	 nop

	/*
	 * Check WR bit in CP0 Config1 to determine if Watch registers
	 * are implemented.
	 */
	mfc0	t0, CP0_CONFIG, 1
	andi	t0, (1 << 3)
	beqz	t0, wr_done
	 nop

	/* Clear Watch Status bits and disable watch exceptions */
	li	t1, 0x7		# Clear I, R and W conditions
	init_wr	0
	init_wr	1
	init_wr	2
	init_wr	3
	init_wr	4
	init_wr	5
	init_wr	6
	init_wr	7
	b	wr_done
	 nop

wr_legacy:
	MTC0	zero, CP0_WATCHLO
	mtc0	zero, CP0_WATCHHI

wr_done:
	/* Clear WP, IV and SW interrupts */
	mtc0	zero, CP0_CAUSE

	/* Clear timer interrupt (CP0_COUNT cleared on branch to 'reset') */
	mtc0	zero, CP0_COMPARE

#ifndef CONFIG_SKIP_LOWLEVEL_INIT
	mfc0	t0, CP0_CONFIG
	and	t0, t0, MIPS_CONF_IMPL
	or	t0, t0, CONF_CM_UNCACHED
	mtc0	t0, CP0_CONFIG
	ehb
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

#ifdef CONFIG_MIPS_CM
	PTR_LA	t9, mips_cm_map
	jalr	t9
	 nop
#endif

#ifdef CONFIG_MIPS_INIT_STACK_IN_SRAM
	/* Set up initial stack and global data */
	setup_stack_gd

# ifdef CONFIG_DEBUG_UART
	/* Earliest point to set up debug uart */
	PTR_LA	t9, debug_uart_init
	jalr	t9
	 nop
# endif
#endif

#ifndef CONFIG_SKIP_LOWLEVEL_INIT
# ifdef CONFIG_SYS_MIPS_CACHE_INIT_RAM_LOAD
	/* Initialize any external memory */
	PTR_LA	t9, lowlevel_init
	jalr	t9
	 nop
# endif

	/* Initialize caches... */
	PTR_LA	t9, mips_cache_reset
	jalr	t9
	 nop

# ifndef CONFIG_SYS_MIPS_CACHE_INIT_RAM_LOAD
	/* Initialize any external memory */
	PTR_LA	t9, lowlevel_init
	jalr	t9
	 nop
# endif
#endif

#ifndef CONFIG_MIPS_INIT_STACK_IN_SRAM
	/* Set up initial stack and global data */
	setup_stack_gd

# ifdef CONFIG_DEBUG_UART
	/* Earliest point to set up debug uart */
	PTR_LA	t9, debug_uart_init
	jalr	t9
	 nop
# endif
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

	PTR_LA	t2, __image_copy_end
	move	t1, a2

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

	/*
	 * Now we want to update GOT.
	 *
	 * GOT[0] is reserved. GOT[1] is also reserved for the dynamic object
	 * generated by GNU ld. Skip these reserved entries from relocation.
	 */
	PTR_LA	t3, num_got_entries
	PTR_LA	t8, _GLOBAL_OFFSET_TABLE_
	PTR_ADD	t8, s1			# t8 now holds relocated _G_O_T_
	PTR_ADDIU t8, t8, 2 * PTRSIZE	# skipping first two entries
	PTR_LI	t2, 2
1:
	PTR_L	t1, 0(t8)
	beqz	t1, 2f
	 PTR_ADD t1, s1
	PTR_S	t1, 0(t8)
2:
	PTR_ADDIU t2, 1
	blt	t2, t3, 1b
	 PTR_ADDIU t8, PTRSIZE

	/* Update dynamic relocations */
	PTR_LA	t1, __rel_dyn_start
	PTR_LA	t2, __rel_dyn_end

	b	2f			# skip first reserved entry
	 PTR_ADDIU t1, 2 * PTRSIZE

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
	 PTR_ADDIU t1, 2 * PTRSIZE	# each rel.dyn entry is 2*PTRSIZE bytes

	/*
	 * Flush caches to ensure our newly modified instructions are visible
	 * to the instruction cache. We're still running with the old GOT, so
	 * apply the reloc offset to the start address.
	 */
	PTR_LA	a0, __text_start
	PTR_LA	a1, __text_end
	PTR_SUB	a1, a1, a0
	PTR_LA	t9, flush_cache
	jalr	t9
	 PTR_ADD	a0, s1

	PTR_ADD	gp, s1			# adjust gp

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
	 PTR_ADDIU t1, PTRSIZE

	move	a0, s0			# a0 <-- gd
	move	a1, s2
	PTR_LA	t9, board_init_r
	jr	t9
	 move	ra, zero

	END(relocate_code)
