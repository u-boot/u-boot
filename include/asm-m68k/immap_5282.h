/*
 * MCF5282 Internal Memory Map
 *
 * Copyright (c) 2003 Josef Baumgartner <josef.baumgartner@telex.de>
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

#ifndef __IMMAP_5282__
#define __IMMAP_5282__

#define MMAP_SCM	(CFG_MBAR + 0x00000000)
#define MMAP_SDRAMC	(CFG_MBAR + 0x00000040)
#define MMAP_FBCS	(CFG_MBAR + 0x00000080)
#define MMAP_DMA0	(CFG_MBAR + 0x00000100)
#define MMAP_DMA1	(CFG_MBAR + 0x00000140)
#define MMAP_DMA2	(CFG_MBAR + 0x00000180)
#define MMAP_DMA3	(CFG_MBAR + 0x000001C0)
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
#define MMAP_QADC	(CFG_MBAR + 0x00190000)
#define MMAP_GPTMRA	(CFG_MBAR + 0x001A0000)
#define MMAP_GPTMRB	(CFG_MBAR + 0x001B0000)
#define MMAP_CAN	(CFG_MBAR + 0x001C0000)
#define MMAP_CFMC	(CFG_MBAR + 0x001D0000)
#define MMAP_CFMMEM	(CFG_MBAR + 0x04000000)

/* System Control Module */
typedef struct scm_ctrl {
	u32 ipsbar;
	u32 res1;
	u32 rambar;
	u32 res2;
	u8 crsr;
	u8 cwcr;
	u8 lpicr;
	u8 cwsr;
	u32 res3;
	u8 mpark;
	u8 res4[3];
	u8 pacr0;
	u8 pacr1;
	u8 pacr2;
	u8 pacr3;
	u8 pacr4;
	u8 res5;
	u8 pacr5;
	u8 pacr6;
	u8 pacr7;
	u8 res6;
	u8 pacr8;
	u8 res7;
	u8 gpacr0;
	u8 gpacr1;
	u16 res8;
} scm_t;

/* Flexbus module Chip select registers */
typedef struct fbcs_ctrl {
	u16 csar0;		/* 0x00 Chip-Select Address Register 0 */
	u16 res0;
	u32 csmr0;		/* 0x04 Chip-Select Mask Register 0 */
	u16 res1;		/* 0x08 */
	u16 cscr0;		/* 0x0A Chip-Select Control Register 0 */

	u16 csar1;		/* 0x0C Chip-Select Address Register 1 */
	u16 res2;
	u32 csmr1;		/* 0x10 Chip-Select Mask Register 1 */
	u16 res3;		/* 0x14 */
	u16 cscr1;		/* 0x16 Chip-Select Control Register 1 */

	u16 csar2;		/* 0x18 Chip-Select Address Register 2 */
	u16 res4;
	u32 csmr2;		/* 0x1C Chip-Select Mask Register 2 */
	u16 res5;		/* 0x20 */
	u16 cscr2;		/* 0x22 Chip-Select Control Register 2 */

	u16 csar3;		/* 0x24 Chip-Select Address Register 3 */
	u16 res6;
	u32 csmr3;		/* 0x28 Chip-Select Mask Register 3 */
	u16 res7;		/* 0x2C */
	u16 cscr3;		/* 0x2E Chip-Select Control Register 3 */

	u16 csar4;		/* 0x30 Chip-Select Address Register 4 */
	u16 res8;
	u32 csmr4;		/* 0x34 Chip-Select Mask Register 4 */
	u16 res9;		/* 0x38 */
	u16 cscr4;		/* 0x3A Chip-Select Control Register 4 */

	u16 csar5;		/* 0x3C Chip-Select Address Register 5 */
	u16 res10;
	u32 csmr5;		/* 0x40 Chip-Select Mask Register 5 */
	u16 res11;		/* 0x44 */
	u16 cscr5;		/* 0x46 Chip-Select Control Register 5 */

	u16 csar6;		/* 0x48 Chip-Select Address Register 5 */
	u16 res12;
	u32 csmr6;		/* 0x4C Chip-Select Mask Register 5 */
	u16 res13;		/* 0x50 */
	u16 cscr6;		/* 0x52 Chip-Select Control Register 5 */

	u16 csar7;		/* 0x54 Chip-Select Address Register 5 */
	u16 res14;
	u32 csmr7;		/* 0x58 Chip-Select Mask Register 5 */
	u16 res15;		/* 0x5C */
	u16 cscr7;		/* 0x5E Chip-Select Control Register 5 */
} fbcs_t;

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

/* Clock Module registers */
typedef struct pll_ctrl {
	u16 syncr;		/* 0x00 synthesizer control register */
	u16 synsr;		/* 0x02 synthesizer status register */
} pll_t;

/* Watchdog registers */
typedef struct wdog_ctrl {
	ushort wcr;
	ushort wmr;
	ushort wcntr;
	ushort wsr;
} wdog_t;

#endif				/* __IMMAP_5282__ */
