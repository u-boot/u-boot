/* * include/asm-armnommu/arch-netarm/netarm_dma_module.h
 *
 * Copyright (C) 2000 NETsilicon, Inc.
 * Copyright (C) 2000 WireSpeed Communications Corporation
 *
 * This software is copyrighted by WireSpeed. LICENSEE agrees that
 * it will not delete this copyright notice, trademarks or protective
 * notices from any copy made by LICENSEE.
 *
 * This software is provided "AS-IS" and any express or implied
 * warranties or conditions, including but not limited to any
 * implied warranties of merchantability and fitness for a particular
 * purpose regarding this software. In no event shall WireSpeed
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
 *             David Smith
 */

#ifndef __NETARM_DMA_MODULE_REGISTERS_H
#define __NETARM_DMA_MODULE_REGISTERS_H

/* GEN unit register offsets */

#define	NETARM_DMA_MODULE_BASE		(0xFF900000)

#define get_dma_reg_addr(c) ((volatile unsigned int *)(NETARM_DMA_MODULE_BASE + (c)))

#define	NETARM_DMA1A_BFR_DESCRPTOR_PTR	(0x00)
#define	NETARM_DMA1A_CONTROL		(0x10)
#define	NETARM_DMA1A_STATUS		(0x14)
#define	NETARM_DMA1B_BFR_DESCRPTOR_PTR	(0x20)
#define	NETARM_DMA1B_CONTROL		(0x30)
#define	NETARM_DMA1B_STATUS		(0x34)
#define	NETARM_DMA1C_BFR_DESCRPTOR_PTR	(0x40)
#define	NETARM_DMA1C_CONTROL		(0x50)
#define	NETARM_DMA1C_STATUS		(0x54)
#define	NETARM_DMA1D_BFR_DESCRPTOR_PTR	(0x60)
#define	NETARM_DMA1D_CONTROL		(0x70)
#define	NETARM_DMA1D_STATUS		(0x74)

#define	NETARM_DMA2_BFR_DESCRPTOR_PTR	(0x80)
#define	NETARM_DMA2_CONTROL		(0x90)
#define	NETARM_DMA2_STATUS		(0x94)

#define	NETARM_DMA3_BFR_DESCRPTOR_PTR	(0xA0)
#define	NETARM_DMA3_CONTROL		(0xB0)
#define	NETARM_DMA3_STATUS		(0xB4)

#define	NETARM_DMA4_BFR_DESCRPTOR_PTR	(0xC0)
#define	NETARM_DMA4_CONTROL		(0xD0)
#define	NETARM_DMA4_STATUS		(0xD4)

#define	NETARM_DMA5_BFR_DESCRPTOR_PTR	(0xE0)
#define	NETARM_DMA5_CONTROL		(0xF0)
#define	NETARM_DMA5_STATUS		(0xF4)

#define	NETARM_DMA6_BFR_DESCRPTOR_PTR	(0x100)
#define	NETARM_DMA6_CONTROL		(0x110)
#define	NETARM_DMA6_STATUS		(0x114)

#define	NETARM_DMA7_BFR_DESCRPTOR_PTR	(0x120)
#define	NETARM_DMA7_CONTROL		(0x130)
#define	NETARM_DMA7_STATUS		(0x134)

#define	NETARM_DMA8_BFR_DESCRPTOR_PTR	(0x140)
#define	NETARM_DMA8_CONTROL		(0x150)
#define	NETARM_DMA8_STATUS		(0x154)

#define	NETARM_DMA9_BFR_DESCRPTOR_PTR	(0x160)
#define	NETARM_DMA9_CONTROL		(0x170)
#define	NETARM_DMA9_STATUS		(0x174)

#define	NETARM_DMA10_BFR_DESCRPTOR_PTR	(0x180)
#define	NETARM_DMA10_CONTROL		(0x190)
#define	NETARM_DMA10_STATUS		(0x194)

/* select bitfield defintions */

/* DMA Control Register ( 0xFF90_0XX0 ) */

#define NETARM_DMA_CTL_ENABLE		(0x80000000)

#define NETARM_DMA_CTL_ABORT		(0x40000000)

