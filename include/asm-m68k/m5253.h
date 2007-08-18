/*
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

#ifndef m5253_h
#define m5253_h
/****************************************************************************/

/*
* PLL Module (PLL)
*/

/* Register read/write macros */
#define PLL_PLLCR		(0x000180)

#define SIM_RSR			(0x000000)
#define SIM_SYPCR		(0x000001)
#define SIM_SWIVR		(0x000002)
#define SIM_SWSR		(0x000003)
#define SIM_MPARK		(0x00000C)

/* Bit definitions and macros for RSR */
#define SIM_RSR_SWTR		(0x20)
#define SIM_RSR_HRST		(0x80)

/* Register read/write macros */
#define CIM_MISCCR		(0x000500)
#define CIM_ATA_DADDR		(0x000504)
#define CIM_ATA_DCOUNT		(0x000508)
#define CIM_RTC_TIME		(0x00050C)
#define CIM_USB_CANCLK		(0x000510)

/* Bit definitions and macros for MISCCR */
#define CIM_MISCCR_ADTA		(0x00000001)
#define CIM_MISCCR_ADTD		(0x00000002)
#define CIM_MISCCR_ADIE		(0x00000004)
#define CIM_MISCCR_ADIC		(0x00000008)
#define CIM_MISCCR_ADIP		(0x00000010)
#define CIM_MISCCR_CPUEND	(0x00000020)
#define CIM_MISCCR_DMAEND	(0x00000040)
#define CIM_MISCCR_RTCCLR	(0x00000080)
#define CIM_MISCCR_RTCPL	(0x00000100)
#define CIM_MISCCR_URIE		(0x00000800)
#define CIM_MISCCR_URIC		(0x00001000)
#define CIM_MISCCR_URIP		(0x00002000)

/* Bit definitions and macros for ATA_DADDR */
#define CIM_ATA_DADDR_ATAADDR(x)	(((x)&0x00003FFF)<<2)
#define CIM_ATA_DADDR_RAMADDR(x)	(((x)&0x00003FFF)<<18)

/* Bit definitions and macros for ATA_DCOUNT */
#define CIM_ATA_DCOUNT_COUNT(x)		(((x)&0x0000FFFF))

#endif				/* m5253_h */
