/*
 * Copyright (C) 2001, 2002 ETC s.r.o.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 * 02111-1307, USA.
 *
 * Written by Marcel Telka <marcel@telka.sk>, 2001, 2002.
 * Changes for U-Boot Peter Figuli <peposh@etc.sk>, 2003.
 *
 * This file is taken from OpenWinCE project hosted by SourceForge.net
 *
 * Documentation:
 * [1] Intel Corporation, "Intel PXA250 and PXA210 Application Processors
 *     Developer's Manual", February 2002, Order Number: 278522-001
 * [2] Samsung Electronics, "8Mx16 SDRAM 54CSP K4S281633D-RL/N/P",
 *     Revision 1.0, February 2002
 * [3] Samsung Electronics, "16Mx16 SDRAM 54CSP K4S561633C-RL(N)",
 *     Revision 1.0, February 2002
 *
*/

#include <config.h>
#include <version.h>
#include <asm/arch/pxa-regs.h>

.globl lowlevel_init
lowlevel_init:

	mov	r10, lr

/*	setup memory - see 6.12 in [1]
 *	Step 1 	- wait 200 us
 */
	mov	r0,#0x2700      		/* wait 200 us @ 99.5 MHz */
1:	subs	r0, r0, #1
	bne	1b
/*	TODO: complete step 1 for Synchronous Static memory*/

	ldr	r0, =0x48000000			/* MC_BASE */


/* 	step 1.a - setup MSCx
 */
	ldr	r1, =0x000012B3			/* MSC0_RRR0(1) | MSC0_RDN0(2) | MSC0_RDF0(11) | MSC0_RT0(3) */
	str	r1, [r0, #0x8]			/* MSC0_OFFSET */

/*	step 1.c - clear MDREFR:K1FREE, set MDREFR:DRI
 *	see AUTO REFRESH chapter in section D. in [2] and in [3]
 *	DRI = (64ms / 4096) * 99.53MHz / 32 = 48 for K4S281633
 *	DRI = (64ms / 8192) * 99.52MHz / 32 = 24 for K4S561633
 *	TODO: complete for Synchronous Static memory
 */
	ldr	r1, [r0, #4]			/* MDREFR_OFFSET */
	ldr	r2, =0x01000FFF			/* MDREFR_K1FREE | MDREFR_DRI_MASK */
	bic	r1, r1, r2
#if defined( WEP_SDRAM_K4S281633 )
	orr	r1, r1, #48			/* MDREFR_DRI(48) */
#elif defined( WEP_SDRAM_K4S561633 )
	orr	r1, r1, #24			/* MDREFR_DRI(24) */
#else
#error SDRAM chip is not defined
#endif

	str	r1, [r0, #4]			/* MDREFR_OFFSET */

/*	Step 2 - only for Synchronous Static memory (TODO)
 *
 *	Step 3 - same as step 4
 *
 *	Step 4
 *
 *	Step 4.a - set MDREFR:K1RUN, clear MDREFR:K1DB2
 */
	orr	r1, r1, #0x00010000		/* MDREFR_K1RUN */
	bic	r1, r1, #0x00020000		/* MDREFR_K1DB2 */
	str	r1, [r0, #4]			/* MDREFR_OFFSET */

/*	Step 4.b - clear MDREFR:SLFRSH */
	bic	r1, r1, #0x00400000		/* MDREFR_SLFRSH */
	str	r1, [r0, #4]			/* MDREFR_OFFSET */

/*	Step 4.c - set MDREFR:E1PIN */
	orr	r1, r1, #0x00008000		/* MDREFR_E1PIN */
	str	r1, [r0, #4]			/* MDREFR_OFFSET */

/*	Step 4.d - automatically done
 *
 *	Steps 4.e and 4.f - configure SDRAM
 */
#if defined( WEP_SDRAM_K4S281633 )
	ldr	r1, =0x00000AA8			/* MDCNFG_DTC0(2) | MDCNFG_DLATCH0 | MDCNFG_DCAC0(1) | MDCNFG_DRAC0(1) | MDCNFG_DNB0 */
#elif defined( WEP_SDRAM_K4S561633 )
	ldr	r1, =0x00000AC8			/* MDCNFG_DTC0(2) | MDCNFG_DLATCH0 | MDCNFG_DCAC0(1) | MDCNFG_DRAC0(2) | MDCNFG_DNB0 */
#else
#error SDRAM chip is not defined
#endif
	str	r1, [r0, #0]			/* MDCNFG_OFFSET */

/*	Step 5 - wait at least 200 us for SDRAM
 *	see section B. in [2]
 */
	mov	r2,#0x2700      		/* wait 200 us @ 99.5 MHz */
1:	subs	r2, r2, #1
	bne	1b

/*	Step 6 - after reset dcache is disabled, so automatically done
 *
 *	Step 7 - eight refresh cycles
 */
	mov	r2, #0xA0000000
	ldr	r3, [r2]
	ldr	r3, [r2]
	ldr	r3, [r2]
	ldr	r3, [r2]
	ldr	r3, [r2]
	ldr	r3, [r2]
	ldr	r3, [r2]
	ldr	r3, [r2]

/*	Step 8 - we don't need dcache now
 *
 *	Step 9 - enable SDRAM partition 0
 */
	orr	r1, r1, #1			/* MDCNFG_DE0 */
	str	r1, [r0, #0]			/* MDCNFG_OFFSET */

/*	Step 10 - write MDMRS */
	mov	r1, #0
	str	r1, [r0, #0x40]			/* MDMRS_OFFSET */

/*	Step 11 - optional (TODO) */

	mov	pc,r10
