/*
 * Copyright (C) 2003	Josef Baumgartner <josef.baumgartner@telex.de>
 * Based on code from Bernhard Kuhn <bkuhn@metrowerks.com>
 *
 * Copyright 2010-2012 Freescale Semiconductor, Inc.
 * TsiChung Liew (Tsi-Chung.Liew@freescale.com)
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <asm-offsets.h>
#include <config.h>
#include <timestamp.h>
#include "version.h"
#include <asm/cache.h>

#ifndef	 CONFIG_IDENT_STRING
#define	 CONFIG_IDENT_STRING ""
#endif

#define _START	_start
#define _FAULT	_fault

#define SAVE_ALL						\
	move.w	#0x2700,%sr;		/* disable intrs */	\
	subl	#60,%sp;		/* space for 15 regs */ \
	moveml	%d0-%d7/%a0-%a6,%sp@;

#define RESTORE_ALL						\
	moveml	%sp@,%d0-%d7/%a0-%a6;				\
	addl	#60,%sp;		/* space for 15 regs */ \
	rte;

#if defined(CONFIG_SERIAL_BOOT)
#define ASM_DRAMINIT	(asm_dram_init - CONFIG_SYS_TEXT_BASE + CONFIG_SYS_INIT_RAM_ADDR)
#define ASM_DRAMINIT_N	(asm_dram_init - CONFIG_SYS_TEXT_BASE)
#define ASM_SBF_IMG_HDR	(asm_sbf_img_hdr - CONFIG_SYS_TEXT_BASE + CONFIG_SYS_INIT_RAM_ADDR)
#endif

.text

/*
 *	Vector table. This is used for initial platform startup.
 *	These vectors are to catch any un-intended traps.
 */
_vectors:
#if defined(CONFIG_SERIAL_BOOT)

INITSP:	.long	0		/* Initial SP	*/
#ifdef CONFIG_CF_SBF
INITPC:	.long	ASM_DRAMINIT	/* Initial PC 	*/
#endif
#ifdef CONFIG_SYS_NAND_BOOT
INITPC:	.long	ASM_DRAMINIT_N	/* Initial PC 	*/
#endif

#else

INITSP:		.long	0	/* Initial SP	*/
INITPC:		.long	_START	/* Initial PC 		*/

#endif

vector02:	.long	_FAULT	/* Access Error		*/
vector03:	.long	_FAULT	/* Address Error	*/
vector04:	.long	_FAULT	/* Illegal Instruction	*/
vector05:	.long	_FAULT	/* Reserved		*/
vector06:	.long	_FAULT	/* Reserved		*/
vector07:	.long	_FAULT	/* Reserved		*/
vector08:	.long	_FAULT	/* Privilege Violation	*/
vector09:	.long	_FAULT	/* Trace		*/
vector0A:	.long	_FAULT	/* Unimplemented A-Line	*/
vector0B:	.long	_FAULT	/* Unimplemented F-Line	*/
vector0C:	.long	_FAULT	/* Debug Interrupt	*/
vector0D:	.long	_FAULT	/* Reserved		*/
vector0E:	.long	_FAULT	/* Format Error		*/
vector0F:	.long	_FAULT	/* Unitialized Int.	*/

/* Reserved */
vector10_17:
.long	_FAULT, _FAULT, _FAULT, _FAULT, _FAULT, _FAULT, _FAULT, _FAULT

vector18:	.long	_FAULT	/* Spurious Interrupt	*/
vector19:	.long	_FAULT	/* Autovector Level 1	*/
vector1A:	.long	_FAULT	/* Autovector Level 2	*/
vector1B:	.long	_FAULT	/* Autovector Level 3	*/
vector1C:	.long	_FAULT	/* Autovector Level 4	*/
vector1D:	.long	_FAULT	/* Autovector Level 5	*/
vector1E:	.long	_FAULT	/* Autovector Level 6	*/
vector1F:	.long	_FAULT	/* Autovector Level 7	*/

#if !defined(CONFIG_SERIAL_BOOT)

