/*
 *  Copyright (C) 1998	Dan Malek <dmalek@jlc.net>
 *  Copyright (C) 1999	Magnus Damm <kieraypc01.p.y.kie.era.ericsson.se>
 *  Copyright (C) 2000,2001,2002 Wolfgang Denk <wd@denx.de>
 *  Copyright (C) 2007 Stefan Roese <sr@denx.de>, DENX Software Engineering
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */
/*------------------------------------------------------------------------------+
 *
 *	 This source code has been made available to you by IBM on an AS-IS
 *	 basis.	 Anyone receiving this source is licensed under IBM
 *	 copyrights to use it in any way he or she deems fit, including
 *	 copying it, modifying it, compiling it, and redistributing it either
 *	 with or without modifications.	 No license under IBM patents or
 *	 patent applications is to be implied by the copyright license.
 *
 *	 Any user of this software should understand that IBM cannot provide
 *	 technical support for this software and will not be responsible for
 *	 any consequences resulting from the use of this software.
 *
 *	 Any person who transfers this source code or any derivative work
 *	 must include the IBM copyright notice, this paragraph, and the
 *	 preceding two paragraphs in the transferred software.
 *
 *	 COPYRIGHT   I B M   CORPORATION 1995
 *	 LICENSED MATERIAL  -  PROGRAM PROPERTY OF I B M
 *-------------------------------------------------------------------------------
 */

/*  U-Boot - Startup Code for AMCC 4xx PowerPC based Embedded Boards
 *
 *
 *  The processor starts at 0xfffffffc and the code is executed
 *  from flash/rom.
 *  in memory, but as long we don't jump around before relocating.
 *  board_init lies at a quite high address and when the cpu has
 *  jumped there, everything is ok.
 *  This works because the cpu gives the FLASH (CS0) the whole
 *  address space at startup, and board_init lies as a echo of
 *  the flash somewhere up there in the memorymap.
 *
 *  board_init will change CS0 to be positioned at the correct
 *  address and (s)dram will be positioned at address 0
 */
#include <config.h>
#include <ppc4xx.h>
#include <version.h>

#define _LINUX_CONFIG_H 1	/* avoid reading Linux autoconf.h file	*/

#include <ppc_asm.tmpl>
#include <ppc_defs.h>

#include <asm/cache.h>
#include <asm/mmu.h>

#ifndef	 CONFIG_IDENT_STRING
#define	 CONFIG_IDENT_STRING ""
#endif

#ifdef CFG_INIT_DCACHE_CS
# if (CFG_INIT_DCACHE_CS == 0)
#  define PBxAP pb0ap
#  define PBxCR pb0cr
# endif
# if (CFG_INIT_DCACHE_CS == 1)
#  define PBxAP pb1ap
#  define PBxCR pb1cr
# endif
# if (CFG_INIT_DCACHE_CS == 2)
#  define PBxAP pb2ap
#  define PBxCR pb2cr
# endif
# if (CFG_INIT_DCACHE_CS == 3)
#  define PBxAP pb3ap
#  define PBxCR pb3cr
# endif
# if (CFG_INIT_DCACHE_CS == 4)
#  define PBxAP pb4ap
#  define PBxCR pb4cr
# endif
# if (CFG_INIT_DCACHE_CS == 5)
#  define PBxAP pb5ap
#  define PBxCR pb5cr
# endif
# if (CFG_INIT_DCACHE_CS == 6)
#  define PBxAP pb6ap
#  define PBxCR pb6cr
# endif
# if (CFG_INIT_DCACHE_CS == 7)
#  define PBxAP pb7ap
#  define PBxCR pb7cr
# endif
#endif /* CFG_INIT_DCACHE_CS */

#if (defined(CFG_INIT_RAM_DCACHE) && (CFG_INIT_RAM_END > (4 << 10)))
#error Only 4k of init-ram is supported - please adjust CFG_INIT_RAM_END!
#endif

#define function_prolog(func_name)	.text; \
					.align 2; \
					.globl func_name; \
					func_name:
#define function_epilog(func_name)	.type func_name,@function; \
					.size func_name,.-func_name

/* We don't want the  MMU yet.
*/
#undef	MSR_KERNEL
#define MSR_KERNEL ( MSR_ME  )	/* Machine Check */


	.extern ext_bus_cntlr_init
	.extern sdram_init
#ifdef CONFIG_NAND_U_BOOT
	.extern reconfig_tlb0
#endif

/*
 * Set up GOT: Global Offset Table
 *
 * Use r14 to access the GOT
 */
#if !defined(CONFIG_NAND_SPL)
	START_GOT
	GOT_ENTRY(_GOT2_TABLE_)
	GOT_ENTRY(_FIXUP_TABLE_)

	GOT_ENTRY(_start)
	GOT_ENTRY(_start_of_vectors)
	GOT_ENTRY(_end_of_vectors)
	GOT_ENTRY(transfer_to_handler)

	GOT_ENTRY(__init_end)
	GOT_ENTRY(_end)
	GOT_ENTRY(__bss_start)
	END_GOT
#endif /* CONFIG_NAND_SPL */

#if defined(CONFIG_NAND_U_BOOT) && !defined(CONFIG_NAND_SPL)
	/*
	 * NAND U-Boot image is started from offset 0
	 */
	.text
#if defined(CONFIG_440)
	bl	reconfig_tlb0
#endif
	GET_GOT
	bl	cpu_init_f	/* run low-level CPU init code	   (from Flash) */
	bl	board_init_f
#endif

/*
 * 440 Startup -- on reset only the top 4k of the effective
 * address space is mapped in by an entry in the instruction
 * and data shadow TLB. The .bootpg section is located in the
 * top 4k & does only what's necessary to map in the the rest
 * of the boot rom. Once the boot rom is mapped in we can
 * proceed with normal startup.
 *
 * NOTE: CS0 only covers the top 2MB of the effective address
 * space after reset.
 */

#if defined(CONFIG_440)
#if !defined(CONFIG_NAND_SPL)
    .section .bootpg,"ax"
#endif
    .globl _start_440

/**************************************************************************/
_start_440:
	/*--------------------------------------------------------------------+
	| 440EPX BUP Change - Hardware team request
	+--------------------------------------------------------------------*/
#if defined(CONFIG_440EPX) || defined(CONFIG_440GRX)
	sync
	nop
	nop
#endif
	/*----------------------------------------------------------------+
	| Core bug fix.  Clear the esr
	+-----------------------------------------------------------------*/
	li	r0,0
	mtspr	esr,r0
	/*----------------------------------------------------------------*/
	/* Clear and set up some registers. */
	/*----------------------------------------------------------------*/
	iccci	r0,r0		/* NOTE: operands not used for 440 */
	dccci	r0,r0		/* NOTE: operands not used for 440 */
	sync
	li	r0,0
	mtspr	srr0,r0
	mtspr	srr1,r0
	mtspr	csrr0,r0
	mtspr	csrr1,r0
	/* NOTE: 440GX adds machine check status regs */
#if defined(CONFIG_440) && !defined(CONFIG_440GP)
	mtspr	mcsrr0,r0
	mtspr	mcsrr1,r0
	mfspr	r1,mcsr
	mtspr	mcsr,r1
#endif

	/*----------------------------------------------------------------*/
	/* CCR0 init */
	/*----------------------------------------------------------------*/
	/* Disable store gathering & broadcast, guarantee inst/data
	* cache block touch, force load/store alignment
	* (see errata 1.12: 440_33)
	*/
	lis	r1,0x0030	/* store gathering & broadcast disable */
	ori	r1,r1,0x6000	/* cache touch */
	mtspr	ccr0,r1

	/*----------------------------------------------------------------*/
	/* Initialize debug */
	/*----------------------------------------------------------------*/
	mfspr	r1,dbcr0
	andis.	r1, r1, 0x8000	/* test DBCR0[EDM] bit			*/
	bne	skip_debug_init	/* if set, don't clear debug register	*/
	mtspr	dbcr0,r0
	mtspr	dbcr1,r0
	mtspr	dbcr2,r0
	mtspr	iac1,r0
	mtspr	iac2,r0
	mtspr	iac3,r0
	mtspr	dac1,r0
	mtspr	dac2,r0
	mtspr	dvc1,r0
	mtspr	dvc2,r0

	mfspr	r1,dbsr
	mtspr	dbsr,r1		/* Clear all valid bits */
skip_debug_init:

#if defined (CONFIG_440SPE)
	/*----------------------------------------------------------------+
	| Initialize Core Configuration Reg1.
	| a. ICDPEI: Record even parity. Normal operation.
	| b. ICTPEI: Record even parity. Normal operation.
	| c. DCTPEI: Record even parity. Normal operation.
	| d. DCDPEI: Record even parity. Normal operation.
	| e. DCUPEI: Record even parity. Normal operation.
	| f. DCMPEI: Record even parity. Normal operation.
	| g. FCOM:   Normal operation
	| h. MMUPEI: Record even parity. Normal operation.
	| i. FFF:    Flush only as much data as necessary.
	| j. TCS:    Timebase increments from CPU clock.
	+-----------------------------------------------------------------*/
	li	r0,0
	mtspr	ccr1, r0

	/*----------------------------------------------------------------+
	| Reset the timebase.
	| The previous write to CCR1 sets the timebase source.
	+-----------------------------------------------------------------*/
	mtspr	tbl, r0
	mtspr	tbu, r0
