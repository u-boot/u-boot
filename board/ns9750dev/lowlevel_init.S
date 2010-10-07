/*
 * Board specific setup info
 *
 * (C) Copyright 2003
 * Texas Instruments, <www.ti.com>
 * Kshitij Gupta <Kshitij@ti.com>
 *
 * Modified for the NS9750 DevBoard by
 * (C) Copyright 2004 by FS Forth-Systeme GmbH.
 * Markus Pietrek <mpietrek@fsforth.de>
 * @References: [1] NS9750 Hardware Reference/December 2003
 *	        [2] ns9750_a.cmd from MAJIC configuration
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

#include <config.h>
#include <version.h>

#if defined(CONFIG_NS9750DEV)
# ifndef CONFIG_SKIP_LOWLEVEL_INIT
#  include <./ns9750_sys.h>
#  include <./ns9750_mem.h>
# endif
#endif

/***********************************************************************
 * @Function: write_register_block
 * @Return: nothing
 * @Descr: Copies the register block of register_offset:register value to
 *         the registers at base r0. The block is assumed to start in RAM at r1
 *         and end at r2. The linked RAM base address of U-Boot is assumed to be
 *	   in r5 while the ROM base address we are running from is r6
 *         Uses r3 and r4 as tempory registers
 ***********************************************************************/

.macro	write_register_block
	@@ map the addresses to high memory
	sub	r1, r1, r5
	add	r1, r1, r6
	sub	r2, r2, r5
	add	r2, r2, r6

	@@ copy all
1:
	@@ Write register/value pair starting at [r1] to register base r0
	ldr	r3, [r1], #4
	ldr	r4, [r1], #4
	str	r4, [r0,r3]
	cmp	r1, r2
	blt	1b
.endm

_TEXT_BASE:
	.word	CONFIG_SYS_TEXT_BASE	@ sdram load addr from config.mk
_PHYS_FLASH:
	.word	PHYS_FLASH_1    @ real flash address (without mirroring)
_CAS_LATENCY:
	.word	0x00022000	@ for CAS2 latency

#ifndef CONFIG_SKIP_LOWLEVEL_INIT
.globl lowlevel_init
lowlevel_init:

	/* U-Boot may be linked to RAM at 0x780000. But this code will run in
	   flash from 0x0. But in order to enable RAM we have to disable the
	   mirror bit, therefore we have to jump to our real flash address
	   beginning at PHYS_FLASH_1 (CS4 Base). Therefore,
	   _run_at_real_flash_address may be 0x500003b0 while be linked to
	   0x7803b0. So we must modify our linked addresses */

	@@ branch to high memory address, away from 0x0
	ldr	r5, _TEXT_BASE
	ldr	r6, _PHYS_FLASH
	ldr	r0, =_run_at_real_flash_address
	sub	r0, r0, r5
	add	r0, r0, r6
	mov	pc, r0
	nop			@ for pipelining

_run_at_real_flash_address:
	@@ now we are running > PHYS_FLASH_1, safe to enable memory controller

	@@ Write Memory Configuration Registers

	ldr	r0, _NS9750_MEM_MODULE_BASE
	ldr	r1, =_MEM_CONFIG_START
	ldr	r2, =_MEM_CONFIG_END

	write_register_block

	@@ Give SDRAM some time to settle
	@@ @TODO. According to [2] it should be 2 AHB cycles. Check

	ldr	r1, =0x50
_sdram_settle:
	subs	r1, r1, #1
	bne	_sdram_settle

_enable_mappings:
	@@ Enable SDRAM Mode

	ldr	r1, =_MEM_MODE_START
	ldr	r2, =_MEM_MODE_END

	write_register_block

	ldr	r3, _CAS_LATENCY @ perform one read from SDRAM
	ldr	r3, [r3]

	@@ Enable SDRAM and memory mappings

	ldr	r1, =_MEM_ENABLE_START
	ldr	r2, =_MEM_ENABLE_END

	write_register_block

	@@ Activate AHB monitor

	ldr	r0, =NS9750_SYS_MODULE_BASE
	ldr	r1, =_AHB_MONITOR_START
	ldr	r2, =_AHB_MONITOR_END

	write_register_block
_relocate_lr:
	/* lr and ip (from cpu_init_crit) are still based on 0x0, relocate it to
	   PHYS_FLASH. */
	mov	r1, ip
	add	r1, r1, r6
	mov	ip, r1

	mov	r1, lr
	add	r1, r1, r6
	mov	lr, r1

	@@ back to arch calling code
	mov	pc,	lr

	.ltorg

_NS9750_MEM_MODULE_BASE:
	.word	NS9750_MEM_MODULE_BASE