/* TRAP #0 - #15 */
vector20_2F:
.long	_FAULT, _FAULT, _FAULT, _FAULT, _FAULT, _FAULT, _FAULT, _FAULT
.long	_FAULT, _FAULT, _FAULT, _FAULT, _FAULT, _FAULT, _FAULT, _FAULT

/* Reserved	*/
vector30_3F:
.long	_FAULT, _FAULT, _FAULT, _FAULT, _FAULT, _FAULT, _FAULT, _FAULT
.long	_FAULT, _FAULT, _FAULT, _FAULT, _FAULT, _FAULT, _FAULT, _FAULT

vector64_127:
.long	_FAULT, _FAULT, _FAULT, _FAULT, _FAULT, _FAULT, _FAULT, _FAULT
.long	_FAULT, _FAULT, _FAULT, _FAULT, _FAULT, _FAULT, _FAULT, _FAULT
.long	_FAULT, _FAULT, _FAULT, _FAULT, _FAULT, _FAULT, _FAULT, _FAULT
.long	_FAULT, _FAULT, _FAULT, _FAULT, _FAULT, _FAULT, _FAULT, _FAULT
.long	_FAULT, _FAULT, _FAULT, _FAULT, _FAULT, _FAULT, _FAULT, _FAULT
.long	_FAULT, _FAULT, _FAULT, _FAULT, _FAULT, _FAULT, _FAULT, _FAULT
.long	_FAULT, _FAULT, _FAULT, _FAULT, _FAULT, _FAULT, _FAULT, _FAULT
.long	_FAULT, _FAULT, _FAULT, _FAULT, _FAULT, _FAULT, _FAULT, _FAULT

vector128_191:
.long	_FAULT, _FAULT, _FAULT, _FAULT, _FAULT, _FAULT, _FAULT, _FAULT
.long	_FAULT, _FAULT, _FAULT, _FAULT, _FAULT, _FAULT, _FAULT, _FAULT
.long	_FAULT, _FAULT, _FAULT, _FAULT, _FAULT, _FAULT, _FAULT, _FAULT
.long	_FAULT, _FAULT, _FAULT, _FAULT, _FAULT, _FAULT, _FAULT, _FAULT
.long	_FAULT, _FAULT, _FAULT, _FAULT, _FAULT, _FAULT, _FAULT, _FAULT
.long	_FAULT, _FAULT, _FAULT, _FAULT, _FAULT, _FAULT, _FAULT, _FAULT
.long	_FAULT, _FAULT, _FAULT, _FAULT, _FAULT, _FAULT, _FAULT, _FAULT
.long	_FAULT, _FAULT, _FAULT, _FAULT, _FAULT, _FAULT, _FAULT, _FAULT

vector192_255:
.long	_FAULT, _FAULT, _FAULT, _FAULT, _FAULT, _FAULT, _FAULT, _FAULT
.long	_FAULT, _FAULT, _FAULT, _FAULT, _FAULT, _FAULT, _FAULT, _FAULT
.long	_FAULT, _FAULT, _FAULT, _FAULT, _FAULT, _FAULT, _FAULT, _FAULT
.long	_FAULT, _FAULT, _FAULT, _FAULT, _FAULT, _FAULT, _FAULT, _FAULT
.long	_FAULT, _FAULT, _FAULT, _FAULT, _FAULT, _FAULT, _FAULT, _FAULT
.long	_FAULT, _FAULT, _FAULT, _FAULT, _FAULT, _FAULT, _FAULT, _FAULT
.long	_FAULT, _FAULT, _FAULT, _FAULT, _FAULT, _FAULT, _FAULT, _FAULT
.long	_FAULT, _FAULT, _FAULT, _FAULT, _FAULT, _FAULT, _FAULT, _FAULT
#endif

#if defined(CONFIG_SERIAL_BOOT)
	/* Image header: chksum 4 bytes, len 4 bytes, img dest 4 bytes */
asm_sbf_img_hdr:
	.long	0x00000000	/* checksum, not yet implemented */
	.long	0x00040000	/* image length */
	.long	CONFIG_SYS_TEXT_BASE	/* image to be relocated at */

