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

#define MCF_GPIO_PAR_SDRAM_PAR_CSSDCS(x)        (((x)&0x03)<<6)

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

#endif	/* _MCF5271_H_ */