#endif

	/*----------------------------------------------------------------*/
	/* Setup interrupt vectors */
	/*----------------------------------------------------------------*/
	mtspr	ivpr,r0		/* Vectors start at 0x0000_0000 */
	li	r1,0x0100
	mtspr	ivor0,r1	/* Critical input */
	li	r1,0x0200
	mtspr	ivor1,r1	/* Machine check */
	li	r1,0x0300
	mtspr	ivor2,r1	/* Data storage */
	li	r1,0x0400
	mtspr	ivor3,r1	/* Instruction storage */
	li	r1,0x0500
	mtspr	ivor4,r1	/* External interrupt */
	li	r1,0x0600
	mtspr	ivor5,r1	/* Alignment */
	li	r1,0x0700
	mtspr	ivor6,r1	/* Program check */
	li	r1,0x0800
	mtspr	ivor7,r1	/* Floating point unavailable */
	li	r1,0x0c00
	mtspr	ivor8,r1	/* System call */
	li	r1,0x0a00
	mtspr	ivor9,r1	/* Auxiliary Processor unavailable */
	li	r1,0x0900
	mtspr	ivor10,r1	/* Decrementer */
	li	r1,0x1300
	mtspr	ivor13,r1	/* Data TLB error */
	li	r1,0x1400
	mtspr	ivor14,r1	/* Instr TLB error */
	li	r1,0x2000
	mtspr	ivor15,r1	/* Debug */

	/*----------------------------------------------------------------*/
	/* Configure cache regions  */
	/*----------------------------------------------------------------*/
	mtspr	inv0,r0
	mtspr	inv1,r0
	mtspr	inv2,r0
	mtspr	inv3,r0
	mtspr	dnv0,r0
	mtspr	dnv1,r0
	mtspr	dnv2,r0
	mtspr	dnv3,r0
	mtspr	itv0,r0
	mtspr	itv1,r0
	mtspr	itv2,r0
	mtspr	itv3,r0
	mtspr	dtv0,r0
	mtspr	dtv1,r0
	mtspr	dtv2,r0
	mtspr	dtv3,r0

	/*----------------------------------------------------------------*/
	/* Cache victim limits */
	/*----------------------------------------------------------------*/
	/* floors 0, ceiling max to use the entire cache -- nothing locked
	*/
	lis	r1,0x0001
	ori	r1,r1,0xf800
	mtspr	ivlim,r1
	mtspr	dvlim,r1

	/*----------------------------------------------------------------+
	|Initialize MMUCR[STID] = 0.
	+-----------------------------------------------------------------*/
	mfspr	r0,mmucr
	addis	r1,0,0xFFFF
	ori	r1,r1,0xFF00
	and	r0,r0,r1
	mtspr	mmucr,r0

	/*----------------------------------------------------------------*/
	/* Clear all TLB entries -- TID = 0, TS = 0 */
	/*----------------------------------------------------------------*/
	addis	r0,0,0x0000
	li	r1,0x003f	/* 64 TLB entries */
	mtctr	r1
rsttlb:	tlbwe	r0,r1,0x0000	/* Invalidate all entries (V=0)*/
	tlbwe	r0,r1,0x0001
	tlbwe	r0,r1,0x0002
	subi	r1,r1,0x0001
	bdnz	rsttlb

	/*----------------------------------------------------------------*/
	/* TLB entry setup -- step thru tlbtab */
	/*----------------------------------------------------------------*/
#if defined(CONFIG_440SPE)
	/*----------------------------------------------------------------*/
	/* We have different TLB tables for revA and rev B of 440SPe */
	/*----------------------------------------------------------------*/
	mfspr	r1, PVR
	lis	r0,0x5342
	ori	r0,r0,0x1891
	cmpw	r7,r1,r0
	bne	r7,..revA
	bl	tlbtabB
	b	..goon
..revA:
	bl	tlbtabA
..goon:
#else
	bl	tlbtab		/* Get tlbtab pointer */
#endif
	mr	r5,r0
	li	r1,0x003f	/* 64 TLB entries max */
	mtctr	r1
	li	r4,0		/* TLB # */

	addi	r5,r5,-4
1:	lwzu	r0,4(r5)
	cmpwi	r0,0
	beq	2f		/* 0 marks end */
	lwzu	r1,4(r5)
	lwzu	r2,4(r5)
	tlbwe	r0,r4,0		/* TLB Word 0 */
	tlbwe	r1,r4,1		/* TLB Word 1 */
	tlbwe	r2,r4,2		/* TLB Word 2 */
	addi	r4,r4,1		/* Next TLB */
	bdnz	1b

	/*----------------------------------------------------------------*/
	/* Continue from 'normal' start */
	/*----------------------------------------------------------------*/
2:

#if defined(CONFIG_NAND_SPL)
#if defined(CONFIG_440EPX) || defined(CONFIG_440GRX)
	/*
	 * Enable internal SRAM (only on 440EPx/GRx, 440EP/GR have no OCM)
	 */
	lis	r2,0x7fff
	ori	r2,r2,0xffff
	mfdcr	r1,isram0_dpc
	and	r1,r1,r2		/* Disable parity check */
	mtdcr	isram0_dpc,r1
	mfdcr	r1,isram0_pmeg
	and	r1,r1,r2		/* Disable pwr mgmt */
	mtdcr	isram0_pmeg,r1
#endif
#if defined(CONFIG_440EP)
	/*
	 * On 440EP with no internal SRAM, we setup SDRAM very early
	 * and copy the NAND_SPL to SDRAM and jump to it
	 */
	/* Clear Dcache to use as RAM */
	addis	r3,r0,CFG_INIT_RAM_ADDR@h
	ori	r3,r3,CFG_INIT_RAM_ADDR@l
	addis	r4,r0,CFG_INIT_RAM_END@h
	ori	r4,r4,CFG_INIT_RAM_END@l
	rlwinm. r5,r4,0,27,31
	rlwinm	r5,r4,27,5,31
	beq	..d_ran3
	addi	r5,r5,0x0001
..d_ran3:
	mtctr	r5
..d_ag3:
	dcbz	r0,r3
	addi	r3,r3,32
	bdnz	..d_ag3
	/*----------------------------------------------------------------*/
	/* Setup the stack in internal SRAM */
	/*----------------------------------------------------------------*/
	lis	r1,CFG_INIT_RAM_ADDR@h
	ori	r1,r1,CFG_INIT_SP_OFFSET@l
	li	r0,0
	stwu	r0,-4(r1)
	stwu	r0,-4(r1)		/* Terminate call chain */

	stwu	r1,-8(r1)		/* Save back chain and move SP */
	lis	r0,RESET_VECTOR@h	/* Address of reset vector */
	ori	r0,r0, RESET_VECTOR@l
	stwu	r1,-8(r1)		/* Save back chain and move SP */
	stw	r0,+12(r1)		/* Save return addr (underflow vect) */
	sync
	bl	early_sdram_init
	sync
#endif /* CONFIG_440EP */

	/*
	 * Copy SPL from cache into internal SRAM
	 */
	li	r4,(CFG_NAND_BOOT_SPL_SIZE >> 2) - 1
	mtctr	r4
	lis	r2,CFG_NAND_BOOT_SPL_SRC@h
	ori	r2,r2,CFG_NAND_BOOT_SPL_SRC@l
	lis	r3,CFG_NAND_BOOT_SPL_DST@h
	ori	r3,r3,CFG_NAND_BOOT_SPL_DST@l
spl_loop:
	lwzu	r4,4(r2)
	stwu	r4,4(r3)
	bdnz	spl_loop

	/*
	 * Jump to code in RAM
	 */
	bl	00f
00:	mflr	r10
	lis	r3,(CFG_NAND_BOOT_SPL_SRC - CFG_NAND_BOOT_SPL_DST)@h
	ori	r3,r3,(CFG_NAND_BOOT_SPL_SRC - CFG_NAND_BOOT_SPL_DST)@l
	sub	r10,r10,r3
	addi	r10,r10,28
	mtlr	r10
	blr

start_ram:
	sync
	isync
#endif /* CONFIG_NAND_SPL */

	bl	3f
	b	_start

3:	li	r0,0
	mtspr	srr1,r0		/* Keep things disabled for now */
	mflr	r1
	mtspr	srr0,r1
	rfi
#endif /* CONFIG_440 */

/*
 * r3 - 1st arg to board_init(): IMMP pointer
 * r4 - 2nd arg to board_init(): boot flag
 */
#ifndef CONFIG_NAND_SPL
	.text
	.long	0x27051956		/* U-Boot Magic Number			*/
	.globl	version_string
version_string:
	.ascii U_BOOT_VERSION
	.ascii " (", __DATE__, " - ", __TIME__, ")"
	.ascii CONFIG_IDENT_STRING, "\0"

	. = EXC_OFF_SYS_RESET
	.globl	_start_of_vectors
_start_of_vectors:

/* Critical input. */
	CRIT_EXCEPTION(0x100, CritcalInput, UnknownException)

#ifdef CONFIG_440
/* Machine check */
	MCK_EXCEPTION(0x200, MachineCheck, MachineCheckException)
#else
	CRIT_EXCEPTION(0x200, MachineCheck, MachineCheckException)
#endif /* CONFIG_440 */

/* Data Storage exception. */
	STD_EXCEPTION(0x300, DataStorage, UnknownException)

/* Instruction Storage exception. */
	STD_EXCEPTION(0x400, InstStorage, UnknownException)

/* External Interrupt exception. */
	STD_EXCEPTION(0x500, ExtInterrupt, external_interrupt)

/* Alignment exception. */
	. = 0x600