asm_dram_init:
	move.w #0x2700,%sr		/* Mask off Interrupt */

#ifdef CONFIG_SYS_NAND_BOOT
	/* for assembly stack */
	move.l	#(CONFIG_SYS_INIT_RAM_ADDR + CONFIG_SYS_INIT_RAM_CTRL), %d0
	movec	%d0, %RAMBAR1

	move.l	#(CONFIG_SYS_INIT_RAM_ADDR + CONFIG_SYS_INIT_SP_OFFSET), %sp
	clr.l %sp@-
#endif

#ifdef CONFIG_CF_SBF
	move.l	#CONFIG_SYS_INIT_RAM_ADDR, %d0
	movec	%d0, %VBR

	move.l	#(CONFIG_SYS_INIT_RAM_ADDR + CONFIG_SYS_INIT_RAM_CTRL), %d0
	movec	%d0, %RAMBAR1

	/* initialize general use internal ram */
	move.l #0, %d0
	move.l #(ICACHE_STATUS), %a1	/* icache */
	move.l #(DCACHE_STATUS), %a2	/* dcache */
	move.l %d0, (%a1)
	move.l %d0, (%a2)

	/* invalidate and disable cache */
	move.l	#(CONFIG_SYS_ICACHE_INV + CONFIG_SYS_DCACHE_INV), %d0
	movec	%d0, %CACR		/* Invalidate cache */
	move.l	#0, %d0
	movec	%d0, %ACR0
	movec	%d0, %ACR1
	movec	%d0, %ACR2
	movec	%d0, %ACR3

	move.l	#(CONFIG_SYS_INIT_RAM_ADDR + CONFIG_SYS_INIT_SP_OFFSET), %sp
	clr.l %sp@-

	/* Must disable global address */
	move.l	#0xFC008000, %a1
	move.l	#(CONFIG_SYS_CS0_BASE), (%a1)
	move.l	#0xFC008008, %a1
	move.l	#(CONFIG_SYS_CS0_CTRL), (%a1)
	move.l	#0xFC008004, %a1
	move.l	#(CONFIG_SYS_CS0_MASK), (%a1)
#endif			/* CONFIG_CF_SBF */

#ifdef CONFIG_MCF5441x
	/* TC: enable all peripherals,
	in the future only enable certain peripherals */
	move.l	#0xFC04002D, %a1

#if defined(CONFIG_CF_SBF)
	move.b	#23, (%a1)	/* dspi */
