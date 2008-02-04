/*
 * MCF5275 Internal Memory Map
 *
 * Copyright (C) 2003-2004, Greg Ungerer (gerg@snapgear.com)
 * Copyright (C) 2004-2008 Arthur Shipkowski (art@videon-central.com)
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

#ifndef	__M5275_H__
#define	__M5275_H__

/*
 * Define the 5275 SIM register set addresses. These are similar,
 * but not quite identical to the 5282 registers and offsets.
 */
#define	MCFICM_INTC0		0x0c00		/* Base for Interrupt Ctrl 0 */
#define	MCFICM_INTC1		0x0d00		/* Base for Interrupt Ctrl 1 */
#define	MCFINTC_IPRH		0x00		/* Interrupt pending 32-63 */
#define	MCFINTC_IPRL		0x04		/* Interrupt pending 1-31 */
#define	MCFINTC_IMRH		0x08		/* Interrupt mask 32-63 */
#define	MCFINTC_IMRL		0x0c		/* Interrupt mask 1-31 */
#define	MCFINTC_INTFRCH		0x10		/* Interrupt force 32-63 */
#define	MCFINTC_INTFRCL		0x14		/* Interrupt force 1-31 */
#define	MCFINTC_IRLR		0x18		/* */
#define	MCFINTC_IACKL		0x19		/* */
#define	MCFINTC_ICR0		0x40		/* Base ICR register */

#define MCF_GPIO_PAR_UART	0x10007c
#define UART0_ENABLE_MASK	0x000f
#define UART1_ENABLE_MASK	0x00f0
#define UART2_ENABLE_MASK	0x3f00

#define MCF_GPIO_PAR_FECI2C	0x100082
#define PAR_SDA_ENABLE_MASK	0x0003
#define PAR_SCL_ENABLE_MASK	0x000c

#define MCFSIM_WRRR		0x140000
#define MCFSIM_SDCR		0x40

/*********************************************************************
 * SDRAM Controller (SDRAMC)
 *********************************************************************/

/* Register read/write macros */
#define MCF_SDRAMC_SDMR		(*(vuint32*)(void*)(&__IPSBAR[0x000040]))
#define MCF_SDRAMC_SDCR		(*(vuint32*)(void*)(&__IPSBAR[0x000044]))
#define MCF_SDRAMC_SDCFG1	(*(vuint32*)(void*)(&__IPSBAR[0x000048]))
#define MCF_SDRAMC_SDCFG2	(*(vuint32*)(void*)(&__IPSBAR[0x00004C]))
#define MCF_SDRAMC_SDBAR0	(*(vuint32*)(void*)(&__IPSBAR[0x000050]))
#define MCF_SDRAMC_SDBAR1	(*(vuint32*)(void*)(&__IPSBAR[0x000058]))
#define MCF_SDRAMC_SDMR0	(*(vuint32*)(void*)(&__IPSBAR[0x000054]))
#define MCF_SDRAMC_SDMR1	(*(vuint32*)(void*)(&__IPSBAR[0x00005C]))

/* Bit definitions and macros for MCF_SDRAMC_SDMR */
#define MCF_SDRAMC_SDMR_CMD		(0x00010000)
#define MCF_SDRAMC_SDMR_AD(x)		(((x)&0x00000FFF)<<18)
#define MCF_SDRAMC_SDMR_BNKAD(x)	(((x)&0x00000003)<<30)
#define MCF_SDRAMC_SDMR_BNKAD_LMR	(0x00000000)
#define MCF_SDRAMC_SDMR_BNKAD_LEMR	(0x40000000)

/* Bit definitions and macros for MCF_SDRAMC_SDCR */
#define MCF_SDRAMC_SDCR_IPALL		(0x00000002)
#define MCF_SDRAMC_SDCR_IREF		(0x00000004)
#define MCF_SDRAMC_SDCR_DQS_OE(x)	(((x)&0x00000003)<<10)
#define MCF_SDRAMC_SDCR_DQP_BP		(0x00008000)
#define MCF_SDRAMC_SDCR_RCNT(x)		(((x)&0x0000003F)<<16)
#define MCF_SDRAMC_SDCR_MUX(x)		(((x)&0x00000003)<<24)
#define MCF_SDRAMC_SDCR_REF		(0x10000000)
#define MCF_SDRAMC_SDCR_CKE		(0x40000000)
#define MCF_SDRAMC_SDCR_MODE_EN		(0x80000000)

