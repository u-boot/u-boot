/*
 *  Copyright (C) 1998	Dan Malek <dmalek@jlc.net>
 *  Copyright (C) 1999	Magnus Damm <kieraypc01.p.y.kie.era.ericsson.se>
 *  Copyright (C) 2000,2001,2002 Wolfgang Denk <wd@denx.de>
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
/*------------------------------------------------------------------------------+ */
/* */
/*	 This source code has been made available to you by IBM on an AS-IS */
/*	 basis.	 Anyone receiving this source is licensed under IBM */
/*	 copyrights to use it in any way he or she deems fit, including */
/*	 copying it, modifying it, compiling it, and redistributing it either */
/*	 with or without modifications.	 No license under IBM patents or */
/*	 patent applications is to be implied by the copyright license. */
/* */
/*	 Any user of this software should understand that IBM cannot provide */
/*	 technical support for this software and will not be responsible for */
/*	 any consequences resulting from the use of this software. */
/* */
/*	 Any person who transfers this source code or any derivative work */
/*	 must include the IBM copyright notice, this paragraph, and the */
/*	 preceding two paragraphs in the transferred software. */
/* */
/*	 COPYRIGHT   I B M   CORPORATION 1995 */
/*	 LICENSED MATERIAL  -  PROGRAM PROPERTY OF I B M */
/*------------------------------------------------------------------------------- */

/*  U-Boot - Startup Code for IBM 4xx PowerPC based Embedded Boards
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
#include <mpc8xx.h>
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

/* We don't want the  MMU yet.
*/
#undef	MSR_KERNEL
#define MSR_KERNEL ( MSR_ME  )	/* Machine Check */


	.extern ext_bus_cntlr_init
	.extern sdram_init

/*
 * Set up GOT: Global Offset Table
 *
 * Use r14 to access the GOT
 */
	START_GOT
	GOT_ENTRY(_GOT2_TABLE_)
	GOT_ENTRY(_FIXUP_TABLE_)

	GOT_ENTRY(_start)
	GOT_ENTRY(_start_of_vectors)
	GOT_ENTRY(_end_of_vectors)
	GOT_ENTRY(transfer_to_handler)

	GOT_ENTRY(_end)
	GOT_ENTRY(.bss)
	END_GOT

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
    .section .bootpg,"ax"
    .globl _start_440

/**************************************************************************/
_start_440:
	/*----------------------------------------------------------------*/
	/* Clear and set up some registers. */
	/*----------------------------------------------------------------*/
	iccci	r0,r0           /* NOTE: operands not used for 440 */
	dccci	r0,r0           /* NOTE: operands not used for 440 */
	sync
	li	r0,0
	mtspr	srr0,r0
	mtspr	srr1,r0
	mtspr	csrr0,r0
	mtspr	csrr1,r0

	/*----------------------------------------------------------------*/
	/* Initialize debug */
	/*----------------------------------------------------------------*/
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
	/* Setup interrupt vectors */
	/*----------------------------------------------------------------*/
	mtspr	ivpr,r0		/* Vectors start at 0x0000_0000 */
	li      r1,0x0100
	mtspr	ivor0,r1	/* Critical input */
	li      r1,0x0200
	mtspr	ivor1,r1	/* Machine check */
	li      r1,0x0300
	mtspr	ivor2,r1	/* Data storage */
	li      r1,0x0400
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
	li	r1,0x1000
	mtspr	ivor10,r1	/* Decrementer (PIT for 440) */
	li	r1,0x1400
	mtspr	ivor13,r1	/* Data TLB error */
	li	r1,0x1300
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

	/*----------------------------------------------------------------*/
	/* Clear all TLB entries -- TID = 0, TS = 0 */
	/*----------------------------------------------------------------*/
	mtspr	mmucr,r0
	li	r1,0x003f	/* 64 TLB entries */
	mtctr	r1
0:	tlbwe	r0,r1,0x0000	/* Invalidate all entries (V=0)*/
	subi	r1,r1,0x0001
	bdnz	0b

	/*----------------------------------------------------------------*/
	/* TLB entry setup -- step thru tlbtab */
	/*----------------------------------------------------------------*/
	bl	tlbtab		/* Get tlbtab pointer */
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
2:	bl	3f
	b	_start

