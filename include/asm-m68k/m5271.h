/*
 * mcf5271.h -- Definitions for Motorola Coldfire 5271
 *
 * (C) Copyright 2006, Lab X Technologies <zachary.landau@labxtechnologies.com>
 * Based on mcf5272sim.h of uCLinux distribution:
 *      (C) Copyright 1999, Greg Ungerer (gerg@snapgear.com)
 *      (C) Copyright 2000, Lineo Inc. (www.lineo.com)
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

#ifndef	_MCF5271_H_
#define	_MCF5271_H_

#define mbar_readLong(x)	*((volatile unsigned long *) (CFG_MBAR + x))
#define mbar_readShort(x)	*((volatile unsigned short *) (CFG_MBAR + x))
#define mbar_readByte(x)	*((volatile unsigned char *) (CFG_MBAR + x))
#define mbar_writeLong(x,y)	*((volatile unsigned long *) (CFG_MBAR + x)) = y
#define mbar_writeShort(x,y)	*((volatile unsigned short *) (CFG_MBAR + x)) = y
#define mbar_writeByte(x,y)	*((volatile unsigned char *) (CFG_MBAR + x)) = y

#define MCF_FMPLL_SYNCR				0x120000
#define MCF_FMPLL_SYNSR				0x120004
#define MCF_FMPLL_SYNCR_MFD(x)			((x&0x7)<<24)
#define MCF_FMPLL_SYNCR_RFD(x)			((x&0x7)<<19)
#define MCF_FMPLL_SYNSR_LOCK			0x8

#define MCF_WTM_WCR				0x140000
#define MCF_WTM_WCNTR				0x140004
#define MCF_WTM_WSR				0x140006
#define MCF_WTM_WCR_EN				0x0001

#define MCF_RCM_RCR				0x110000
#define MCF_RCM_RCR_FRCRSTOUT			0x40
#define MCF_RCM_RCR_SOFTRST			0x80

#define MCF_GPIO_PAR_AD				0x100040
#define MCF_GPIO_PAR_CS				0x100045
#define MCF_GPIO_PAR_SDRAM			0x100046
#define MCF_GPIO_PAR_FECI2C			0x100047
#define MCF_GPIO_PAR_UART			0x100048

#define MCF_CCM_CIR				0x11000A
#define MCF_CCM_CIR_PRN_MASK			0x3F
#define MCF_CCM_CIR_PIN_LEN			6
#define MCF_CCM_CIR_PIN_MCF5270			0x2e
#define MCF_CCM_CIR_PIN_MCF5271			0x80

#define MCF_GPIO_AD_ADDR23			0x80
#define MCF_GPIO_AD_ADDR22			0x40
#define MCF_GPIO_AD_ADDR21			0x20
#define MCF_GPIO_AD_DATAL			0x01
#define MCF_GPIO_AD_MASK			0xe1

#define MCF_GPIO_PAR_CS_PAR_CS2			0x04

#define MCF_GPIO_SDRAM_CSSDCS_00		0x00	/* CS[3:2] pins: CS3, CS2 */
#define MCF_GPIO_SDRAM_CSSDCS_01		0x40	/* CS[3:2] pins: CS3, SD_CS0 */
#define MCF_GPIO_SDRAM_CSSDCS_10		0x80	/* CS[3:2] pins: SD_CS1, SC2 */
#define MCF_GPIO_SDRAM_CSSDCS_11		0xc0	/* CS[3:2] pins: SD_CS1, SD_CS0 */
#define MCF_GPIO_SDRAM_SDWE			0x20	/* WE pin */
#define MCF_GPIO_SDRAM_SCAS			0x10	/* CAS pin */
#define MCF_GPIO_SDRAM_SRAS			0x08	/* RAS pin */
#define MCF_GPIO_SDRAM_SCKE			0x04	/* CKE pin */
#define MCF_GPIO_SDRAM_SDCS_00			0x00	/* SD_CS[0:1] pins: GPIO, GPIO */
#define MCF_GPIO_SDRAM_SDCS_01			0x01	/* SD_CS[0:1] pins: GPIO, SD_CS0 */
#define MCF_GPIO_SDRAM_SDCS_10			0x02	/* SD_CS[0:1] pins: SD_CS1, GPIO */
#define MCF_GPIO_SDRAM_SDCS_11			0x03	/* SD_CS[0:1] pins: SD_CS1, SD_CS0 */

#define MCF_GPIO_PAR_UART_U0RTS			0x0001
#define MCF_GPIO_PAR_UART_U0CTS			0x0002
#define MCF_GPIO_PAR_UART_U0TXD			0x0004
#define MCF_GPIO_PAR_UART_U0RXD			0x0008
#define MCF_GPIO_PAR_UART_U1RXD_UART1		0x0C00
#define MCF_GPIO_PAR_UART_U1TXD_UART1		0x0300

