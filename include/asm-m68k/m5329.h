/*
 * mcf5329.h -- Definitions for Freescale Coldfire 5329
 *
 * Copyright (C) 2004-2007 Freescale Semiconductor, Inc.
 * TsiChung Liew (Tsi-Chung.Liew@freescale.com)
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

#ifndef mcf5329_h
#define mcf5329_h
/****************************************************************************/

/*********************************************************************
* System Control Module (SCM)
*********************************************************************/
/* Bit definitions and macros for SCM_MPR */
#define SCM_MPR_MPROT0(x)		(((x)&0x0F)<<28)
#define SCM_MPR_MPROT1(x)		(((x)&0x0F)<<24)
#define SCM_MPR_MPROT2(x)		(((x)&0x0F)<<20)
#define SCM_MPR_MPROT4(x)		(((x)&0x0F)<<12)
#define SCM_MPR_MPROT5(x)		(((x)&0x0F)<<8)
#define SCM_MPR_MPROT6(x)		(((x)&0x0F)<<4)
#define MPROT_MTR			4
#define MPROT_MTW			2
#define MPROT_MPL			1

/* Bit definitions and macros for SCM_BMT */
#define BMT_BME				(0x08)
#define BMT_8				(0x07)
#define BMT_16				(0x06)
#define BMT_32				(0x05)
#define BMT_64				(0x04)
#define BMT_128				(0x03)
#define BMT_256				(0x02)
#define BMT_512				(0x01)
#define BMT_1024			(0x00)

/* Bit definitions and macros for SCM_PACRA */
#define SCM_PACRA_PACR0(x)		(((x)&0x0F)<<28)
#define SCM_PACRA_PACR1(x)		(((x)&0x0F)<<24)
#define SCM_PACRA_PACR2(x)		(((x)&0x0F)<<20)
#define PACR_SP	4
#define PACR_WP	2
#define PACR_TP	1

/* Bit definitions and macros for SCM_PACRB */
#define SCM_PACRB_PACR8(x)		(((x)&0x0F)<<28)
#define SCM_PACRB_PACR12(x)		(((x)&0x0F)<<12)

/* Bit definitions and macros for SCM_PACRC */
#define SCM_PACRC_PACR16(x)		(((x)&0x0F)<<28)
#define SCM_PACRC_PACR17(x)		(((x)&0x0F)<<24)
#define SCM_PACRC_PACR18(x)		(((x)&0x0F)<<20)
#define SCM_PACRC_PACR19(x)		(((x)&0x0F)<<16)
#define SCM_PACRC_PACR21(x)		(((x)&0x0F)<<8)
#define SCM_PACRC_PACR22(x)		(((x)&0x0F)<<4)
#define SCM_PACRC_PACR23(x)		(((x)&0x0F)<<0)

/* Bit definitions and macros for SCM_PACRD */
#define SCM_PACRD_PACR24(x)		(((x)&0x0F)<<28)
#define SCM_PACRD_PACR25(x)		(((x)&0x0F)<<24)
#define SCM_PACRD_PACR26(x)		(((x)&0x0F)<<20)
#define SCM_PACRD_PACR28(x)		(((x)&0x0F)<<12)
#define SCM_PACRD_PACR29(x)		(((x)&0x0F)<<8)
#define SCM_PACRD_PACR30(x)		(((x)&0x0F)<<4)
#define SCM_PACRD_PACR31(x)		(((x)&0x0F)<<0)

/* Bit definitions and macros for SCM_PACRE */
#define SCM_PACRE_PACR32(x)		(((x)&0x0F)<<28)
#define SCM_PACRE_PACR33(x)		(((x)&0x0F)<<24)
#define SCM_PACRE_PACR34(x)		(((x)&0x0F)<<20)
#define SCM_PACRE_PACR35(x)		(((x)&0x0F)<<16)
#define SCM_PACRE_PACR36(x)		(((x)&0x0F)<<12)
#define SCM_PACRE_PACR37(x)		(((x)&0x0F)<<8)
#define SCM_PACRE_PACR38(x)		(((x)&0x0F)<<4)

/* Bit definitions and macros for SCM_PACRF */
#define SCM_PACRF_PACR40(x)		(((x)&0x0F)<<28)
#define SCM_PACRF_PACR41(x)		(((x)&0x0F)<<24)
#define SCM_PACRF_PACR42(x)		(((x)&0x0F)<<20)
#define SCM_PACRF_PACR43(x)		(((x)&0x0F)<<16)
#define SCM_PACRF_PACR44(x)		(((x)&0x0F)<<12)
#define SCM_PACRF_PACR45(x)		(((x)&0x0F)<<8)
#define SCM_PACRF_PACR46(x)		(((x)&0x0F)<<4)
#define SCM_PACRF_PACR47(x)		(((x)&0x0F)<<0)

/* Bit definitions and macros for SCM_PACRG */
#define SCM_PACRG_PACR48(x)		(((x)&0x0F)<<28)

/* Bit definitions and macros for SCM_PACRH */
#define SCM_PACRH_PACR56(x)		(((x)&0x0F)<<28)
#define SCM_PACRH_PACR57(x)		(((x)&0x0F)<<24)
#define SCM_PACRH_PACR58(x)		(((x)&0x0F)<<20)

/* PACRn Assignments */
#define PACR0(x)			SCM_PACRA_PACR0(x)
#define PACR1(x)			SCM_PACRA_PACR1(x)
#define PACR2(x)			SCM_PACRA_PACR2(x)
#define PACR8(x)			SCM_PACRB_PACR8(x)
#define PACR12(x)			SCM_PACRB_PACR12(x)
#define PACR16(x)			SCM_PACRC_PACR16(x)
#define PACR17(x)			SCM_PACRC_PACR17(x)
#define PACR18(x)			SCM_PACRC_PACR18(x)
#define PACR19(x)			SCM_PACRC_PACR19(x)
#define PACR21(x)			SCM_PACRC_PACR21(x)
#define PACR22(x)			SCM_PACRC_PACR22(x)
#define PACR23(x)			SCM_PACRC_PACR23(x)
#define PACR24(x)			SCM_PACRD_PACR24(x)
#define PACR25(x)			SCM_PACRD_PACR25(x)
#define PACR26(x)			SCM_PACRD_PACR26(x)
#define PACR28(x)			SCM_PACRD_PACR28(x)
#define PACR29(x)			SCM_PACRD_PACR29(x)
#define PACR30(x)			SCM_PACRD_PACR30(x)
#define PACR31(x)			SCM_PACRD_PACR31(x)
#define PACR32(x)			SCM_PACRE_PACR32(x)
#define PACR33(x)			SCM_PACRE_PACR33(x)
#define PACR34(x)			SCM_PACRE_PACR34(x)
#define PACR35(x)			SCM_PACRE_PACR35(x)
#define PACR36(x)			SCM_PACRE_PACR36(x)
#define PACR37(x)			SCM_PACRE_PACR37(x)
#define PACR38(x)			SCM_PACRE_PACR38(x)
#define PACR40(x)			SCM_PACRF_PACR40(x)
#define PACR41(x)			SCM_PACRF_PACR41(x)
#define PACR42(x)			SCM_PACRF_PACR42(x)
#define PACR43(x)			SCM_PACRF_PACR43(x)
#define PACR44(x)			SCM_PACRF_PACR44(x)
#define PACR45(x)			SCM_PACRF_PACR45(x)
#define PACR46(x)			SCM_PACRF_PACR46(x)
#define PACR47(x)			SCM_PACRF_PACR47(x)
#define PACR48(x)			SCM_PACRG_PACR48(x)
#define PACR56(x)			SCM_PACRH_PACR56(x)
#define PACR57(x)			SCM_PACRH_PACR57(x)
#define PACR58(x)			SCM_PACRH_PACR58(x)

/* Bit definitions and macros for SCM_CWCR */
#define CWCR_RO				(0x8000)
#define CWCR_CWR_WH			(0x0100)
#define CWCR_CWE			(0x0080)
#define CWRI_WINDOW			(0x0060)
#define CWRI_RESET			(0x0040)
#define CWRI_INT_RESET			(0x0020)
#define CWRI_INT			(0x0000)
#define CWCR_CWT(x)			(((x)&0x001F))

/* Bit definitions and macros for SCM_ISR */
#define SCMISR_CFEI			(0x02)
#define SCMISR_CWIC			(0x01)

/* Bit definitions and macros for SCM_BCR */
#define BCR_GBR				(0x00000200)
#define BCR_GBW				(0x00000100)
#define BCR_S7				(0x00000080)
#define BCR_S6				(0x00000040)
#define BCR_S4				(0x00000010)
#define BCR_S1				(0x00000002)

/* Bit definitions and macros for SCM_CFIER */
#define CFIER_ECFEI			(0x01)

/* Bit definitions and macros for SCM_CFLOC */
#define CFLOC_LOC			(0x80)

/* Bit definitions and macros for SCM_CFATR */
#define CFATR_WRITE			(0x80)
#define CFATR_SZ32			(0x20)
#define CFATR_SZ16			(0x10)
#define CFATR_SZ08			(0x00)
#define CFATR_CACHE			(0x08)
#define CFATR_MODE			(0x02)
#define CFATR_TYPE			(0x01)

/*********************************************************************
* FlexBus Chip Selects (FBCS)
*********************************************************************/
/* Bit definitions and macros for FBCS_CSAR */
#define CSAR_BA(x)			(((x)&0xFFFF)<<16)

/* Bit definitions and macros for FBCS_CSMR */
#define CSMR_BAM(x)			(((x)&0xFFFF)<<16)
#define CSMR_BAM_4G			(0xFFFF0000)
#define CSMR_BAM_2G			(0x7FFF0000)
#define CSMR_BAM_1G			(0x3FFF0000)
#define CSMR_BAM_1024M			(0x3FFF0000)
#define CSMR_BAM_512M			(0x1FFF0000)
#define CSMR_BAM_256M			(0x0FFF0000)
#define CSMR_BAM_128M			(0x07FF0000)
#define CSMR_BAM_64M			(0x03FF0000)
#define CSMR_BAM_32M			(0x01FF0000)
#define CSMR_BAM_16M			(0x00FF0000)
#define CSMR_BAM_8M			(0x007F0000)
#define CSMR_BAM_4M			(0x003F0000)
#define CSMR_BAM_2M			(0x001F0000)
#define CSMR_BAM_1M			(0x000F0000)
#define CSMR_BAM_1024K			(0x000F0000)
#define CSMR_BAM_512K			(0x00070000)
#define CSMR_BAM_256K			(0x00030000)
#define CSMR_BAM_128K			(0x00010000)
#define CSMR_BAM_64K			(0x00000000)
#define CSMR_WP				(0x00000100)
#define CSMR_V				(0x00000001)

/* Bit definitions and macros for FBCS_CSCR */
#define CSCR_SWS(x)			(((x)&0x3F)<<26)
#define CSCR_ASET(x)			(((x)&0x03)<<20)
#define CSCR_SWSEN			(0x00800000)
#define CSCR_ASET_4CLK			(0x00300000)
#define CSCR_ASET_3CLK			(0x00200000)
#define CSCR_ASET_2CLK			(0x00100000)
#define CSCR_ASET_1CLK			(0x00000000)
#define CSCR_RDAH(x)			(((x)&0x03)<<18)
#define CSCR_RDAH_4CYC			(0x000C0000)
#define CSCR_RDAH_3CYC			(0x00080000)
#define CSCR_RDAH_2CYC			(0x00040000)
#define CSCR_RDAH_1CYC			(0x00000000)
#define CSCR_WRAH(x)			(((x)&0x03)<<16)
#define CSCR_WDAH_4CYC			(0x00003000)
#define CSCR_WDAH_3CYC			(0x00002000)
#define CSCR_WDAH_2CYC			(0x00001000)
#define CSCR_WDAH_1CYC			(0x00000000)
#define CSCR_WS(x)			(((x)&0x3F)<<10)
#define CSCR_SBM			(0x00000200)
#define CSCR_AA				(0x00000100)
#define CSCR_PS_MASK			(0x000000C0)
#define CSCR_PS_32			(0x00000000)
#define CSCR_PS_16			(0x00000080)
#define CSCR_PS_8			(0x00000040)
#define CSCR_BEM			(0x00000020)
#define CSCR_BSTR			(0x00000010)
#define CSCR_BSTW			(0x00000008)

