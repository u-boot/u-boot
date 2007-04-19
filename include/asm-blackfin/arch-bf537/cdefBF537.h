/*
 * Copyright (C) 2004 Analog Devices Inc., All Rights Reserved.
 *
 ***********************************************************************************
 *
 * This include file contains a list of macro "defines" to enable the programmer
 * to use symbolic names for register-access.
 *
 *   ----------------------------
 *   revision 0.1
 *   date: 2004/03/01 21:23:01;  author: joeb
 *   Initial revision
 *
 *   ----------------------------
 *   revision 0.2
 *   date: 2004/05/15 16:30:00;  author: joeb
 *   comments: removed I2C/IIC references to TWI, changed GPIO sections
 *
 *   ----------------------------
 *   revision 0.3
 *   date: 2004/06/08 12:25:00;  author: joeb
 *   comments: renamed some TWI and GPIO registers
 *
 *   ----------------------------
 *   revision 0.4
 *   date: 2004/06/09 14:25:00;  author: joeb
 *   comments: changed Timer status register to 32-bit, renamed EMAC count registers
 *
 *   ----------------------------
 *   revision 0.5
 *   date: 2004/08/10 10:25:00;  author: joeb
 *   comments: Renamed EMAC wake-up registers, changed bit-names in EMAC registers
 *
 *   ----------------------------
 *   revision 0.6
 *   date: 2004/08/17 16:25:00;  author: joeb
 *   comments: Renamed TWI_INT_ENABLE to TWI_INT_MASK
 *
 *   ----------------------------
 *   revision 0.7
 *   date: 2004/08/18 13:21:00;  author: joeb
 *   comments: Renamed GPIO registers to remove _D, _S, _C, _T suffixes
 *
 *   ----------------------------
 *   revision 0.8
 *   date: 2004/08/20 10:27:00;  author: joeb
 *   comments: Renamed External DMA to Handshake DMA
 *
 *   ----------------------------
 *   revision 0.9
 *   date: 2004/08/23 13:42:00;  author: joeb
 *   comments: Renamed Handshake DMA Register Set
 *
 *   ----------------------------
 *   revision 0.10
 *   date: 2004/10/28 15:40:00;  author: joeb
 *   comments: Shortened EMAC Count Register Names
 *
 *   ----------------------------
 *   revision 0.11
 *   date: 2004/12/13 11:05:00;  author: joeb
 *   comments: Fixed address pointers - (volatile void **) to (void * volatile *)
 *
 *   ----------------------------
 *   revision 0.12
 *   date: 2004/12/17 14:25:00;  author: joeb
 *   comments: Replaced C++ Single-Line Comments w/C-standard Comments
 *				Changed EMAC EQ1024 TX/RX References to GE1024
 *
 *   ----------------------------
 *   revision 0.13
 *   date: 2005/01/05 10:50:00;  author: joeb
 *   comments: Removed excess white space in CAN_AM section
 *				Added support for CAN Macros to Index AM and Mailbox Areas
 *
 *   ----------------------------
 *   revision 0.14
 *   date: 2005/01/26 14:10:00;  author: joeb
 *   comments: Fixed Typo In EMAC_RXC_PAUSE register
 *
 *   ----------------------------
 *   revision 0.15
 *   date: 2005/01/27 14:41:00;  author: joeb
 *   comments: Moved Common MMRs to cdefBF534.h
 */

/*
 * System MMR Register Map
 */

#ifndef _CDEF_BF537_H
#define _CDEF_BF537_H

/* Include MMRs Common to BF534 */
#include <asm/arch-bf537/cdefBF534.h>

/* Include all Core registers and bit definitions */
#include <asm/arch-bf537/defBF537.h>