#endif
	move.b	#46, (%a1)	/* DDR */

	/* slew settings */
	move.l	#0xEC094060, %a1
	move.b	#0, (%a1)

	/* use vco instead of cpu*2 clock for ddr clock */
	move.l	#0xEC09001A, %a1
	move.w	#0xE01D, (%a1)

	/* DDR settings */
	move.l	#0xFC0B8180, %a1
	move.l	#0x00000000, (%a1)
	move.l	#0x40000000, (%a1)

	move.l	#0xFC0B81AC, %a1
	move.l	#0x01030203, (%a1)

	move.l	#0xFC0B8000, %a1
	move.l	#0x01010101, (%a1)+	/* 0x00 */
	move.l	#0x00000101, (%a1)+	/* 0x04 */
	move.l	#0x01010100, (%a1)+	/* 0x08 */
	move.l	#0x01010000, (%a1)+	/* 0x0C */
	move.l	#0x00010101, (%a1)+	/* 0x10 */
	move.l	#0xFC0B8018, %a1
	move.l	#0x00010100, (%a1)+	/* 0x18 */
	move.l	#0x00000001, (%a1)+	/* 0x1C */
	move.l	#0x01000001, (%a1)+	/* 0x20 */
	move.l	#0x00000100, (%a1)+	/* 0x24 */
	move.l	#0x00010001, (%a1)+	/* 0x28 */
	move.l	#0x00000200, (%a1)+	/* 0x2C */
	move.l	#0x01000002, (%a1)+	/* 0x30 */
	move.l	#0x00000000, (%a1)+	/* 0x34 */
	move.l	#0x00000100, (%a1)+	/* 0x38 */
	move.l	#0x02000100, (%a1)+	/* 0x3C */
	move.l	#0x02000407, (%a1)+	/* 0x40 */
	move.l	#0x02030007, (%a1)+	/* 0x44 */
	move.l	#0x02000100, (%a1)+	/* 0x48 */
	move.l	#0x0A030203, (%a1)+	/* 0x4C */
	move.l	#0x00020708, (%a1)+	/* 0x50 */
	move.l	#0x00050008, (%a1)+	/* 0x54 */
	move.l	#0x04030002, (%a1)+	/* 0x58 */
	move.l	#0x00000004, (%a1)+	/* 0x5C */
	move.l	#0x020A0000, (%a1)+	/* 0x60 */
	move.l	#0x0C00000E, (%a1)+	/* 0x64 */
	move.l	#0x00002004, (%a1)+	/* 0x68 */
	move.l	#0x00000000, (%a1)+	/* 0x6C */
	move.l	#0x00100010, (%a1)+	/* 0x70 */
	move.l	#0x00100010, (%a1)+	/* 0x74 */
	move.l	#0x00000000, (%a1)+	/* 0x78 */
	move.l	#0x07990000, (%a1)+	/* 0x7C */
	move.l	#0xFC0B80A0, %a1
	move.l	#0x00000000, (%a1)+	/* 0xA0 */
	move.l	#0x00C80064, (%a1)+	/* 0xA4 */
	move.l	#0x44520002, (%a1)+	/* 0xA8 */
	move.l	#0x00C80023, (%a1)+	/* 0xAC */
	move.l	#0xFC0B80B4, %a1
	move.l	#0x0000C350, (%a1)	/* 0xB4 */
	move.l	#0xFC0B80E0, %a1
	move.l	#0x04000000, (%a1)+	/* 0xE0 */
	move.l	#0x03000304, (%a1)+	/* 0xE4 */
	move.l	#0x40040000, (%a1)+	/* 0xE8 */
	move.l	#0xC0004004, (%a1)+	/* 0xEC */
	move.l	#0x0642C000, (%a1)+	/* 0xF0 */
	move.l	#0x00000642, (%a1)+	/* 0xF4 */
	move.l	#0xFC0B8024, %a1
	tpf
	move.l	#0x01000100, (%a1)	/* 0x24 */

	move.l	#0x2000, %d1
	jsr	asm_delay
#endif		/* CONFIG_MCF5441x */

#ifdef CONFIG_MCF5445x
	/* Dram Initialization a1, a2, and d0 */
	/* mscr sdram */
	move.l	#0xFC0A4074, %a1
	move.b	#(CONFIG_SYS_SDRAM_DRV_STRENGTH), (%a1)
	nop

	/* SDRAM Chip 0 and 1 */
	move.l	#0xFC0B8110, %a1
	move.l	#0xFC0B8114, %a2

	/* calculate the size */
	move.l	#0x13, %d1
	move.l	#(CONFIG_SYS_SDRAM_SIZE), %d2
#ifdef CONFIG_SYS_SDRAM_BASE1
	lsr.l	#1, %d2
#endif

dramsz_loop:
	lsr.l	#1, %d2
	add.l	#1, %d1
	cmp.l	#1, %d2
	bne	dramsz_loop
#ifdef CONFIG_SYS_NAND_BOOT
	beq	asm_nand_chk_status
#endif
	/* SDRAM Chip 0 and 1 */
	move.l	#(CONFIG_SYS_SDRAM_BASE), (%a1)
	or.l	%d1, (%a1)
#ifdef CONFIG_SYS_SDRAM_BASE1
	move.l	#(CONFIG_SYS_SDRAM_BASE1), (%a2)
	or.l	%d1, (%a2)
#endif
	nop

	/* dram cfg1 and cfg2 */
	move.l	#0xFC0B8008, %a1
	move.l	#(CONFIG_SYS_SDRAM_CFG1), (%a1)
	nop
	move.l	#0xFC0B800C, %a2
	move.l	#(CONFIG_SYS_SDRAM_CFG2), (%a2)
	nop

	move.l	#0xFC0B8000, %a1	/* Mode */
	move.l	#0xFC0B8004, %a2	/* Ctrl */

	/* Issue PALL */
	move.l	#(CONFIG_SYS_SDRAM_CTRL + 2), (%a2)
	nop