/*********************************************************************
* FlexCAN Module (CAN)
*********************************************************************/
/* Bit definitions and macros for CAN_CANMCR */
#define CANMCR_MDIS			(0x80000000)
#define CANMCR_FRZ			(0x40000000)
#define CANMCR_HALT			(0x10000000)
#define CANMCR_NORDY			(0x08000000)
#define CANMCR_SOFTRST			(0x02000000)
#define CANMCR_FRZACK			(0x01000000)
#define CANMCR_SUPV			(0x00800000)
#define CANMCR_LPMACK			(0x00100000)
#define CANMCR_MAXMB(x)			(((x)&0x0F))

/* Bit definitions and macros for CAN_CANCTRL */
#define CANCTRL_PRESDIV(x)		(((x)&0xFF)<<24)
#define CANCTRL_RJW(x)			(((x)&0x03)<<22)
#define CANCTRL_PSEG1(x)		(((x)&0x07)<<19)
#define CANCTRL_PSEG2(x)		(((x)&0x07)<<16)
#define CANCTRL_BOFFMSK			(0x00008000)
#define CANCTRL_ERRMSK			(0x00004000)
#define CANCTRL_CLKSRC			(0x00002000)
#define CANCTRL_LPB			(0x00001000)
#define CANCTRL_SMP			(0x00000080)
#define CANCTRL_BOFFREC			(0x00000040)
#define CANCTRL_TSYNC			(0x00000020)
#define CANCTRL_LBUF			(0x00000010)
#define CANCTRL_LOM			(0x00000008)
#define CANCTRL_PROPSEG(x)		(((x)&0x07))

/* Bit definitions and macros for CAN_TIMER */
#define TIMER_TIMER(x)			((x)&0xFFFF)

/* Bit definitions and macros for CAN_RXGMASK */
#define RXGMASK_MI(x)			((x)&0x1FFFFFFF)

/* Bit definitions and macros for CAN_ERRCNT */
#define ERRCNT_TXECTR(x)		(((x)&0xFF))
#define ERRCNT_RXECTR(x)		(((x)&0xFF)<<8)

/* Bit definitions and macros for CAN_ERRSTAT */
#define ERRSTAT_BITERR1			(0x00008000)
#define ERRSTAT_BITERR0			(0x00004000)
#define ERRSTAT_ACKERR			(0x00002000)
#define ERRSTAT_CRCERR			(0x00001000)
#define ERRSTAT_FRMERR			(0x00000800)
#define ERRSTAT_STFERR			(0x00000400)
#define ERRSTAT_TXWRN			(0x00000200)
#define ERRSTAT_RXWRN			(0x00000100)
#define ERRSTAT_IDLE			(0x00000080)
#define ERRSTAT_TXRX			(0x00000040)
#define ERRSTAT_FLT_BUSOFF		(0x00000020)
#define ERRSTAT_FLT_PASSIVE		(0x00000010)
#define ERRSTAT_FLT_ACTIVE		(0x00000000)
#define ERRSTAT_BOFFINT			(0x00000004)
#define ERRSTAT_ERRINT			(0x00000002)
#define ERRSTAT_WAKINT			(0x00000001)

/* Bit definitions and macros for CAN_IMASK */
#define IMASK_BUF15M			(0x00008000)
#define IMASK_BUF14M			(0x00004000)
#define IMASK_BUF13M			(0x00002000)
#define IMASK_BUF12M			(0x00001000)
#define IMASK_BUF11M			(0x00000800)
#define IMASK_BUF10M			(0x00000400)
#define IMASK_BUF9M			(0x00000200)
#define IMASK_BUF8M			(0x00000100)
#define IMASK_BUF7M			(0x00000080)
#define IMASK_BUF6M			(0x00000040)
#define IMASK_BUF5M			(0x00000020)
#define IMASK_BUF4M			(0x00000010)
#define IMASK_BUF3M			(0x00000008)
#define IMASK_BUF2M			(0x00000004)
#define IMASK_BUF1M			(0x00000002)
#define IMASK_BUF0M			(0x00000001)

/* Bit definitions and macros for CAN_IFLAG */
#define IFLAG_BUF15I			(0x00008000)
#define IFLAG_BUF14I			(0x00004000)
#define IFLAG_BUF13I			(0x00002000)
#define IFLAG_BUF12I			(0x00001000)
#define IFLAG_BUF11I			(0x00000800)
#define IFLAG_BUF10I			(0x00000400)
#define IFLAG_BUF9I			(0x00000200)
#define IFLAG_BUF8I			(0x00000100)
#define IFLAG_BUF7I			(0x00000080)
#define IFLAG_BUF6I			(0x00000040)
#define IFLAG_BUF5I			(0x00000020)
#define IFLAG_BUF4I			(0x00000010)
#define IFLAG_BUF3I			(0x00000008)
#define IFLAG_BUF2I			(0x00000004)
#define IFLAG_BUF1I			(0x00000002)
#define IFLAG_BUF0I			(0x00000001)

/*********************************************************************
* Interrupt Controller (INTC)
*********************************************************************/
#define INTC0_EPORT			INTC_IPRL_INT1

#define INT0_LO_RSVD0			(0)
#define INT0_LO_EPORT1			(1)
#define INT0_LO_EPORT2			(2)
#define INT0_LO_EPORT3			(3)
#define INT0_LO_EPORT4			(4)
#define INT0_LO_EPORT5			(5)
#define INT0_LO_EPORT6			(6)
#define INT0_LO_EPORT7			(7)
#define INT0_LO_EDMA_00			(8)
#define INT0_LO_EDMA_01			(9)
#define INT0_LO_EDMA_02			(10)
#define INT0_LO_EDMA_03			(11)
#define INT0_LO_EDMA_04			(12)
#define INT0_LO_EDMA_05			(13)
#define INT0_LO_EDMA_06			(14)
#define INT0_LO_EDMA_07			(15)
#define INT0_LO_EDMA_08			(16)
#define INT0_LO_EDMA_09			(17)
#define INT0_LO_EDMA_10			(18)
#define INT0_LO_EDMA_11			(19)
#define INT0_LO_EDMA_12			(20)
#define INT0_LO_EDMA_13			(21)
#define INT0_LO_EDMA_14			(22)
#define INT0_LO_EDMA_15			(23)
#define INT0_LO_EDMA_ERR		(24)
#define INT0_LO_SCM			(25)
#define INT0_LO_UART0			(26)
#define INT0_LO_UART1			(27)
#define INT0_LO_UART2			(28)
#define INT0_LO_RSVD1			(29)
#define INT0_LO_I2C			(30)
#define INT0_LO_QSPI			(31)
#define INT0_HI_DTMR0			(32)
#define INT0_HI_DTMR1			(33)
#define INT0_HI_DTMR2			(34)
#define INT0_HI_DTMR3			(35)
#define INT0_HI_FEC_TXF			(36)
#define INT0_HI_FEC_TXB			(37)
#define INT0_HI_FEC_UN			(38)
#define INT0_HI_FEC_RL			(39)
#define INT0_HI_FEC_RXF			(40)
#define INT0_HI_FEC_RXB			(41)
#define INT0_HI_FEC_MII			(42)
#define INT0_HI_FEC_LC			(43)
#define INT0_HI_FEC_HBERR		(44)
#define INT0_HI_FEC_GRA			(45)
#define INT0_HI_FEC_EBERR		(46)
#define INT0_HI_FEC_BABT		(47)
#define INT0_HI_FEC_BABR		(48)
/* 49 - 61 Reserved */
#define INT0_HI_SCM			(62)

/* Bit definitions and macros for INTC_IPRH */
#define INTC_IPRH_INT63			(0x80000000)
#define INTC_IPRH_INT62			(0x40000000)
#define INTC_IPRH_INT61			(0x20000000)
#define INTC_IPRH_INT60			(0x10000000)
#define INTC_IPRH_INT59			(0x08000000)
#define INTC_IPRH_INT58			(0x04000000)
#define INTC_IPRH_INT57			(0x02000000)
#define INTC_IPRH_INT56			(0x01000000)
#define INTC_IPRH_INT55			(0x00800000)
#define INTC_IPRH_INT54			(0x00400000)
#define INTC_IPRH_INT53			(0x00200000)
#define INTC_IPRH_INT52			(0x00100000)
#define INTC_IPRH_INT51			(0x00080000)
#define INTC_IPRH_INT50			(0x00040000)
#define INTC_IPRH_INT49			(0x00020000)
#define INTC_IPRH_INT48			(0x00010000)
#define INTC_IPRH_INT47			(0x00008000)
#define INTC_IPRH_INT46			(0x00004000)
#define INTC_IPRH_INT45			(0x00002000)
#define INTC_IPRH_INT44			(0x00001000)
#define INTC_IPRH_INT43			(0x00000800)
#define INTC_IPRH_INT42			(0x00000400)
#define INTC_IPRH_INT41			(0x00000200)
#define INTC_IPRH_INT40			(0x00000100)
#define INTC_IPRH_INT39			(0x00000080)
#define INTC_IPRH_INT38			(0x00000040)
#define INTC_IPRH_INT37			(0x00000020)
#define INTC_IPRH_INT36			(0x00000010)
#define INTC_IPRH_INT35			(0x00000008)
#define INTC_IPRH_INT34			(0x00000004)
#define INTC_IPRH_INT33			(0x00000002)
#define INTC_IPRH_INT32			(0x00000001)

/* Bit definitions and macros for INTC_IPRL */
#define INTC_IPRL_INT31			(0x80000000)
#define INTC_IPRL_INT30			(0x40000000)
#define INTC_IPRL_INT29			(0x20000000)
#define INTC_IPRL_INT28			(0x10000000)
#define INTC_IPRL_INT27			(0x08000000)
#define INTC_IPRL_INT26			(0x04000000)
#define INTC_IPRL_INT25			(0x02000000)
#define INTC_IPRL_INT24			(0x01000000)
#define INTC_IPRL_INT23			(0x00800000)
#define INTC_IPRL_INT22			(0x00400000)
#define INTC_IPRL_INT21			(0x00200000)
#define INTC_IPRL_INT20			(0x00100000)
#define INTC_IPRL_INT19			(0x00080000)
#define INTC_IPRL_INT18			(0x00040000)
#define INTC_IPRL_INT17			(0x00020000)
#define INTC_IPRL_INT16			(0x00010000)
#define INTC_IPRL_INT15			(0x00008000)
#define INTC_IPRL_INT14			(0x00004000)
#define INTC_IPRL_INT13			(0x00002000)
#define INTC_IPRL_INT12			(0x00001000)
#define INTC_IPRL_INT11			(0x00000800)
#define INTC_IPRL_INT10			(0x00000400)
#define INTC_IPRL_INT9			(0x00000200)
#define INTC_IPRL_INT8			(0x00000100)
#define INTC_IPRL_INT7			(0x00000080)
#define INTC_IPRL_INT6			(0x00000040)
#define INTC_IPRL_INT5			(0x00000020)
#define INTC_IPRL_INT4			(0x00000010)
#define INTC_IPRL_INT3			(0x00000008)
#define INTC_IPRL_INT2			(0x00000004)
#define INTC_IPRL_INT1			(0x00000002)
#define INTC_IPRL_INT0			(0x00000001)

