/*
 * include/asm-armnommu/arch-netarm/netarm_mem_module.h
 *
 * Copyright (C) 2005
 * Art Shipkowski, Videon Central, Inc., <art@videon-central.com>
 *
 * Copyright (C) 2000, 2001 NETsilicon, Inc.
 * Copyright (C) 2000, 2001 Red Hat, Inc.
 *
 * This software is copyrighted by Red Hat. LICENSEE agrees that
 * it will not delete this copyright notice, trademarks or protective
 * notices from any copy made by LICENSEE.
 *
 * This software is provided "AS-IS" and any express or implied
 * warranties or conditions, including but not limited to any
 * implied warranties of merchantability and fitness for a particular
 * purpose regarding this software. In no event shall Red Hat
 * be liable for any indirect, consequential, or incidental damages,
 * loss of profits or revenue, loss of use or data, or interruption
 * of business, whether the alleged damages are labeled in contract,
 * tort, or indemnity.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 * author(s) : Joe deBlaquiere
 *
 * Modified to support NS7520 by Art Shipkowski.
 */

#ifndef __NETARM_MEM_MODULE_REGISTERS_H
#define __NETARM_MEM_MODULE_REGISTERS_H

/* GEN unit register offsets */

#define	NETARM_MEM_MODULE_BASE		(0xFFC00000)

#define	NETARM_MEM_MODULE_CONFIG	(0x00)
#define	NETARM_MEM_CS0_BASE_ADDR	(0x10)
#define	NETARM_MEM_CS0_OPTIONS		(0x14)
#define	NETARM_MEM_CS1_BASE_ADDR	(0x20)
#define	NETARM_MEM_CS1_OPTIONS		(0x24)
#define	NETARM_MEM_CS2_BASE_ADDR	(0x30)
#define	NETARM_MEM_CS2_OPTIONS		(0x34)
#define	NETARM_MEM_CS3_BASE_ADDR	(0x40)
#define	NETARM_MEM_CS3_OPTIONS		(0x44)
#define	NETARM_MEM_CS4_BASE_ADDR	(0x50)
#define	NETARM_MEM_CS4_OPTIONS		(0x54)

/* select bitfield defintions */

/* Module Configuration Register ( 0xFFC0_0000 ) */

#define NETARM_MEM_CFG_REFR_COUNT_MASK	(0xFF000000)
#define NETARM_MEM_CFG_REFRESH_EN	(0x00800000)

#define NETARM_MEM_CFG_REFR_CYCLE_8CLKS	(0x00000000)
#define NETARM_MEM_CFG_REFR_CYCLE_6CLKS	(0x00200000)
#define NETARM_MEM_CFG_REFR_CYCLE_5CLKS	(0x00400000)
#define NETARM_MEM_CFG_REFR_CYCLE_4CLKS	(0x00600000)

#define NETARM_MEM_CFG_PORTC_AMUX	(0x00100000)

#define NETARM_MEM_CFG_A27_ADDR		(0x00080000)
#define NETARM_MEM_CFG_A27_CS0OE	(0x00000000)

#define NETARM_MEM_CFG_A26_ADDR		(0x00040000)
#define NETARM_MEM_CFG_A26_CS0WE	(0x00000000)

#define NETARM_MEM_CFG_A25_ADDR		(0x00020000)
#define NETARM_MEM_CFG_A25_BLAST	(0x00000000)

#define NETARM_MEM_CFG_PORTC_AMUX2	(0x00010000)


/* range on this period is about 1 to 275 usec (with 18.432MHz clock)   */
/* the expression will round down, so make sure to reverse it to verify */
/* it is what you want. period = [( count + 1 ) * 20] / Fcrystal        */
/* (note: Fxtal = Fcrystal/5, see HWRefGuide sections 8.2.5 and 11.3.2) */

#define	NETARM_MEM_REFR_PERIOD_USEC(p)	(NETARM_MEM_CFG_REFR_COUNT_MASK & \
					 (((((NETARM_XTAL_FREQ/(1000))*p)/(20000) \
					    ) - (1) ) << (24)))

#if 0
/* range on this period is about 1 to 275 usec (with 18.432MHz clock) */
/* the expression will round down, so make sure to reverse it toverify */
/* it is what you want. period = [( count + 1 ) * 4] / Fxtal          */

#define	NETARM_MEM_REFR_PERIOD_USEC(p)	(NETARM_MEM_CFG_REFR_COUNT_MASK & \
					 (((((NETARM_XTAL_FREQ/(1000))*p)/(4000) \
					    ) - (1) ) << (24)))