Alignment:
	EXCEPTION_PROLOG(SRR0, SRR1)
	mfspr	r4,DAR
	stw	r4,_DAR(r21)
	mfspr	r5,DSISR
	stw	r5,_DSISR(r21)
	addi	r3,r1,STACK_FRAME_OVERHEAD
	li	r20,MSR_KERNEL
	rlwimi	r20,r23,0,16,16		/* copy EE bit from saved MSR */
	lwz	r6,GOT(transfer_to_handler)
	mtlr	r6
	blrl
.L_Alignment:
	.long	AlignmentException - _start + _START_OFFSET
	.long	int_return - _start + _START_OFFSET

/* Program check exception */
	. = 0x700
ProgramCheck:
	EXCEPTION_PROLOG(SRR0, SRR1)
	addi	r3,r1,STACK_FRAME_OVERHEAD
	li	r20,MSR_KERNEL
	rlwimi	r20,r23,0,16,16		/* copy EE bit from saved MSR */
	lwz	r6,GOT(transfer_to_handler)
	mtlr	r6
	blrl
.L_ProgramCheck:
	.long	ProgramCheckException - _start + _START_OFFSET
	.long	int_return - _start + _START_OFFSET

#ifdef CONFIG_440
	STD_EXCEPTION(0x800, FPUnavailable, UnknownException)
	STD_EXCEPTION(0x900, Decrementer, DecrementerPITException)
	STD_EXCEPTION(0xa00, APU, UnknownException)
#endif
	STD_EXCEPTION(0xc00, SystemCall, UnknownException)

#ifdef CONFIG_440
	STD_EXCEPTION(0x1300, DataTLBError, UnknownException)
	STD_EXCEPTION(0x1400, InstructionTLBError, UnknownException)
#else
	STD_EXCEPTION(0x1000, PIT, DecrementerPITException)
	STD_EXCEPTION(0x1100, InstructionTLBMiss, UnknownException)
	STD_EXCEPTION(0x1200, DataTLBMiss, UnknownException)
#endif
	CRIT_EXCEPTION(0x2000, DebugBreakpoint, DebugException )

	.globl	_end_of_vectors
_end_of_vectors:
	. = _START_OFFSET
#endif
	.globl	_start
_start:

/*****************************************************************************/
#if defined(CONFIG_440)

	/*----------------------------------------------------------------*/
	/* Clear and set up some registers. */
	/*----------------------------------------------------------------*/
	li	r0,0x0000
	lis	r1,0xffff
	mtspr	dec,r0			/* prevent dec exceptions */
	mtspr	tbl,r0			/* prevent fit & wdt exceptions */
	mtspr	tbu,r0
	mtspr	tsr,r1			/* clear all timer exception status */
	mtspr	tcr,r0			/* disable all */
	mtspr	esr,r0			/* clear exception syndrome register */
	mtxer	r0			/* clear integer exception register */

	/*----------------------------------------------------------------*/
	/* Debug setup -- some (not very good) ice's need an event*/
	/* to establish control :-( Define CFG_INIT_DBCR to the dbsr */
	/* value you need in this case 0x8cff 0000 should do the trick */
	/*----------------------------------------------------------------*/
#if defined(CFG_INIT_DBCR)
	lis	r1,0xffff
	ori	r1,r1,0xffff
	mtspr	dbsr,r1			/* Clear all status bits */
	lis	r0,CFG_INIT_DBCR@h
	ori	r0,r0,CFG_INIT_DBCR@l
	mtspr	dbcr0,r0
	isync
#endif

	/*----------------------------------------------------------------*/
	/* Setup the internal SRAM */
	/*----------------------------------------------------------------*/
	li	r0,0

#ifdef CFG_INIT_RAM_DCACHE
	/* Clear Dcache to use as RAM */
	addis	r3,r0,CFG_INIT_RAM_ADDR@h
	ori	r3,r3,CFG_INIT_RAM_ADDR@l
	addis	r4,r0,CFG_INIT_RAM_END@h
	ori	r4,r4,CFG_INIT_RAM_END@l
	rlwinm. r5,r4,0,27,31
	rlwinm	r5,r4,27,5,31
	beq	..d_ran
	addi	r5,r5,0x0001
..d_ran:
	mtctr	r5
..d_ag:
	dcbz	r0,r3
	addi	r3,r3,32
	bdnz	..d_ag

	/*
	 * Lock the init-ram/stack in d-cache, so that other regions
	 * may use d-cache as well
	 * Note, that this current implementation locks exactly 4k
	 * of d-cache, so please make sure that you don't define a
	 * bigger init-ram area. Take a look at the lwmon5 440EPx
	 * implementation as a reference.
	 */
	msync
	isync
	/* 8. set TFLOOR/NFLOOR to 8 (-> 8*16*32 bytes locked -> 4k) */
	lis	r1,0x0201
	ori	r1,r1,0xf808
	mtspr	dvlim,r1
	lis	r1,0x0808
	ori	r1,r1,0x0808
	mtspr	dnv0,r1
	mtspr	dnv1,r1
	mtspr	dnv2,r1
	mtspr	dnv3,r1
	mtspr	dtv0,r1
	mtspr	dtv1,r1
	mtspr	dtv2,r1
	mtspr	dtv3,r1
	msync
	isync
#endif /* CFG_INIT_RAM_DCACHE */

	/* 440EP & 440GR are only 440er PPC's without internal SRAM */
#if !defined(CONFIG_440EP) && !defined(CONFIG_440GR)
	/* not all PPC's have internal SRAM usable as L2-cache */
#if defined(CONFIG_440GX) || \
    defined(CONFIG_440SP) || defined(CONFIG_440SPE) || \
    defined(CONFIG_460EX) || defined(CONFIG_460GT)
	mtdcr	l2_cache_cfg,r0		/* Ensure L2 Cache is off */
#endif

	lis	r2,0x7fff
	ori	r2,r2,0xffff
	mfdcr	r1,isram0_dpc
	and	r1,r1,r2		/* Disable parity check */
	mtdcr	isram0_dpc,r1
	mfdcr	r1,isram0_pmeg
	and	r1,r1,r2		/* Disable pwr mgmt */
	mtdcr	isram0_pmeg,r1

	lis	r1,0x8000		/* BAS = 8000_0000 */
#if defined(CONFIG_440GX) || defined(CONFIG_440SP)
	ori	r1,r1,0x0980		/* first 64k */
	mtdcr	isram0_sb0cr,r1
	lis	r1,0x8001
	ori	r1,r1,0x0980		/* second 64k */
	mtdcr	isram0_sb1cr,r1
	lis	r1, 0x8002
	ori	r1,r1, 0x0980		/* third 64k */
	mtdcr	isram0_sb2cr,r1
	lis	r1, 0x8003
	ori	r1,r1, 0x0980		/* fourth 64k */
	mtdcr	isram0_sb3cr,r1
#elif defined(CONFIG_440SPE)
	lis	r1,0x0000		/* BAS = 0000_0000 */
	ori	r1,r1,0x0984		/* first 64k */
	mtdcr	isram0_sb0cr,r1
	lis	r1,0x0001
	ori	r1,r1,0x0984		/* second 64k */
	mtdcr	isram0_sb1cr,r1
	lis	r1, 0x0002
	ori	r1,r1, 0x0984		/* third 64k */
	mtdcr	isram0_sb2cr,r1
	lis	r1, 0x0003
	ori	r1,r1, 0x0984		/* fourth 64k */
	mtdcr	isram0_sb3cr,r1
#elif defined(CONFIG_460EX) || defined(CONFIG_460GT)
	lis	r1,0x4000		/* BAS = 8000_0000 */
	ori	r1,r1,0x4580		/* 16k */
	mtdcr	isram0_sb0cr,r1
#elif defined(CONFIG_440GP)
	ori	r1,r1,0x0380		/* 8k rw */
	mtdcr	isram0_sb0cr,r1
	mtdcr	isram0_sb1cr,r0		/* Disable bank 1 */
#endif
#endif /* #if !defined(CONFIG_440EP) && !defined(CONFIG_440GR) */

	/*----------------------------------------------------------------*/
	/* Setup the stack in internal SRAM */
	/*----------------------------------------------------------------*/
	lis	r1,CFG_INIT_RAM_ADDR@h
	ori	r1,r1,CFG_INIT_SP_OFFSET@l
	li	r0,0
	stwu	r0,-4(r1)
	stwu	r0,-4(r1)		/* Terminate call chain */

	stwu	r1,-8(r1)		/* Save back chain and move SP */
	lis	r0,RESET_VECTOR@h	/* Address of reset vector */
	ori	r0,r0, RESET_VECTOR@l
	stwu	r1,-8(r1)		/* Save back chain and move SP */
	stw	r0,+12(r1)		/* Save return addr (underflow vect) */

#ifdef CONFIG_NAND_SPL
	bl	nand_boot		/* will not return */
#else
	GET_GOT

	bl	cpu_init_f	/* run low-level CPU init code	   (from Flash) */
	bl	board_init_f
#endif

#endif /* CONFIG_440 */

