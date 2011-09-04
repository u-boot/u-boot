/*
 * Memory setup for ORIGEN board based on S5PV310
 *
 * Copyright (C) 2011 Samsung Electronics
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
#include "origen_setup.h"
#define SET_MIU

	.globl mem_ctrl_asm_init
mem_ctrl_asm_init:
	/*
	 * Async bridge configuration at CPU_core:
	 * 1: half_sync
	 * 0: full_sync
	 */
	ldr r0, =ASYNC_CONFIG
	mov r1, #1
	str r1, [r0]

#ifdef SET_MIU
	ldr	r0, =S5PC210_MIU_BASE
	/* Interleave: 2Bit, Interleave_bit1: 0x21, Interleave_bit2: 0x7 */
	ldr	r1, =0x20001507
	str	r1, [r0, #APB_SFR_INTERLEAVE_CONF_OFFSET]

	/* Update MIU Configuration */
	ldr	r1, =0x00000001
	str	r1, [r0, #APB_SFR_ARBRITATION_CONF_OFFSET]
#endif
	/* DREX0 */
	ldr	r0, =S5PC210_DMC0_BASE

	/*
	 * DLL Parameter Setting:
	 * Termination: Enable R/W
	 * Phase Delay for DQS Cleaning: 180' Shift
	 */
	ldr	r1, =0xe0000086
	str	r1, [r0, #DMC_PHYCONTROL1]

	/*
	 * ZQ Calibration
	 * Termination: Disable
	 * Auto Calibration Start: Enable
	 */
	ldr	r1, =0xE3855703
	str	r1, [r0, #DMC_PHYZQCONTROL]

	/* Wait ?us*/
	mov	r2, #0x100000
1:	subs	r2, r2, #1
	bne	1b

	/*
	 * Update DLL Information:
	 * Force DLL Resyncronization
	 */
	ldr	r1, =0xe000008e
	str	r1, [r0, #DMC_PHYCONTROL1]

	/* Reset Force DLL Resyncronization */
	ldr	r1, =0xe0000086
	str	r1, [r0, #DMC_PHYCONTROL1]

	/* Enable Differential DQS, DLL Off*/
	ldr	r1, =0x71101008
	str	r1, [r0, #DMC_PHYCONTROL0]

	/* Activate PHY DLL: DLL On */
	ldr	r1, =0x7110100A
	str	r1, [r0, #DMC_PHYCONTROL0]

	/* Set DLL Parameters */
	ldr	r1, =0xe0000086
	str	r1, [r0, #DMC_PHYCONTROL1]

	/* DLL Start */
	ldr	r1, =0x7110100B
	str	r1, [r0, #DMC_PHYCONTROL0]

	ldr	r1, =0x00000000
	str	r1, [r0, #DMC_PHYCONTROL2]

	/* Set Clock Ratio of Bus clock to Memory Clock */
	ldr	r1, =0x0FFF301a
	str	r1, [r0, #DMC_CONCONTROL]

	/*
	 * Memor Burst length: 8
	 * Number of chips: 2
	 * Memory Bus width: 32 bit
	 * Memory Type: DDR3
	 * Additional Latancy for PLL: 1 Cycle
	 */
	ldr	r1, =0x00312640
	str	r1, [r0, #DMC_MEMCONTROL]

	/*
	 * Memory Configuration Chip 0
	 * Address Mapping: Interleaved
	 * Number of Column address Bits: 10 bits
	 * Number of Rows Address Bits: 14
	 * Number of Banks: 8
	 */
	ldr	r1, =0x20e01323
	str	r1, [r0, #DMC_MEMCONFIG0]

	/*
	 * Memory Configuration Chip 1
	 * Address Mapping: Interleaved
	 * Number of Column address Bits: 10 bits
	 * Number of Rows Address Bits: 14
	 * Number of Banks: 8
	 */
	ldr	r1, =0x40e01323
	str	r1, [r0, #DMC_MEMCONFIG1]

	/* Config Precharge Policy */
	ldr	r1, =0xff000000
	str	r1, [r0, #DMC_PRECHCONFIG]

	/*
	 * TimingAref, TimingRow, TimingData, TimingPower Setting:
	 * Values as per Memory AC Parameters
	 */
	ldr	r1, =0x000000BB
	str	r1, [r0, #DMC_TIMINGAREF]
	ldr	r1, =0x4046654f
	str	r1, [r0, #DMC_TIMINGROW]
	ldr	r1, =0x46400506
	str	r1, [r0, #DMC_TIMINGDATA]
	ldr	r1, =0x52000A3C
	str	r1, [r0, #DMC_TIMINGPOWER]

	/* Chip0: NOP Command: Assert and Hold CKE to high level */
	ldr	r1, =0x07000000
	str	r1, [r0, #DMC_DIRECTCMD]

	/* Wait ?us*/
	mov	r2, #0x100000
2:	subs	r2, r2, #1
	bne	2b

	/* Chip0: EMRS2, EMRS3, EMRS, MRS Commands Using Direct Command */
	ldr	r1, =0x00020000
	str	r1, [r0, #DMC_DIRECTCMD]
	ldr	r1, =0x00030000
	str	r1, [r0, #DMC_DIRECTCMD]
	ldr	r1, =0x00010002
	str	r1, [r0, #DMC_DIRECTCMD]
	ldr	r1, =0x00000328
	str	r1, [r0, #DMC_DIRECTCMD]

	/* Wait ?us*/
	mov	r2, #0x100000
3:	subs	r2, r2, #1
	bne	3b

	/* Chip0: ZQINIT */
	ldr	r1, =0x0a000000
	str	r1, [r0, #DMC_DIRECTCMD]

	/* Wait ?us*/
	mov	r2, #0x100000
4:	subs	r2, r2, #1
	bne	4b

	/* Chip1: NOP Command: Assert and Hold CKE to high level */
	ldr	r1, =0x07100000
	str	r1, [r0, #DMC_DIRECTCMD]

	/* Wait ?us*/
	mov	r2, #0x100000
5:	subs	r2, r2, #1
	bne	5b

	/* Chip1: EMRS2, EMRS3, EMRS, MRS Commands Using Direct Command */
	ldr	r1, =0x00120000
	str	r1, [r0, #DMC_DIRECTCMD]
	ldr	r1, =0x00130000
	str	r1, [r0, #DMC_DIRECTCMD]
	ldr	r1, =0x00110002
	str	r1, [r0, #DMC_DIRECTCMD]
	ldr	r1, =0x00100328
	str	r1, [r0, #DMC_DIRECTCMD]

	/* Wait ?us*/
	mov	r2, #0x100000
6:	subs	r2, r2, #1
	bne	6b

	/* Chip1: ZQINIT */
	ldr	r1, =0x0a100000
	str	r1, [r0, #DMC_DIRECTCMD]

	/* Wait ?us*/
	mov	r2, #0x100000
7:	subs	r2, r2, #1
	bne	7b

	ldr	r1, =0xe000008e
	str	r1, [r0, #DMC_PHYCONTROL1]
	ldr	r1, =0xe0000086
	str	r1, [r0, #DMC_PHYCONTROL1]

	/* Wait ?us*/
	mov	r2, #0x100000
8:	subs	r2, r2, #1
	bne	8b

	/* DREX1 */
	ldr	r0, =S5PC210_DMC1_BASE	@0x10410000

	/*
	 * DLL Parameter Setting:
	 * Termination: Enable R/W
	 * Phase Delay for DQS Cleaning: 180' Shift
	 */
	ldr	r1, =0xe0000086
	str	r1, [r0, #DMC_PHYCONTROL1]

	/*
	 * ZQ Calibration:
	 * Termination: Disable
	 * Auto Calibration Start: Enable
	 */
	ldr	r1, =0xE3855703
	str	r1, [r0, #DMC_PHYZQCONTROL]

	/* Wait ?us*/
	mov	r2, #0x100000
1:	subs	r2, r2, #1
	bne	1b

	/*
	 * Update DLL Information:
	 * Force DLL Resyncronization
	 */
	ldr	r1, =0xe000008e
	str	r1, [r0, #DMC_PHYCONTROL1]

	/* Reset Force DLL Resyncronization */
	ldr	r1, =0xe0000086
	str	r1, [r0, #DMC_PHYCONTROL1]

	/* Enable Differential DQS, DLL Off*/
	ldr	r1, =0x71101008
	str	r1, [r0, #DMC_PHYCONTROL0]

	/* Activate PHY DLL: DLL On */
	ldr	r1, =0x7110100A
	str	r1, [r0, #DMC_PHYCONTROL0]

	/* Set DLL Parameters */
	ldr	r1, =0xe0000086
	str	r1, [r0, #DMC_PHYCONTROL1]

	/* DLL Start */
	ldr	r1, =0x7110100B
	str	r1, [r0, #DMC_PHYCONTROL0]

	ldr	r1, =0x00000000
	str	r1, [r0, #DMC_PHYCONTROL2]

	/* Set Clock Ratio of Bus clock to Memory Clock */
	ldr	r1, =0x0FFF301a
	str	r1, [r0, #DMC_CONCONTROL]

	/*
	 * Memor Burst length: 8
	 * Number of chips: 2
	 * Memory Bus width: 32 bit
	 * Memory Type: DDR3
	 * Additional Latancy for PLL: 1 Cycle
	 */
	ldr	r1, =0x00312640
	str	r1, [r0, #DMC_MEMCONTROL]

	/*
	 * Memory Configuration Chip 0
	 * Address Mapping: Interleaved
	 * Number of Column address Bits: 10 bits
	 * Number of Rows Address Bits: 14
	 * Number of Banks: 8
	 */
	ldr	r1, =0x20e01323
	str	r1, [r0, #DMC_MEMCONFIG0]

	/*
	 * Memory Configuration Chip 1
	 * Address Mapping: Interleaved
	 * Number of Column address Bits: 10 bits
	 * Number of Rows Address Bits: 14
	 * Number of Banks: 8
	 */
	ldr	r1, =0x40e01323
	str	r1, [r0, #DMC_MEMCONFIG1]

	/* Config Precharge Policy */
	ldr	r1, =0xff000000
	str	r1, [r0, #DMC_PRECHCONFIG]

	/*
	 * TimingAref, TimingRow, TimingData, TimingPower Setting:
	 * Values as per Memory AC Parameters
	 */
	ldr	r1, =0x000000BB
	str	r1, [r0, #DMC_TIMINGAREF]
	ldr	r1, =0x4046654f
	str	r1, [r0, #DMC_TIMINGROW]
	ldr	r1, =0x46400506
	str	r1, [r0, #DMC_TIMINGDATA]
	ldr	r1, =0x52000A3C
	str	r1, [r0, #DMC_TIMINGPOWER]

	/* Chip0: NOP Command: Assert and Hold CKE to high level */
	ldr	r1, =0x07000000
	str	r1, [r0, #DMC_DIRECTCMD]

	/* Wait ?us*/
	mov	r2, #0x100000
2:	subs	r2, r2, #1
	bne	2b

	/* Chip0: EMRS2, EMRS3, EMRS, MRS Commands Using Direct Command */
	ldr	r1, =0x00020000
	str	r1, [r0, #DMC_DIRECTCMD]
	ldr	r1, =0x00030000
	str	r1, [r0, #DMC_DIRECTCMD]
	ldr	r1, =0x00010002
	str	r1, [r0, #DMC_DIRECTCMD]
	ldr	r1, =0x00000328
	str	r1, [r0, #DMC_DIRECTCMD]

	/* Wait ?us*/
	mov	r2, #0x100000
3:	subs	r2, r2, #1
	bne	3b

	/* Chip 0: ZQINIT */
	ldr	r1, =0x0a000000
	str	r1, [r0, #DMC_DIRECTCMD]

	/* Wait ?us*/
	mov	r2, #0x100000
4:	subs	r2, r2, #1
	bne	4b

	/* Chip1: NOP Command: Assert and Hold CKE to high level */
	ldr	r1, =0x07100000
	str	r1, [r0, #DMC_DIRECTCMD]

	/* Wait ?us*/
	mov	r2, #0x100000
5:	subs	r2, r2, #1
	bne	5b

	/* Chip1: EMRS2, EMRS3, EMRS, MRS Commands Using Direct Command */
	ldr	r1, =0x00120000
	str	r1, [r0, #DMC_DIRECTCMD]
	ldr	r1, =0x00130000
	str	r1, [r0, #DMC_DIRECTCMD]
	ldr	r1, =0x00110002
	str	r1, [r0, #DMC_DIRECTCMD]
	ldr	r1, =0x00100328
	str	r1, [r0, #DMC_DIRECTCMD]

	/* Wait ?us*/
	mov	r2, #0x100000
6:	subs	r2, r2, #1
	bne	6b

	/* Chip1: ZQINIT */
	ldr	r1, =0x0a100000
	str	r1, [r0, #DMC_DIRECTCMD]

	/* Wait ?us*/
	mov	r2, #0x100000
7:	subs	r2, r2, #1
	bne	7b

	ldr	r1, =0xe000008e
	str	r1, [r0, #DMC_PHYCONTROL1]
	ldr	r1, =0xe0000086
	str	r1, [r0, #DMC_PHYCONTROL1]

	/* Wait ?us*/
	mov	r2, #0x100000
8:	subs	r2, r2, #1
	bne	8b

	/* turn on DREX0, DREX1 */
	ldr	r0, =S5PC210_DMC0_BASE
	ldr	r1, =0x0FFF303a
	str	r1, [r0, #DMC_CONCONTROL]

	ldr	r0, =S5PC210_DMC1_BASE
	ldr	r1, =0x0FFF303a
	str	r1, [r0, #DMC_CONCONTROL]

	mov	pc, lr