3:	li	r0,0
	mtspr	srr1,r0		/* Keep things disabled for now */
	mflr	r1
	mtspr	srr0,r1
	rfi
#endif

/*
 * r3 - 1st arg to board_init(): IMMP pointer
 * r4 - 2nd arg to board_init(): boot flag
 */
	.text
	.long	0x27051956		/* U-Boot Magic Number			*/
	.globl	version_string
version_string:
	.ascii U_BOOT_VERSION
	.ascii " (", __DATE__, " - ", __TIME__, ")"
	.ascii CONFIG_IDENT_STRING, "\0"

/*
 * Maybe this should be moved somewhere else because the current
 * location (0x100) is where the CriticalInput Execption should be.
 */
	. = EXC_OFF_SYS_RESET
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
	lis	r1,0x0002		/* set CE bit (Critical Exceptions) */
	ori	r1,r1,0x1000		/* set ME bit (Machine Exceptions) */
	mtmsr	r1			/* change MSR */

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
	mtdcr	isram0_sb1cr,r0		/* Disable bank 1 */

	li	r2,0x7fff
	ori	r2,r2,0xffff
	mfdcr	r1,isram0_dpc
	and	r1,r1,r2		/* Disable parity check */
	mtdcr	isram0_dpc,r1
	mfdcr	r1,isram0_pmeg
	andis.	r1,r1,r2		/* Disable pwr mgmt */
	mtdcr	isram0_pmeg,r1

	lis	r1,0x8000		/* BAS = 8000_0000 */
	ori	r1,r1,0x0380		/* 8k rw */
	mtdcr	isram0_sb0cr,r1

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

	GET_GOT
	bl	board_init_f

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
	addi	r4,r0,0x1000		/* set ME bit (Machine Exceptions) */
	oris	r4,r4,0x2		/* set CE bit (Critical Exceptions) */
	mtmsr	r4			/* change MSR */
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
	addis	r1,r0,0x8000
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
#if defined(CONFIG_405GP) || defined(CONFIG_405CR) || defined(CONFIG_405)
	/*----------------------------------------------------------------------- */
	/* Clear and set up some registers. */
	/*----------------------------------------------------------------------- */
	addi	r4,r0,0x0000
	mtspr	sgr,r4
	mtspr	dcwr,r4
	mtesr	r4			/* clear Exception Syndrome Reg */
	mttcr	r4			/* clear Timer Control Reg */
	mtxer	r4			/* clear Fixed-Point Exception Reg */
	mtevpr	r4			/* clear Exception Vector Prefix Reg */
	addi	r4,r0,0x1000		/* set ME bit (Machine Exceptions) */
	oris	r4,r4,0x0002		/* set CE bit (Critical Exceptions) */
	mtmsr	r4			/* change MSR */
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
	addis	r4,r0,0x8000
	addi	r4,r4,0x0001
	mticcr	r4			/* instruction cache */
	isync

	addis	r4,r0,0x0000
	addi	r4,r4,0x0000
	mtdccr	r4			/* data cache */

#if !(defined(CFG_EBC_PB0AP) && defined(CFG_EBC_PB0CR))
	/*----------------------------------------------------------------------- */
	/* Tune the speed and size for flash CS0  */
	/*----------------------------------------------------------------------- */
	bl	ext_bus_cntlr_init
#endif

#if defined(CFG_OCM_DATA_ADDR) && defined(CFG_OCM_DATA_SIZE)
	/********************************************************************
	 * Setup OCM - On Chip Memory
	 *******************************************************************/
	/* Setup OCM */
 	lis	r0, 0x7FFF
 	ori	r0, r0, 0xFFFF
 	mfdcr	r3, ocmiscntl 		/* get instr-side IRAM config */
 	mfdcr	r4, ocmdscntl	/* get data-side IRAM config */
 	and	r3, r3, r0	/* disable data-side IRAM */
 	and	r4, r4, r0	/* disable data-side IRAM */
 	mtdcr	ocmiscntl, r3	/* set instr-side IRAM config */
 	mtdcr	ocmdscntl, r4	/* set data-side IRAM config */
 	isync

	addis	r3, 0, CFG_OCM_DATA_ADDR@h /* OCM location */
	mtdcr	ocmdsarc, r3
	addis	r4, 0, 0xC000		/* OCM data area enabled */
	mtdcr	ocmdscntl, r4
 	isync