#define MCF_GPIO_PAR_SDRAM_PAR_CSSDCS(x)	(((x)&0x03)<<6)

#define MCF_SDRAMC_DCR				0x000040
#define MCF_SDRAMC_DACR0			0x000048
#define MCF_SDRAMC_DMR0				0x00004C

#define MCF_SDRAMC_DCR_RC(x)			(((x)&0x01FF)<<0)
#define MCF_SDRAMC_DCR_RTIM(x)			(((x)&0x0003)<<9)
#define MCF_SDRAMC_DCR_IS			0x0800
#define MCF_SDRAMC_DCR_COC			0x1000
#define MCF_SDRAMC_DCR_NAM			0x2000

#define MCF_SDRAMC_DACRn_IP			0x00000008
#define MCF_SDRAMC_DACRn_PS(x)			(((x)&0x00000003)<<4)
#define MCF_SDRAMC_DACRn_MRS			0x00000040
#define MCF_SDRAMC_DACRn_CBM(x)			(((x)&0x00000007)<<8)
#define MCF_SDRAMC_DACRn_CASL(x)		(((x)&0x00000003)<<12)
#define MCF_SDRAMC_DACRn_RE			0x00008000
#define MCF_SDRAMC_DACRn_BA(x)			(((x)&0x00003FFF)<<18)

#define MCF_SDRAMC_DMRn_BAM_8M			0x007C0000
#define MCF_SDRAMC_DMRn_BAM_16M			0x00FC0000
#define MCF_SDRAMC_DMRn_V			0x00000001

#define MCFSIM_ICR1				0x000C41

/*********************************************************************
* Interrupt Controller (INTC)
*********************************************************************/
#define INT0_LO_RSVD0			(0)
#define INT0_LO_EPORT1			(1)
#define INT0_LO_EPORT2			(2)
#define INT0_LO_EPORT3			(3)
#define INT0_LO_EPORT4			(4)
#define INT0_LO_EPORT5			(5)
#define INT0_LO_EPORT6			(6)
#define INT0_LO_EPORT7			(7)
#define INT0_LO_SCM			(8)
#define INT0_LO_DMA0			(9)
#define INT0_LO_DMA1			(10)
#define INT0_LO_DMA2			(11)
#define INT0_LO_DMA3			(12)
#define INT0_LO_UART0			(13)
#define INT0_LO_UART1			(14)
#define INT0_LO_UART2			(15)
#define INT0_LO_RSVD1			(16)
#define INT0_LO_I2C			(17)
#define INT0_LO_QSPI			(18)
#define INT0_LO_DTMR0			(19)
#define INT0_LO_DTMR1			(20)
#define INT0_LO_DTMR2			(21)
#define INT0_LO_DTMR3			(22)
#define INT0_LO_FEC_TXF			(23)
#define INT0_LO_FEC_TXB			(24)
#define INT0_LO_FEC_UN			(25)
#define INT0_LO_FEC_RL			(26)
#define INT0_LO_FEC_RXF			(27)
#define INT0_LO_FEC_RXB			(28)
#define INT0_LO_FEC_MII			(29)
#define INT0_LO_FEC_LC			(30)
#define INT0_LO_FEC_HBERR		(31)
#define INT0_HI_FEC_GRA			(32)
#define INT0_HI_FEC_EBERR		(33)
#define INT0_HI_FEC_BABT		(34)
#define INT0_HI_FEC_BABR		(35)
#define INT0_HI_PIT0			(36)
#define INT0_HI_PIT1			(37)
#define INT0_HI_PIT2			(38)
#define INT0_HI_PIT3			(39)
#define INT0_HI_RNG			(40)
#define INT0_HI_SKHA			(41)
#define INT0_HI_MDHA			(42)
#define INT0_HI_CAN1_BUF0I		(43)
#define INT0_HI_CAN1_BUF1I		(44)
#define INT0_HI_CAN1_BUF2I		(45)
#define INT0_HI_CAN1_BUF3I		(46)
#define INT0_HI_CAN1_BUF4I		(47)
#define INT0_HI_CAN1_BUF5I		(48)
#define INT0_HI_CAN1_BUF6I		(49)
#define INT0_HI_CAN1_BUF7I		(50)
#define INT0_HI_CAN1_BUF8I		(51)
#define INT0_HI_CAN1_BUF9I		(52)
#define INT0_HI_CAN1_BUF10I		(53)
#define INT0_HI_CAN1_BUF11I		(54)
#define INT0_HI_CAN1_BUF12I		(55)
#define INT0_HI_CAN1_BUF13I		(56)
#define INT0_HI_CAN1_BUF14I		(57)
#define INT0_HI_CAN1_BUF15I		(58)
#define INT0_HI_CAN1_ERRINT		(59)
#define INT0_HI_CAN1_BOFFINT		(60)
/* 60-63 Reserved */

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

#endif				/* _MCF5271_H_ */