/*****************************************************************************/
#ifdef CONFIG_IOP480
	/*----------------------------------------------------------------------- */
	/* Set up some machine state registers. */
	/*----------------------------------------------------------------------- */
	addi	r0,r0,0x0000		/* initialize r0 to zero */
	mtspr	esr,r0			/* clear Exception Syndrome Reg */
	mttcr	r0			/* timer control register */
	mtexier r0			/* disable all interrupts */
	addis	r4,r0,0xFFFF		/* set r4 to 0xFFFFFFFF (status in the */
	ori	r4,r4,0xFFFF		/* dbsr is cleared by setting bits to 1) */
	mtdbsr	r4			/* clear/reset the dbsr */
	mtexisr r4			/* clear all pending interrupts */
	addis	r4,r0,0x8000
	mtexier r4			/* enable critical exceptions */
	addis	r4,r0,0x0000		/* assume 403GCX - enable core clk */
	ori	r4,r4,0x4020		/* dbling (no harm done on GA and GC */
	mtiocr	r4			/* since bit not used) & DRC to latch */
					/* data bus on rising edge of CAS */
	/*----------------------------------------------------------------------- */
	/* Clear XER. */
	/*----------------------------------------------------------------------- */
	mtxer	r0
	/*----------------------------------------------------------------------- */
	/* Invalidate i-cache and d-cache TAG arrays. */
	/*----------------------------------------------------------------------- */
	addi	r3,0,1024		/* 1/4 of I-cache size, half of D-cache */
	addi	r4,0,1024		/* 1/4 of I-cache */
..cloop:
	iccci	0,r3
	iccci	r4,r3
	dccci	0,r3
	addic.	r3,r3,-16		/* move back one cache line */
	bne	..cloop			/* loop back to do rest until r3 = 0 */

	/* */
	/* initialize IOP480 so it can read 1 MB code area for SRAM spaces */
	/* this requires enabling MA[17..0], by default only MA[12..0] are enabled. */
	/* */

	/* first copy IOP480 register base address into r3 */
	addis	r3,0,0x5000		/* IOP480 register base address hi */
/*	ori	r3,r3,0x0000		/  IOP480 register base address lo */

#ifdef CONFIG_ADCIOP
	/* use r4 as the working variable */
	/* turn on CS3 (LOCCTL.7) */
	lwz	r4,0x84(r3)		/* LOCTL is at offset 0x84 */
	andi.	r4,r4,0xff7f		/* make bit 7 = 0 -- CS3 mode */
	stw	r4,0x84(r3)		/* LOCTL is at offset 0x84 */
#endif

#ifdef CONFIG_DASA_SIM
	/* use r4 as the working variable */
	/* turn on MA17 (LOCCTL.7) */
	lwz	r4,0x84(r3)		/* LOCTL is at offset 0x84 */
	ori	r4,r4,0x80		/* make bit 7 = 1 -- MA17 mode */
	stw	r4,0x84(r3)		/* LOCTL is at offset 0x84 */
#endif

	/* turn on MA16..13 (LCS0BRD.12 = 0) */
	lwz	r4,0x100(r3)		/* LCS0BRD is at offset 0x100 */
	andi.	r4,r4,0xefff		/* make bit 12 = 0 */
	stw	r4,0x100(r3)		/* LCS0BRD is at offset 0x100 */

	/* make sure above stores all comlete before going on */
	sync

	/* last thing, set local init status done bit (DEVINIT.31) */
	lwz	r4,0x80(r3)		/* DEVINIT is at offset 0x80 */
	oris	r4,r4,0x8000		/* make bit 31 = 1 */
	stw	r4,0x80(r3)		/* DEVINIT is at offset 0x80 */

	/* clear all pending interrupts and disable all interrupts */
	li	r4,-1			/* set p1 to 0xffffffff */
	stw	r4,0x1b0(r3)		/* clear all pending interrupts */
	stw	r4,0x1b8(r3)		/* clear all pending interrupts */
	li	r4,0			/* set r4 to 0 */
	stw	r4,0x1b4(r3)		/* disable all interrupts */
	stw	r4,0x1bc(r3)		/* disable all interrupts */

	/* make sure above stores all comlete before going on */
	sync

	/*----------------------------------------------------------------------- */
	/* Enable two 128MB cachable regions. */
	/*----------------------------------------------------------------------- */
	addis	r1,r0,0xc000
	addi	r1,r1,0x0001
	mticcr	r1			/* instruction cache */

	addis	r1,r0,0x0000
	addi	r1,r1,0x0000
	mtdccr	r1			/* data cache */

	addis	r1,r0,CFG_INIT_RAM_ADDR@h
	ori	r1,r1,CFG_INIT_SP_OFFSET	  /* set up the stack to SDRAM */
	li	r0, 0			/* Make room for stack frame header and */
	stwu	r0, -4(r1)		/* clear final stack frame so that	*/
	stwu	r0, -4(r1)		/* stack backtraces terminate cleanly	*/

	GET_GOT			/* initialize GOT access			*/

	bl	board_init_f	/* run first part of init code (from Flash)	*/

#endif	/* CONFIG_IOP480 */

/*****************************************************************************/
#if defined(CONFIG_405GP) || defined(CONFIG_405CR) || \
    defined(CONFIG_405EP) || defined(CONFIG_405EZ) || \
    defined(CONFIG_405EX) || defined(CONFIG_405)
	/*----------------------------------------------------------------------- */
	/* Clear and set up some registers. */
	/*----------------------------------------------------------------------- */
	addi	r4,r0,0x0000
#if !defined(CONFIG_405EX)
	mtspr	sgr,r4
#else
	/*
	 * On 405EX, completely clearing the SGR leads to PPC hangup
	 * upon PCIe configuration access. The PCIe memory regions
	 * need to be guarded!
	 */
	lis	r3,0x0000
	ori	r3,r3,0x7FFC
	mtspr	sgr,r3
#endif
	mtspr	dcwr,r4
	mtesr	r4			/* clear Exception Syndrome Reg */
	mttcr	r4			/* clear Timer Control Reg */
	mtxer	r4			/* clear Fixed-Point Exception Reg */
	mtevpr	r4			/* clear Exception Vector Prefix Reg */
	addi	r4,r0,(0xFFFF-0x10000)		/* set r4 to 0xFFFFFFFF (status in the */
					/* dbsr is cleared by setting bits to 1) */
	mtdbsr	r4			/* clear/reset the dbsr */

	/*----------------------------------------------------------------------- */
	/* Invalidate I and D caches. Enable I cache for defined memory regions */
	/* to speed things up. Leave the D cache disabled for now. It will be */
	/* enabled/left disabled later based on user selected menu options. */
	/* Be aware that the I cache may be disabled later based on the menu */
	/* options as well. See miscLib/main.c. */
	/*----------------------------------------------------------------------- */
	bl	invalidate_icache
	bl	invalidate_dcache

	/*----------------------------------------------------------------------- */
	/* Enable two 128MB cachable regions. */
	/*----------------------------------------------------------------------- */
	lis	r4,0xc000
	ori	r4,r4,0x0001
	mticcr	r4			/* instruction cache */
	isync

	lis	r4,0x0000
	ori	r4,r4,0x0000
	mtdccr	r4			/* data cache */

#if !(defined(CFG_EBC_PB0AP) && defined(CFG_EBC_PB0CR)) || defined(CONFIG_405EX)
	/*----------------------------------------------------------------------- */
	/* Tune the speed and size for flash CS0  */
	/*----------------------------------------------------------------------- */
	bl	ext_bus_cntlr_init
#endif
#if !(defined(CFG_INIT_DCACHE_CS) || defined(CFG_TEMP_STACK_OCM))
	/*
	 * Boards like the Kilauea (405EX) don't have OCM and can't use
	 * DCache for init-ram. So setup stack here directly after the
	 * SDRAM is initialized.
	 */
	lis	r1, CFG_INIT_RAM_ADDR@h
	ori	r1,r1,CFG_INIT_SP_OFFSET /* set up the stack in SDRAM */

	li	r0, 0			/* Make room for stack frame header and */
	stwu	r0, -4(r1)		/* clear final stack frame so that	*/
	stwu	r0, -4(r1)		/* stack backtraces terminate cleanly	*/
	/*
	 * Set up a dummy frame to store reset vector as return address.
	 * this causes stack underflow to reset board.
	 */
	stwu	r1, -8(r1)		/* Save back chain and move SP */
	lis	r0, RESET_VECTOR@h	/* Address of reset vector */
	ori	r0, r0, RESET_VECTOR@l
	stwu	r1, -8(r1)		/* Save back chain and move SP */
	stw	r0, +12(r1)		/* Save return addr (underflow vect) */
#endif /* !(CFG_INIT_DCACHE_CS	|| !CFG_TEM_STACK_OCM) */

#if defined(CONFIG_405EP)
	/*----------------------------------------------------------------------- */
	/* DMA Status, clear to come up clean */
	/*----------------------------------------------------------------------- */
	addis	r3,r0, 0xFFFF	      /* Clear all existing DMA status */
	ori	r3,r3, 0xFFFF
	mtdcr	dmasr, r3

	bl	ppc405ep_init	      /* do ppc405ep specific init */
#endif /* CONFIG_405EP */

#if defined(CFG_OCM_DATA_ADDR) && defined(CFG_OCM_DATA_SIZE)
#if defined(CONFIG_405EZ)
	/********************************************************************
	 * Setup OCM - On Chip Memory - PPC405EZ uses OCM Controller V2
	 *******************************************************************/
	/*
	 * We can map the OCM on the PLB3, so map it at
	 * CFG_OCM_DATA_ADDR + 0x8000
	 */
	lis	r3,CFG_OCM_DATA_ADDR@h	/* OCM location */
	ori	r3,r3,CFG_OCM_DATA_ADDR@l
	ori	r3,r3,0x0270		/* 16K for Bank 1, R/W/Enable */
	mtdcr	ocmplb3cr1,r3		/* Set PLB Access */
	ori	r3,r3,0x4000		/* Add 0x4000 for bank 2 */
	mtdcr	ocmplb3cr2,r3		/* Set PLB Access */
	isync

	lis	r3,CFG_OCM_DATA_ADDR@h	/* OCM location */
	ori	r3,r3,CFG_OCM_DATA_ADDR@l
	ori	r3,r3,0x0270		/* 16K for Bank 1, R/W/Enable */
	mtdcr	ocmdscr1, r3		/* Set Data Side */
	mtdcr	ocmiscr1, r3		/* Set Instruction Side */
	ori	r3,r3,0x4000		/* Add 0x4000 for bank 2 */
	mtdcr	ocmdscr2, r3		/* Set Data Side */
	mtdcr	ocmiscr2, r3		/* Set Instruction Side */
	addis	r3,0,0x0800		/* OCM Data Parity Disable - 1 Wait State */
	mtdcr	ocmdsisdpc,r3

	isync