#endif

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

	/* turn on data chache for this region */
	lis	r4,0x0080
	mtdccr	r4

	/* set stack pointer and clear stack to known value */

	lis	r1,CFG_INIT_RAM_ADDR@h
	ori     r1,r1,CFG_INIT_SP_OFFSET@l

	li	r4,2048			/* we store 2048 words to stack */
	mtctr	r4

	lis	r2,CFG_INIT_RAM_ADDR@h		/* we also clear data area */
	ori	r2,r2,CFG_INIT_RAM_END@l 	/* so cant copy value from r1 */

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
	/* Initialize SDRAM Controller  */
	/*----------------------------------------------------------------------- */
	bl	sdram_init

	/*
	 * Setup temporary stack pointer only for boards
	 * that do not use SDRAM SPD I2C stuff since it
	 * is already initialized to use DCACHE or OCM
	 * stacks.
	 */
#if !(defined(CFG_INIT_DCACHE_CS) || defined(CFG_TEMP_STACK_OCM))
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
#endif /* !(CFG_INIT_DCACHE_CS  || !CFG_TEM_STACK_OCM) */

	GET_GOT			/* initialize GOT access			*/

       	bl	cpu_init_f	/* run low-level CPU init code     (from Flash)	*/

	/* NEVER RETURNS! */
	bl	board_init_f	/* run first part of init code (from Flash)	*/

#endif	/* CONFIG_405GP || CONFIG_405CR */


	.globl	_start_of_vectors
_start_of_vectors:

#if 0
/*TODO Fixup _start above so we can do this*/
/* Critical input. */
	CRIT_EXCEPTION(0x100, CritcalInput, CritcalInputException)
#endif

/* Machine check */
	STD_EXCEPTION(0x200, MachineCheck, MachineCheckException)

/* Data Storage exception. */
	STD_EXCEPTION(0x300, DataStorage, UnknownException)

/* Instruction Storage exception. */
	STD_EXCEPTION(0x400, InstStorage, UnknownException)

/* External Interrupt exception. */
	STD_EXCEPTION(0x500, ExtInterrupt, external_interrupt)

/* Alignment exception. */
	. = 0x600
Alignment:
	EXCEPTION_PROLOG
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
	.long	AlignmentException - _start + EXC_OFF_SYS_RESET
	.long	int_return - _start + EXC_OFF_SYS_RESET

/* Program check exception */
	. = 0x700
ProgramCheck:
	EXCEPTION_PROLOG
	addi	r3,r1,STACK_FRAME_OVERHEAD
	li	r20,MSR_KERNEL
	rlwimi	r20,r23,0,16,16		/* copy EE bit from saved MSR */
	lwz	r6,GOT(transfer_to_handler)
	mtlr	r6
	blrl
.L_ProgramCheck:
	.long	ProgramCheckException - _start + EXC_OFF_SYS_RESET
	.long	int_return - _start + EXC_OFF_SYS_RESET

	/* No FPU on MPC8xx.  This exception is not supposed to happen.
	*/
	STD_EXCEPTION(0x800, FPUnavailable, UnknownException)

	/* I guess we could implement decrementer, and may have
	 * to someday for timekeeping.
	 */
	STD_EXCEPTION(0x900, Decrementer, timer_interrupt)
	STD_EXCEPTION(0xa00, Trap_0a, UnknownException)
	STD_EXCEPTION(0xb00, Trap_0b, UnknownException)

	. = 0xc00
/*
 * r0 - SYSCALL number
 * r3-... arguments
 */