/* Bit definitions and macros for INTC_ICONFIG */
#define INTC_ICFG_ELVLPRI7		(0x8000)
#define INTC_ICFG_ELVLPRI6		(0x4000)
#define INTC_ICFG_ELVLPRI5		(0x2000)
#define INTC_ICFG_ELVLPRI4		(0x1000)
#define INTC_ICFG_ELVLPRI3		(0x0800)
#define INTC_ICFG_ELVLPRI2		(0x0400)
#define INTC_ICFG_ELVLPRI1		(0x0200)
#define INTC_ICFG_EMASK			(0x0020)

/* Bit definitions and macros for INTC_SIMR */
#define INTC_SIMR_SALL			(0x40)
#define INTC_SIMR_SIMR(x)		((x)&0x3F)

/* Bit definitions and macros for INTC_CIMR */
#define INTC_CIMR_CALL			(0x40)
#define INTC_CIMR_CIMR(x)		((x)&0x3F)

/* Bit definitions and macros for INTC_CLMASK */
#define INTC_CLMASK_CLMASK(x)		((x)&0x0F)

/* Bit definitions and macros for INTC_SLMASK */
#define INTC_SLMASK_SLMASK(x)		((x)&0x0F)

/* Bit definitions and macros for INTC_ICR */
#define INTC_ICR_IL(x)			((x)&0x07)

/*********************************************************************
* Queued Serial Peripheral Interface (QSPI)
*********************************************************************/
/* Bit definitions and macros for QSPI_QMR */
#define QSPI_QMR_MSTR			(0x8000)
#define QSPI_QMR_DOHIE			(0x4000)
#define QSPI_QMR_BITS(x)		(((x)&0x000F)<<10)
#define QSPI_QMR_CPOL			(0x0200)
#define QSPI_QMR_CPHA			(0x0100)
#define QSPI_QMR_BAUD(x)		((x)&0x00FF)

/* Bit definitions and macros for QSPI_QDLYR */
#define QSPI_QDLYR_SPE			(0x8000)
#define QSPI_QDLYR_QCD(x)		(((x)&0x007F)<<8)
#define QSPI_QDLYR_DTL(x)		((x)&0x00FF)

/* Bit definitions and macros for QSPI_QWR */
#define QSPI_QWR_NEWQP(x)		((x)&0x000F)
#define QSPI_QWR_ENDQP(x)		(((x)&0x000F)<<8)
#define QSPI_QWR_CSIV			(0x1000)
#define QSPI_QWR_WRTO			(0x2000)
#define QSPI_QWR_WREN			(0x4000)
#define QSPI_QWR_HALT			(0x8000)

/* Bit definitions and macros for QSPI_QIR */
#define QSPI_QIR_WCEFB			(0x8000)
#define QSPI_QIR_ABRTB			(0x4000)
#define QSPI_QIR_ABRTL			(0x1000)
#define QSPI_QIR_WCEFE			(0x0800)
#define QSPI_QIR_ABRTE			(0x0400)
#define QSPI_QIR_SPIFE			(0x0100)
#define QSPI_QIR_WCEF			(0x0008)
#define QSPI_QIR_ABRT			(0x0004)
#define QSPI_QIR_SPIF			(0x0001)

/* Bit definitions and macros for QSPI_QAR */
#define QSPI_QAR_ADDR(x)		((x)&0x003F)
#define QSPI_QAR_TRANS			(0x0000)
#define QSPI_QAR_RECV			(0x0010)
#define QSPI_QAR_CMD			(0x0020)

/* Bit definitions and macros for QSPI_QDR */
#define QSPI_QDR_CONT			(0x8000)
#define QSPI_QDR_BITSE			(0x4000)
#define QSPI_QDR_DT			(0x2000)
#define QSPI_QDR_DSCK			(0x1000)
#define QSPI_QDR_QSPI_CS3		(0x0800)
#define QSPI_QDR_QSPI_CS2		(0x0400)
#define QSPI_QDR_QSPI_CS1		(0x0200)
#define QSPI_QDR_QSPI_CS0		(0x0100)

/*********************************************************************
* Pulse Width Modulation (PWM)
*********************************************************************/
/* Bit definitions and macros for PWM_E */
#define PWM_EN_PWME7			(0x80)
#define PWM_EN_PWME5			(0x20)
#define PWM_EN_PWME3			(0x08)
#define PWM_EN_PWME1			(0x02)

/* Bit definitions and macros for PWM_POL */
#define PWM_POL_PPOL7			(0x80)
#define PWM_POL_PPOL5			(0x20)
#define PWM_POL_PPOL3			(0x08)
#define PWM_POL_PPOL1			(0x02)

/* Bit definitions and macros for PWM_CLK */
#define PWM_CLK_PCLK7			(0x80)
#define PWM_CLK_PCLK5			(0x20)
#define PWM_CLK_PCLK3			(0x08)
#define PWM_CLK_PCLK1			(0x02)

/* Bit definitions and macros for PWM_PRCLK */
#define PWM_PRCLK_PCKB(x)		(((x)&0x07)<<4)
#define PWM_PRCLK_PCKA(x)		((x)&0x07)

/* Bit definitions and macros for PWM_CAE */
#define PWM_CAE_CAE7			(0x80)
#define PWM_CAE_CAE5			(0x20)
#define PWM_CAE_CAE3			(0x08)
#define PWM_CAE_CAE1			(0x02)

/* Bit definitions and macros for PWM_CTL */
#define PWM_CTL_CON67			(0x80)
#define PWM_CTL_CON45			(0x40)
#define PWM_CTL_CON23			(0x20)
#define PWM_CTL_CON01			(0x10)
#define PWM_CTL_PSWAR			(0x08)
#define PWM_CTL_PFRZ			(0x04)

/* Bit definitions and macros for PWM_SDN */
#define PWM_SDN_IF			(0x80)
#define PWM_SDN_IE			(0x40)
#define PWM_SDN_RESTART			(0x20)
#define PWM_SDN_LVL			(0x10)
#define PWM_SDN_PWM7IN			(0x04)
#define PWM_SDN_PWM7IL			(0x02)
#define PWM_SDN_SDNEN			(0x01)

/*********************************************************************
* Watchdog Timer Modules (WTM)
*********************************************************************/
/* Bit definitions and macros for WTM_WCR */
#define WTM_WCR_WAIT			(0x0008)
#define WTM_WCR_DOZE			(0x0004)
#define WTM_WCR_HALTED			(0x0002)
#define WTM_WCR_EN			(0x0001)

/*********************************************************************
* Chip Configuration Module (CCM)
*********************************************************************/
/* Bit definitions and macros for CCM_CCR */
#define CCM_CCR_CSC(x)			(((x)&0x0003)<<8|0x0001)
#define CCM_CCR_LIMP			(0x0041)
#define CCM_CCR_LOAD			(0x0021)
#define CCM_CCR_BOOTPS(x)		(((x)&0x0003)<<3|0x0001)
#define CCM_CCR_OSC_MODE		(0x0005)
#define CCM_CCR_PLL_MODE		(0x0003)
#define CCM_CCR_RESERVED		(0x0001)

/* Bit definitions and macros for CCM_RCON */
#define CCM_RCON_CSC(x)			(((x)&0x0003)<<8|0x0001)
#define CCM_RCON_LIMP			(0x0041)
#define CCM_RCON_LOAD			(0x0021)
#define CCM_RCON_BOOTPS(x)		(((x)&0x0003)<<3|0x0001)
#define CCM_RCON_OSC_MODE		(0x0005)
#define CCM_RCON_PLL_MODE		(0x0003)
#define CCM_RCON_RESERVED		(0x0001)

/* Bit definitions and macros for CCM_CIR */
#define CCM_CIR_PIN(x)			(((x)&0x03FF)<<6)
#define CCM_CIR_PRN(x)			((x)&0x003F)

/* Bit definitions and macros for CCM_MISCCR */
#define CCM_MISCCR_PLL_LOCK		(0x2000)
#define CCM_MISCCR_LIMP			(0x1000)
#define CCM_MISCCR_LCD_CHEN		(0x0100)
#define CCM_MISCCR_SSI_PUE		(0x0080)
#define CCM_MISCCR_SSI_PUS		(0x0040)
#define CCM_MISCCR_TIM_DMA		(0x0020)
#define CCM_MISCCR_SSI_SRC		(0x0010)
#define CCM_MISCCR_USBDIV		(0x0002)
#define CCM_MISCCR_USBSRC		(0x0001)

/* Bit definitions and macros for CCM_CDR */
#define CCM_CDR_LPDIV(x)		(((x)&0x000F)<<8)
#define CCM_CDR_SSIDIV(x)		((x)&0x000F)

/* Bit definitions and macros for CCM_UHCSR */
#define CCM_UHCSR_PORTIND(x)		(((x)&0x0003)<<14)
#define CCM_UHCSR_WKUP			(0x0004)
#define CCM_UHCSR_UHMIE			(0x0002)
#define CCM_UHCSR_XPDE			(0x0001)

/* Bit definitions and macros for CCM_UOCSR */
#define CCM_UOCSR_PORTIND(x)		(((x)&0x0003)<<14)
#define CCM_UOCSR_DPPD			(0x2000)
#define CCM_UOCSR_DMPD			(0x1000)
#define CCM_UOCSR_DRV_VBUS		(0x0800)
#define CCM_UOCSR_CRG_VBUS		(0x0400)
#define CCM_UOCSR_DCR_VBUS		(0x0200)
#define CCM_UOCSR_DPPU			(0x0100)
#define CCM_UOCSR_AVLD			(0x0080)
#define CCM_UOCSR_BVLD			(0x0040)
#define CCM_UOCSR_VVLD			(0x0020)
#define CCM_UOCSR_SEND			(0x0010)
#define CCM_UOCSR_PWRFLT		(0x0008)
#define CCM_UOCSR_WKUP			(0x0004)
#define CCM_UOCSR_UOMIE			(0x0002)
#define CCM_UOCSR_XPDE			(0x0001)

/* not done yet */
/*********************************************************************
* General Purpose I/O (GPIO)
*********************************************************************/
/* Bit definitions and macros for GPIO_PODR_FECH_L */
#define GPIO_PODR_FECH_L7		(0x80)
#define GPIO_PODR_FECH_L6		(0x40)
#define GPIO_PODR_FECH_L5		(0x20)
#define GPIO_PODR_FECH_L4		(0x10)
#define GPIO_PODR_FECH_L3		(0x08)
#define GPIO_PODR_FECH_L2		(0x04)
#define GPIO_PODR_FECH_L1		(0x02)
#define GPIO_PODR_FECH_L0		(0x01)

/* Bit definitions and macros for GPIO_PODR_SSI */
#define GPIO_PODR_SSI_4			(0x10)
#define GPIO_PODR_SSI_3			(0x08)
#define GPIO_PODR_SSI_2			(0x04)
#define GPIO_PODR_SSI_1			(0x02)
#define GPIO_PODR_SSI_0			(0x01)

/* Bit definitions and macros for GPIO_PODR_BUSCTL */
#define GPIO_PODR_BUSCTL_3		(0x08)
#define GPIO_PODR_BUSCTL_2		(0x04)
#define GPIO_PODR_BUSCTL_1		(0x02)
#define GPIO_PODR_BUSCTL_0		(0x01)

