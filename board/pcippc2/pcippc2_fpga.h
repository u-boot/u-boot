/*
 * (C) Copyright 2002
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#ifndef _PCIPPC2_FPGA_H_
#define _PCIPPC2_FPGA_H_

#define FPGA_VENDOR_ID			0x1310
#define FPGA_DEVICE_ID			0x000d

#define HW_FPGA0_INT			0x0000
#define HW_FPGA0_BOARD		0x0060
#define HW_FPGA0_UART1			0x0080
#define HW_FPGA0_UART2			0x0100
#define HW_FPGA0_RTC			0x2000
#define HW_FPGA0_DOC			0x4000
#define HW_FPGA1_RTC			0x0000
#define HW_FPGA1_DOC			0x4000

#define HW_FPGA0_INT_INTR_MASK		0x30
#define HW_FPGA0_INT_INTR_STATUS	0x34
#define HW_FPGA0_INT_INTR_EOI		0x40
#define HW_FPGA0_INT_SERIAL_CONFIG	0x5c

#define HW_FPGA0_WDT_CTRL		0x44
#define HW_FPGA0_WDT_PROG		0x48
#define HW_FPGA0_WDT_VAL		0x4c
#define HW_FPGA0_WDT_REFRESH		0x50

#endif