#else /* CONFIG_405EZ */
	/********************************************************************
	 * Setup OCM - On Chip Memory
	 *******************************************************************/
	/* Setup OCM */
	lis	r0, 0x7FFF
	ori	r0, r0, 0xFFFF
	mfdcr	r3, ocmiscntl		/* get instr-side IRAM config */
	mfdcr	r4, ocmdscntl		/* get data-side IRAM config */
	and	r3, r3, r0		/* disable data-side IRAM */
	and	r4, r4, r0		/* disable data-side IRAM */
	mtdcr	ocmiscntl, r3		/* set instr-side IRAM config */
	mtdcr	ocmdscntl, r4		/* set data-side IRAM config */
	isync

	lis	r3,CFG_OCM_DATA_ADDR@h	/* OCM location */
	ori	r3,r3,CFG_OCM_DATA_ADDR@l
	mtdcr	ocmdsarc, r3
	addis	r4, 0, 0xC000		/* OCM data area enabled */
	mtdcr	ocmdscntl, r4
	isync
#endif /* CONFIG_405EZ */
#endif

#ifdef CONFIG_NAND_SPL
	/*
	 * Copy SPL from cache into internal SRAM
	 */
	li	r4,(CFG_NAND_BOOT_SPL_SIZE >> 2) - 1
	mtctr	r4
	lis	r2,CFG_NAND_BOOT_SPL_SRC@h
	ori	r2,r2,CFG_NAND_BOOT_SPL_SRC@l
	lis	r3,CFG_NAND_BOOT_SPL_DST@h
	ori	r3,r3,CFG_NAND_BOOT_SPL_DST@l
spl_loop:
	lwzu	r4,4(r2)
	stwu	r4,4(r3)
	bdnz	spl_loop

	/*
	 * Jump to code in RAM
	 */
	bl	00f
00:	mflr	r10
	lis	r3,(CFG_NAND_BOOT_SPL_SRC - CFG_NAND_BOOT_SPL_DST)@h
	ori	r3,r3,(CFG_NAND_BOOT_SPL_SRC - CFG_NAND_BOOT_SPL_DST)@l
	sub	r10,r10,r3
	addi	r10,r10,28
	mtlr	r10
	blr

start_ram:
	sync
	isync
#endif /* CONFIG_NAND_SPL */

	/*----------------------------------------------------------------------- */
	/* Setup temporary stack in DCACHE or OCM if needed for SDRAM SPD. */
	/*----------------------------------------------------------------------- */
#ifdef CFG_INIT_DCACHE_CS
	/*----------------------------------------------------------------------- */
	/* Memory Bank x (nothingness) initialization 1GB+64MEG */
	/* used as temporary stack pointer for stage0  */
	/*----------------------------------------------------------------------- */
	li	r4,PBxAP
	mtdcr	ebccfga,r4
	lis	r4,0x0380
	ori	r4,r4,0x0480
	mtdcr	ebccfgd,r4

	addi	r4,0,PBxCR
	mtdcr	ebccfga,r4
	lis	r4,0x400D
	ori	r4,r4,0xa000
	mtdcr	ebccfgd,r4

	/* turn on data cache for this region */
	lis	r4,0x0080
	mtdccr	r4

	/* set stack pointer and clear stack to known value */

	lis	r1,CFG_INIT_RAM_ADDR@h
	ori	r1,r1,CFG_INIT_SP_OFFSET@l

	li	r4,2048			/* we store 2048 words to stack */
	mtctr	r4

	lis	r2,CFG_INIT_RAM_ADDR@h		/* we also clear data area */
	ori	r2,r2,CFG_INIT_RAM_END@l	/* so cant copy value from r1 */

	lis	r4,0xdead		/* we store 0xdeaddead in the stack */
	ori	r4,r4,0xdead

..stackloop:
	stwu	r4,-4(r2)
	bdnz	..stackloop

	li	r0, 0			/* Make room for stack frame header and */
	stwu	r0, -4(r1)		/* clear final stack frame so that	*/
	stwu	r0, -4(r1)		/* stack backtraces terminate cleanly	*/
	/*
	 * Set up a dummy frame to store reset vector as return address.
	 * this causes stack underflow to reset board.
	 */
	stwu	r1, -8(r1)		/* Save back chain and move SP */
	addis	r0, 0, RESET_VECTOR@h	/* Address of reset vector */
	ori	r0, r0, RESET_VECTOR@l
	stwu	r1, -8(r1)		/* Save back chain and move SP */
	stw	r0, +12(r1)		/* Save return addr (underflow vect) */

#elif defined(CFG_TEMP_STACK_OCM) && \
	(defined(CFG_OCM_DATA_ADDR) && defined(CFG_OCM_DATA_SIZE))
	/*
	 * Stack in OCM.
	 */

	/* Set up Stack at top of OCM */
	lis	r1, (CFG_INIT_RAM_ADDR + CFG_INIT_SP_OFFSET)@h
	ori	r1, r1, (CFG_INIT_RAM_ADDR + CFG_INIT_SP_OFFSET)@l

	/* Set up a zeroized stack frame so that backtrace works right */
	li	r0, 0
	stwu	r0, -4(r1)
	stwu	r0, -4(r1)

	/*
	 * Set up a dummy frame to store reset vector as return address.
	 * this causes stack underflow to reset board.
	 */
	stwu	r1, -8(r1)		/* Save back chain and move SP */
	lis	r0, RESET_VECTOR@h	/* Address of reset vector */
	ori	r0, r0, RESET_VECTOR@l
	stwu	r1, -8(r1)		/* Save back chain and move SP */
	stw	r0, +12(r1)		/* Save return addr (underflow vect) */
#endif /* CFG_INIT_DCACHE_CS */

	/*----------------------------------------------------------------------- */
	/* Initialize SDRAM Controller	*/
	/*----------------------------------------------------------------------- */
	bl	sdram_init

#ifdef CONFIG_NAND_SPL
	bl	nand_boot		/* will not return */
#else
	GET_GOT			/* initialize GOT access			*/

	bl	cpu_init_f	/* run low-level CPU init code	   (from Flash) */

	/* NEVER RETURNS! */
	bl	board_init_f	/* run first part of init code (from Flash)	*/
#endif /* CONFIG_NAND_SPL */

#endif	/* CONFIG_405GP || CONFIG_405CR || CONFIG_405 || CONFIG_405EP */
	/*----------------------------------------------------------------------- */


#ifndef CONFIG_NAND_SPL
/*
 * This code finishes saving the registers to the exception frame
 * and jumps to the appropriate handler for the exception.
 * Register r21 is pointer into trap frame, r1 has new stack pointer.
 */
	.globl	transfer_to_handler
transfer_to_handler:
	stw	r22,_NIP(r21)
	lis	r22,MSR_POW@h
	andc	r23,r23,r22
	stw	r23,_MSR(r21)
	SAVE_GPR(7, r21)
	SAVE_4GPRS(8, r21)
	SAVE_8GPRS(12, r21)
	SAVE_8GPRS(24, r21)
	mflr	r23
	andi.	r24,r23,0x3f00		/* get vector offset */
	stw	r24,TRAP(r21)
	li	r22,0
	stw	r22,RESULT(r21)
	mtspr	SPRG2,r22		/* r1 is now kernel sp */
	lwz	r24,0(r23)		/* virtual address of handler */
	lwz	r23,4(r23)		/* where to go when done */
	mtspr	SRR0,r24
	mtspr	SRR1,r20
	mtlr	r23
	SYNC
	rfi				/* jump to handler, enable MMU */

int_return:
	mfmsr	r28		/* Disable interrupts */
	li	r4,0
	ori	r4,r4,MSR_EE
	andc	r28,r28,r4
	SYNC			/* Some chip revs need this... */
	mtmsr	r28
	SYNC
	lwz	r2,_CTR(r1)
	lwz	r0,_LINK(r1)
	mtctr	r2
	mtlr	r0
	lwz	r2,_XER(r1)
	lwz	r0,_CCR(r1)
	mtspr	XER,r2
	mtcrf	0xFF,r0
	REST_10GPRS(3, r1)
	REST_10GPRS(13, r1)
	REST_8GPRS(23, r1)
	REST_GPR(31, r1)
	lwz	r2,_NIP(r1)	/* Restore environment */
	lwz	r0,_MSR(r1)
	mtspr	SRR0,r2
	mtspr	SRR1,r0
	lwz	r0,GPR0(r1)
	lwz	r2,GPR2(r1)
	lwz	r1,GPR1(r1)
	SYNC
	rfi

