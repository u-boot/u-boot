/*
 * MCF5272 Internal Memory Map
 *
 * Copyright (c) 2003 Josef Baumgartner <josef.baumgartner@telex.de>
 *               2006 Zachary P. Landau <zachary.landau@labxtechnologies.com>
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

#ifndef __IMMAP_5271__
#define __IMMAP_5271__

#define MMAP_SCM	(CFG_MBAR + 0x00000000)
#define MMAP_SDRAM	(CFG_MBAR + 0x00000040)
#define MMAP_FBCS	(CFG_MBAR + 0x00000080)
#define MMAP_DMA0	(CFG_MBAR + 0x00000100)
#define MMAP_DMA1	(CFG_MBAR + 0x00000110)
#define MMAP_DMA2	(CFG_MBAR + 0x00000120)
#define MMAP_DMA3	(CFG_MBAR + 0x00000130)
#define MMAP_UART0	(CFG_MBAR + 0x00000200)
#define MMAP_UART1	(CFG_MBAR + 0x00000240)
#define MMAP_UART2	(CFG_MBAR + 0x00000280)
#define MMAP_I2C	(CFG_MBAR + 0x00000300)
#define MMAP_QSPI	(CFG_MBAR + 0x00000340)
#define MMAP_DTMR0	(CFG_MBAR + 0x00000400)
#define MMAP_DTMR1	(CFG_MBAR + 0x00000440)
#define MMAP_DTMR2	(CFG_MBAR + 0x00000480)
#define MMAP_DTMR3	(CFG_MBAR + 0x000004C0)
#define MMAP_INTC0	(CFG_MBAR + 0x00000C00)
#define MMAP_INTC1	(CFG_MBAR + 0x00000D00)
#define MMAP_INTCACK	(CFG_MBAR + 0x00000F00)
#define MMAP_FEC	(CFG_MBAR + 0x00001000)
#define MMAP_FECFIFO	(CFG_MBAR + 0x00001400)
#define MMAP_GPIO	(CFG_MBAR + 0x00100000)
#define MMAP_CCM	(CFG_MBAR + 0x00110000)
#define MMAP_PLL	(CFG_MBAR + 0x00120000)
#define MMAP_EPORT	(CFG_MBAR + 0x00130000)
#define MMAP_WDOG	(CFG_MBAR + 0x00140000)
#define MMAP_PIT0	(CFG_MBAR + 0x00150000)
#define MMAP_PIT1	(CFG_MBAR + 0x00160000)
#define MMAP_PIT2	(CFG_MBAR + 0x00170000)
#define MMAP_PIT3	(CFG_MBAR + 0x00180000)
#define MMAP_MDHA	(CFG_MBAR + 0x00190000)
#define MMAP_RNG	(CFG_MBAR + 0x001A0000)
#define MMAP_SKHA	(CFG_MBAR + 0x001B0000)
#define MMAP_CAN1	(CFG_MBAR + 0x001C0000)
#define MMAP_ETPU	(CFG_MBAR + 0x001D0000)
#define MMAP_CAN2	(CFG_MBAR + 0x001F0000)

/* Interrupt module registers */
typedef struct int0_ctrl {
	/* Interrupt Controller 0 */
	u32 iprh0;		/* 0x00 Pending Register High */
	u32 iprl0;		/* 0x04 Pending Register Low */
	u32 imrh0;		/* 0x08 Mask Register High */
	u32 imrl0;		/* 0x0C Mask Register Low */
	u32 frch0;		/* 0x10 Force Register High */
	u32 frcl0;		/* 0x14 Force Register Low */
	u8 irlr;		/* 0x18 */
	u8 iacklpr;		/* 0x19 */
	u16 res1[19];		/* 0x1a - 0x3c */
	u8 icr0[64];		/* 0x40 - 0x7F Control registers */
	u32 res3[24];		/* 0x80 - 0xDF */
	u8 swiack0;		/* 0xE0 Software Interrupt Acknowledge */
	u8 res4[3];		/* 0xE1 - 0xE3 */
	u8 Lniack0_1;		/* 0xE4 Level n interrupt acknowledge resister */
	u8 res5[3];		/* 0xE5 - 0xE7 */
	u8 Lniack0_2;		/* 0xE8 Level n interrupt acknowledge resister */
	u8 res6[3];		/* 0xE9 - 0xEB */
	u8 Lniack0_3;		/* 0xEC Level n interrupt acknowledge resister */
	u8 res7[3];		/* 0xED - 0xEF */
	u8 Lniack0_4;		/* 0xF0 Level n interrupt acknowledge resister */
	u8 res8[3];		/* 0xF1 - 0xF3 */
	u8 Lniack0_5;		/* 0xF4 Level n interrupt acknowledge resister */
	u8 res9[3];		/* 0xF5 - 0xF7 */
	u8 Lniack0_6;		/* 0xF8 Level n interrupt acknowledge resister */
	u8 resa[3];		/* 0xF9 - 0xFB */
	u8 Lniack0_7;		/* 0xFC Level n interrupt acknowledge resister */
	u8 resb[3];		/* 0xFD - 0xFF */
} int0_t;

#endif				/* __IMMAP_5271__ */