/* Bit definitions and macros for MCF_SDRAMC_SDCFG1 */
#define MCF_SDRAMC_SDCFG1_WTLAT(x)	(((x)&0x00000007)<<4)
#define MCF_SDRAMC_SDCFG1_REF2ACT(x)	(((x)&0x0000000F)<<8)
#define MCF_SDRAMC_SDCFG1_PRE2ACT(x)	(((x)&0x00000007)<<12)
#define MCF_SDRAMC_SDCFG1_ACT2RW(x)	(((x)&0x00000007)<<16)
#define MCF_SDRAMC_SDCFG1_RDLAT(x)	(((x)&0x0000000F)<<20)
#define MCF_SDRAMC_SDCFG1_SWT2RD(x)	(((x)&0x00000007)<<24)
#define MCF_SDRAMC_SDCFG1_SRD2RW(x)	(((x)&0x0000000F)<<28)

/* Bit definitions and macros for MCF_SDRAMC_SDCFG2 */
#define MCF_SDRAMC_SDCFG2_BL(x)		(((x)&0x0000000F)<<16)
#define MCF_SDRAMC_SDCFG2_BRD2WT(x)	(((x)&0x0000000F)<<20)
#define MCF_SDRAMC_SDCFG2_BWT2RW(x)	(((x)&0x0000000F)<<24)
#define MCF_SDRAMC_SDCFG2_BRD2PRE(x)	(((x)&0x0000000F)<<28)

/* Bit definitions and macros for MCF_SDRAMC_SDBARn */
#define MCF_SDRAMC_SDBARn_BASE(x)	(((x)&0x00003FFF)<<18)
#define MCF_SDRAMC_SDBARn_BA(x)		((x)&0xFFFF0000)

/* Bit definitions and macros for MCF_SDRAMC_SDMRn */
#define MCF_SDRAMC_SDMRn_V		(0x00000001)
#define MCF_SDRAMC_SDMRn_WP		(0x00000080)
#define MCF_SDRAMC_SDMRn_MASK(x)	(((x)&0x00003FFF)<<18)
#define MCF_SDRAMC_SDMRn_BAM_4G		(0xFFFF0000)
#define MCF_SDRAMC_SDMRn_BAM_2G		(0x7FFF0000)
#define MCF_SDRAMC_SDMRn_BAM_1G		(0x3FFF0000)
#define MCF_SDRAMC_SDMRn_BAM_1024M	(0x3FFF0000)
#define MCF_SDRAMC_SDMRn_BAM_512M	(0x1FFF0000)
#define MCF_SDRAMC_SDMRn_BAM_256M	(0x0FFF0000)
#define MCF_SDRAMC_SDMRn_BAM_128M	(0x07FF0000)
#define MCF_SDRAMC_SDMRn_BAM_64M	(0x03FF0000)
#define MCF_SDRAMC_SDMRn_BAM_32M	(0x01FF0000)
#define MCF_SDRAMC_SDMRn_BAM_16M	(0x00FF0000)
#define MCF_SDRAMC_SDMRn_BAM_8M		(0x007F0000)
#define MCF_SDRAMC_SDMRn_BAM_4M		(0x003F0000)
#define MCF_SDRAMC_SDMRn_BAM_2M		(0x001F0000)
#define MCF_SDRAMC_SDMRn_BAM_1M		(0x000F0000)
#define MCF_SDRAMC_SDMRn_BAM_1024K	(0x000F0000)
#define MCF_SDRAMC_SDMRn_BAM_512K	(0x00070000)
#define MCF_SDRAMC_SDMRn_BAM_256K	(0x00030000)
#define MCF_SDRAMC_SDMRn_BAM_128K	(0x00010000)
#define MCF_SDRAMC_SDMRn_BAM_64K	(0x00000000)

/*********************************************************************
 * Interrupt Controller (INTC)
 ********************************************************************/