crit_return:
	mfmsr	r28		/* Disable interrupts */
	li	r4,0
	ori	r4,r4,MSR_EE
	andc	r28,r28,r4
	SYNC			/* Some chip revs need this... */
	mtmsr	r28
	SYNC
	lwz	r2,_CTR(r1)
	lwz	r0,_LINK(r1)
	mtctr	r2
	mtlr	r0
	lwz	r2,_XER(r1)
	lwz	r0,_CCR(r1)
	mtspr	XER,r2
	mtcrf	0xFF,r0
	REST_10GPRS(3, r1)
	REST_10GPRS(13, r1)
	REST_8GPRS(23, r1)
	REST_GPR(31, r1)
	lwz	r2,_NIP(r1)	/* Restore environment */
	lwz	r0,_MSR(r1)
	mtspr	csrr0,r2
	mtspr	csrr1,r0
	lwz	r0,GPR0(r1)
	lwz	r2,GPR2(r1)
	lwz	r1,GPR1(r1)
	SYNC
	rfci

#ifdef CONFIG_440
mck_return:
	mfmsr	r28		/* Disable interrupts */
	li	r4,0
	ori	r4,r4,MSR_EE
	andc	r28,r28,r4
	SYNC			/* Some chip revs need this... */
	mtmsr	r28
	SYNC
	lwz	r2,_CTR(r1)
	lwz	r0,_LINK(r1)
	mtctr	r2
	mtlr	r0
	lwz	r2,_XER(r1)
	lwz	r0,_CCR(r1)
	mtspr	XER,r2
	mtcrf	0xFF,r0
	REST_10GPRS(3, r1)
	REST_10GPRS(13, r1)
	REST_8GPRS(23, r1)
	REST_GPR(31, r1)
	lwz	r2,_NIP(r1)	/* Restore environment */
	lwz	r0,_MSR(r1)
	mtspr	mcsrr0,r2
	mtspr	mcsrr1,r0
	lwz	r0,GPR0(r1)
	lwz	r2,GPR2(r1)
	lwz	r1,GPR1(r1)
	SYNC
	rfmci
#endif /* CONFIG_440 */


	.globl get_pvr
get_pvr:
	mfspr	r3, PVR
	blr

/*------------------------------------------------------------------------------- */
/* Function:	 out16 */
/* Description:	 Output 16 bits */
/*------------------------------------------------------------------------------- */
	.globl	out16
out16:
	sth	r4,0x0000(r3)
	blr

/*------------------------------------------------------------------------------- */
/* Function:	 out16r */
/* Description:	 Byte reverse and output 16 bits */
/*------------------------------------------------------------------------------- */
	.globl	out16r
out16r:
	sthbrx	r4,r0,r3
	blr

/*------------------------------------------------------------------------------- */
/* Function:	 out32r */
/* Description:	 Byte reverse and output 32 bits */
/*------------------------------------------------------------------------------- */
	.globl	out32r
out32r:
	stwbrx	r4,r0,r3
	blr

/*------------------------------------------------------------------------------- */
/* Function:	 in16 */
/* Description:	 Input 16 bits */
/*------------------------------------------------------------------------------- */
	.globl	in16
in16:
	lhz	r3,0x0000(r3)
	blr

/*------------------------------------------------------------------------------- */
/* Function:	 in16r */
/* Description:	 Input 16 bits and byte reverse */
/*------------------------------------------------------------------------------- */
	.globl	in16r
in16r:
	lhbrx	r3,r0,r3
	blr

/*------------------------------------------------------------------------------- */
/* Function:	 in32r */
/* Description:	 Input 32 bits and byte reverse */
/*------------------------------------------------------------------------------- */
	.globl	in32r
in32r:
	lwbrx	r3,r0,r3
	blr

/*
 * void relocate_code (addr_sp, gd, addr_moni)
 *
 * This "function" does not return, instead it continues in RAM
 * after relocating the monitor code.
 *
 * r3 = dest
 * r4 = src
 * r5 = length in bytes
 * r6 = cachelinesize
 */
	.globl	relocate_code
relocate_code:
#ifdef CONFIG_4xx_DCACHE
	/*
	 * We need to flush the Init Data before the dcache will be
	 * invalidated
	 */

	/* save regs */
	mr	r9,r3
	mr	r10,r4
	mr	r11,r5

	mr	r3,r4
	addi	r4,r4,0x200	/* should be enough for init data */
	bl	flush_dcache_range

	/* restore regs */
	mr	r3,r9
	mr	r4,r10
	mr	r5,r11
#endif

#ifdef CFG_INIT_RAM_DCACHE
	/*
	 * Unlock the previously locked d-cache
	 */
	msync
	isync
	/* set TFLOOR/NFLOOR to 0 again */
	lis	r6,0x0001
	ori	r6,r6,0xf800
	mtspr	dvlim,r6
	lis	r6,0x0000
	ori	r6,r6,0x0000
	mtspr	dnv0,r6
	mtspr	dnv1,r6
	mtspr	dnv2,r6
	mtspr	dnv3,r6
	mtspr	dtv0,r6
	mtspr	dtv1,r6
	mtspr	dtv2,r6
	mtspr	dtv3,r6
	msync
	isync
#endif /* CFG_INIT_RAM_DCACHE */

#if defined(CONFIG_440EP) || defined(CONFIG_440GR) || \
    defined(CONFIG_440EPX) || defined(CONFIG_440GRX) || \
    defined(CONFIG_440SP) || defined(CONFIG_440SPE) || \
    defined(CONFIG_460EX) || defined(CONFIG_460GT)
	/*
	 * On some 440er platforms the cache is enabled in the first TLB (Boot-CS)
	 * to speed up the boot process. Now this cache needs to be disabled.
	 */
	iccci	0,0			/* Invalidate inst cache */
	dccci	0,0			/* Invalidate data cache, now no longer our stack */
	sync
	isync
#ifdef CFG_TLB_FOR_BOOT_FLASH
	addi	r1,r0,CFG_TLB_FOR_BOOT_FLASH	/* Use defined TLB */
#else
	addi	r1,r0,0x0000		/* Default TLB entry is #0 */
#endif
	tlbre	r0,r1,0x0002		/* Read contents */
	ori	r0,r0,0x0c00		/* Or in the inhibit, write through bit */
	tlbwe	r0,r1,0x0002		/* Save it out */
	sync
	isync
#endif
	mr	r1,  r3		/* Set new stack pointer		*/
	mr	r9,  r4		/* Save copy of Init Data pointer	*/
	mr	r10, r5		/* Save copy of Destination Address	*/

	mr	r3,  r5				/* Destination Address	*/
	lis	r4, CFG_MONITOR_BASE@h		/* Source      Address	*/
	ori	r4, r4, CFG_MONITOR_BASE@l
	lwz	r5, GOT(__init_end)
	sub	r5, r5, r4
	li	r6, L1_CACHE_BYTES		/* Cache Line Size	*/

	/*
	 * Fix GOT pointer:
	 *
	 * New GOT-PTR = (old GOT-PTR - CFG_MONITOR_BASE) + Destination Address
	 *
	 * Offset:
	 */
	sub	r15, r10, r4

	/* First our own GOT */
	add	r14, r14, r15
	/* the the one used by the C code */
	add	r30, r30, r15

	/*
	 * Now relocate code
	 */

	cmplw	cr1,r3,r4
	addi	r0,r5,3
	srwi.	r0,r0,2
	beq	cr1,4f		/* In place copy is not necessary	*/
	beq	7f		/* Protect against 0 count		*/
	mtctr	r0
	bge	cr1,2f

	la	r8,-4(r4)
	la	r7,-4(r3)
1:	lwzu	r0,4(r8)
	stwu	r0,4(r7)
	bdnz	1b
	b	4f

2:	slwi	r0,r0,2
	add	r8,r4,r0
	add	r7,r3,r0
3:	lwzu	r0,-4(r8)
	stwu	r0,-4(r7)
	bdnz	3b

/*
 * Now flush the cache: note that we must start from a cache aligned
 * address. Otherwise we might miss one cache line.
 */
4:	cmpwi	r6,0
	add	r5,r3,r5
	beq	7f		/* Always flush prefetch queue in any case */
	subi	r0,r6,1
	andc	r3,r3,r0
	mr	r4,r3
5:	dcbst	0,r4
	add	r4,r4,r6
	cmplw	r4,r5
	blt	5b
	sync			/* Wait for all dcbst to complete on bus */
	mr	r4,r3
6:	icbi	0,r4
	add	r4,r4,r6
	cmplw	r4,r5
	blt	6b
7:	sync			/* Wait for all icbi to complete on bus */
	isync

/*
 * We are done. Do not return, instead branch to second part of board
 * initialization, now running from RAM.
 */

	addi	r0, r10, in_ram - _start + _START_OFFSET
	mtlr	r0
	blr				/* NEVER RETURNS! */

in_ram:

	/*
	 * Relocation Function, r14 point to got2+0x8000
	 *
	 * Adjust got2 pointers, no need to check for 0, this code
	 * already puts a few entries in the table.
	 */
	li	r0,__got2_entries@sectoff@l
	la	r3,GOT(_GOT2_TABLE_)
	lwz	r11,GOT(_GOT2_TABLE_)
	mtctr	r0
	sub	r11,r3,r11
	addi	r3,r3,-4
1:	lwzu	r0,4(r3)
	add	r0,r0,r11
	stw	r0,0(r3)
	bdnz	1b

	/*
	 * Now adjust the fixups and the pointers to the fixups
	 * in case we need to move ourselves again.
	 */
2:	li	r0,__fixup_entries@sectoff@l
	lwz	r3,GOT(_FIXUP_TABLE_)
	cmpwi	r0,0
	mtctr	r0
	addi	r3,r3,-4
	beq	4f
3:	lwzu	r4,4(r3)
	lwzux	r0,r4,r11
	add	r0,r0,r11
	stw	r10,0(r3)
	stw	r0,0(r4)
	bdnz	3b
