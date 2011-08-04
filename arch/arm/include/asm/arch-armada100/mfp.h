/*
 * Based on linux/arch/arm/mach-mpp/include/mfp-pxa168.h
 * (C) Copyright 2007
 * Marvell Semiconductor <www.marvell.com>
 * 2007-08-21: eric miao <eric.miao@marvell.com>
 *
 * (C) Copyright 2010
 * Marvell Semiconductor <www.marvell.com>
 * Written-by: Prafulla Wadaskar <prafulla@marvell.com>
 * Contributor: Mahavir Jain <mjain@marvell.com>
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301 USA
 */

#ifndef __ARMADA100_MFP_H
#define __ARMADA100_MFP_H

/*
 * Frequently used MFP Configuration macros for all ARMADA100 family of SoCs
 *
 * 				    offset, pull,pF, drv,dF, edge,eF ,afn,aF
 */
/* UART1 */
#define MFP107_UART1_TXD	(MFP_REG(0x01ac) | MFP_AF1 | MFP_DRIVE_FAST)
#define MFP107_UART1_RXD	(MFP_REG(0x01ac) | MFP_AF2 | MFP_DRIVE_FAST)
#define MFP108_UART1_RXD	(MFP_REG(0x01b0) | MFP_AF1 | MFP_DRIVE_FAST)
#define MFP108_UART1_TXD	(MFP_REG(0x01b0) | MFP_AF2 | MFP_DRIVE_FAST)
#define MFP109_UART1_CTS	(MFP_REG(0x01b4) | MFP_AF1 | MFP_DRIVE_MEDIUM)
#define MFP109_UART1_RTS	(MFP_REG(0x01b4) | MFP_AF2 | MFP_DRIVE_MEDIUM)
#define MFP110_UART1_RTS	(MFP_REG(0x01b8) | MFP_AF1 | MFP_DRIVE_MEDIUM)
#define MFP110_UART1_CTS	(MFP_REG(0x01b8) | MFP_AF2 | MFP_DRIVE_MEDIUM)
#define MFP111_UART1_RI		(MFP_REG(0x01bc) | MFP_AF1 | MFP_DRIVE_MEDIUM)
#define MFP111_UART1_DSR	(MFP_REG(0x01bc) | MFP_AF2 | MFP_DRIVE_MEDIUM)
#define MFP112_UART1_DTR	(MFP_REG(0x01c0) | MFP_AF1 | MFP_DRIVE_MEDIUM)
#define MFP112_UART1_DCD	(MFP_REG(0x01c0) | MFP_AF2 | MFP_DRIVE_MEDIUM)

/* UART2 */
#define MFP47_UART2_RXD		(MFP_REG(0x0028) | MFP_AF6 | MFP_DRIVE_MEDIUM)
#define MFP48_UART2_TXD		(MFP_REG(0x002c) | MFP_AF6 | MFP_DRIVE_MEDIUM)
#define MFP88_UART2_RXD		(MFP_REG(0x0160) | MFP_AF2 | MFP_DRIVE_MEDIUM)
#define MFP89_UART2_TXD		(MFP_REG(0x0164) | MFP_AF2 | MFP_DRIVE_MEDIUM)

/* UART3 */
#define MFPO8_UART3_TXD		(MFP_REG(0x06c) | MFP_AF2 | MFP_DRIVE_MEDIUM)
#define MFPO9_UART3_RXD		(MFP_REG(0x070) | MFP_AF2 | MFP_DRIVE_MEDIUM)

/* I2c */
#define MFP105_CI2C_SDA		(MFP_REG(0x1a4) | MFP_AF1 | MFP_DRIVE_MEDIUM)
#define MFP106_CI2C_SCL		(MFP_REG(0x1a8) | MFP_AF1 | MFP_DRIVE_MEDIUM)

/* More macros can be defined here... */

#define MFP_PIN_MAX	117

#endif /* __ARMADA100_MFP_H */