_MEM_CONFIG_START:
	/* Table of 2 32bit entries. First word is register address offset
	   relative to NS9750_MEM_MODULE_BASE, second one is value. They are
	   written in order of appearance */

	@@ Register values taken from [2]
	.word	NS9750_MEM_CTRL
	.word	NS9750_MEM_CTRL_E

	.word	NS9750_MEM_DYN_REFRESH
	.word	(0x6 & NS9750_MEM_DYN_REFRESH_MA)

	.word	NS9750_MEM_DYN_READ_CFG
	.word	(0x1 & NS9750_MEM_DYN_READ_CFG_MA)

	.word	NS9750_MEM_DYN_TRP
	.word	(0x1 & NS9750_MEM_DYN_TRP_MA)

	.word	NS9750_MEM_DYN_TRAS
	.word	(0x4 & NS9750_MEM_DYN_TRAS_MA)

	.word	NS9750_MEM_DYN_TAPR
	.word	(0x1 & NS9750_MEM_DYN_TRAS_MA)

	.word	NS9750_MEM_DYN_TDAL
	.word	(0x5 & NS9750_MEM_DYN_TDAL_MA)

	.word	NS9750_MEM_DYN_TWR
	.word	(0x1 & NS9750_MEM_DYN_TWR_MA)

	.word	NS9750_MEM_DYN_TRC
	.word	(0x6 & NS9750_MEM_DYN_TRC_MA)

	.word	NS9750_MEM_DYN_TRFC
	.word	(0x6 & NS9750_MEM_DYN_TRFC_MA)

	.word	NS9750_MEM_DYN_TRRD
	.word	(0x1 & NS9750_MEM_DYN_TRRD_MA)

	.word	NS9750_MEM_DYN_TMRD
	.word	(0x1 & NS9750_MEM_DYN_TMRD_MA)

	@@ CS 4
	.word	NS9750_MEM_DYN_CFG(0)
	.word	(NS9750_MEM_DYN_CFG_AM | \
		 (0x280 & NS9750_MEM_DYN_CFG_AM_MA))

	.word	NS9750_MEM_DYN_RAS_CAS(0)
	.word	((0x200 & NS9750_MEM_DYN_RAS_CAS_CAS_MA) | \
		 (0x03 & NS9750_MEM_DYN_RAS_CAS_RAS_MA))

	@@ CS 5
	.word	NS9750_MEM_DYN_CFG(1)
	.word	(NS9750_MEM_DYN_CFG_AM | \
		 (0x280 & NS9750_MEM_DYN_CFG_AM_MA))

	.word	NS9750_MEM_DYN_RAS_CAS(1)
	.word	((0x200 & NS9750_MEM_DYN_RAS_CAS_CAS_MA) | \
		 (0x03 & NS9750_MEM_DYN_RAS_CAS_RAS_MA))

	@@ CS 6
	.word	NS9750_MEM_DYN_CFG(2)
	.word	(NS9750_MEM_DYN_CFG_AM | \
		 (0x280 & NS9750_MEM_DYN_CFG_AM_MA))

	.word	NS9750_MEM_DYN_RAS_CAS(2)
	.word	((0x200 & NS9750_MEM_DYN_RAS_CAS_CAS_MA) | \
		 (0x03 & NS9750_MEM_DYN_RAS_CAS_RAS_MA))

	@@ CS 7
	.word	NS9750_MEM_DYN_CFG(3)
	.word	(NS9750_MEM_DYN_CFG_AM | \
		 (0x280 & NS9750_MEM_DYN_CFG_AM_MA))

	.word	NS9750_MEM_DYN_RAS_CAS(3)
	.word	((0x200 & NS9750_MEM_DYN_RAS_CAS_CAS_MA) | \
		 (0x03 & NS9750_MEM_DYN_RAS_CAS_RAS_MA))

	.word	NS9750_MEM_DYN_CTRL
	.word	(NS9750_MEM_DYN_CTRL_I_PALL | \
		 NS9750_MEM_DYN_CTRL_SR | \
		 NS9750_MEM_DYN_CTRL_CE )

	.word	NS9750_MEM_DYN_REFRESH
	.word	(0x1 & NS9750_MEM_DYN_REFRESH_MA)
	@@ No further register settings after refresh
_MEM_CONFIG_END:

_MEM_MODE_START:
	.word	NS9750_MEM_DYN_REFRESH
	.word	(0x30 & NS9750_MEM_DYN_REFRESH_MA)

	.word	NS9750_MEM_DYN_CTRL
	.word	(NS9750_MEM_DYN_CTRL_I_MODE | \
		 NS9750_MEM_DYN_CTRL_SR | \
		 NS9750_MEM_DYN_CTRL_CE )
_MEM_MODE_END:

_MEM_ENABLE_START:
	.word	NS9750_MEM_DYN_CTRL
	.word	(NS9750_MEM_DYN_CTRL_I_NORMAL | \
		 NS9750_MEM_DYN_CTRL_SR | \
		 NS9750_MEM_DYN_CTRL_CE )

	@@ CS 4
	.word	NS9750_MEM_DYN_CFG(0)
	.word	(NS9750_MEM_DYN_CFG_BDMC | \
		 NS9750_MEM_DYN_CFG_AM | \
		 (0x280 & NS9750_MEM_DYN_CFG_AM_MA))

	@@ CS 5
	.word	NS9750_MEM_DYN_CFG(1)
	.word	(NS9750_MEM_DYN_CFG_BDMC | \
		 NS9750_MEM_DYN_CFG_AM | \
		 (0x280 & NS9750_MEM_DYN_CFG_AM_MA))

	@@ CS 6
	.word	NS9750_MEM_DYN_CFG(2)
	.word	(NS9750_MEM_DYN_CFG_BDMC | \
		 NS9750_MEM_DYN_CFG_AM | \
		 (0x280 & NS9750_MEM_DYN_CFG_AM_MA))

	@@ CS 7
	.word	NS9750_MEM_DYN_CFG(3)
	.word	(NS9750_MEM_DYN_CFG_BDMC | \
		 NS9750_MEM_DYN_CFG_AM | \
		 (0x280 & NS9750_MEM_DYN_CFG_AM_MA))
_MEM_ENABLE_END:

_AHB_MONITOR_START:
	.word	NS9750_SYS_AHB_TIMEOUT
	.word	0x01000100	@ @TODO not calculated yet

	.word	NS9750_SYS_AHB_MON
	.word	(NS9750_SYS_AHB_MON_BMTC_GEN_IRQ | \
		 NS9750_SYS_AHB_MON_BATC_GEN_IRQ)
_AHB_MONITOR_END:

#endif /* CONFIG_SKIP_LOWLEVEL_INIT */