#ifdef CONFIG_M54455EVB
	/* Issue LEMR */
	move.l	#(CONFIG_SYS_SDRAM_EMOD + 0x408), (%a1)
	nop
	move.l	#(CONFIG_SYS_SDRAM_MODE + 0x300), (%a1)
	nop
#endif

	move.l	#1000, %d1
	jsr	asm_delay

	/* Issue PALL */
	move.l	#(CONFIG_SYS_SDRAM_CTRL + 2), (%a2)
	nop

	/* Perform two refresh cycles */
	move.l	#(CONFIG_SYS_SDRAM_CTRL + 4), %d0
	nop
	move.l	%d0, (%a2)
	move.l	%d0, (%a2)
	nop

#ifdef CONFIG_M54455EVB
	move.l	#(CONFIG_SYS_SDRAM_MODE + 0x200), (%a1)
	nop
#elif defined(CONFIG_M54451EVB)
	/* Issue LEMR */
	move.l	#(CONFIG_SYS_SDRAM_MODE), (%a1)
	nop
	move.l	#(CONFIG_SYS_SDRAM_EMOD), (%a1)
#endif

	move.l	#500, %d1
	jsr	asm_delay

	move.l	#(CONFIG_SYS_SDRAM_CTRL), %d1
	and.l	#0x7FFFFFFF, %d1
#ifdef CONFIG_M54455EVB
	or.l	#0x10000C00, %d1
#elif defined(CONFIG_M54451EVB)
	or.l	#0x10000C00, %d1
#endif
	move.l	%d1, (%a2)
	nop

	move.l	#2000, %d1
	jsr	asm_delay
#endif		/* CONFIG_MCF5445x */

#ifdef CONFIG_CF_SBF
	/*
	 * DSPI Initialization
	 * a0 - general, sram - 0x80008000 - 32, see M54455EVB.h
	 * a1 - dspi status
	 * a2 - dtfr
	 * a3 - drfr
	 * a4 - Dst addr
	 */
	/* Enable pins for DSPI mode - chip-selects are enabled later */
asm_dspi_init:
#ifdef CONFIG_MCF5441x
	move.l	#0xEC09404E, %a1
	move.l	#0xEC09404F, %a2
	move.b	#0xFF, (%a1)
	move.b	#0x80, (%a2)
#endif

#ifdef CONFIG_MCF5445x
	move.l	#0xFC0A4063, %a0
	move.b	#0x7F, (%a0)
#endif
	/* Configure DSPI module */
	move.l	#0xFC05C000, %a0
	move.l	#0x80FF0C00, (%a0)	/* Master, clear TX/RX FIFO */

	move.l	#0xFC05C00C, %a0
#ifdef CONFIG_MCF5441x
	move.l	#0x3E000016, (%a0)
#endif
#ifdef CONFIG_MCF5445x
	move.l	#0x3E000011, (%a0)
#endif

	move.l	#0xFC05C034, %a2	/* dtfr */
	move.l	#0xFC05C03B, %a3	/* drfr */

	move.l	#(ASM_SBF_IMG_HDR + 4), %a1
	move.l	(%a1)+, %d5
	move.l	(%a1), %a4

	move.l	#(CONFIG_SYS_INIT_RAM_ADDR + CONFIG_SYS_SBFHDR_DATA_OFFSET), %a0
	move.l	#(CONFIG_SYS_SBFHDR_SIZE), %d4

	move.l	#0xFC05C02C, %a1	/* dspi status */

	/* Issue commands and address */
	move.l	#0x8002000B, %d2	/* Fast Read Cmd */
	jsr	asm_dspi_wr_status
	jsr	asm_dspi_rd_status

	move.l	#0x80020000, %d2	/* Address byte 2 */
	jsr	asm_dspi_wr_status
	jsr	asm_dspi_rd_status

	move.l	#0x80020000, %d2	/* Address byte 1 */
	jsr	asm_dspi_wr_status
	jsr	asm_dspi_rd_status

	move.l	#0x80020000, %d2	/* Address byte 0 */
	jsr	asm_dspi_wr_status
	jsr	asm_dspi_rd_status

	move.l	#0x80020000, %d2	/* Dummy Wr and Rd */
	jsr	asm_dspi_wr_status
	jsr	asm_dspi_rd_status

	/* Transfer serial boot header to sram */