/* Bit definitions and macros for GPIO_PODR_BE */
#define GPIO_PODR_BE_3			(0x08)
#define GPIO_PODR_BE_2			(0x04)
#define GPIO_PODR_BE_1			(0x02)
#define GPIO_PODR_BE_0			(0x01)

/* Bit definitions and macros for GPIO_PODR_CS */
#define GPIO_PODR_CS_5			(0x20)
#define GPIO_PODR_CS_4			(0x10)
#define GPIO_PODR_CS_3			(0x08)
#define GPIO_PODR_CS_2			(0x04)
#define GPIO_PODR_CS_1			(0x02)

/* Bit definitions and macros for GPIO_PODR_PWM */
#define GPIO_PODR_PWM_5			(0x20)
#define GPIO_PODR_PWM_4			(0x10)
#define GPIO_PODR_PWM_3			(0x08)
#define GPIO_PODR_PWM_2			(0x04)

/* Bit definitions and macros for GPIO_PODR_FECI2C */
#define GPIO_PODR_FECI2C_3		(0x08)
#define GPIO_PODR_FECI2C_2		(0x04)
#define GPIO_PODR_FECI2C_1		(0x02)
#define GPIO_PODR_FECI2C_0		(0x01)

/* Bit definitions and macros for GPIO_PODR_UART */
#define GPIO_PODR_UART_7		(0x80)
#define GPIO_PODR_UART_6		(0x40)
#define GPIO_PODR_UART_5		(0x20)
#define GPIO_PODR_UART_4		(0x10)
#define GPIO_PODR_UART_3		(0x08)
#define GPIO_PODR_UART_2		(0x04)
#define GPIO_PODR_UART_1		(0x02)
#define GPIO_PODR_UART_0		(0x01)

/* Bit definitions and macros for GPIO_PODR_QSPI */
#define GPIO_PODR_QSPI_5		(0x20)
#define GPIO_PODR_QSPI_4		(0x10)
#define GPIO_PODR_QSPI_3		(0x08)
#define GPIO_PODR_QSPI_2		(0x04)
#define GPIO_PODR_QSPI_1		(0x02)
#define GPIO_PODR_QSPI_0		(0x01)

/* Bit definitions and macros for GPIO_PODR_TIMER */
#define GPIO_PODR_TIMER_3		(0x08)
#define GPIO_PODR_TIMER_2		(0x04)
#define GPIO_PODR_TIMER_1		(0x02)
#define GPIO_PODR_TIMER_0		(0x01)

/* Bit definitions and macros for GPIO_PODR_LCDDATAH */
#define GPIO_PODR_LCDDATAH_1		(0x02)
#define GPIO_PODR_LCDDATAH_0		(0x01)

/* Bit definitions and macros for GPIO_PODR_LCDDATAM */
#define GPIO_PODR_LCDDATAM_7		(0x80)
#define GPIO_PODR_LCDDATAM_6		(0x40)
#define GPIO_PODR_LCDDATAM_5		(0x20)
#define GPIO_PODR_LCDDATAM_4		(0x10)
#define GPIO_PODR_LCDDATAM_3		(0x08)
#define GPIO_PODR_LCDDATAM_2		(0x04)
#define GPIO_PODR_LCDDATAM_1		(0x02)
#define GPIO_PODR_LCDDATAM_0		(0x01)

/* Bit definitions and macros for GPIO_PODR_LCDDATAL */
#define GPIO_PODR_LCDDATAL_7		(0x80)
#define GPIO_PODR_LCDDATAL_6		(0x40)
#define GPIO_PODR_LCDDATAL_5		(0x20)
#define GPIO_PODR_LCDDATAL_4		(0x10)
#define GPIO_PODR_LCDDATAL_3		(0x08)
#define GPIO_PODR_LCDDATAL_2		(0x04)
#define GPIO_PODR_LCDDATAL_1		(0x02)
#define GPIO_PODR_LCDDATAL_0		(0x01)

/* Bit definitions and macros for GPIO_PODR_LCDCTLH */
#define GPIO_PODR_LCDCTLH_0		(0x01)

/* Bit definitions and macros for GPIO_PODR_LCDCTLL */
#define GPIO_PODR_LCDCTLL_7		(0x80)
#define GPIO_PODR_LCDCTLL_6		(0x40)
#define GPIO_PODR_LCDCTLL_5		(0x20)
#define GPIO_PODR_LCDCTLL_4		(0x10)
#define GPIO_PODR_LCDCTLL_3		(0x08)
#define GPIO_PODR_LCDCTLL_2		(0x04)
#define GPIO_PODR_LCDCTLL_1		(0x02)
#define GPIO_PODR_LCDCTLL_0		(0x01)

/* Bit definitions and macros for GPIO_PDDR_FECH */
#define GPIO_PDDR_FECH_L7		(0x80)
#define GPIO_PDDR_FECH_L6		(0x40)
#define GPIO_PDDR_FECH_L5		(0x20)
#define GPIO_PDDR_FECH_L4		(0x10)
#define GPIO_PDDR_FECH_L3		(0x08)
#define GPIO_PDDR_FECH_L2		(0x04)
#define GPIO_PDDR_FECH_L1		(0x02)
#define GPIO_PDDR_FECH_L0		(0x01)

/* Bit definitions and macros for GPIO_PDDR_SSI */
#define GPIO_PDDR_SSI_4			(0x10)
#define GPIO_PDDR_SSI_3			(0x08)
#define GPIO_PDDR_SSI_2			(0x04)
#define GPIO_PDDR_SSI_1			(0x02)
#define GPIO_PDDR_SSI_0			(0x01)

/* Bit definitions and macros for GPIO_PDDR_BUSCTL */
#define GPIO_PDDR_BUSCTL_3		(0x08)
#define GPIO_PDDR_BUSCTL_2		(0x04)
#define GPIO_PDDR_BUSCTL_1		(0x02)
#define GPIO_PDDR_BUSCTL_0		(0x01)

/* Bit definitions and macros for GPIO_PDDR_BE */
#define GPIO_PDDR_BE_3			(0x08)
#define GPIO_PDDR_BE_2			(0x04)
#define GPIO_PDDR_BE_1			(0x02)
#define GPIO_PDDR_BE_0			(0x01)

/* Bit definitions and macros for GPIO_PDDR_CS */
#define GPIO_PDDR_CS_1			(0x02)
#define GPIO_PDDR_CS_2			(0x04)
#define GPIO_PDDR_CS_3			(0x08)
#define GPIO_PDDR_CS_4			(0x10)
#define GPIO_PDDR_CS_5			(0x20)

/* Bit definitions and macros for GPIO_PDDR_PWM */
#define GPIO_PDDR_PWM_2			(0x04)
#define GPIO_PDDR_PWM_3			(0x08)
#define GPIO_PDDR_PWM_4			(0x10)
#define GPIO_PDDR_PWM_5			(0x20)

/* Bit definitions and macros for GPIO_PDDR_FECI2C */
#define GPIO_PDDR_FECI2C_0		(0x01)
#define GPIO_PDDR_FECI2C_1		(0x02)
#define GPIO_PDDR_FECI2C_2		(0x04)
#define GPIO_PDDR_FECI2C_3		(0x08)

/* Bit definitions and macros for GPIO_PDDR_UART */
#define GPIO_PDDR_UART_0		(0x01)
#define GPIO_PDDR_UART_1		(0x02)
#define GPIO_PDDR_UART_2		(0x04)
#define GPIO_PDDR_UART_3		(0x08)
#define GPIO_PDDR_UART_4		(0x10)
#define GPIO_PDDR_UART_5		(0x20)
#define GPIO_PDDR_UART_6		(0x40)
#define GPIO_PDDR_UART_7		(0x80)

/* Bit definitions and macros for GPIO_PDDR_QSPI */
#define GPIO_PDDR_QSPI_0		(0x01)
#define GPIO_PDDR_QSPI_1		(0x02)
#define GPIO_PDDR_QSPI_2		(0x04)
#define GPIO_PDDR_QSPI_3		(0x08)
#define GPIO_PDDR_QSPI_4		(0x10)
#define GPIO_PDDR_QSPI_5		(0x20)

/* Bit definitions and macros for GPIO_PDDR_TIMER */
#define GPIO_PDDR_TIMER_0		(0x01)
#define GPIO_PDDR_TIMER_1		(0x02)
#define GPIO_PDDR_TIMER_2		(0x04)
#define GPIO_PDDR_TIMER_3		(0x08)

/* Bit definitions and macros for GPIO_PDDR_LCDDATAH */
#define GPIO_PDDR_LCDDATAH_0		(0x01)
#define GPIO_PDDR_LCDDATAH_1		(0x02)

/* Bit definitions and macros for GPIO_PDDR_LCDDATAM */
#define GPIO_PDDR_LCDDATAM_0		(0x01)
#define GPIO_PDDR_LCDDATAM_1		(0x02)
#define GPIO_PDDR_LCDDATAM_2		(0x04)
#define GPIO_PDDR_LCDDATAM_3		(0x08)
#define GPIO_PDDR_LCDDATAM_4		(0x10)
#define GPIO_PDDR_LCDDATAM_5		(0x20)
#define GPIO_PDDR_LCDDATAM_6		(0x40)
#define GPIO_PDDR_LCDDATAM_7		(0x80)

/* Bit definitions and macros for GPIO_PDDR_LCDDATAL */
#define GPIO_PDDR_LCDDATAL_0		(0x01)
#define GPIO_PDDR_LCDDATAL_1		(0x02)
#define GPIO_PDDR_LCDDATAL_2		(0x04)
#define GPIO_PDDR_LCDDATAL_3		(0x08)
#define GPIO_PDDR_LCDDATAL_4		(0x10)
#define GPIO_PDDR_LCDDATAL_5		(0x20)
#define GPIO_PDDR_LCDDATAL_6		(0x40)
#define GPIO_PDDR_LCDDATAL_7		(0x80)

/* Bit definitions and macros for GPIO_PDDR_LCDCTLH */
#define GPIO_PDDR_LCDCTLH_0		(0x01)

/* Bit definitions and macros for GPIO_PDDR_LCDCTLL */
#define GPIO_PDDR_LCDCTLL_0		(0x01)
#define GPIO_PDDR_LCDCTLL_1		(0x02)
#define GPIO_PDDR_LCDCTLL_2		(0x04)
#define GPIO_PDDR_LCDCTLL_3		(0x08)
#define GPIO_PDDR_LCDCTLL_4		(0x10)
#define GPIO_PDDR_LCDCTLL_5		(0x20)
#define GPIO_PDDR_LCDCTLL_6		(0x40)
#define GPIO_PDDR_LCDCTLL_7		(0x80)

/* Bit definitions and macros for GPIO_PPDSDR_FECH */
#define GPIO_PPDSDR_FECH_L0		(0x01)
#define GPIO_PPDSDR_FECH_L1		(0x02)
#define GPIO_PPDSDR_FECH_L2		(0x04)
#define GPIO_PPDSDR_FECH_L3		(0x08)
#define GPIO_PPDSDR_FECH_L4		(0x10)
#define GPIO_PPDSDR_FECH_L5		(0x20)
#define GPIO_PPDSDR_FECH_L6		(0x40)
#define GPIO_PPDSDR_FECH_L7		(0x80)

/* Bit definitions and macros for GPIO_PPDSDR_SSI */
#define GPIO_PPDSDR_SSI_0		(0x01)
#define GPIO_PPDSDR_SSI_1		(0x02)
#define GPIO_PPDSDR_SSI_2		(0x04)
#define GPIO_PPDSDR_SSI_3		(0x08)
#define GPIO_PPDSDR_SSI_4		(0x10)