SystemCall:
	addis	r11,r0,0		/* get functions table addr */
	ori	r11,r11,0		/* Note: this code is patched in trap_init */
	addis	r12,r0,0		/* get number of functions */
	ori	r12,r12,0

	cmplw	0, r0, r12
	bge	1f

	rlwinm	r0,r0,2,0,31		/* fn_addr = fn_tbl[r0] */
	add	r11,r11,r0
	lwz	r11,0(r11)

	li	r20,0xd00-4		/* Get stack pointer */
	lwz	r12,0(r20)
	subi	r12,r12,12		/* Adjust stack pointer */
	li	r0,0xc00+_end_back-SystemCall
	cmplw	0, r0, r12		/* Check stack overflow */
	bgt	1f
	stw	r12,0(r20)

	mflr	r0
	stw	r0,0(r12)
	mfspr	r0,SRR0
	stw	r0,4(r12)
	mfspr	r0,SRR1
	stw	r0,8(r12)

	li	r12,0xc00+_back-SystemCall
	mtlr	r12
	mtspr	SRR0,r11

1:	SYNC
	rfi

_back:

	mfmsr	r11			/* Disable interrupts */
	li	r12,0
	ori	r12,r12,MSR_EE
	andc	r11,r11,r12
	SYNC				/* Some chip revs need this... */
	mtmsr	r11
	SYNC

	li	r12,0xd00-4		/* restore regs */
	lwz	r12,0(r12)

	lwz	r11,0(r12)
	mtlr	r11
	lwz	r11,4(r12)
	mtspr	SRR0,r11
	lwz	r11,8(r12)
	mtspr	SRR1,r11

	addi	r12,r12,12		/* Adjust stack pointer */
	li	r20,0xd00-4
	stw	r12,0(r20)

	SYNC
	rfi
_end_back:

	STD_EXCEPTION(0xd00, SingleStep, UnknownException)

	STD_EXCEPTION(0xe00, Trap_0e, UnknownException)
	STD_EXCEPTION(0xf00, Trap_0f, UnknownException)

	/* On the MPC8xx, this is a software emulation interrupt.  It occurs
	 * for all unimplemented and illegal instructions.
	 */
	STD_EXCEPTION(0x1000, PIT, PITException)

	STD_EXCEPTION(0x1100, InstructionTLBMiss, UnknownException)
	STD_EXCEPTION(0x1200, DataTLBMiss, UnknownException)
	STD_EXCEPTION(0x1300, InstructionTLBError, UnknownException)
	STD_EXCEPTION(0x1400, DataTLBError, UnknownException)

	STD_EXCEPTION(0x1500, Reserved5, UnknownException)
	STD_EXCEPTION(0x1600, Reserved6, UnknownException)
	STD_EXCEPTION(0x1700, Reserved7, UnknownException)
	STD_EXCEPTION(0x1800, Reserved8, UnknownException)
	STD_EXCEPTION(0x1900, Reserved9, UnknownException)
	STD_EXCEPTION(0x1a00, ReservedA, UnknownException)
	STD_EXCEPTION(0x1b00, ReservedB, UnknownException)

	STD_EXCEPTION(0x1c00, DataBreakpoint, UnknownException)
	STD_EXCEPTION(0x1d00, InstructionBreakpoint, UnknownException)
	STD_EXCEPTION(0x1e00, PeripheralBreakpoint, UnknownException)
	STD_EXCEPTION(0x1f00, DevPortBreakpoint, UnknownException)

	CRIT_EXCEPTION(0x2000, DebugBreakpoint, DebugException )

	.globl	_end_of_vectors
_end_of_vectors:


	. = 0x2100

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
#if 0
	andi.	r23,r23,MSR_PR
	mfspr	r23,SPRG3		/* if from user, fix up tss.regs */
	beq	2f
	addi	r24,r1,STACK_FRAME_OVERHEAD
	stw	r24,PT_REGS(r23)
2:	addi	r2,r23,-TSS		/* set r2 to current */
	tovirt(r2,r2,r23)
#endif
	mflr	r23
	andi.	r24,r23,0x3f00		/* get vector offset */
	stw	r24,TRAP(r21)
	li	r22,0
	stw	r22,RESULT(r21)
	mtspr	SPRG2,r22		/* r1 is now kernel sp */
#if 0
	addi	r24,r2,TASK_STRUCT_SIZE /* check for kernel stack overflow */
	cmplw	0,r1,r2
	cmplw	1,r1,r24
	crand	1,1,4
	bgt	stack_ovf		/* if r2 < r1 < r2+TASK_STRUCT_SIZE */