asm_dspi_rd_loop1:
	move.l	#0x80020000, %d2
	jsr	asm_dspi_wr_status
	jsr	asm_dspi_rd_status

	move.b	%d1, (%a0)		/* read, copy to dst */

	add.l	#1, %a0			/* inc dst by 1 */
	sub.l	#1, %d4			/* dec cnt by 1 */
	bne	asm_dspi_rd_loop1

	/* Transfer u-boot from serial flash to memory */
asm_dspi_rd_loop2:
	move.l	#0x80020000, %d2
	jsr	asm_dspi_wr_status
	jsr	asm_dspi_rd_status

	move.b	%d1, (%a4)		/* read, copy to dst */

	add.l	#1, %a4			/* inc dst by 1 */
	sub.l	#1, %d5			/* dec cnt by 1 */
	bne	asm_dspi_rd_loop2

	move.l	#0x00020000, %d2	/* Terminate */
	jsr	asm_dspi_wr_status
	jsr	asm_dspi_rd_status

	/* jump to memory and execute */
	move.l	#(CONFIG_SYS_TEXT_BASE + 0x400), %a0
	jmp	(%a0)

asm_dspi_wr_status:
	move.l	(%a1), %d0		/* status */
	and.l	#0x0000F000, %d0
	cmp.l	#0x00003000, %d0
	bgt	asm_dspi_wr_status

	move.l	%d2, (%a2)
	rts

asm_dspi_rd_status:
	move.l	(%a1), %d0		/* status */
	and.l	#0x000000F0, %d0
	lsr.l	#4, %d0
	cmp.l	#0, %d0
	beq	asm_dspi_rd_status

	move.b	(%a3), %d1
	rts
#endif			/* CONFIG_CF_SBF */

#ifdef CONFIG_SYS_NAND_BOOT
	/* copy 4 boot pages to dram as soon as possible */
	/* each page is 996 bytes (1056 total with 60 ECC bytes */
	move.l  #0x00000000, %a1	/* src */
	move.l	#CONFIG_SYS_TEXT_BASE, %a2		/* dst */
	move.l	#0x3E0, %d0		/* sz in long */

asm_boot_nand_copy:
	move.l	(%a1)+, (%a2)+
	subq.l	#1, %d0
	bne	asm_boot_nand_copy

	/* jump to memory and execute */
	move.l	#(asm_nand_init), %a0
	jmp	(%a0)