/* Bit definitions and macros for GPIO_PPDSDR_BUSCTL */
#define GPIO_PPDSDR_BUSCTL_0		(0x01)
#define GPIO_PPDSDR_BUSCTL_1		(0x02)
#define GPIO_PPDSDR_BUSCTL_2		(0x04)
#define GPIO_PPDSDR_BUSCTL_3		(0x08)

/* Bit definitions and macros for GPIO_PPDSDR_BE */
#define GPIO_PPDSDR_BE_0		(0x01)
#define GPIO_PPDSDR_BE_1		(0x02)
#define GPIO_PPDSDR_BE_2		(0x04)
#define GPIO_PPDSDR_BE_3		(0x08)

/* Bit definitions and macros for GPIO_PPDSDR_CS */
#define GPIO_PPDSDR_CS_1		(0x02)
#define GPIO_PPDSDR_CS_2		(0x04)
#define GPIO_PPDSDR_CS_3		(0x08)
#define GPIO_PPDSDR_CS_4		(0x10)
#define GPIO_PPDSDR_CS_5		(0x20)

/* Bit definitions and macros for GPIO_PPDSDR_PWM */
#define GPIO_PPDSDR_PWM_2		(0x04)
#define GPIO_PPDSDR_PWM_3		(0x08)
#define GPIO_PPDSDR_PWM_4		(0x10)
#define GPIO_PPDSDR_PWM_5		(0x20)

/* Bit definitions and macros for GPIO_PPDSDR_FECI2C */
#define GPIO_PPDSDR_FECI2C_0		(0x01)
#define GPIO_PPDSDR_FECI2C_1		(0x02)
#define GPIO_PPDSDR_FECI2C_2		(0x04)
#define GPIO_PPDSDR_FECI2C_3		(0x08)

/* Bit definitions and macros for GPIO_PPDSDR_UART */
#define GPIO_PPDSDR_UART_0		(0x01)
#define GPIO_PPDSDR_UART_1		(0x02)
#define GPIO_PPDSDR_UART_2		(0x04)
#define GPIO_PPDSDR_UART_3		(0x08)
#define GPIO_PPDSDR_UART_4		(0x10)
#define GPIO_PPDSDR_UART_5		(0x20)
#define GPIO_PPDSDR_UART_6		(0x40)
#define GPIO_PPDSDR_UART_7		(0x80)

/* Bit definitions and macros for GPIO_PPDSDR_QSPI */
#define GPIO_PPDSDR_QSPI_0		(0x01)
#define GPIO_PPDSDR_QSPI_1		(0x02)
#define GPIO_PPDSDR_QSPI_2		(0x04)
#define GPIO_PPDSDR_QSPI_3		(0x08)
#define GPIO_PPDSDR_QSPI_4		(0x10)
#define GPIO_PPDSDR_QSPI_5		(0x20)

/* Bit definitions and macros for GPIO_PPDSDR_TIMER */
#define GPIO_PPDSDR_TIMER_0		(0x01)
#define GPIO_PPDSDR_TIMER_1		(0x02)
#define GPIO_PPDSDR_TIMER_2		(0x04)
#define GPIO_PPDSDR_TIMER_3		(0x08)

/* Bit definitions and macros for GPIO_PPDSDR_LCDDATAH */
#define GPIO_PPDSDR_LCDDATAH_0		(0x01)
#define GPIO_PPDSDR_LCDDATAH_1		(0x02)

/* Bit definitions and macros for GPIO_PPDSDR_LCDDATAM */
#define GPIO_PPDSDR_LCDDATAM_0		(0x01)
#define GPIO_PPDSDR_LCDDATAM_1		(0x02)
#define GPIO_PPDSDR_LCDDATAM_2		(0x04)
#define GPIO_PPDSDR_LCDDATAM_3		(0x08)
#define GPIO_PPDSDR_LCDDATAM_4		(0x10)
#define GPIO_PPDSDR_LCDDATAM_5		(0x20)
#define GPIO_PPDSDR_LCDDATAM_6		(0x40)
#define GPIO_PPDSDR_LCDDATAM_7		(0x80)

/* Bit definitions and macros for GPIO_PPDSDR_LCDDATAL */
#define GPIO_PPDSDR_LCDDATAL_0		(0x01)
#define GPIO_PPDSDR_LCDDATAL_1		(0x02)
#define GPIO_PPDSDR_LCDDATAL_2		(0x04)
#define GPIO_PPDSDR_LCDDATAL_3		(0x08)
#define GPIO_PPDSDR_LCDDATAL_4		(0x10)
#define GPIO_PPDSDR_LCDDATAL_5		(0x20)
#define GPIO_PPDSDR_LCDDATAL_6		(0x40)
#define GPIO_PPDSDR_LCDDATAL_7		(0x80)

/* Bit definitions and macros for GPIO_PPDSDR_LCDCTLH */
#define GPIO_PPDSDR_LCDCTLH_0		(0x01)

/* Bit definitions and macros for GPIO_PPDSDR_LCDCTLL */
#define GPIO_PPDSDR_LCDCTLL_0		(0x01)
#define GPIO_PPDSDR_LCDCTLL_1		(0x02)
#define GPIO_PPDSDR_LCDCTLL_2		(0x04)
#define GPIO_PPDSDR_LCDCTLL_3		(0x08)
#define GPIO_PPDSDR_LCDCTLL_4		(0x10)
#define GPIO_PPDSDR_LCDCTLL_5		(0x20)
#define GPIO_PPDSDR_LCDCTLL_6		(0x40)
#define GPIO_PPDSDR_LCDCTLL_7		(0x80)

/* Bit definitions and macros for GPIO_PCLRR_FECH */
#define GPIO_PCLRR_FECH_L0		(0x01)
#define GPIO_PCLRR_FECH_L1		(0x02)
#define GPIO_PCLRR_FECH_L2		(0x04)
#define GPIO_PCLRR_FECH_L3		(0x08)
#define GPIO_PCLRR_FECH_L4		(0x10)
#define GPIO_PCLRR_FECH_L5		(0x20)
#define GPIO_PCLRR_FECH_L6		(0x40)
#define GPIO_PCLRR_FECH_L7		(0x80)

/* Bit definitions and macros for GPIO_PCLRR_SSI */
#define GPIO_PCLRR_SSI_0		(0x01)
#define GPIO_PCLRR_SSI_1		(0x02)
#define GPIO_PCLRR_SSI_2		(0x04)
#define GPIO_PCLRR_SSI_3		(0x08)
#define GPIO_PCLRR_SSI_4		(0x10)

/* Bit definitions and macros for GPIO_PCLRR_BUSCTL */
#define GPIO_PCLRR_BUSCTL_L0		(0x01)
#define GPIO_PCLRR_BUSCTL_L1		(0x02)
#define GPIO_PCLRR_BUSCTL_L2		(0x04)
#define GPIO_PCLRR_BUSCTL_L3		(0x08)

/* Bit definitions and macros for GPIO_PCLRR_BE */
#define GPIO_PCLRR_BE_0			(0x01)
#define GPIO_PCLRR_BE_1			(0x02)
#define GPIO_PCLRR_BE_2			(0x04)
#define GPIO_PCLRR_BE_3			(0x08)

/* Bit definitions and macros for GPIO_PCLRR_CS */
#define GPIO_PCLRR_CS_1			(0x02)
#define GPIO_PCLRR_CS_2			(0x04)
#define GPIO_PCLRR_CS_3			(0x08)
#define GPIO_PCLRR_CS_4			(0x10)
#define GPIO_PCLRR_CS_5			(0x20)

/* Bit definitions and macros for GPIO_PCLRR_PWM */
#define GPIO_PCLRR_PWM_2		(0x04)
#define GPIO_PCLRR_PWM_3		(0x08)
#define GPIO_PCLRR_PWM_4		(0x10)
#define GPIO_PCLRR_PWM_5		(0x20)

/* Bit definitions and macros for GPIO_PCLRR_FECI2C */
#define GPIO_PCLRR_FECI2C_0		(0x01)
#define GPIO_PCLRR_FECI2C_1		(0x02)
#define GPIO_PCLRR_FECI2C_2		(0x04)
#define GPIO_PCLRR_FECI2C_3		(0x08)

/* Bit definitions and macros for GPIO_PCLRR_UART */
#define GPIO_PCLRR_UART0		(0x01)
#define GPIO_PCLRR_UART1		(0x02)
#define GPIO_PCLRR_UART2		(0x04)
#define GPIO_PCLRR_UART3		(0x08)
#define GPIO_PCLRR_UART4		(0x10)
#define GPIO_PCLRR_UART5		(0x20)
#define GPIO_PCLRR_UART6		(0x40)
#define GPIO_PCLRR_UART7		(0x80)

/* Bit definitions and macros for GPIO_PCLRR_QSPI */
#define GPIO_PCLRR_QSPI0		(0x01)
#define GPIO_PCLRR_QSPI1		(0x02)
#define GPIO_PCLRR_QSPI2		(0x04)
#define GPIO_PCLRR_QSPI3		(0x08)
#define GPIO_PCLRR_QSPI4		(0x10)
#define GPIO_PCLRR_QSPI5		(0x20)

/* Bit definitions and macros for GPIO_PCLRR_TIMER */
#define GPIO_PCLRR_TIMER0		(0x01)
#define GPIO_PCLRR_TIMER1		(0x02)
#define GPIO_PCLRR_TIMER2		(0x04)
#define GPIO_PCLRR_TIMER3		(0x08)

/* Bit definitions and macros for GPIO_PCLRR_LCDDATAH */
#define GPIO_PCLRR_LCDDATAH0		(0x01)
#define GPIO_PCLRR_LCDDATAH1		(0x02)

/* Bit definitions and macros for GPIO_PCLRR_LCDDATAM */
#define GPIO_PCLRR_LCDDATAM0		(0x01)
#define GPIO_PCLRR_LCDDATAM1		(0x02)
#define GPIO_PCLRR_LCDDATAM2		(0x04)
#define GPIO_PCLRR_LCDDATAM3		(0x08)
#define GPIO_PCLRR_LCDDATAM4		(0x10)
#define GPIO_PCLRR_LCDDATAM5		(0x20)
#define GPIO_PCLRR_LCDDATAM6		(0x40)
#define GPIO_PCLRR_LCDDATAM7		(0x80)

/* Bit definitions and macros for GPIO_PCLRR_LCDDATAL */
#define GPIO_PCLRR_LCDDATAL0		(0x01)
#define GPIO_PCLRR_LCDDATAL1		(0x02)
#define GPIO_PCLRR_LCDDATAL2		(0x04)
#define GPIO_PCLRR_LCDDATAL3		(0x08)
#define GPIO_PCLRR_LCDDATAL4		(0x10)
#define GPIO_PCLRR_LCDDATAL5		(0x20)
#define GPIO_PCLRR_LCDDATAL6		(0x40)
#define GPIO_PCLRR_LCDDATAL7		(0x80)

/* Bit definitions and macros for GPIO_PCLRR_LCDCTLH */
#define GPIO_PCLRR_LCDCTLH_PCLRR_LCDCTLH0		(0x01)

/* Bit definitions and macros for GPIO_PCLRR_LCDCTLL */
#define GPIO_PCLRR_LCDCTLL0		(0x01)
#define GPIO_PCLRR_LCDCTLL1		(0x02)
#define GPIO_PCLRR_LCDCTLL2		(0x04)
#define GPIO_PCLRR_LCDCTLL3		(0x08)
#define GPIO_PCLRR_LCDCTLL4		(0x10)
#define GPIO_PCLRR_LCDCTLL5		(0x20)
#define GPIO_PCLRR_LCDCTLL6		(0x40)
#define GPIO_PCLRR_LCDCTLL7		(0x80)