#define NETARM_DMA_CTL_BUS_100_PERCENT	(0x00000000)
#define NETARM_DMA_CTL_BUS_75_PERCENT	(0x10000000)
#define NETARM_DMA_CTL_BUS_50_PERCENT	(0x20000000)
#define NETARM_DMA_CTL_BUS_25_PERCENT	(0x30000000)

#define NETARM_DMA_CTL_BUS_MASK		(0x30000000)

#define NETARM_DMA_CTL_MODE_FB_TO_MEM	(0x00000000)
#define NETARM_DMA_CTL_MODE_FB_FROM_MEM	(0x04000000)
#define NETARM_DMA_CTL_MODE_MEM_TO_MEM	(0x08000000)

#define NETARM_DMA_CTL_BURST_NONE	(0x00000000)
#define NETARM_DMA_CTL_BURST_8_BYTE	(0x01000000)
#define NETARM_DMA_CTL_BURST_16_BYTE	(0x02000000)

#define NETARM_DMA_CTL_BURST_MASK	(0x03000000)

#define NETARM_DMA_CTL_SRC_INCREMENT	(0x00200000)

#define NETARM_DMA_CTL_DST_INCREMENT	(0x00100000)

/* these apply only to ext xfers on DMA 3 or 4 */

#define NETARM_DMA_CTL_CH_3_4_REQ_EXT	(0x00800000)

#define NETARM_DMA_CTL_CH_3_4_DATA_32	(0x00000000)
#define NETARM_DMA_CTL_CH_3_4_DATA_16	(0x00010000)
#define NETARM_DMA_CTL_CH_3_4_DATA_8	(0x00020000)

#define NETARM_DMA_CTL_STATE(X)	((X) & 0xFC00)
#define NETARM_DMA_CTL_INDEX(X)	((X) & 0x03FF)

/* DMA Status Register ( 0xFF90_0XX4 ) */

#define NETARM_DMA_STAT_NC_INTPEN	(0x80000000)
#define NETARM_DMA_STAT_EC_INTPEN	(0x40000000)
#define NETARM_DMA_STAT_NR_INTPEN	(0x20000000)
#define NETARM_DMA_STAT_CA_INTPEN	(0x10000000)
#define NETARM_DMA_STAT_INTPEN_MASK	(0xF0000000)

#define NETARM_DMA_STAT_NC_INT_EN	(0x00800000)
#define NETARM_DMA_STAT_EC_INT_EN	(0x00400000)
#define NETARM_DMA_STAT_NR_INT_EN	(0x00200000)
#define NETARM_DMA_STAT_CA_INT_EN	(0x00100000)
#define NETARM_DMA_STAT_INT_EN_MASK	(0x00F00000)

#define NETARM_DMA_STAT_WRAP		(0x00080000)
#define NETARM_DMA_STAT_IDONE		(0x00040000)
#define NETARM_DMA_STAT_LAST		(0x00020000)
#define NETARM_DMA_STAT_FULL		(0x00010000)

#define	NETARM_DMA_STAT_BUFLEN(X)	((X) & 0x7FFF)

/* DMA Buffer Descriptor Word 0 bitfields. */

#define NETARM_DMA_BD0_WRAP		(0x80000000)
#define NETARM_DMA_BD0_IDONE		(0x40000000)
#define NETARM_DMA_BD0_LAST		(0x20000000)
#define NETARM_DMA_BD0_BUFPTR_MASK	(0x1FFFFFFF)

/* DMA Buffer Descriptor Word 1 bitfields. */

#define NETARM_DMA_BD1_STATUS_MASK	(0xFFFF0000)
#define NETARM_DMA_BD1_FULL		(0x00008000)
#define NETARM_DMA_BD1_BUFLEN_MASK	(0x00007FFF)

#ifndef	__ASSEMBLER__

typedef	struct __NETARM_DMA_Buff_Desc_FlyBy
{
	unsigned int word0;
	unsigned int word1;
} NETARM_DMA_Buff_Desc_FlyBy, *pNETARM_DMA_Buff_Desc_FlyBy ;

typedef	struct __NETARM_DMA_Buff_Desc_M_to_M
{
	unsigned int word0;
	unsigned int word1;
	unsigned int word2;
	unsigned int word3;
} NETARM_DMA_Buff_Desc_M_to_M, *pNETARM_DMA_Buff_Desc_M_to_M ;

#endif

#endif