#endif
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
	mtspr	990,r2		/* SRR2 */
	mtspr	991,r0		/* SRR3 */
	lwz	r0,GPR0(r1)
	lwz	r2,GPR2(r1)
	lwz	r1,GPR1(r1)
	SYNC
	rfci

/* Cache functions.
*/
invalidate_icache:
	iccci	r0,r0			/* for 405, iccci invalidates the */
	blr				/*   entire I cache */

invalidate_dcache:
	addi	r6,0,0x0000		/* clear GPR 6 */
	/* Do loop for # of dcache congruence classes. */
	addi	r7,r0, (CFG_DCACHE_SIZE / CFG_CACHELINE_SIZE / 2)
					/* NOTE: dccci invalidates both */
	mtctr	r7			/* ways in the D cache */
..dcloop:
	dccci	0,r6			/* invalidate line */
	addi	r6,r6, CFG_CACHELINE_SIZE /* bump to next line */
	bdnz	..dcloop
	blr

flush_dcache:
	addis	r9,r0,0x0002		/* set mask for EE and CE msr bits */
	ori	r9,r9,0x8000
	mfmsr	r12			/* save msr */
	andc	r9,r12,r9
	mtmsr	r9			/* disable EE and CE */
	addi	r10,r0,0x0001		/* enable data cache for unused memory */
	mfdccr	r9			/* region 0xF8000000-0xFFFFFFFF via */
	or	r10,r10,r9		/* bit 31 in dccr */
	mtdccr	r10

	/* do loop for # of congruence classes. */
	addi	r10,r0,(CFG_DCACHE_SIZE / CFG_CACHELINE_SIZE / 2)
	addi	r11,r0,(CFG_DCACHE_SIZE / 2) /* D cache set size - 2 way sets */
	mtctr	r10
	addi	r10,r0,(0xE000-0x10000)	/* start at 0xFFFFE000 */
	add	r11,r10,r11		/* add to get to other side of cache line */
..flush_dcache_loop:
	lwz	r3,0(r10)		/* least recently used side */
	lwz	r3,0(r11)		/* the other side */
	dccci	r0,r11			/* invalidate both sides */
	addi	r10,r10,CFG_CACHELINE_SIZE /* bump to next line */
	addi	r11,r11,CFG_CACHELINE_SIZE /* bump to next line */
	bdnz	..flush_dcache_loop
	sync				/* allow memory access to complete */
	mtdccr	r9			/* restore dccr */
	mtmsr	r12			/* restore msr */
	blr

	.globl	icache_enable
icache_enable:
	mflr	r8
	bl	invalidate_icache
	mtlr	r8
	isync
	addis	r3,r0, 0x8000	      /* set bit 0 */
	mticcr	r3
	blr

	.globl	icache_disable
icache_disable:
	addis	r3,r0, 0x0000	      /* clear bit 0 */
	mticcr	r3
	isync
	blr

	.globl	icache_status
icache_status:
	mficcr	r3
	srwi	r3, r3, 31	/* >>31 => select bit 0 */
	blr

	.globl	dcache_enable
dcache_enable:
	mflr	r8
	bl	invalidate_dcache
	mtlr	r8
	isync
	addis	r3,r0, 0x8000	      /* set bit 0 */
	mtdccr	r3
	blr

	.globl	dcache_disable
dcache_disable:
	mflr	r8
	bl	flush_dcache
	mtlr	r8
	addis	r3,r0, 0x0000	      /* clear bit 0 */
	mtdccr	r3
	blr

	.globl	dcache_status
dcache_status:
	mfdccr	r3
	srwi	r3, r3, 31	/* >>31 => select bit 0 */
	blr

	.globl get_pvr
get_pvr:
	mfspr	r3, PVR
	blr

#if !defined(CONFIG_440)
	.globl wr_pit
wr_pit:
	mtspr	pit, r3
	blr
#endif

	.globl wr_tcr
wr_tcr:
	mtspr	tcr, r3
	blr

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
/* Function:	 out32 */
/* Description:	 Output 32 bits */
/*------------------------------------------------------------------------------- */
	.globl	out32
out32:
	stw	r4,0x0000(r3)
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
/* Function:	 in32 */
/* Description:	 Input 32 bits */
/*------------------------------------------------------------------------------- */
	.globl	in32
in32:
	lwz	3,0x0000(3)
	blr

