/*
 * Copyright (C) 2007, Guennadi Liakhovetski <lg@denx.de>
 *
 * (C) Copyright 2008-2010 Freescale Semiconductor, Inc.
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

/*
 * AIPS setup - Only setup MPROTx registers.
 * The PACR default values are good.
 */
.macro init_aips
	/*
	 * Set all MPROTx to be non-bufferable, trusted for R/W,
	 * not forced to user-mode.
	 */
	ldr r0, =AIPS1_BASE_ADDR
	ldr r1, =AIPS_MPR_CONFIG
	str r1, [r0, #0x00]
	str r1, [r0, #0x04]
	ldr r0, =AIPS2_BASE_ADDR
	str r1, [r0, #0x00]
	str r1, [r0, #0x04]

	/*
	 * Clear the on and off peripheral modules Supervisor Protect bit
	 * for SDMA to access them. Did not change the AIPS control registers
	 * (offset 0x20) access type
	 */
	ldr r0, =AIPS1_BASE_ADDR
	ldr r1, =AIPS_OPACR_CONFIG
	str r1, [r0, #0x40]
	str r1, [r0, #0x44]
	str r1, [r0, #0x48]
	str r1, [r0, #0x4C]
	str r1, [r0, #0x50]
	ldr r0, =AIPS2_BASE_ADDR
	str r1, [r0, #0x40]
	str r1, [r0, #0x44]
	str r1, [r0, #0x48]
	str r1, [r0, #0x4C]
	str r1, [r0, #0x50]
.endm

/* MAX (Multi-Layer AHB Crossbar Switch) setup */
.macro init_max
	ldr r0, =MAX_BASE_ADDR
	/* MPR - priority is M4 > M2 > M3 > M5 > M0 > M1 */
	ldr r1, =MAX_MPR_CONFIG
	str r1, [r0, #0x000]        /* for S0 */
	str r1, [r0, #0x100]        /* for S1 */
	str r1, [r0, #0x200]        /* for S2 */
	str r1, [r0, #0x300]        /* for S3 */
	str r1, [r0, #0x400]        /* for S4 */
	/* SGPCR - always park on last master */
	ldr r1, =MAX_SGPCR_CONFIG
	str r1, [r0, #0x010]        /* for S0 */
	str r1, [r0, #0x110]        /* for S1 */
	str r1, [r0, #0x210]        /* for S2 */
	str r1, [r0, #0x310]        /* for S3 */
	str r1, [r0, #0x410]        /* for S4 */
	/* MGPCR - restore default values */
	ldr r1, =MAX_MGPCR_CONFIG
	str r1, [r0, #0x800]        /* for M0 */
	str r1, [r0, #0x900]        /* for M1 */
	str r1, [r0, #0xA00]        /* for M2 */
	str r1, [r0, #0xB00]        /* for M3 */
	str r1, [r0, #0xC00]        /* for M4 */
	str r1, [r0, #0xD00]        /* for M5 */
.endm

/* M3IF setup */
.macro init_m3if
	/* Configure M3IF registers */
	ldr r1, =M3IF_BASE_ADDR
	/*
	* M3IF Control Register (M3IFCTL)
	* MRRP[0] = L2CC0 not on priority list (0 << 0)	= 0x00000000
	* MRRP[1] = L2CC1 not on priority list (0 << 0)	= 0x00000000
	* MRRP[2] = MBX not on priority list (0 << 0)	= 0x00000000
	* MRRP[3] = MAX1 not on priority list (0 << 0)	= 0x00000000
	* MRRP[4] = SDMA not on priority list (0 << 0)	= 0x00000000
	* MRRP[5] = MPEG4 not on priority list (0 << 0)	= 0x00000000
	* MRRP[6] = IPU1 on priority list (1 << 6)	= 0x00000040
	* MRRP[7] = IPU2 not on priority list (0 << 0)	= 0x00000000
	*						------------
	*						  0x00000040
	*/
	ldr r0, =M3IF_CONFIG
	str r0, [r1]  /* M3IF control reg */
.endm

.macro core_init
	mrc 15, 0, r1, c1, c0, 0

	mrc 15, 0, r0, c1, c0, 1
	orr r0, r0, #7
	mcr 15, 0, r0, c1, c0, 1
	orr r1, r1, #(1<<11)

	/* Set unaligned access enable */
	orr r1, r1, #(1<<22)

	/* Set low int latency enable */
	orr r1, r1, #(1<<21)

	mcr 15, 0, r1, c1, c0, 0

	mov r0, #0

	/* Set branch prediction enable */
	mcr 15, 0, r0, c15, c2, 4

	mcr 15, 0, r0, c7, c7, 0        /* invalidate I cache and D cache */
	mcr 15, 0, r0, c8, c7, 0        /* invalidate TLBs */
	mcr 15, 0, r0, c7, c10, 4       /* Drain the write buffer */

	/*
	 * initializes very early AIPS
	 * Then it also initializes Multi-Layer AHB Crossbar Switch,
	 * M3IF
	 * Also setup the Peripheral Port Remap register inside the core
	 */
	ldr r0, =0x40000015        /* start from AIPS 2GB region */
	mcr p15, 0, r0, c15, c2, 4
.endm
