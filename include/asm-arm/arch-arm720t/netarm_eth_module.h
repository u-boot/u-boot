/*
 * include/asm-armnommu/arch-netarm/netarm_eth_module.h
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
 * author(s) : Jackie Smith Cashion
 *             David Smith
 */

#ifndef __NETARM_ETH_MODULE_REGISTERS_H
#define __NETARM_ETH_MODULE_REGISTERS_H

/* ETH unit register offsets */

#define	NETARM_ETH_MODULE_BASE		(0xFF800000)

#define get_eth_reg_addr(c) ((volatile unsigned int *)(NETARM_ETH_MODULE_BASE + (c)))

#define NETARM_ETH_GEN_CTRL		(0x000) /* Ethernet Gen Control Reg */
#define NETARM_ETH_GEN_STAT		(0x004) /* Ethernet Gen Status Reg */
#define NETARM_ETH_FIFO_DAT1            (0x008) /* Fifo Data Reg 1 */
#define NETARM_ETH_FIFO_DAT2            (0x00C) /* Fifo Data Reg 2 */
#define NETARM_ETH_TX_STAT              (0x010) /* Transmit Status Reg */
#define NETARM_ETH_RX_STAT              (0x014) /* Receive Status Reg */

#define NETARM_ETH_MAC_CFG		(0x400) /* MAC Configuration Reg */
#define NETARM_ETH_PCS_CFG		(0x408) /* PCS Configuration Reg */
#define NETARM_ETH_STL_CFG		(0x410) /* STL Configuration Reg */
#define NETARM_ETH_B2B_IPG_GAP_TMR	(0x440) /* Back-to-back IPG
						   Gap Timer Reg */
#define NETARM_ETH_NB2B_IPG_GAP_TMR	(0x444) /* Non Back-to-back
						   IPG Gap Timer Reg */
#define NETARM_ETH_MII_CMD		(0x540) /* MII (PHY) Command Reg */
#define NETARM_ETH_MII_ADDR		(0x544) /* MII Address Reg */
#define NETARM_ETH_MII_WRITE		(0x548) /* MII Write Data Reg */
#define NETARM_ETH_MII_READ		(0x54C) /* MII Read Data Reg */
#define NETARM_ETH_MII_IND		(0x550) /* MII Indicators Reg */
#define NETARM_ETH_MIB_CRCEC		(0x580) /* (MIB) CRC Error Counter */
#define NETARM_ETH_MIB_AEC		(0x584) /* Alignment Error Counter */
#define NETARM_ETH_MIB_CEC		(0x588) /* Code Error Counter */
#define NETARM_ETH_MIB_LFC		(0x58C) /* Long Frame Counter */
#define NETARM_ETH_MIB_SFC		(0x590) /* Short Frame Counter */
#define NETARM_ETH_MIB_LCC		(0x594) /* Late Collision Counter */
#define NETARM_ETH_MIB_EDC		(0x598) /* Excessive Deferral
						   Counter */
#define NETARM_ETH_MIB_MCC		(0x59C) /* Maximum Collision Counter */
#define NETARM_ETH_SAL_FILTER		(0x5C0) /* SAL Station Address
						   Filter Reg */
#define NETARM_ETH_SAL_STATION_ADDR_1	(0x5C4) /* SAL Station Address
						   Reg */
#define NETARM_ETH_SAL_STATION_ADDR_2	(0x5C8)
#define NETARM_ETH_SAL_STATION_ADDR_3	(0x5CC)
#define NETARM_ETH_SAL_HASH_TBL_1	(0x5D0) /* SAL Multicast Hash Table*/
#define NETARM_ETH_SAL_HASH_TBL_2	(0x5D4)
#define NETARM_ETH_SAL_HASH_TBL_3	(0x5D8)
#define NETARM_ETH_SAL_HASH_TBL_4	(0x5DC)

/* select bitfield defintions */

/* Ethernet General Control Register (0xFF80_0000) */

#define NETARM_ETH_GCR_ERX		(0x80000000) /* Enable Receive FIFO */
#define NETARM_ETH_GCR_ERXDMA		(0x40000000) /* Enable Receive DMA */
#define NETARM_ETH_GCR_ETX		(0x00800000) /* Enable Transmit FIFO */
#define NETARM_ETH_GCR_ETXDMA		(0x00400000) /* Enable Transmit DMA */
#define NETARM_ETH_GCR_ETXWM_50		(0x00100000) /* Transmit FIFO Water
							Mark.  Start transmit
							when FIFO is 50%
							full. */
#define NETARM_ETH_GCR_PNA		(0x00000400) /* pSOS pNA Buffer
							Descriptor Format */

/* Ethernet General Status Register (0xFF80_0004) */

#define NETARM_ETH_GST_RXFDB            (0x30000000)
#define NETARM_ETH_GST_RXREGR		(0x08000000) /* Receive Register
							Ready */
#define NETARM_ETH_GST_RXFIFOH		(0x04000000)
#define NETARM_ETH_GST_RXBR		(0x02000000)
#define NETARM_ETH_GST_RXSKIP		(0x01000000)

#define NETARM_ETH_GST_TXBC             (0x00020000)


/* Ethernet Transmit Status Register (0xFF80_0010) */

#define NETARM_ETH_TXSTAT_TXOK          (0x00008000)


/* Ethernet Receive Status Register (0xFF80_0014) */

#define NETARM_ETH_RXSTAT_SIZE          (0xFFFF0000)
#define NETARM_ETH_RXSTAT_RXOK          (0x00002000)


/* PCS Configuration Register (0xFF80_0408) */

#define NETARM_ETH_PCSC_NOCFR		(0x1) /* Disable Ciphering */
#define NETARM_ETH_PCSC_ENJAB		(0x2) /* Enable Jabber Protection */
#define NETARM_ETH_PCSC_CLKS_25M	(0x0) /* 25 MHz Clock Speed Select */
#define NETARM_ETH_PCSC_CLKS_33M	(0x4) /* 33 MHz Clock Speed Select */

/* STL Configuration Register (0xFF80_0410) */

#define NETARM_ETH_STLC_RXEN		(0x2) /* Enable Packet Receiver */
#define NETARM_ETH_STLC_AUTOZ		(0x4) /* Auto Zero Statistics */

/* MAC Configuration Register (0xFF80_0400) */

#define NETARM_ETH_MACC_HUGEN		(0x1) /* Enable Unlimited Transmit
						 Frame Sizes */
#define NETARM_ETH_MACC_PADEN		(0x4) /* Automatic Pad Fill Frames
						 to 64 Bytes */
#define NETARM_ETH_MACC_CRCEN		(0x8) /* Append CRC to Transmit
						 Frames */

/* MII (PHY) Command Register (0xFF80_0540) */

#define NETARM_ETH_MIIC_RSTAT		(0x1) /* Single Scan for Read Data */

/* MII Indicators Register (0xFF80_0550) */

#define NETARM_ETH_MIII_BUSY		(0x1) /* MII I/F Busy with
						 Read/Write */

/* SAL Station Address Filter Register (0xFF80_05C0) */

#define NETARM_ETH_SALF_PRO		(0x8) /* Enable Promiscuous Mode */
#define NETARM_ETH_SALF_PRM		(0x4) /* Accept All Multicast
						 Packets */
#define NETARM_ETH_SALF_PRA		(0x2) /* Accept Mulitcast Packets
						 using Hash Table */
#define NETARM_ETH_SALF_BROAD		(0x1) /* Accept All Broadcast
						 Packets */


#endif /* __NETARM_GEN_MODULE_REGISTERS_H */