/*------------------------------------------------------------------------------- */
/* Function:	 in32r */
/* Description:	 Input 32 bits and byte reverse */
/*------------------------------------------------------------------------------- */
	.globl	in32r
in32r:
	lwbrx	r3,r0,r3
	blr

/*------------------------------------------------------------------------------- */
/* Function:	 ppcDcbf */
/* Description:	 Data Cache block flush */
/* Input:	 r3 = effective address */
/* Output:	 none. */
/*------------------------------------------------------------------------------- */
	.globl	ppcDcbf
ppcDcbf:
	dcbf	r0,r3
	blr

/*------------------------------------------------------------------------------- */
/* Function:	 ppcDcbi */
/* Description:	 Data Cache block Invalidate */
/* Input:	 r3 = effective address */
/* Output:	 none. */
/*------------------------------------------------------------------------------- */
	.globl	ppcDcbi
ppcDcbi:
	dcbi	r0,r3
	blr

/*------------------------------------------------------------------------------- */
/* Function:	 ppcSync */
/* Description:	 Processor Synchronize */
/* Input:	 none. */
/* Output:	 none. */
/*------------------------------------------------------------------------------- */
	.globl	ppcSync
ppcSync:
	sync
	blr

/*------------------------------------------------------------------------------*/

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
	mr	r1,  r3		/* Set new stack pointer		*/
	mr	r9,  r4		/* Save copy of Init Data pointer	*/
	mr	r10, r5		/* Save copy of Destination Address	*/

	mr	r3,  r5				/* Destination Address	*/
	lis	r4, CFG_MONITOR_BASE@h		/* Source      Address	*/
	ori	r4, r4, CFG_MONITOR_BASE@l
	lis	r5, CFG_MONITOR_LEN@h		/* Length in Bytes	*/
	ori	r5, r5, CFG_MONITOR_LEN@l
	li	r6, CFG_CACHELINE_SIZE		/* Cache Line Size	*/

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

	addi	r0, r10, in_ram - _start + EXC_OFF_SYS_RESET
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
	lwz	r3,GOT(.bss)
	lwz	r4,GOT(_end)

	cmplw	0, r3, r4
	beq	6f

	li	r0, 0
5:
	stw	r0, 0(r3)
	addi	r3, r3, 4
	cmplw	0, r3, r4
	bne	5b
6:

	mr	r3, r9		/* Init Data pointer		*/
	mr	r4, r10		/* Destination Address		*/
	bl	board_init_r

	/* Problems accessing "end" in C, so do it here */
	.globl	get_endaddr
get_endaddr:
	lwz	r3,GOT(_end)
	blr

	/*
	 * Copy exception vector code to low memory
	 *
	 * r3: dest_addr
	 * r7: source address, r8: end address, r9: target address
	 */
	.globl	trap_init
trap_init:
	lwz	r7, GOT(_start)
	lwz	r8, GOT(_end_of_vectors)

	rlwinm	r9, r7, 0, 18, 31	/* _start & 0x3FFF	*/

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
	li	r7, .L_MachineCheck - _start + EXC_OFF_SYS_RESET
	li	r8, Alignment - _start + EXC_OFF_SYS_RESET
2:
	bl	trap_reloc
	addi	r7, r7, 0x100		/* next exception vector	*/
	cmplw	0, r7, r8
	blt	2b

	li	r7, .L_Alignment - _start + EXC_OFF_SYS_RESET
	bl	trap_reloc

	li	r7, .L_ProgramCheck - _start + EXC_OFF_SYS_RESET
	bl	trap_reloc

	li	r7, .L_FPUnavailable - _start + EXC_OFF_SYS_RESET
	li	r8, SystemCall - _start + EXC_OFF_SYS_RESET
3:
	bl	trap_reloc
	addi	r7, r7, 0x100		/* next exception vector	*/
	cmplw	0, r7, r8
	blt	3b

	li	r7, .L_SingleStep - _start + EXC_OFF_SYS_RESET
	li	r8, _end_of_vectors - _start + EXC_OFF_SYS_RESET
4:
	bl	trap_reloc
	addi	r7, r7, 0x100		/* next exception vector	*/
	cmplw	0, r7, r8
	blt	4b

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