/* Include Macro "Defines" For EMAC (Unique to BF536/BF537 */
/* 10/100 Ethernet Controller	(0xFFC03000 - 0xFFC031FF) */
#define	pEMAC_OPMODE		((volatile unsigned long  *)EMAC_OPMODE)
#define pEMAC_ADDRLO		((volatile unsigned long  *)EMAC_ADDRLO)
#define pEMAC_ADDRHI		((volatile unsigned long  *)EMAC_ADDRHI)
#define pEMAC_HASHLO		((volatile unsigned long  *)EMAC_HASHLO)
#define pEMAC_HASHHI		((volatile unsigned long  *)EMAC_HASHHI)
#define pEMAC_STAADD		((volatile unsigned long  *)EMAC_STAADD)
#define pEMAC_STADAT		((volatile unsigned long  *)EMAC_STADAT)
#define pEMAC_FLC		((volatile unsigned long  *)EMAC_FLC)
#define pEMAC_VLAN1		((volatile unsigned long  *)EMAC_VLAN1)
#define pEMAC_VLAN2		((volatile unsigned long  *)EMAC_VLAN2)
#define pEMAC_WKUP_CTL		((volatile unsigned long  *)EMAC_WKUP_CTL)
#define pEMAC_WKUP_FFMSK0	((volatile unsigned long  *)EMAC_WKUP_FFMSK0)
#define pEMAC_WKUP_FFMSK1	((volatile unsigned long  *)EMAC_WKUP_FFMSK1)
#define pEMAC_WKUP_FFMSK2	((volatile unsigned long  *)EMAC_WKUP_FFMSK2)
#define pEMAC_WKUP_FFMSK3	((volatile unsigned long  *)EMAC_WKUP_FFMSK3)
#define pEMAC_WKUP_FFCMD	((volatile unsigned long  *)EMAC_WKUP_FFCMD)
#define pEMAC_WKUP_FFOFF	((volatile unsigned long  *)EMAC_WKUP_FFOFF)
#define pEMAC_WKUP_FFCRC0	((volatile unsigned long  *)EMAC_WKUP_FFCRC0)
#define pEMAC_WKUP_FFCRC1	((volatile unsigned long  *)EMAC_WKUP_FFCRC1)

#define	pEMAC_SYSCTL		((volatile unsigned long  *)EMAC_SYSCTL)
#define pEMAC_SYSTAT		((volatile unsigned long  *)EMAC_SYSTAT)
#define pEMAC_RX_STAT		((volatile unsigned long  *)EMAC_RX_STAT)
#define pEMAC_RX_STKY		((volatile unsigned long  *)EMAC_RX_STKY)
#define pEMAC_RX_IRQE		((volatile unsigned long  *)EMAC_RX_IRQE)
#define pEMAC_TX_STAT		((volatile unsigned long  *)EMAC_TX_STAT)
#define pEMAC_TX_STKY		((volatile unsigned long  *)EMAC_TX_STKY)
#define pEMAC_TX_IRQE		((volatile unsigned long  *)EMAC_TX_IRQE)

#define pEMAC_MMC_CTL		((volatile unsigned long  *)EMAC_MMC_CTL)
#define pEMAC_MMC_RIRQS		((volatile unsigned long  *)EMAC_MMC_RIRQS)
#define pEMAC_MMC_RIRQE		((volatile unsigned long  *)EMAC_MMC_RIRQE)
#define pEMAC_MMC_TIRQS		((volatile unsigned long  *)EMAC_MMC_TIRQS)
#define pEMAC_MMC_TIRQE		((volatile unsigned long  *)EMAC_MMC_TIRQE)

