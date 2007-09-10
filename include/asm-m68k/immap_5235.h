/*
 * MCF5329 Internal Memory Map
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

#ifndef __IMMAP_5235__
#define __IMMAP_5235__

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

/* System Control Module register */
typedef struct scm_ctrl {
	u32 ipsbar;		/* 0x00 - MBAR */
	u32 res1;		/* 0x04 */
	u32 rambar;		/* 0x08 - RAMBAR */
	u32 res2;		/* 0x0C */
	u8 crsr;		/* 0x10 Core Reset Status Register */
	u8 cwcr;		/* 0x11 Core Watchdog Control Register */
	u8 lpicr;		/* 0x12 Low-Power Interrupt Control Register */
	u8 cwsr;		/* 0x13 Core Watchdog Service Register */
	u32 dmareqc;		/* 0x14 */
	u32 res3;		/* 0x18 */
	u32 mpark;		/* 0x1C */
	u8 mpr;			/* 0x20 */
	u8 res4[3];		/* 0x21 - 0x23 */
	u8 pacr0;		/* 0x24 */
	u8 pacr1;		/* 0x25 */
	u8 pacr2;		/* 0x26 */
	u8 pacr3;		/* 0x27 */
	u8 pacr4;		/* 0x28 */
	u32 res5;		/* 0x29 */
	u8 pacr5;		/* 0x2a */
	u8 pacr6;		/* 0x2b */
	u8 pacr7;		/* 0x2c */
	u32 res6;		/* 0x2d */
	u8 pacr8;		/* 0x2e */
	u32 res7;		/* 0x2f */
	u8 gpacr;		/* 0x30 */
	u8 res8[3];		/* 0x31 - 0x33 */
} scm_t;

/* SDRAM controller registers */
typedef struct sdram_ctrl {
	u16 dcr;		/* 0x00 Control register */
	u16 res1[3];		/* 0x02 - 0x07 */
	u32 dacr0;		/* 0x08 address and control register 0 */
	u32 dmr0;		/* 0x0C mask register block 0 */
	u32 dacr1;		/* 0x10 address and control register 1 */
	u32 dmr1;		/* 0x14 mask register block 1 */
} sdram_t;

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

/* QSPI module registers */
typedef struct qspi_ctrl {
	u16 qmr;		/* Mode register */
	u16 res1;
	u16 qdlyr;		/* Delay register */
	u16 res2;
	u16 qwr;		/* Wrap register */
	u16 res3;
	u16 qir;		/* Interrupt register */
	u16 res4;
	u16 qar;		/* Address register */
	u16 res5;
	u16 qdr;		/* Data register */
	u16 res6;
} qspi_t;

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

typedef struct int1_ctrl {
	/* Interrupt Controller 1 */
	u32 iprh1;		/* 0x00 Pending Register High */
	u32 iprl1;		/* 0x04 Pending Register Low */
	u32 imrh1;		/* 0x08 Mask Register High */
	u32 imrl1;		/* 0x0C Mask Register Low */
	u32 frch1;		/* 0x10 Force Register High */
	u32 frcl1;		/* 0x14 Force Register Low */
	u8 irlr;		/* 0x18 */
	u8 iacklpr;		/* 0x19 */
	u16 res1[19];		/* 0x1a - 0x3c */
	u8 icr1[64];		/* 0x40 - 0x7F */
	u32 res4[24];		/* 0x80 - 0xDF */
	u8 swiack1;		/* 0xE0 Software Interrupt Acknowledge */
	u8 res5[3];		/* 0xE1 - 0xE3 */
	u8 Lniack1_1;		/* 0xE4 Level n interrupt acknowledge resister */
	u8 res6[3];		/* 0xE5 - 0xE7 */
	u8 Lniack1_2;		/* 0xE8 Level n interrupt acknowledge resister */
	u8 res7[3];		/* 0xE9 - 0xEB */
	u8 Lniack1_3;		/* 0xEC Level n interrupt acknowledge resister */
	u8 res8[3];		/* 0xED - 0xEF */
	u8 Lniack1_4;		/* 0xF0 Level n interrupt acknowledge resister */
	u8 res9[3];		/* 0xF1 - 0xF3 */
	u8 Lniack1_5;		/* 0xF4 Level n interrupt acknowledge resister */
	u8 resa[3];		/* 0xF5 - 0xF7 */
	u8 Lniack1_6;		/* 0xF8 Level n interrupt acknowledge resister */
	u8 resb[3];		/* 0xF9 - 0xFB */
	u8 Lniack1_7;		/* 0xFC Level n interrupt acknowledge resister */
	u8 resc[3];		/* 0xFD - 0xFF */
} int1_t;

typedef struct intgack_ctrl1 {
	/* Global IACK Registers */
	u8 swiack;		/* 0xE0 Global Software Interrupt Acknowledge */
	u8 Lniack[7];		/* 0xE1 - 0xE7 Global Level 0 Interrupt Acknowledge */
} intgack_t;

