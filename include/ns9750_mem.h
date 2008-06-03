/***********************************************************************
 *
 * Copyright (C) 2004 by FS Forth-Systeme GmbH.
 * All rights reserved.
 *
 * $Id: ns9750_mem.h,v 1.1 2004/02/16 10:37:20 mpietrek Exp $
 * @Author: Markus Pietrek
 * @Descr: Definitions for Memory Control Module
 * @References: [1] NS9750 Hardware Reference Manual/December 2003 Chap. 5
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
 *
 ***********************************************************************/

#ifndef FS_NS9750_MEM_H
#define FS_NS9750_SYS_H

#define NS9750_MEM_MODULE_BASE		(0xA0700000)

#define get_mem_reg_addr(c) \
	((volatile unsigned int *)(NS9750_MEM_MODULE_BASE+(unsigned int) (c)))

/* the register addresses */

#define NS9750_MEM_CTRL			(0x0000)
#define NS9750_MEM_STATUS		(0x0004)
#define NS9750_MEM_CFG			(0x0008)
#define NS9750_MEM_DYN_CTRL		(0x0020)
#define NS9750_MEM_DYN_REFRESH		(0x0024)
#define NS9750_MEM_DYN_READ_CFG		(0x0028)
#define NS9750_MEM_DYN_TRP		(0x0030)
#define NS9750_MEM_DYN_TRAS		(0x0034)
#define NS9750_MEM_DYN_TSREX		(0x0038)
#define NS9750_MEM_DYN_TAPR		(0x003C)
#define NS9750_MEM_DYN_TDAL		(0x0040)
#define NS9750_MEM_DYN_TWR		(0x0044)
#define NS9750_MEM_DYN_TRC		(0x0048)
#define NS9750_MEM_DYN_TRFC		(0x004C)
#define NS9750_MEM_DYN_TXSR		(0x0050)
#define NS9750_MEM_DYN_TRRD		(0x0054)
#define NS9750_MEM_DYN_TMRD		(0x0058)
#define NS9750_MEM_STAT_EXT_WAIT	(0x0080)
#define NS9750_MEM_DYN_CFG_BASE		(0x0100)
#define NS9750_MEM_DYN_RAS_CAS_BASE	(0x0104)
#define NS9750_MEM_STAT_CFG_BASE	(0x0200)
#define NS9750_MEM_STAT_WAIT_WEN_BASE	(0x0204)
#define NS9750_MEM_STAT_WAIT_OEN_BASE	(0x0208)
#define NS9750_MEM_STAT_WAIT_RD_BASE	(0x020C)
#define NS9750_MEM_STAT_WAIT_PAGE_BASE	(0x0210)
#define NS9750_MEM_STAT_WAIR_WR_BASE	(0x0214)
#define NS9750_MEM_STAT_WAIT_TURN_BASE	(0x0218)

/* the vectored register addresses */

#define NS9750_MEM_DYN_CFG(c)		(NS9750_MEM_DYN_CFG_BASE + (c)*0x20)
#define NS9750_MEM_DYN_RAS_CAS(c)	(NS9750_MEM_DYN_RAS_CAS_BASE + (c)*0x20)
#define NS9750_MEM_STAT_CFG(c)		(NS9750_MEM_STAT_CFG_BASE + (c)*0x20)
#define NS9750_MEM_STAT_WAIT_WEN(c)	(NS9750_MEM_STAT_WAIT_WEN_BASE+(c)*0x20)
#define NS9750_MEM_STAT_WAIT_OEN(c)	(NS9750_MEM_STAT_WAIT_OEN_BASE+(c)*0x20)
#define NS9750_MEM_STAT_RD(c)		(NS9750_MEM_STAT_WAIT_RD_BASE+(c)*0x20)
#define NS9750_MEM_STAT_PAGE(c)		(NS9750_MEM_STAT_WAIT_PAGE_BASE+(c)*0x20)
#define NS9750_MEM_STAT_WR(c)		(NS9750_MEM_STAT_WAIT_WR_BASE+(c)*0x20)
#define NS9750_MEM_STAT_TURN(c)		(NS9750_MEM_STAT_WAIT_TURN_BASE+(c)*0x20)

/* register bit fields */

#define NS9750_MEM_CTRL_L		(0x00000004)
#define NS9750_MEM_CTRL_M		(0x00000002)
#define NS9750_MEM_CTRL_E		(0x00000001)

#define NS9750_MEM_STAT_SA		(0x00000004)
#define NS9750_MEM_STAT_S		(0x00000002)
#define NS9750_MEM_STAT_B		(0x00000001)

#define NS9750_MEM_CFG_CLK		(0x00000010)
#define NS9750_MEM_CFG_N		(0x00000001)