/* Bit definitions and macros for GPIO_PAR_FEC */
#define GPIO_PAR_FEC_MII(x)		(((x)&0x03)<<0)
#define GPIO_PAR_FEC_7W(x)		(((x)&0x03)<<2)
#define GPIO_PAR_FEC_7W_GPIO		(0x00)
#define GPIO_PAR_FEC_7W_URTS1		(0x04)
#define GPIO_PAR_FEC_7W_FEC		(0x0C)
#define GPIO_PAR_FEC_MII_GPIO		(0x00)
#define GPIO_PAR_FEC_MII_UART		(0x01)
#define GPIO_PAR_FEC_MII_FEC		(0x03)

/* Bit definitions and macros for GPIO_PAR_PWM */
#define GPIO_PAR_PWM1(x)		(((x)&0x03)<<0)
#define GPIO_PAR_PWM3(x)		(((x)&0x03)<<2)
#define GPIO_PAR_PWM5			(0x10)
#define GPIO_PAR_PWM7			(0x20)

/* Bit definitions and macros for GPIO_PAR_BUSCTL */
#define GPIO_PAR_BUSCTL_TS(x)		(((x)&0x03)<<3)
#define GPIO_PAR_BUSCTL_RWB		(0x20)
#define GPIO_PAR_BUSCTL_TA		(0x40)
#define GPIO_PAR_BUSCTL_OE		(0x80)
#define GPIO_PAR_BUSCTL_OE_GPIO		(0x00)
#define GPIO_PAR_BUSCTL_OE_OE		(0x80)
#define GPIO_PAR_BUSCTL_TA_GPIO		(0x00)
#define GPIO_PAR_BUSCTL_TA_TA		(0x40)
#define GPIO_PAR_BUSCTL_RWB_GPIO	(0x00)
#define GPIO_PAR_BUSCTL_RWB_RWB		(0x20)
#define GPIO_PAR_BUSCTL_TS_GPIO		(0x00)
#define GPIO_PAR_BUSCTL_TS_DACK0	(0x10)
#define GPIO_PAR_BUSCTL_TS_TS		(0x18)

/* Bit definitions and macros for GPIO_PAR_FECI2C */
#define GPIO_PAR_FECI2C_SDA(x)		(((x)&0x03)<<0)
#define GPIO_PAR_FECI2C_SCL(x)		(((x)&0x03)<<2)
#define GPIO_PAR_FECI2C_MDIO(x)		(((x)&0x03)<<4)
#define GPIO_PAR_FECI2C_MDC(x)		(((x)&0x03)<<6)
#define GPIO_PAR_FECI2C_MDC_GPIO	(0x00)
#define GPIO_PAR_FECI2C_MDC_UTXD2	(0x40)
#define GPIO_PAR_FECI2C_MDC_SCL		(0x80)
#define GPIO_PAR_FECI2C_MDC_EMDC	(0xC0)
#define GPIO_PAR_FECI2C_MDIO_GPIO	(0x00)
#define GPIO_PAR_FECI2C_MDIO_URXD2	(0x10)
#define GPIO_PAR_FECI2C_MDIO_SDA	(0x20)
#define GPIO_PAR_FECI2C_MDIO_EMDIO	(0x30)
#define GPIO_PAR_FECI2C_SCL_GPIO	(0x00)
#define GPIO_PAR_FECI2C_SCL_UTXD2	(0x04)
#define GPIO_PAR_FECI2C_SCL_SCL		(0x0C)
#define GPIO_PAR_FECI2C_SDA_GPIO	(0x00)
#define GPIO_PAR_FECI2C_SDA_URXD2	(0x02)
#define GPIO_PAR_FECI2C_SDA_SDA		(0x03)

/* Bit definitions and macros for GPIO_PAR_BE */
#define GPIO_PAR_BE0			(0x01)
#define GPIO_PAR_BE1			(0x02)
#define GPIO_PAR_BE2			(0x04)
#define GPIO_PAR_BE3			(0x08)

/* Bit definitions and macros for GPIO_PAR_CS */
#define GPIO_PAR_CS1			(0x02)
#define GPIO_PAR_CS2			(0x04)
#define GPIO_PAR_CS3			(0x08)
#define GPIO_PAR_CS4			(0x10)
#define GPIO_PAR_CS5			(0x20)
#define GPIO_PAR_CS1_GPIO		(0x00)
#define GPIO_PAR_CS1_SDCS1		(0x01)
#define GPIO_PAR_CS1_CS1		(0x03)

/* Bit definitions and macros for GPIO_PAR_SSI */
#define GPIO_PAR_SSI_MCLK		(0x0080)
#define GPIO_PAR_SSI_TXD(x)		(((x)&0x0003)<<8)
#define GPIO_PAR_SSI_RXD(x)		(((x)&0x0003)<<10)
#define GPIO_PAR_SSI_FS(x)		(((x)&0x0003)<<12)
#define GPIO_PAR_SSI_BCLK(x)		(((x)&0x0003)<<14)

/* Bit definitions and macros for GPIO_PAR_UART */
#define GPIO_PAR_UART_TXD0		(0x0001)
#define GPIO_PAR_UART_RXD0		(0x0002)
#define GPIO_PAR_UART_RTS0		(0x0004)
#define GPIO_PAR_UART_CTS0		(0x0008)
#define GPIO_PAR_UART_TXD1(x)		(((x)&0x0003)<<4)
#define GPIO_PAR_UART_RXD1(x)		(((x)&0x0003)<<6)
#define GPIO_PAR_UART_RTS1(x)		(((x)&0x0003)<<8)
#define GPIO_PAR_UART_CTS1(x)		(((x)&0x0003)<<10)
#define GPIO_PAR_UART_CTS1_GPIO		(0x0000)
#define GPIO_PAR_UART_CTS1_SSI_BCLK	(0x0800)
#define GPIO_PAR_UART_CTS1_ULPI_D7	(0x0400)
#define GPIO_PAR_UART_CTS1_UCTS1	(0x0C00)
#define GPIO_PAR_UART_RTS1_GPIO		(0x0000)
#define GPIO_PAR_UART_RTS1_SSI_FS	(0x0200)
#define GPIO_PAR_UART_RTS1_ULPI_D6	(0x0100)
#define GPIO_PAR_UART_RTS1_URTS1	(0x0300)
#define GPIO_PAR_UART_RXD1_GPIO		(0x0000)
#define GPIO_PAR_UART_RXD1_SSI_RXD	(0x0080)
#define GPIO_PAR_UART_RXD1_ULPI_D5	(0x0040)
#define GPIO_PAR_UART_RXD1_URXD1	(0x00C0)
#define GPIO_PAR_UART_TXD1_GPIO		(0x0000)
#define GPIO_PAR_UART_TXD1_SSI_TXD	(0x0020)
#define GPIO_PAR_UART_TXD1_ULPI_D4	(0x0010)
#define GPIO_PAR_UART_TXD1_UTXD1	(0x0030)

/* Bit definitions and macros for GPIO_PAR_QSPI */
#define GPIO_PAR_QSPI_SCK(x)		(((x)&0x0003)<<4)
#define GPIO_PAR_QSPI_DOUT(x)		(((x)&0x0003)<<6)
#define GPIO_PAR_QSPI_DIN(x)		(((x)&0x0003)<<8)
#define GPIO_PAR_QSPI_PCS0(x)		(((x)&0x0003)<<10)
#define GPIO_PAR_QSPI_PCS1(x)		(((x)&0x0003)<<12)
#define GPIO_PAR_QSPI_PCS2(x)		(((x)&0x0003)<<14)

/* Bit definitions and macros for GPIO_PAR_TIMER */
#define GPIO_PAR_TIN0(x)		(((x)&0x03)<<0)
#define GPIO_PAR_TIN1(x)		(((x)&0x03)<<2)
#define GPIO_PAR_TIN2(x)		(((x)&0x03)<<4)
#define GPIO_PAR_TIN3(x)		(((x)&0x03)<<6)
#define GPIO_PAR_TIN3_GPIO		(0x00)
#define GPIO_PAR_TIN3_TOUT3		(0x80)
#define GPIO_PAR_TIN3_URXD2		(0x40)
#define GPIO_PAR_TIN3_TIN3		(0xC0)
#define GPIO_PAR_TIN2_GPIO		(0x00)
#define GPIO_PAR_TIN2_TOUT2		(0x20)
#define GPIO_PAR_TIN2_UTXD2		(0x10)
#define GPIO_PAR_TIN2_TIN2		(0x30)
#define GPIO_PAR_TIN1_GPIO		(0x00)
#define GPIO_PAR_TIN1_TOUT1		(0x08)
#define GPIO_PAR_TIN1_DACK1		(0x04)
#define GPIO_PAR_TIN1_TIN1		(0x0C)
#define GPIO_PAR_TIN0_GPIO		(0x00)
#define GPIO_PAR_TIN0_TOUT0		(0x02)
#define GPIO_PAR_TIN0_DREQ0		(0x01)
#define GPIO_PAR_TIN0_TIN0		(0x03)

/* Bit definitions and macros for GPIO_PAR_LCDDATA */
#define GPIO_PAR_LCDDATA_LD7_0(x)	((x)&0x03)
#define GPIO_PAR_LCDDATA_LD15_8(x)	(((x)&0x03)<<2)
#define GPIO_PAR_LCDDATA_LD16(x)	(((x)&0x03)<<4)
#define GPIO_PAR_LCDDATA_LD17(x)	(((x)&0x03)<<6)

/* Bit definitions and macros for GPIO_PAR_LCDCTL */
#define GPIO_PAR_LCDCTL_CLS		(0x0001)
#define GPIO_PAR_LCDCTL_PS		(0x0002)
#define GPIO_PAR_LCDCTL_REV		(0x0004)
#define GPIO_PAR_LCDCTL_SPL_SPR		(0x0008)
#define GPIO_PAR_LCDCTL_CONTRAST	(0x0010)
#define GPIO_PAR_LCDCTL_LSCLK		(0x0020)
#define GPIO_PAR_LCDCTL_LP_HSYNC	(0x0040)
#define GPIO_PAR_LCDCTL_FLM_VSYNC	(0x0080)
#define GPIO_PAR_LCDCTL_ACD_OE		(0x0100)

/* Bit definitions and macros for GPIO_PAR_IRQ */
#define GPIO_PAR_IRQ1(x)		(((x)&0x0003)<<4)
#define GPIO_PAR_IRQ2(x)		(((x)&0x0003)<<6)
#define GPIO_PAR_IRQ4(x)		(((x)&0x0003)<<8)
#define GPIO_PAR_IRQ5(x)		(((x)&0x0003)<<10)
#define GPIO_PAR_IRQ6(x)		(((x)&0x0003)<<12)

/* Bit definitions and macros for GPIO_MSCR_FLEXBUS */
#define GPIO_MSCR_FLEXBUS_ADDRCTL(x)	((x)&0x03)
#define GPIO_MSCR_FLEXBUS_DLOWER(x)	(((x)&0x03)<<2)
#define GPIO_MSCR_FLEXBUS_DUPPER(x)	(((x)&0x03)<<4)

/* Bit definitions and macros for GPIO_MSCR_SDRAM */
#define GPIO_MSCR_SDRAM_SDRAM(x)	((x)&0x03)
#define GPIO_MSCR_SDRAM_SDCLK(x)	(((x)&0x03)<<2)
#define GPIO_MSCR_SDRAM_SDCLKB(x)	(((x)&0x03)<<4)

/* Bit definitions and macros for GPIO_DSCR_I2C */
#define GPIO_DSCR_I2C_DSE(x)		((x)&0x03)

/* Bit definitions and macros for GPIO_DSCR_PWM */
#define GPIO_DSCR_PWM_DSE(x)		((x)&0x03)