/* GPIO port registers */
typedef struct gpio_ctrl {
	/* Port Output Data Registers */
	u8 podr_addr;		/* 0x00 */
	u8 podr_datah;		/* 0x01 */
	u8 podr_datal;		/* 0x02 */
	u8 podr_busctl;		/* 0x03 */
	u8 podr_bs;		/* 0x04 */
	u8 podr_cs;		/* 0x05 */
	u8 podr_sdram;		/* 0x06 */
	u8 podr_feci2c;		/* 0x07 */
	u8 podr_uarth;		/* 0x08 */
	u8 podr_uartl;		/* 0x09 */
	u8 podr_qspi;		/* 0x0A */
	u8 podr_timer;		/* 0x0B */
	u8 podr_etpu;		/* 0x0C */
	u8 res1[3];		/* 0x0D - 0x0F */

	/* Port Data Direction Registers */
	u8 pddr_addr;		/* 0x10 */
	u8 pddr_datah;		/* 0x11 */
	u8 pddr_datal;		/* 0x12 */
	u8 pddr_busctl;		/* 0x13 */
	u8 pddr_bs;		/* 0x14 */
	u8 pddr_cs;		/* 0x15 */
	u8 pddr_sdram;		/* 0x16 */
	u8 pddr_feci2c;		/* 0x17 */
	u8 pddr_uarth;		/* 0x18 */
	u8 pddr_uartl;		/* 0x19 */
	u8 pddr_qspi;		/* 0x1A */
	u8 pddr_timer;		/* 0x1B */
	u8 pddr_etpu;		/* 0x1C */
	u8 res2[3];		/* 0x1D - 0x1F */

	/* Port Data Direction Registers */
	u8 ppdsdr_addr;		/* 0x20 */
	u8 ppdsdr_datah;	/* 0x21 */
	u8 ppdsdr_datal;	/* 0x22 */
	u8 ppdsdr_busctl;	/* 0x23 */
	u8 ppdsdr_bs;		/* 0x24 */
	u8 ppdsdr_cs;		/* 0x25 */
	u8 ppdsdr_sdram;	/* 0x26 */
	u8 ppdsdr_feci2c;	/* 0x27 */
	u8 ppdsdr_uarth;	/* 0x28 */
	u8 ppdsdr_uartl;	/* 0x29 */
	u8 ppdsdr_qspi;		/* 0x2A */
	u8 ppdsdr_timer;	/* 0x2B */
	u8 ppdsdr_etpu;		/* 0x2C */
	u8 res3[3];		/* 0x2D - 0x2F */

	/* Port Clear Output Data Registers */
	u8 pclrr_addr;		/* 0x30 */
	u8 pclrr_datah;		/* 0x31 */
	u8 pclrr_datal;		/* 0x32 */
	u8 pclrr_busctl;	/* 0x33 */
	u8 pclrr_bs;		/* 0x34 */
	u8 pclrr_cs;		/* 0x35 */
	u8 pclrr_sdram;		/* 0x36 */
	u8 pclrr_feci2c;	/* 0x37 */
	u8 pclrr_uarth;		/* 0x38 */
	u8 pclrr_uartl;		/* 0x39 */
	u8 pclrr_qspi;		/* 0x3A */
	u8 pclrr_timer;		/* 0x3B */
	u8 pclrr_etpu;		/* 0x3C */
	u8 res4[3];		/* 0x3D - 0x3F */

	/* Pin Assignment Registers */
	u8 par_ad;		/* 0x40 */
	u8 res5;		/* 0x41 */
	u16 par_busctl;		/* 0x42 */
	u8 par_bs;		/* 0x44 */
	u8 par_cs;		/* 0x45 */
	u8 par_sdram;		/* 0x46 */
	u8 par_feci2c;		/* 0x47 */
	u16 par_uart;		/* 0x48 */
	u8 par_qspi;		/* 0x4A */
	u8 res6;		/* 0x4B */
	u16 par_timer;		/* 0x4C */
	u8 par_etpu;		/* 0x4E */
	u8 res7;		/* 0x4F */

	/* Drive Strength Control Registers */
	u8 dscr_eim;		/* 0x50 */
	u8 dscr_etpu;		/* 0x51 */
	u8 dscr_feci2c;		/* 0x52 */
	u8 dscr_uart;		/* 0x53 */
	u8 dscr_qspi;		/* 0x54 */
	u8 dscr_timer;		/* 0x55 */
	u16 res8;		/* 0x56 */
} gpio_t;

/*Chip configuration module registers */
typedef struct ccm_ctrl {
	u8 rcr;			/* 0x01 */
	u8 rsr;			/* 0x02 */
	u16 res1;		/* 0x03 */
	u16 ccr;		/* 0x04 Chip configuration register */
	u16 lpcr;		/* 0x06 Low-power Control register */
	u16 rcon;		/* 0x08 Rreset configuration register */
	u16 cir;		/* 0x0a Chip identification register */
} ccm_t;

/* Clock Module registers */
typedef struct pll_ctrl {
	u32 syncr;		/* 0x00 synthesizer control register */
	u32 synsr;		/* 0x04 synthesizer status register */
} pll_t;

/* Watchdog registers */
typedef struct wdog_ctrl {
	u16 cr;			/* 0x00 Control register */
	u16 mr;			/* 0x02 Modulus register */
	u16 cntr;		/* 0x04 Count register */
	u16 sr;			/* 0x06 Service register */
} wdog_t;

/* FlexCan module registers */
typedef struct can_ctrl {
	u32 mcr;		/* 0x00 Module Configuration register */
	u32 ctrl;		/* 0x04 Control register */
	u32 timer;		/* 0x08 Free Running Timer */
	u32 res1;		/* 0x0C */
	u32 rxgmask;		/* 0x10 Rx Global Mask */
	u32 rx14mask;		/* 0x14 RxBuffer 14 Mask */
	u32 rx15mask;		/* 0x18 RxBuffer 15 Mask */
	u32 errcnt;		/* 0x1C Error Counter Register */
	u32 errstat;		/* 0x20 Error and status Register */
	u32 res2;		/* 0x24 */
	u32 imask;		/* 0x28 Interrupt Mask Register */
	u32 res3;		/* 0x2C */
	u32 iflag;		/* 0x30 Interrupt Flag Register */
	u32 res4[19];		/* 0x34 - 0x7F */
	u32 MB0_15[2048];	/* 0x80 Message Buffer 0-15 */
} can_t;

#endif				/* __IMMAP_5235__ */