#endif

/* Base Address Registers (0xFFC0_00X0) */

#define NETARM_MEM_BAR_BASE_MASK	(0xFFFFF000)

/* macro to define base */

#define NETARM_MEM_BAR_BASE(x)		((x) & NETARM_MEM_BAR_BASE_MASK)

#define NETARM_MEM_BAR_DRAM_FP		(0x00000000)
#define NETARM_MEM_BAR_DRAM_EDO		(0x00000100)
#define NETARM_MEM_BAR_DRAM_SYNC	(0x00000200)

#define NETARM_MEM_BAR_DRAM_MUX_INT	(0x00000000)
#define NETARM_MEM_BAR_DRAM_MUX_EXT	(0x00000080)

#define NETARM_MEM_BAR_DRAM_MUX_BAL	(0x00000000)
#define NETARM_MEM_BAR_DRAM_MUX_UNBAL	(0x00000020)

#define NETARM_MEM_BAR_1BCLK_IDLE	(0x00000010)

#define NETARM_MEM_BAR_DRAM_SEL		(0x00000008)

#define NETARM_MEM_BAR_BURST_EN		(0x00000004)

#define NETARM_MEM_BAR_WRT_PROT		(0x00000002)

#define NETARM_MEM_BAR_VALID		(0x00000001)

/* Option Registers (0xFFC0_00X4) */

/* macro to define which bits of the base are significant */

#define NETARM_MEM_OPT_BASE_USE(x)	((x) & NETARM_MEM_BAR_BASE_MASK)

#define NETARM_MEM_OPT_WAIT_MASK	(0x00000F00)

#define	NETARM_MEM_OPT_WAIT_STATES(x)	(((x) << 8 ) & NETARM_MEM_OPT_WAIT_MASK )

#define NETARM_MEM_OPT_BCYC_1		(0x00000000)
#define NETARM_MEM_OPT_BCYC_2		(0x00000040)
#define NETARM_MEM_OPT_BCYC_3		(0x00000080)
#define NETARM_MEM_OPT_BCYC_4		(0x000000C0)

#define NETARM_MEM_OPT_BSIZE_2		(0x00000000)
#define NETARM_MEM_OPT_BSIZE_4		(0x00000010)
#define NETARM_MEM_OPT_BSIZE_8		(0x00000020)
#define NETARM_MEM_OPT_BSIZE_16		(0x00000030)

#define NETARM_MEM_OPT_32BIT		(0x00000000)
#define NETARM_MEM_OPT_16BIT		(0x00000004)
#define NETARM_MEM_OPT_8BIT		(0x00000008)
#define NETARM_MEM_OPT_32BIT_EXT_ACK	(0x0000000C)

#define NETARM_MEM_OPT_BUS_SIZE_MASK	(0x0000000C)

#define NETARM_MEM_OPT_READ_ASYNC	(0x00000000)
#define NETARM_MEM_OPT_READ_SYNC	(0x00000002)

#define NETARM_MEM_OPT_WRITE_ASYNC	(0x00000000)
#define NETARM_MEM_OPT_WRITE_SYNC	(0x00000001)

#ifdef CONFIG_NETARM_NS7520
/* The NS7520 has a second options register for each chip select */
#define	NETARM_MEM_CS0_OPTIONS_B  (0x18)
#define	NETARM_MEM_CS1_OPTIONS_B  (0x28)
#define	NETARM_MEM_CS2_OPTIONS_B  (0x38)
#define	NETARM_MEM_CS3_OPTIONS_B  (0x48)
#define	NETARM_MEM_CS4_OPTIONS_B  (0x58)

/* Option B Registers (0xFFC0_00x8) */
#define NETARM_MEM_OPTB_SYNC_1_STAGE	(0x00000001)
#define NETARM_MEM_OPTB_SYNC_2_STAGE	(0x00000002)
#define NETARM_MEM_OPTB_BCYC_PLUS0	(0x00000000)
#define NETARM_MEM_OPTB_BCYC_PLUS4	(0x00000004)
#define NETARM_MEM_OPTB_BCYC_PLUS8	(0x00000008)
#define NETARM_MEM_OPTB_BCYC_PLUS12	(0x0000000C)

#define NETARM_MEM_OPTB_WAIT_PLUS0	(0x00000000)
#define NETARM_MEM_OPTB_WAIT_PLUS16	(0x00000010)
#define NETARM_MEM_OPTB_WAIT_PLUS32	(0x00000020)
#define NETARM_MEM_OPTB_WAIT_PLUS48	(0x00000030)
#endif

#endif