asm_nand_init:
	/* exit nand boot-mode */
	move.l	#0xFC0FFF30, %a1
	or.l	#0x00000040, %d1
	move.l	%d1, (%a1)

	/* initialize general use internal ram */
	move.l #0, %d0
	move.l #(CACR_STATUS), %a1	/* CACR */
	move.l #(ICACHE_STATUS), %a2	/* icache */
	move.l #(DCACHE_STATUS), %a3	/* dcache */
	move.l %d0, (%a1)
	move.l %d0, (%a2)
	move.l %d0, (%a3)

	/* invalidate and disable cache */
	move.l	#0x01004100, %d0	/* Invalidate cache cmd */
	movec	%d0, %CACR		/* Invalidate cache */
	move.l	#0, %d0
	movec	%d0, %ACR0
	movec	%d0, %ACR1
	movec	%d0, %ACR2
	movec	%d0, %ACR3

	/* Must disable global address */
	move.l	#0xFC008000, %a1
	move.l	#(CONFIG_SYS_CS0_BASE), (%a1)
	move.l	#0xFC008008, %a1
	move.l	#(CONFIG_SYS_CS0_CTRL), (%a1)
	move.l	#0xFC008004, %a1
	move.l	#(CONFIG_SYS_CS0_MASK), (%a1)

	/* NAND port configuration */
	move.l	#0xEC094048, %a1
	move.b	#0xFD, (%a1)+
	move.b	#0x5F, (%a1)+
	move.b	#0x04, (%a1)+

	/* reset nand */
	move.l  #0xFC0FFF38, %a1	/* isr */
	move.l  #0x000e0000, (%a1)
	move.l	#0xFC0FFF08, %a2
	move.l	#0x00000000, (%a2)+	/* car */
	move.l	#0x11000000, (%a2)+	/* rar */
	move.l	#0x00000000, (%a2)+	/* rpt */
	move.l	#0x00000000, (%a2)+	/* rai */
	move.l  #0xFC0FFF2c, %a2	/* cfg */
	move.l  #0x00000000, (%a2)+	/* secsz */
	move.l  #0x000e0681, (%a2)+
	move.l  #0xFC0FFF04, %a2	/* cmd2 */
	move.l  #0xFF404001, (%a2)
	move.l  #0x000e0000, (%a1)

	move.l	#0x2000, %d1
	jsr	asm_delay

	/* setup nand */
	move.l  #0xFC0FFF00, %a1
	move.l  #0x30700000, (%a1)+	/* cmd1 */
	move.l  #0x007EF000, (%a1)+	/* cmd2 */

	move.l  #0xFC0FFF2C, %a1
	move.l  #0x00000841, (%a1)+	/* secsz */
	move.l  #0x000e0681, (%a1)+	/* cfg */

	move.l	#100, %d4		/* 100 pages ~200KB */
	move.l	#4, %d2			/* start at 4 */
	move.l  #0xFC0FFF04, %a0	/* cmd2 */
	move.l  #0xFC0FFF0C, %a1	/* rar */
	move.l	#(CONFIG_SYS_TEXT_BASE + 0xF80), %a2	/* dst */

asm_nand_read:
	move.l	#0x11000000, %d0	/* rar */
	or.l	%d2, %d0
	move.l	%d0, (%a1)
	add.l	#1, %d2

	move.l	(%a0), %d0		/* cmd2 */
	or.l	#1, %d0
	move.l	%d0, (%a0)

	move.l	#0x200, %d1
	jsr	asm_delay

asm_nand_chk_status:
	move.l  #0xFC0FFF38, %a4	/* isr */
	move.l	(%a4), %d0
	and.l	#0x40000000, %d0
	tst.l	%d0
	beq	asm_nand_chk_status

	move.l  #0xFC0FFF38, %a4	/* isr */
	move.l	(%a4), %d0
	or.l	#0x000E0000, %d0
	move.l	%d0, (%a4)

	move.l	#0x200, %d3
	move.l	#0xFC0FC000, %a3	/* buf 1 */
asm_nand_copy:
	move.l	(%a3)+, (%a2)+
	subq.l	#1, %d3
	bgt	asm_nand_copy

	subq.l	#1, %d4
	bgt	asm_nand_read

	/* jump to memory and execute */
	move.l	#(CONFIG_SYS_TEXT_BASE + 0x400), %a0
	jmp	(%a0)

#endif			/* CONFIG_SYS_NAND_BOOT */

asm_delay:
	nop
	subq.l	#1, %d1
	bne	asm_delay
	rts
#endif			/* CONFIG_CF_SBF || CONFIG_NAND_U_BOOT */

	.text
	. = 0x400
	.globl	_start
_start:
#if !defined(CONFIG_SERIAL_BOOT)
	nop
	nop
	move.w #0x2700,%sr		/* Mask off Interrupt */

	/* Set vector base register at the beginning of the Flash */
	move.l	#CONFIG_SYS_FLASH_BASE, %d0
	movec	%d0, %VBR

	move.l	#(CONFIG_SYS_INIT_RAM_ADDR + CONFIG_SYS_INIT_RAM_CTRL), %d0
	movec	%d0, %RAMBAR1

	/* initialize general use internal ram */
	move.l #0, %d0
	move.l #(ICACHE_STATUS), %a1	/* icache */
	move.l #(DCACHE_STATUS), %a2	/* dcache */
	move.l %d0, (%a1)
	move.l %d0, (%a2)

	/* invalidate and disable cache */
	move.l	#(CONFIG_SYS_ICACHE_INV + CONFIG_SYS_DCACHE_INV), %d0
	movec	%d0, %CACR		/* Invalidate cache */
	move.l	#0, %d0
	movec	%d0, %ACR0
	movec	%d0, %ACR1
	movec	%d0, %ACR2
	movec	%d0, %ACR3