#define INT0_LO_RSVD0		(0)
#define INT0_LO_EPORT1		(1)
#define INT0_LO_EPORT2		(2)
#define INT0_LO_EPORT3		(3)
#define INT0_LO_EPORT4		(4)
#define INT0_LO_EPORT5		(5)
#define INT0_LO_EPORT6		(6)
#define INT0_LO_EPORT7		(7)
#define INT0_LO_SCM		(8)
#define INT0_LO_DMA0		(9)
#define INT0_LO_DMA1		(10)
#define INT0_LO_DMA2		(11)
#define INT0_LO_DMA3		(12)
#define INT0_LO_UART0		(13)
#define INT0_LO_UART1		(14)
#define INT0_LO_UART2		(15)
#define INT0_LO_RSVD1		(16)
#define INT0_LO_I2C		(17)
#define INT0_LO_QSPI		(18)
#define INT0_LO_DTMR0		(19)
#define INT0_LO_DTMR1		(20)
#define INT0_LO_DTMR2		(21)
#define INT0_LO_DTMR3		(22)
#define INT0_LO_FEC0_TXF	(23)
#define INT0_LO_FEC0_TXB	(24)
#define INT0_LO_FEC0_UN		(25)
#define INT0_LO_FEC0_RL		(26)
#define INT0_LO_FEC0_RXF	(27)
#define INT0_LO_FEC0_RXB	(28)
#define INT0_LO_FEC0_MII	(29)
#define INT0_LO_FEC0_LC		(30)
#define INT0_LO_FEC0_HBERR	(31)
#define INT0_HI_FEC0_GRA	(32)
#define INT0_HI_FEC0_EBERR	(33)
#define INT0_HI_FEC0_BABT	(34)
#define INT0_HI_FEC0_BABR	(35)
#define INT0_HI_PIT0		(36)
#define INT0_HI_PIT1		(37)
#define INT0_HI_PIT2		(38)
#define INT0_HI_PIT3		(39)
#define INT0_HI_RNG		(40)
#define INT0_HI_SKHA		(41)
#define INT0_HI_MDHA		(42)
#define INT0_HI_USB		(43)
#define INT0_HI_USB_EP0		(44)
#define INT0_HI_USB_EP1		(45)
#define INT0_HI_USB_EP2		(46)
#define INT0_HI_USB_EP3		(47)
/* 48-63 Reserved */

/* 0-22 Reserved */
#define INT1_LO_FEC1_TXF	(23)
#define INT1_LO_FEC1_TXB	(24)
#define INT1_LO_FEC1_UN		(25)
#define INT1_LO_FEC1_RL		(26)
#define INT1_LO_FEC1_RXF	(27)
#define INT1_LO_FEC1_RXB	(28)
#define INT1_LO_FEC1_MII	(29)
#define INT1_LO_FEC1_LC		(30)
#define INT1_LO_FEC1_HBERR	(31)
#define INT1_HI_FEC1_GRA	(32)
#define INT1_HI_FEC1_EBERR	(33)
#define INT1_HI_FEC1_BABT	(34)
#define INT1_HI_FEC1_BABR	(35)
/* 36-63 Reserved */

/* Bit definitions and macros for INTC_IPRL */
#define INTC_IPRL_INT31		(0x80000000)
#define INTC_IPRL_INT30		(0x40000000)
#define INTC_IPRL_INT29		(0x20000000)
#define INTC_IPRL_INT28		(0x10000000)
#define INTC_IPRL_INT27		(0x08000000)
#define INTC_IPRL_INT26		(0x04000000)
#define INTC_IPRL_INT25		(0x02000000)
#define INTC_IPRL_INT24		(0x01000000)
#define INTC_IPRL_INT23		(0x00800000)
#define INTC_IPRL_INT22		(0x00400000)
#define INTC_IPRL_INT21		(0x00200000)
#define INTC_IPRL_INT20		(0x00100000)
#define INTC_IPRL_INT19		(0x00080000)
#define INTC_IPRL_INT18		(0x00040000)
#define INTC_IPRL_INT17		(0x00020000)
#define INTC_IPRL_INT16		(0x00010000)
#define INTC_IPRL_INT15		(0x00008000)
#define INTC_IPRL_INT14		(0x00004000)
#define INTC_IPRL_INT13		(0x00002000)
#define INTC_IPRL_INT12		(0x00001000)
#define INTC_IPRL_INT11		(0x00000800)
#define INTC_IPRL_INT10		(0x00000400)
#define INTC_IPRL_INT9		(0x00000200)
#define INTC_IPRL_INT8		(0x00000100)
#define INTC_IPRL_INT7		(0x00000080)
#define INTC_IPRL_INT6		(0x00000040)
#define INTC_IPRL_INT5		(0x00000020)
#define INTC_IPRL_INT4		(0x00000010)
#define INTC_IPRL_INT3		(0x00000008)
#define INTC_IPRL_INT2		(0x00000004)
#define INTC_IPRL_INT1		(0x00000002)
#define INTC_IPRL_INT0		(0x00000001)

/* Bit definitions and macros for RCR */
#define RCM_RCR_FRCRSTOUT	(0x40)
#define RCM_RCR_SOFTRST		(0x80)

#define FMPLL_SYNSR_LOCK	(0x00000008)

#endif	/* __M5275_H__ */