/* Bit definitions and macros for GPIO_DSCR_FEC */
#define GPIO_DSCR_FEC_DSE(x)		((x)&0x03)

/* Bit definitions and macros for GPIO_DSCR_UART */
#define GPIO_DSCR_UART0_DSE(x)		((x)&0x03)
#define GPIO_DSCR_UART1_DSE(x)		(((x)&0x03)<<2)

/* Bit definitions and macros for GPIO_DSCR_QSPI */
#define GPIO_DSCR_QSPI_DSE(x)		((x)&0x03)

/* Bit definitions and macros for GPIO_DSCR_TIMER */
#define GPIO_DSCR_TIMER_DSE(x)		((x)&0x03)

/* Bit definitions and macros for GPIO_DSCR_SSI */
#define GPIO_DSCR_SSI_DSE(x)		((x)&0x03)

/* Bit definitions and macros for GPIO_DSCR_LCD */
#define GPIO_DSCR_LCD_DSE(x)		((x)&0x03)

/* Bit definitions and macros for GPIO_DSCR_DEBUG */
#define GPIO_DSCR_DEBUG_DSE(x)		((x)&0x03)

/* Bit definitions and macros for GPIO_DSCR_CLKRST */
#define GPIO_DSCR_CLKRST_DSE(x)		((x)&0x03)

/* Bit definitions and macros for GPIO_DSCR_IRQ */
#define GPIO_DSCR_IRQ_DSE(x)		((x)&0x03)

/* not done yet */
/*********************************************************************
* LCD Controller (LCDC)
*********************************************************************/
/* Bit definitions and macros for LCDC_LSSAR */
#define LCDC_LSSAR_SSA(x)		(((x)&0x3FFFFFFF)<<2)

/* Bit definitions and macros for LCDC_LSR */
#define LCDC_LSR_YMAX(x)		(((x)&0x000003FF)<<0)
#define LCDC_LSR_XMAX(x)		(((x)&0x0000003F)<<20)

/* Bit definitions and macros for LCDC_LVPWR */
#define LCDC_LVPWR_VPW(x)		(((x)&0x000003FF)<<0)

/* Bit definitions and macros for LCDC_LCPR */
#define LCDC_LCPR_CYP(x)		(((x)&0x000003FF)<<0)
#define LCDC_LCPR_CXP(x)		(((x)&0x000003FF)<<16)
#define LCDC_LCPR_OP			(0x10000000)
#define LCDC_LCPR_CC(x)			(((x)&0x00000003)<<30)
#define LCDC_LCPR_CC_TRANSPARENT	(0x00000000)
#define LCDC_LCPR_CC_OR			(0x40000000)
#define LCDC_LCPR_CC_XOR		(0x80000000)
#define LCDC_LCPR_CC_AND		(0xC0000000)
#define LCDC_LCPR_OP_ON			(0x10000000)
#define LCDC_LCPR_OP_OFF		(0x00000000)

/* Bit definitions and macros for LCDC_LCWHBR */
#define LCDC_LCWHBR_BD(x)		(((x)&0x000000FF)<<0)
#define LCDC_LCWHBR_CH(x)		(((x)&0x0000001F)<<16)
#define LCDC_LCWHBR_CW(x)		(((x)&0x0000001F)<<24)
#define LCDC_LCWHBR_BK_EN		(0x80000000)
#define LCDC_LCWHBR_BK_EN_ON		(0x80000000)
#define LCDC_LCWHBR_BK_EN_OFF		(0x00000000)

/* Bit definitions and macros for LCDC_LCCMR */
#define LCDC_LCCMR_CUR_COL_B(x)		(((x)&0x0000003F)<<0)
#define LCDC_LCCMR_CUR_COL_G(x)		(((x)&0x0000003F)<<6)
#define LCDC_LCCMR_CUR_COL_R(x)		(((x)&0x0000003F)<<12)

/* Bit definitions and macros for LCDC_LPCR */
#define LCDC_LPCR_PCD(x)		(((x)&0x0000003F)<<0)
#define LCDC_LPCR_SHARP			(0x00000040)
#define LCDC_LPCR_SCLKSEL		(0x00000080)
#define LCDC_LPCR_ACD(x)		(((x)&0x0000007F)<<8)
#define LCDC_LPCR_ACDSEL		(0x00008000)
#define LCDC_LPCR_REV_VS		(0x00010000)
#define LCDC_LPCR_SWAP_SEL		(0x00020000)
#define LCDC_LPCR_ENDSEL		(0x00040000)
#define LCDC_LPCR_SCLKIDLE		(0x00080000)
#define LCDC_LPCR_OEPOL			(0x00100000)
#define LCDC_LPCR_CLKPOL		(0x00200000)
#define LCDC_LPCR_LPPOL			(0x00400000)
#define LCDC_LPCR_FLM			(0x00800000)
#define LCDC_LPCR_PIXPOL		(0x01000000)
#define LCDC_LPCR_BPIX(x)		(((x)&0x00000007)<<25)
#define LCDC_LPCR_PBSIZ(x)		(((x)&0x00000003)<<28)
#define LCDC_LPCR_COLOR			(0x40000000)
#define LCDC_LPCR_TFT			(0x80000000)
#define LCDC_LPCR_MODE_MONOCHROME	(0x00000000)
#define LCDC_LPCR_MODE_CSTN		(0x40000000)
#define LCDC_LPCR_MODE_TFT		(0xC0000000)
#define LCDC_LPCR_PBSIZ_1		(0x00000000)
#define LCDC_LPCR_PBSIZ_2		(0x10000000)
#define LCDC_LPCR_PBSIZ_4		(0x20000000)
#define LCDC_LPCR_PBSIZ_8		(0x30000000)
#define LCDC_LPCR_BPIX_1bpp		(0x00000000)
#define LCDC_LPCR_BPIX_2bpp		(0x02000000)
#define LCDC_LPCR_BPIX_4bpp		(0x04000000)
#define LCDC_LPCR_BPIX_8bpp		(0x06000000)
#define LCDC_LPCR_BPIX_12bpp		(0x08000000)
#define LCDC_LPCR_BPIX_16bpp		(0x0A000000)
#define LCDC_LPCR_BPIX_18bpp		(0x0C000000)

#define LCDC_LPCR_PANEL_TYPE(x)		(((x)&0x00000003)<<30)

/* Bit definitions and macros for LCDC_LHCR */
#define LCDC_LHCR_H_WAIT_2(x)		(((x)&0x000000FF)<<0)
#define LCDC_LHCR_H_WAIT_1(x)		(((x)&0x000000FF)<<8)
#define LCDC_LHCR_H_WIDTH(x)		(((x)&0x0000003F)<<26)

/* Bit definitions and macros for LCDC_LVCR */
#define LCDC_LVCR_V_WAIT_2(x)		(((x)&0x000000FF)<<0)
#define LCDC_LVCR_V_WAIT_1(x)		(((x)&0x000000FF)<<8)
#define LCDC_LVCR_V_WIDTH(x)		(((x)&0x0000003F)<<26)

/* Bit definitions and macros for LCDC_LPOR */
#define LCDC_LPOR_POS(x)		(((x)&0x0000001F)<<0)

/* Bit definitions and macros for LCDC_LPCCR */
#define LCDC_LPCCR_PW(x)		(((x)&0x000000FF)<<0)
#define LCDC_LPCCR_CC_EN		(0x00000100)
#define LCDC_LPCCR_SCR(x)		(((x)&0x00000003)<<9)
#define LCDC_LPCCR_LDMSK		(0x00008000)
#define LCDC_LPCCR_CLS_HI_WIDTH(x)	(((x)&0x000001FF)<<16)
#define LCDC_LPCCR_SCR_LINEPULSE	(0x00000000)
#define LCDC_LPCCR_SCR_PIXELCLK		(0x00002000)
#define LCDC_LPCCR_SCR_LCDCLOCK		(0x00004000)

/* Bit definitions and macros for LCDC_LDCR */
#define LCDC_LDCR_TM(x)			(((x)&0x0000001F)<<0)
#define LCDC_LDCR_HM(x)			(((x)&0x0000001F)<<16)
#define LCDC_LDCR_BURST			(0x80000000)

/* Bit definitions and macros for LCDC_LRMCR */
#define LCDC_LRMCR_SEL_REF		(0x00000001)

/* Bit definitions and macros for LCDC_LICR */
#define LCDC_LICR_INTCON		(0x00000001)
#define LCDC_LICR_INTSYN		(0x00000004)
#define LCDC_LICR_GW_INT_CON		(0x00000010)

/* Bit definitions and macros for LCDC_LIER */
#define LCDC_LIER_BOF_EN		(0x00000001)
#define LCDC_LIER_EOF_EN		(0x00000002)
#define LCDC_LIER_ERR_RES_EN		(0x00000004)
#define LCDC_LIER_UDR_ERR_EN		(0x00000008)
#define LCDC_LIER_GW_BOF_EN		(0x00000010)
#define LCDC_LIER_GW_EOF_EN		(0x00000020)
#define LCDC_LIER_GW_ERR_RES_EN		(0x00000040)
#define LCDC_LIER_GW_UDR_ERR_EN		(0x00000080)

/* Bit definitions and macros for LCDC_LISR */
#define LCDC_LISR_BOF			(0x00000001)
#define LCDC_LISR_EOF			(0x00000002)
#define LCDC_LISR_ERR_RES		(0x00000004)
#define LCDC_LISR_UDR_ERR		(0x00000008)
#define LCDC_LISR_GW_BOF		(0x00000010)
#define LCDC_LISR_GW_EOF		(0x00000020)
#define LCDC_LISR_GW_ERR_RES		(0x00000040)
#define LCDC_LISR_GW_UDR_ERR		(0x00000080)

/* Bit definitions and macros for LCDC_LGWSAR */
#define LCDC_LGWSAR_GWSA(x)		(((x)&0x3FFFFFFF)<<2)

/* Bit definitions and macros for LCDC_LGWSR */
#define LCDC_LGWSR_GWH(x)		(((x)&0x000003FF)<<0)
#define LCDC_LGWSR_GWW(x)		(((x)&0x0000003F)<<20)

/* Bit definitions and macros for LCDC_LGWVPWR */
#define LCDC_LGWVPWR_GWVPW(x)		(((x)&0x000003FF)<<0)

/* Bit definitions and macros for LCDC_LGWPOR */
#define LCDC_LGWPOR_GWPO(x)		(((x)&0x0000001F)<<0)

/* Bit definitions and macros for LCDC_LGWPR */
#define LCDC_LGWPR_GWYP(x)		(((x)&0x000003FF)<<0)
#define LCDC_LGWPR_GWXP(x)		(((x)&0x000003FF)<<16)

/* Bit definitions and macros for LCDC_LGWCR */
#define LCDC_LGWCR_GWCKB(x)		(((x)&0x0000003F)<<0)
#define LCDC_LGWCR_GWCKG(x)		(((x)&0x0000003F)<<6)
#define LCDC_LGWCR_GWCKR(x)		(((x)&0x0000003F)<<12)
#define LCDC_LGWCR_GW_RVS		(0x00200000)
#define LCDC_LGWCR_GWE			(0x00400000)
#define LCDC_LGWCR_GWCKE		(0x00800000)
#define LCDC_LGWCR_GWAV(x)		(((x)&0x000000FF)<<24)

/* Bit definitions and macros for LCDC_LGWDCR */
#define LCDC_LGWDCR_GWTM(x)		(((x)&0x0000001F)<<0)
#define LCDC_LGWDCR_GWHM(x)		(((x)&0x0000001F)<<16)
#define LCDC_LGWDCR_GWBT		(0x80000000)