#else
	move.l	#(CONFIG_SYS_INIT_RAM_ADDR + CONFIG_SYS_INIT_RAM_CTRL), %d0
	movec	%d0, %RAMBAR1
#endif

	/* set stackpointer to end of internal ram to get some stackspace for
	   the first c-code */
	move.l	#(CONFIG_SYS_INIT_RAM_ADDR + CONFIG_SYS_INIT_SP_OFFSET), %sp
	clr.l %sp@-

	move.l #__got_start, %a5	/* put relocation table address to a5 */

	bsr cpu_init_f			/* run low-level CPU init code (from flash) */
	bsr board_init_f		/* run low-level board init code (from flash) */

	/* board_init_f() does not return */

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
	link.w %a6,#0
	move.l 8(%a6), %sp		/* set new stack pointer */

	move.l 12(%a6), %d0		/* Save copy of Global Data pointer */
	move.l 16(%a6), %a0		/* Save copy of Destination Address */

	move.l #CONFIG_SYS_MONITOR_BASE, %a1
	move.l #__init_end, %a2
	move.l %a0, %a3

	/* copy the code to RAM */
1:
	move.l (%a1)+, (%a3)+
	cmp.l  %a1,%a2
	bgt.s	 1b

/*
 * We are done. Do not return, instead branch to second part of board
 * initialization, now running from RAM.
 */
	move.l	%a0, %a1
	add.l	#(in_ram - CONFIG_SYS_MONITOR_BASE), %a1
	jmp	(%a1)

in_ram:

clear_bss:
	/*
	 * Now clear BSS segment
	 */
	move.l	%a0, %a1
	add.l	#(_sbss - CONFIG_SYS_MONITOR_BASE),%a1
	move.l	%a0, %d1
	add.l	#(_ebss - CONFIG_SYS_MONITOR_BASE),%d1
6:
	clr.l	(%a1)+
	cmp.l	%a1,%d1
	bgt.s	6b

	/*
	 * fix got table in RAM
	 */
	move.l	%a0, %a1
	add.l	#(__got_start - CONFIG_SYS_MONITOR_BASE),%a1
	move.l	%a1,%a5			/* * fix got pointer register a5 */

	move.l	%a0, %a2
	add.l	#(__got_end - CONFIG_SYS_MONITOR_BASE),%a2

7:
	move.l	(%a1),%d1
	sub.l	#_start,%d1
	add.l	%a0,%d1
	move.l	%d1,(%a1)+
	cmp.l	%a2, %a1
	bne	7b

	/* calculate relative jump to board_init_r in ram */
	move.l %a0, %a1
	add.l #(board_init_r - CONFIG_SYS_MONITOR_BASE), %a1

	/* set parameters for board_init_r */
	move.l %a0,-(%sp)		/* dest_addr */
	move.l %d0,-(%sp)		/* gd */
	jsr	(%a1)

/*------------------------------------------------------------------------------*/
/* exception code */
	.globl _fault
_fault:
	bra _fault
	.globl	_exc_handler

_exc_handler:
	SAVE_ALL
	movel	%sp,%sp@-
	bsr exc_handler
	addql	#4,%sp
	RESTORE_ALL

	.globl	_int_handler
_int_handler:
	SAVE_ALL
	movel	%sp,%sp@-
	bsr int_handler
	addql	#4,%sp
	RESTORE_ALL

/*------------------------------------------------------------------------------*/

	.globl	version_string
version_string:
	.ascii U_BOOT_VERSION_STRING, "\0"
	.align 4