4:
clear_bss:
	/*
	 * Now clear BSS segment
	 */
	lwz	r3,GOT(__bss_start)
	lwz	r4,GOT(_end)

	cmplw	0, r3, r4
	beq	7f

	li	r0, 0

	andi.	r5, r4, 3
	beq	6f
	sub	r4, r4, r5
	mtctr	r5
	mr	r5, r4
5:	stb	r0, 0(r5)
	addi	r5, r5, 1
	bdnz	5b
6:
	stw	r0, 0(r3)
	addi	r3, r3, 4
	cmplw	0, r3, r4
	bne	6b

7:
	mr	r3, r9		/* Init Data pointer		*/
	mr	r4, r10		/* Destination Address		*/
	bl	board_init_r

	/*
	 * Copy exception vector code to low memory
	 *
	 * r3: dest_addr
	 * r7: source address, r8: end address, r9: target address
	 */
	.globl	trap_init
trap_init:
	lwz	r7, GOT(_start_of_vectors)
	lwz	r8, GOT(_end_of_vectors)

	li	r9, 0x100		/* reset vector always at 0x100 */

	cmplw	0, r7, r8
	bgelr				/* return if r7>=r8 - just in case */

	mflr	r4			/* save link register		*/
1:
	lwz	r0, 0(r7)
	stw	r0, 0(r9)
	addi	r7, r7, 4
	addi	r9, r9, 4
	cmplw	0, r7, r8
	bne	1b

	/*
	 * relocate `hdlr' and `int_return' entries
	 */
	li	r7, .L_MachineCheck - _start + _START_OFFSET
	li	r8, Alignment - _start + _START_OFFSET
2:
	bl	trap_reloc
	addi	r7, r7, 0x100		/* next exception vector */
	cmplw	0, r7, r8
	blt	2b

	li	r7, .L_Alignment - _start + _START_OFFSET
	bl	trap_reloc

	li	r7, .L_ProgramCheck - _start + _START_OFFSET
	bl	trap_reloc

#ifdef CONFIG_440
	li	r7, .L_FPUnavailable - _start + _START_OFFSET
	bl	trap_reloc

	li	r7, .L_Decrementer - _start + _START_OFFSET
	bl	trap_reloc

	li	r7, .L_APU - _start + _START_OFFSET
	bl	trap_reloc

	li	r7, .L_InstructionTLBError - _start + _START_OFFSET
	bl	trap_reloc

	li	r7, .L_DataTLBError - _start + _START_OFFSET
	bl	trap_reloc
#else /* CONFIG_440 */
	li	r7, .L_PIT - _start + _START_OFFSET
	bl	trap_reloc

	li	r7, .L_InstructionTLBMiss - _start + _START_OFFSET
	bl	trap_reloc

	li	r7, .L_DataTLBMiss - _start + _START_OFFSET
	bl	trap_reloc
#endif /* CONFIG_440 */

	li	r7, .L_DebugBreakpoint - _start + _START_OFFSET
	bl	trap_reloc

#if !defined(CONFIG_440)
	addi	r7,r0,0x1000		/* set ME bit (Machine Exceptions) */
	oris	r7,r7,0x0002		/* set CE bit (Critical Exceptions) */
	mtmsr	r7			/* change MSR */
#else
	bl	__440_msr_set
	b	__440_msr_continue

__440_msr_set:
	addi	r7,r0,0x1000		/* set ME bit (Machine Exceptions) */
	oris	r7,r7,0x0002		/* set CE bit (Critical Exceptions) */
	mtspr	srr1,r7
	mflr	r7
	mtspr	srr0,r7
	rfi
__440_msr_continue:
#endif

	mtlr	r4			/* restore link register	*/
	blr

	/*
	 * Function: relocate entries for one exception vector
	 */
trap_reloc:
	lwz	r0, 0(r7)		/* hdlr ...			*/
	add	r0, r0, r3		/*  ... += dest_addr		*/
	stw	r0, 0(r7)

	lwz	r0, 4(r7)		/* int_return ...		*/
	add	r0, r0, r3		/*  ... += dest_addr		*/
	stw	r0, 4(r7)

	blr

#if defined(CONFIG_440)
/*----------------------------------------------------------------------------+
| dcbz_area.
+----------------------------------------------------------------------------*/
	function_prolog(dcbz_area)
	rlwinm. r5,r4,0,27,31
	rlwinm	r5,r4,27,5,31
	beq	..d_ra2
	addi	r5,r5,0x0001
..d_ra2:mtctr	r5
..d_ag2:dcbz	r0,r3
	addi	r3,r3,32
	bdnz	..d_ag2
	sync
	blr
	function_epilog(dcbz_area)

/*----------------------------------------------------------------------------+
| dflush.  Assume 32K at vector address is cachable.
+----------------------------------------------------------------------------*/
	function_prolog(dflush)
	mfmsr	r9
	rlwinm	r8,r9,0,15,13
	rlwinm	r8,r8,0,17,15
	mtmsr	r8
	mfspr	r8,dvlim
	addi	r3,r0,0x0000
	mtspr	dvlim,r3
	mfspr	r3,ivpr
	addi	r4,r0,1024
	mtctr	r4
..dflush_loop:
	lwz	r6,0x0(r3)
	addi	r3,r3,32
	bdnz	..dflush_loop
	addi	r3,r3,-32
	mtctr	r4
..ag:	dcbf	r0,r3
	addi	r3,r3,-32
	bdnz	..ag
	mtspr	dvlim,r8
	sync
	mtmsr	r9
	blr
	function_epilog(dflush)
#endif /* CONFIG_440 */
#endif /* CONFIG_NAND_SPL */

/*------------------------------------------------------------------------------- */
/* Function:	 in8 */
/* Description:	 Input 8 bits */
/*------------------------------------------------------------------------------- */
	.globl	in8
in8:
	lbz	r3,0x0000(r3)
	blr

/*------------------------------------------------------------------------------- */
/* Function:	 out8 */
/* Description:	 Output 8 bits */
/*------------------------------------------------------------------------------- */
	.globl	out8
out8:
	stb	r4,0x0000(r3)
	blr

/*------------------------------------------------------------------------------- */
/* Function:	 out32 */
/* Description:	 Output 32 bits */
/*------------------------------------------------------------------------------- */
	.globl	out32
out32:
	stw	r4,0x0000(r3)
	blr

/*------------------------------------------------------------------------------- */
/* Function:	 in32 */
/* Description:	 Input 32 bits */
/*------------------------------------------------------------------------------- */
	.globl	in32
in32:
	lwz	3,0x0000(3)
	blr

/**************************************************************************/
/* PPC405EP specific stuff						  */
/**************************************************************************/
#ifdef CONFIG_405EP
ppc405ep_init:

#ifdef CONFIG_BUBINGA
	/*
	 * Initialize EBC chip selects 1 & 4 and GPIO pins (for alternate
	 * function) to support FPGA and NVRAM accesses below.
	 */

	lis	r3,GPIO0_OSRH@h		/* config GPIO output select */
	ori	r3,r3,GPIO0_OSRH@l
	lis	r4,CFG_GPIO0_OSRH@h
	ori	r4,r4,CFG_GPIO0_OSRH@l
	stw	r4,0(r3)
	lis	r3,GPIO0_OSRL@h
	ori	r3,r3,GPIO0_OSRL@l
	lis	r4,CFG_GPIO0_OSRL@h
	ori	r4,r4,CFG_GPIO0_OSRL@l
	stw	r4,0(r3)

	lis	r3,GPIO0_ISR1H@h	/* config GPIO input select */
	ori	r3,r3,GPIO0_ISR1H@l
	lis	r4,CFG_GPIO0_ISR1H@h
	ori	r4,r4,CFG_GPIO0_ISR1H@l
	stw	r4,0(r3)
	lis	r3,GPIO0_ISR1L@h
	ori	r3,r3,GPIO0_ISR1L@l
	lis	r4,CFG_GPIO0_ISR1L@h
	ori	r4,r4,CFG_GPIO0_ISR1L@l
	stw	r4,0(r3)

	lis	r3,GPIO0_TSRH@h		/* config GPIO three-state select */
	ori	r3,r3,GPIO0_TSRH@l
	lis	r4,CFG_GPIO0_TSRH@h
	ori	r4,r4,CFG_GPIO0_TSRH@l
	stw	r4,0(r3)
	lis	r3,GPIO0_TSRL@h
	ori	r3,r3,GPIO0_TSRL@l
	lis	r4,CFG_GPIO0_TSRL@h
	ori	r4,r4,CFG_GPIO0_TSRL@l
	stw	r4,0(r3)

	lis	r3,GPIO0_TCR@h		/* config GPIO driver output enables */
	ori	r3,r3,GPIO0_TCR@l
	lis	r4,CFG_GPIO0_TCR@h
	ori	r4,r4,CFG_GPIO0_TCR@l
	stw	r4,0(r3)

	li	r3,pb1ap		/* program EBC bank 1 for RTC access */
	mtdcr	ebccfga,r3
	lis	r3,CFG_EBC_PB1AP@h
	ori	r3,r3,CFG_EBC_PB1AP@l
	mtdcr	ebccfgd,r3
	li	r3,pb1cr
	mtdcr	ebccfga,r3
	lis	r3,CFG_EBC_PB1CR@h
	ori	r3,r3,CFG_EBC_PB1CR@l
	mtdcr	ebccfgd,r3

	li	r3,pb1ap		/* program EBC bank 1 for RTC access */
	mtdcr	ebccfga,r3
	lis	r3,CFG_EBC_PB1AP@h
	ori	r3,r3,CFG_EBC_PB1AP@l
	mtdcr	ebccfgd,r3
	li	r3,pb1cr
	mtdcr	ebccfga,r3
	lis	r3,CFG_EBC_PB1CR@h
	ori	r3,r3,CFG_EBC_PB1CR@l
	mtdcr	ebccfgd,r3

	li	r3,pb4ap		/* program EBC bank 4 for FPGA access */
	mtdcr	ebccfga,r3
	lis	r3,CFG_EBC_PB4AP@h
	ori	r3,r3,CFG_EBC_PB4AP@l
	mtdcr	ebccfgd,r3
	li	r3,pb4cr
	mtdcr	ebccfga,r3
	lis	r3,CFG_EBC_PB4CR@h
	ori	r3,r3,CFG_EBC_PB4CR@l
	mtdcr	ebccfgd,r3