#define pEMAC_RXC_OK		((volatile unsigned long  *)EMAC_RXC_OK)
#define pEMAC_RXC_FCS		((volatile unsigned long  *)EMAC_RXC_FCS)
#define pEMAC_RXC_ALIGN		((volatile unsigned long  *)EMAC_RXC_ALIGN)
#define pEMAC_RXC_OCTET		((volatile unsigned long  *)EMAC_RXC_OCTET)
#define pEMAC_RXC_DMAOVF	((volatile unsigned long  *)EMAC_RXC_DMAOVF)
#define pEMAC_RXC_UNICST	((volatile unsigned long  *)EMAC_RXC_UNICST)
#define pEMAC_RXC_MULTI		((volatile unsigned long  *)EMAC_RXC_MULTI)
#define pEMAC_RXC_BROAD		((volatile unsigned long  *)EMAC_RXC_BROAD)
#define pEMAC_RXC_LNERRI	((volatile unsigned long  *)EMAC_RXC_LNERRI)
#define pEMAC_RXC_LNERRO	((volatile unsigned long  *)EMAC_RXC_LNERRO)
#define pEMAC_RXC_LONG		((volatile unsigned long  *)EMAC_RXC_LONG)
#define pEMAC_RXC_MACCTL	((volatile unsigned long  *)EMAC_RXC_MACCTL)
#define pEMAC_RXC_OPCODE	((volatile unsigned long  *)EMAC_RXC_OPCODE)
#define pEMAC_RXC_PAUSE		((volatile unsigned long  *)EMAC_RXC_PAUSE)
#define pEMAC_RXC_ALLFRM	((volatile unsigned long  *)EMAC_RXC_ALLFRM)
#define pEMAC_RXC_ALLOCT	((volatile unsigned long  *)EMAC_RXC_ALLOCT)
#define pEMAC_RXC_TYPED		((volatile unsigned long  *)EMAC_RXC_TYPED)
#define pEMAC_RXC_SHORT		((volatile unsigned long  *)EMAC_RXC_SHORT)
#define pEMAC_RXC_EQ64		((volatile unsigned long  *)EMAC_RXC_EQ64)
#define	pEMAC_RXC_LT128		((volatile unsigned long  *)EMAC_RXC_LT128)
#define pEMAC_RXC_LT256		((volatile unsigned long  *)EMAC_RXC_LT256)
#define pEMAC_RXC_LT512		((volatile unsigned long  *)EMAC_RXC_LT512)
#define pEMAC_RXC_LT1024	((volatile unsigned long  *)EMAC_RXC_LT1024)
#define pEMAC_RXC_GE1024	((volatile unsigned long  *)EMAC_RXC_GE1024)

#define pEMAC_TXC_OK		((volatile unsigned long  *)EMAC_TXC_OK)
#define pEMAC_TXC_1COL		((volatile unsigned long  *)EMAC_TXC_1COL)
#define pEMAC_TXC_GT1COL	((volatile unsigned long  *)EMAC_TXC_GT1COL)
#define pEMAC_TXC_OCTET		((volatile unsigned long  *)EMAC_TXC_OCTET)
#define pEMAC_TXC_DEFER		((volatile unsigned long  *)EMAC_TXC_DEFER)
#define pEMAC_TXC_LATECL	((volatile unsigned long  *)EMAC_TXC_LATECL)
#define pEMAC_TXC_XS_COL	((volatile unsigned long  *)EMAC_TXC_XS_COL)
#define pEMAC_TXC_DMAUND	((volatile unsigned long  *)EMAC_TXC_DMAUND)
#define pEMAC_TXC_CRSERR	((volatile unsigned long  *)EMAC_TXC_CRSERR)
#define pEMAC_TXC_UNICST	((volatile unsigned long  *)EMAC_TXC_UNICST)
#define pEMAC_TXC_MULTI		((volatile unsigned long  *)EMAC_TXC_MULTI)
#define pEMAC_TXC_BROAD		((volatile unsigned long  *)EMAC_TXC_BROAD)
#define pEMAC_TXC_XS_DFR	((volatile unsigned long  *)EMAC_TXC_XS_DFR)
#define pEMAC_TXC_MACCTL	((volatile unsigned long  *)EMAC_TXC_MACCTL)
#define pEMAC_TXC_ALLFRM	((volatile unsigned long  *)EMAC_TXC_ALLFRM)
#define pEMAC_TXC_ALLOCT	((volatile unsigned long  *)EMAC_TXC_ALLOCT)
#define pEMAC_TXC_EQ64		((volatile unsigned long  *)EMAC_TXC_EQ64)
#define pEMAC_TXC_LT128		((volatile unsigned long  *)EMAC_TXC_LT128)
#define pEMAC_TXC_LT256		((volatile unsigned long  *)EMAC_TXC_LT256)
#define pEMAC_TXC_LT512		((volatile unsigned long  *)EMAC_TXC_LT512)
#define pEMAC_TXC_LT1024	((volatile unsigned long  *)EMAC_TXC_LT1024)
#define pEMAC_TXC_GE1024	((volatile unsigned long  *)EMAC_TXC_GE1024)
#define pEMAC_TXC_ABORT		((volatile unsigned long  *)EMAC_TXC_ABORT)

#endif				/* _CDEF_BF537_H */