#define NS9750_MEM_DYN_CTRL_NRP		(0x00004000)
#define NS9750_MEM_DYN_CTRL_DP		(0x00002000)
#define NS9750_MEM_DYN_CTRL_I_MA	(0x00000180)
#define NS9750_MEM_DYN_CTRL_I_NORMAL	(0x00000000)
#define NS9750_MEM_DYN_CTRL_I_MODE	(0x00000080)
#define NS9750_MEM_DYN_CTRL_I_PALL	(0x00000100)
#define NS9750_MEM_DYN_CTRL_I_NOP	(0x00000180)
#define NS9750_MEM_DYN_CTRL_SR		(0x00000002)
#define NS9750_MEM_DYN_CTRL_CE		(0x00000001)


#define NS9750_MEM_DYN_REFRESH_MA	(0x000007FF)

#define NS9750_MEM_DYN_READ_CFG_MA	(0x00000003)
#define NS9750_MEM_DYN_READ_CFG_DELAY0	(0x00000001)
#define NS9750_MEM_DYN_READ_CFG_DELAY1  (0x00000002)
#define NS9750_MEM_DYN_READ_CFG_DELAY2	(0x00000003)

#define NS9750_MEM_DYN_TRP_MA		(0x0000000F)

#define NS9750_MEM_DYN_TRAS_MA		(0x0000000F)

#define NS9750_MEM_DYN_TSREX_MA		(0x0000000F)

#define NS9750_MEM_DYN_TAPR_MA		(0x0000000F)

#define NS9750_MEM_DYN_TDAL_MA		(0x0000000F)

#define NS9750_MEM_DYN_TWR_MA		(0x0000000F)

#define NS9750_MEM_DYN_TRC_MA		(0x0000001F)

#define NS9750_MEM_DYN_TRFC_MA		(0x0000001F)

#define NS9750_MEM_DYN_TXSR_MA		(0x0000001F)

#define NS9750_MEM_DYN_TRRD_MA		(0x0000000F)

#define NS9750_MEM_DYN_TMRD_MA		(0x0000000F)

#define NS9750_MEM_STAT_EXTW_WAIT_MA	(0x0000003F)

#define NS9750_MEM_DYN_CFG_P		(0x00100000)
#define NS9750_MEM_DYN_CFG_BDMC		(0x00080000)
#define NS9750_MEM_DYN_CFG_AM		(0x00004000)
#define NS9750_MEM_DYN_CFG_AM_MA	(0x00001F80)
#define NS9750_MEM_DYN_CFG_MD		(0x00000018)

#define NS9750_MEM_DYN_RAS_CAS_CAS_MA	(0x00000300)
#define NS9750_MEM_DYN_RAS_CAS_CAS_1	(0x00000100)
#define NS9750_MEM_DYN_RAS_CAS_CAS_2	(0x00000200)
#define NS9750_MEM_DYN_RAS_CAS_CAS_3	(0x00000300)
#define NS9750_MEM_DYN_RAS_CAS_RAS_MA	(0x00000003)
#define NS9750_MEM_DYN_RAS_CAS_RAS_1	(0x00000001)
#define NS9750_MEM_DYN_RAS_CAS_RAS_2	(0x00000002)
#define NS9750_MEM_DYN_RAS_CAS_RAS_3	(0x00000003)

#define NS9750_MEM_STAT_CFG_PSMC	(0x00100000)
#define NS9750_MEM_STAT_CFG_BSMC	(0x00080000)
#define NS9750_MEM_STAT_CFG_EW		(0x00000100)
#define NS9750_MEM_STAT_CFG_PB		(0x00000080)
#define NS9750_MEM_STAT_CFG_PC		(0x00000040)
#define NS9750_MEM_STAT_CFG_PM		(0x00000008)
#define NS9750_MEM_STAT_CFG_MW_MA	(0x00000003)
#define NS9750_MEM_STAT_CFG_MW_8	(0x00000000)
#define NS9750_MEM_STAT_CFG_MW_16	(0x00000001)
#define NS9750_MEM_STAT_CFG_MW_32	(0x00000002)

#define NS9750_MEM_STAT_WAIT_WEN_MA	(0x0000000F)

#define NS9750_MEM_STAT_WAIT_OEN_MA	(0x0000000F)

#define NS9750_MEM_STAT_WAIT_RD_MA	(0x0000001F)

#define NS9750_MEM_STAT_WAIT_PAGE_MA	(0x0000001F)

#define NS9750_MEM_STAT_WAIT_WR_MA	(0x0000001F)

#define NS9750_MEM_STAT_WAIT_TURN_MA	(0x0000000F)


#endif /* FS_NS9750_MEM_H */