#endif

	/*
	!-----------------------------------------------------------------------
	! Check to see if chip is in bypass mode.
	! If so, write stored CPC0_PLLMR0 and CPC0_PLLMR1 values and perform a
	! CPU reset   Otherwise, skip this step and keep going.
	! Note:	 Running BIOS in bypass mode is not supported since PLB speed
	!	 will not be fast enough for the SDRAM (min 66MHz)
	!-----------------------------------------------------------------------
	*/
	mfdcr	r5, CPC0_PLLMR1
	rlwinm	r4,r5,1,0x1	       /* get system clock source (SSCS) */
	cmpi	cr0,0,r4,0x1

	beq    pll_done			  /* if SSCS =b'1' then PLL has */
					  /* already been set */
					  /* and CPU has been reset */
					  /* so skip to next section */

#ifdef CONFIG_BUBINGA
	/*
	!-----------------------------------------------------------------------
	! Read NVRAM to get value to write in PLLMR.
	! If value has not been correctly saved, write default value
	! Default config values (assuming on-board 33MHz SYS_CLK) are above.
	! See CPU_DEFAULT_200 and CPU_DEFAULT_266 above.
	!
	! WARNING:  This code assumes the first three words in the nvram_t
	!	    structure in openbios.h.  Changing the beginning of
	!	    the structure will break this code.
	!
	!-----------------------------------------------------------------------
	*/
	addis	r3,0,NVRAM_BASE@h
	addi	r3,r3,NVRAM_BASE@l

	lwz	r4, 0(r3)
	addis	r5,0,NVRVFY1@h
	addi	r5,r5,NVRVFY1@l
	cmp	cr0,0,r4,r5	       /* Compare 1st NVRAM Magic number*/
	bne	..no_pllset
	addi	r3,r3,4
	lwz	r4, 0(r3)
	addis	r5,0,NVRVFY2@h
	addi	r5,r5,NVRVFY2@l
	cmp	cr0,0,r4,r5	       /* Compare 2 NVRAM Magic number */
	bne	..no_pllset
	addi	r3,r3,8			/* Skip over conf_size */
	lwz	r4, 4(r3)		/* Load PLLMR1 value from NVRAM */
	lwz	r3, 0(r3)		/* Load PLLMR0 value from NVRAM */
	rlwinm	r5,r4,1,0x1		/* get system clock source (SSCS) */
	cmpi	 cr0,0,r5,1		/* See if PLL is locked */
	beq	pll_write
..no_pllset:
#endif /* CONFIG_BUBINGA */

#ifdef CONFIG_TAIHU
	mfdcr	r4, CPC0_BOOT
	andi.	r5, r4, CPC0_BOOT_SEP@l
	bne	strap_1			/* serial eeprom present */
	addis	r5,0,CPLD_REG0_ADDR@h
	ori	r5,r5,CPLD_REG0_ADDR@l
	andi.	r5, r5, 0x10
	bne	_pci_66mhz
#endif /* CONFIG_TAIHU */

#if defined(CONFIG_ZEUS)
	mfdcr	r4, CPC0_BOOT
	andi.	r5, r4, CPC0_BOOT_SEP@l
	bne	strap_1  	/* serial eeprom present */
	lis	r3,0x0000
	addi	r3,r3,0x3030
	lis	r4,0x8042
	addi	r4,r4,0x223e
	b	1f
strap_1:
	mfdcr	r3, CPC0_PLLMR0
	mfdcr	r4, CPC0_PLLMR1
	b	1f
#endif

	addis	r3,0,PLLMR0_DEFAULT@h	    /* PLLMR0 default value */
	ori	r3,r3,PLLMR0_DEFAULT@l	   /* */
	addis	r4,0,PLLMR1_DEFAULT@h	    /* PLLMR1 default value */
	ori	r4,r4,PLLMR1_DEFAULT@l	   /* */

#ifdef CONFIG_TAIHU
	b	1f
_pci_66mhz:
	addis	r3,0,PLLMR0_DEFAULT_PCI66@h
	ori	r3,r3,PLLMR0_DEFAULT_PCI66@l
	addis	r4,0,PLLMR1_DEFAULT_PCI66@h
	ori	r4,r4,PLLMR1_DEFAULT_PCI66@l
	b	1f
strap_1:
	mfdcr	r3, CPC0_PLLMR0
	mfdcr	r4, CPC0_PLLMR1
#endif /* CONFIG_TAIHU */

1:
	b	pll_write		  /* Write the CPC0_PLLMR with new value */

pll_done:
	/*
	!-----------------------------------------------------------------------
	! Clear Soft Reset Register
	! This is needed to enable PCI if not booting from serial EPROM
	!-----------------------------------------------------------------------
		*/
	addi	r3, 0, 0x0
	mtdcr	CPC0_SRR, r3

	addis	 r3,0,0x0010
	mtctr	r3
pci_wait:
	bdnz	pci_wait

	blr				  /* return to main code */

/*
!-----------------------------------------------------------------------------
! Function:	pll_write
! Description:	Updates the value of the CPC0_PLLMR according to CMOS27E documentation
!		That is:
!			  1.  Pll is first disabled (de-activated by putting in bypass mode)
!			  2.  PLL is reset
!			  3.  Clock dividers are set while PLL is held in reset and bypassed
!			  4.  PLL Reset is cleared
!			  5.  Wait 100us for PLL to lock
!			  6.  A core reset is performed
! Input: r3 = Value to write to CPC0_PLLMR0
! Input: r4 = Value to write to CPC0_PLLMR1
! Output r3 = none
!-----------------------------------------------------------------------------
*/
pll_write:
	mfdcr  r5, CPC0_UCR
	andis. r5,r5,0xFFFF
	ori    r5,r5,0x0101		 /* Stop the UART clocks */
	mtdcr  CPC0_UCR,r5		 /* Before changing PLL */

	mfdcr  r5, CPC0_PLLMR1
	rlwinm r5,r5,0,0x7FFFFFFF	 /* Disable PLL */
	mtdcr	CPC0_PLLMR1,r5
	oris   r5,r5,0x4000		 /* Set PLL Reset */
	mtdcr	CPC0_PLLMR1,r5

	mtdcr	CPC0_PLLMR0,r3		 /* Set clock dividers */
	rlwinm r5,r4,0,0x3FFFFFFF	 /* Reset & Bypass new PLL dividers */
	oris   r5,r5,0x4000		 /* Set PLL Reset */
	mtdcr	CPC0_PLLMR1,r5		 /* Set clock dividers */
	rlwinm r5,r5,0,0xBFFFFFFF	 /* Clear PLL Reset */
	mtdcr	CPC0_PLLMR1,r5

		/*
	! Wait min of 100us for PLL to lock.
	! See CMOS 27E databook for more info.
	! At 200MHz, that means waiting 20,000 instructions
		 */
	addi	r3,0,20000		/* 2000 = 0x4e20 */
	mtctr	r3
pll_wait:
	bdnz	pll_wait

	oris   r5,r5,0x8000		/* Enable PLL */
	mtdcr	CPC0_PLLMR1,r5		/* Engage */

	/*
	 * Reset CPU to guarantee timings are OK
	 * Not sure if this is needed...
	 */
	addis r3,0,0x1000
	mtspr dbcr0,r3		     /* This will cause a CPU core reset, and */
				     /* execution will continue from the poweron */
				     /* vector of 0xfffffffc */
#endif /* CONFIG_405EP */

#if defined(CONFIG_440)
/*----------------------------------------------------------------------------+
| mttlb3.
+----------------------------------------------------------------------------*/
	function_prolog(mttlb3)
	TLBWE(4,3,2)
	blr
	function_epilog(mttlb3)

/*----------------------------------------------------------------------------+
| mftlb3.
+----------------------------------------------------------------------------*/
	function_prolog(mftlb3)
	TLBRE(3,3,2)
	blr
	function_epilog(mftlb3)

/*----------------------------------------------------------------------------+
| mttlb2.
+----------------------------------------------------------------------------*/
	function_prolog(mttlb2)
	TLBWE(4,3,1)
	blr
	function_epilog(mttlb2)

/*----------------------------------------------------------------------------+
| mftlb2.
+----------------------------------------------------------------------------*/
	function_prolog(mftlb2)
	TLBRE(3,3,1)
	blr
	function_epilog(mftlb2)

/*----------------------------------------------------------------------------+
| mttlb1.
+----------------------------------------------------------------------------*/
	function_prolog(mttlb1)
	TLBWE(4,3,0)
	blr
	function_epilog(mttlb1)

/*----------------------------------------------------------------------------+
| mftlb1.
+----------------------------------------------------------------------------*/
	function_prolog(mftlb1)
	TLBRE(3,3,0)
	blr
	function_epilog(mftlb1)
#endif /* CONFIG_440 */