/*********************************************************************
* SDRAM Controller (SDRAMC)
*********************************************************************/
/* Bit definitions and macros for SDRAMC_SDMR */
#define SDRAMC_SDMR_BNKAD_LEMR		(0x40000000)
#define SDRAMC_SDMR_BNKAD_LMR		(0x00000000)
#define SDRAMC_SDMR_AD(x)		(((x)&0x00000FFF)<<18)
#define SDRAMC_SDMR_CMD			(0x00010000)

/* Bit definitions and macros for SDRAMC_SDCR */
#define SDRAMC_SDCR_MODE_EN		(0x80000000)
#define SDRAMC_SDCR_CKE			(0x40000000)
#define SDRAMC_SDCR_DDR			(0x20000000)
#define SDRAMC_SDCR_REF			(0x10000000)
#define SDRAMC_SDCR_MUX(x)		(((x)&0x00000003)<<24)
#define SDRAMC_SDCR_OE_RULE		(0x00400000)
#define SDRAMC_SDCR_RCNT(x)		(((x)&0x0000003F)<<16)
#define SDRAMC_SDCR_PS_32		(0x00000000)
#define SDRAMC_SDCR_PS_16		(0x00002000)
#define SDRAMC_SDCR_DQS_OE(x)		(((x)&0x0000000F)<<8)
#define SDRAMC_SDCR_IREF		(0x00000004)
#define SDRAMC_SDCR_IPALL		(0x00000002)

/* Bit definitions and macros for SDRAMC_SDCFG1 */
#define SDRAMC_SDCFG1_SRD2RW(x)		(((x)&0x0000000F)<<28)
#define SDRAMC_SDCFG1_SWT2RD(x)		(((x)&0x00000007)<<24)
#define SDRAMC_SDCFG1_RDLAT(x)		(((x)&0x0000000F)<<20)
#define SDRAMC_SDCFG1_ACT2RW(x)		(((x)&0x00000007)<<16)
#define SDRAMC_SDCFG1_PRE2ACT(x)	(((x)&0x00000007)<<12)
#define SDRAMC_SDCFG1_REF2ACT(x)	(((x)&0x0000000F)<<8)
#define SDRAMC_SDCFG1_WTLAT(x)		(((x)&0x00000007)<<4)

/* Bit definitions and macros for SDRAMC_SDCFG2 */
#define SDRAMC_SDCFG2_BRD2PRE(x)	(((x)&0x0000000F)<<28)
#define SDRAMC_SDCFG2_BWT2RW(x)		(((x)&0x0000000F)<<24)
#define SDRAMC_SDCFG2_BRD2WT(x)		(((x)&0x0000000F)<<20)
#define SDRAMC_SDCFG2_BL(x)		(((x)&0x0000000F)<<16)

/* Bit definitions and macros for SDRAMC_SDDS */
#define SDRAMC_SDDS_SB_E(x)		(((x)&0x00000003)<<8)
#define SDRAMC_SDDS_SB_C(x)		(((x)&0x00000003)<<6)
#define SDRAMC_SDDS_SB_A(x)		(((x)&0x00000003)<<4)
#define SDRAMC_SDDS_SB_S(x)		(((x)&0x00000003)<<2)
#define SDRAMC_SDDS_SB_D(x)		((x)&0x00000003)

/* Bit definitions and macros for SDRAMC_SDCS */
#define SDRAMC_SDCS_BASE(x)		(((x)&0x00000FFF)<<20)
#define SDRAMC_SDCS_CSSZ(x)		((x)&0x0000001F)
#define SDRAMC_SDCS_CSSZ_4GBYTE		(0x0000001F)
#define SDRAMC_SDCS_CSSZ_2GBYTE		(0x0000001E)
#define SDRAMC_SDCS_CSSZ_1GBYTE		(0x0000001D)
#define SDRAMC_SDCS_CSSZ_512MBYTE	(0x0000001C)
#define SDRAMC_SDCS_CSSZ_256MBYTE	(0x0000001B)
#define SDRAMC_SDCS_CSSZ_128MBYTE	(0x0000001A)
#define SDRAMC_SDCS_CSSZ_64MBYTE	(0x00000019)
#define SDRAMC_SDCS_CSSZ_32MBYTE	(0x00000018)
#define SDRAMC_SDCS_CSSZ_16MBYTE	(0x00000017)
#define SDRAMC_SDCS_CSSZ_8MBYTE		(0x00000016)
#define SDRAMC_SDCS_CSSZ_4MBYTE		(0x00000015)
#define SDRAMC_SDCS_CSSZ_2MBYTE		(0x00000014)
#define SDRAMC_SDCS_CSSZ_1MBYTE		(0x00000013)
#define SDRAMC_SDCS_CSSZ_DIABLE		(0x00000000)

/*********************************************************************
* Synchronous Serial Interface (SSI)
*********************************************************************/
/* Bit definitions and macros for SSI_CR */
#define SSI_CR_CIS			(0x00000200)
#define SSI_CR_TCH			(0x00000100)
#define SSI_CR_MCE			(0x00000080)
#define SSI_CR_I2S_SLAVE		(0x00000040)
#define SSI_CR_I2S_MASTER		(0x00000020)
#define SSI_CR_I2S_NORMAL		(0x00000000)
#define SSI_CR_SYN			(0x00000010)
#define SSI_CR_NET			(0x00000008)
#define SSI_CR_RE			(0x00000004)
#define SSI_CR_TE			(0x00000002)
#define SSI_CR_SSI_EN			(0x00000001)

/* Bit definitions and macros for SSI_ISR */
#define SSI_ISR_CMDAU			(0x00040000)
#define SSI_ISR_CMDDU			(0x00020000)
#define SSI_ISR_RXT			(0x00010000)
#define SSI_ISR_RDR1			(0x00008000)
#define SSI_ISR_RDR0			(0x00004000)
#define SSI_ISR_TDE1			(0x00002000)
#define SSI_ISR_TDE0			(0x00001000)
#define SSI_ISR_ROE1			(0x00000800)
#define SSI_ISR_ROE0			(0x00000400)
#define SSI_ISR_TUE1			(0x00000200)
#define SSI_ISR_TUE0			(0x00000100)
#define SSI_ISR_TFS			(0x00000080)
#define SSI_ISR_RFS			(0x00000040)
#define SSI_ISR_TLS			(0x00000020)
#define SSI_ISR_RLS			(0x00000010)
#define SSI_ISR_RFF1			(0x00000008)
#define SSI_ISR_RFF0			(0x00000004)
#define SSI_ISR_TFE1			(0x00000002)
#define SSI_ISR_TFE0			(0x00000001)

/* Bit definitions and macros for SSI_IER */
#define SSI_IER_RDMAE			(0x00400000)
#define SSI_IER_RIE			(0x00200000)
#define SSI_IER_TDMAE			(0x00100000)
#define SSI_IER_TIE			(0x00080000)
#define SSI_IER_CMDAU			(0x00040000)
#define SSI_IER_CMDU			(0x00020000)
#define SSI_IER_RXT			(0x00010000)
#define SSI_IER_RDR1			(0x00008000)
#define SSI_IER_RDR0			(0x00004000)
#define SSI_IER_TDE1			(0x00002000)
#define SSI_IER_TDE0			(0x00001000)
#define SSI_IER_ROE1			(0x00000800)
#define SSI_IER_ROE0			(0x00000400)
#define SSI_IER_TUE1			(0x00000200)
#define SSI_IER_TUE0			(0x00000100)
#define SSI_IER_TFS			(0x00000080)
#define SSI_IER_RFS			(0x00000040)
#define SSI_IER_TLS			(0x00000020)
#define SSI_IER_RLS			(0x00000010)
#define SSI_IER_RFF1			(0x00000008)
#define SSI_IER_RFF0			(0x00000004)
#define SSI_IER_TFE1			(0x00000002)
#define SSI_IER_TFE0			(0x00000001)

/* Bit definitions and macros for SSI_TCR */
#define SSI_TCR_TXBIT0			(0x00000200)
#define SSI_TCR_TFEN1			(0x00000100)
#define SSI_TCR_TFEN0			(0x00000080)
#define SSI_TCR_TFDIR			(0x00000040)
#define SSI_TCR_TXDIR			(0x00000020)
#define SSI_TCR_TSHFD			(0x00000010)
#define SSI_TCR_TSCKP			(0x00000008)
#define SSI_TCR_TFSI			(0x00000004)
#define SSI_TCR_TFSL			(0x00000002)
#define SSI_TCR_TEFS			(0x00000001)

/* Bit definitions and macros for SSI_RCR */
#define SSI_RCR_RXEXT			(0x00000400)
#define SSI_RCR_RXBIT0			(0x00000200)
#define SSI_RCR_RFEN1			(0x00000100)
#define SSI_RCR_RFEN0			(0x00000080)
#define SSI_RCR_RSHFD			(0x00000010)
#define SSI_RCR_RSCKP			(0x00000008)
#define SSI_RCR_RFSI			(0x00000004)
#define SSI_RCR_RFSL			(0x00000002)
#define SSI_RCR_REFS			(0x00000001)

/* Bit definitions and macros for SSI_CCR */
#define SSI_CCR_DIV2			(0x00040000)
#define SSI_CCR_PSR			(0x00020000)
#define SSI_CCR_WL(x)			(((x)&0x0000000F)<<13)
#define SSI_CCR_DC(x)			(((x)&0x0000001F)<<8)
#define SSI_CCR_PM(x)			((x)&0x000000FF)

/* Bit definitions and macros for SSI_FCSR */
#define SSI_FCSR_RFCNT1(x)		(((x)&0x0000000F)<<28)
#define SSI_FCSR_TFCNT1(x)		(((x)&0x0000000F)<<24)
#define SSI_FCSR_RFWM1(x)		(((x)&0x0000000F)<<20)
#define SSI_FCSR_TFWM1(x)		(((x)&0x0000000F)<<16)
#define SSI_FCSR_RFCNT0(x)		(((x)&0x0000000F)<<12)
#define SSI_FCSR_TFCNT0(x)		(((x)&0x0000000F)<<8)
#define SSI_FCSR_RFWM0(x)		(((x)&0x0000000F)<<4)
#define SSI_FCSR_TFWM0(x)		((x)&0x0000000F)

/* Bit definitions and macros for SSI_ACR */
#define SSI_ACR_FRDIV(x)		(((x)&0x0000003F)<<5)
#define SSI_ACR_WR			(0x00000010)
#define SSI_ACR_RD			(0x00000008)
#define SSI_ACR_TIF			(0x00000004)
#define SSI_ACR_FV			(0x00000002)
#define SSI_ACR_AC97EN			(0x00000001)

/* Bit definitions and macros for SSI_ACADD */
#define SSI_ACADD_SSI_ACADD(x)		((x)&0x0007FFFF)

/* Bit definitions and macros for SSI_ACDAT */
#define SSI_ACDAT_SSI_ACDAT(x)		((x)&0x0007FFFF)

/* Bit definitions and macros for SSI_ATAG */
#define SSI_ATAG_DDI_ATAG(x)		((x)&0x0000FFFF)

/*********************************************************************
* Phase Locked Loop (PLL)
*********************************************************************/
/* Bit definitions and macros for PLL_PODR */
#define PLL_PODR_CPUDIV(x)		(((x)&0x0F)<<4)
#define PLL_PODR_BUSDIV(x)		((x)&0x0F)

/* Bit definitions and macros for PLL_PLLCR */
#define PLL_PLLCR_DITHEN		(0x80)
#define PLL_PLLCR_DITHDEV(x)		((x)&0x07)

#endif				/* mcf5329_h */
