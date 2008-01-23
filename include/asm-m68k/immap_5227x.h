/*
 * MCF5227x Internal Memory Map
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

#ifndef __IMMAP_5227X__
#define __IMMAP_5227X__

/* Module Base Addresses */
#define MMAP_SCM1	(CFG_MBAR + 0x00000000)
#define MMAP_XBS	(CFG_MBAR + 0x00004000)
#define MMAP_FBCS	(CFG_MBAR + 0x00008000)
#define MMAP_CAN	(CFG_MBAR + 0x00020000)
#define MMAP_RTC	(CFG_MBAR + 0x0003C000)
#define MMAP_SCM2	(CFG_MBAR + 0x00040010)
#define MMAP_SCM3	(CFG_MBAR + 0x00040070)
#define MMAP_EDMA	(CFG_MBAR + 0x00044000)
#define MMAP_INTC0	(CFG_MBAR + 0x00048000)
#define MMAP_INTC1	(CFG_MBAR + 0x0004C000)
#define MMAP_IACK	(CFG_MBAR + 0x00054000)
#define MMAP_I2C	(CFG_MBAR + 0x00058000)
#define MMAP_DSPI	(CFG_MBAR + 0x0005C000)
#define MMAP_UART0	(CFG_MBAR + 0x00060000)
#define MMAP_UART1	(CFG_MBAR + 0x00064000)
#define MMAP_UART2	(CFG_MBAR + 0x00068000)
#define MMAP_DTMR0	(CFG_MBAR + 0x00070000)
#define MMAP_DTMR1	(CFG_MBAR + 0x00074000)
#define MMAP_DTMR2	(CFG_MBAR + 0x00078000)
#define MMAP_DTMR3	(CFG_MBAR + 0x0007C000)
#define MMAP_PIT0	(CFG_MBAR + 0x00080000)
#define MMAP_PIT1	(CFG_MBAR + 0x00084000)
#define MMAP_PWM	(CFG_MBAR + 0x00090000)
#define MMAP_EPORT	(CFG_MBAR + 0x00094000)
#define MMAP_RCM	(CFG_MBAR + 0x000A0000)
#define MMAP_CCM	(CFG_MBAR + 0x000A0004)
#define MMAP_GPIO	(CFG_MBAR + 0x000A4000)
#define MMAP_ADC	(CFG_MBAR + 0x000A8000)
#define MMAP_LCD	(CFG_MBAR + 0x000AC000)
#define MMAP_LCD_BGLUT	(CFG_MBAR + 0x000AC800)
#define MMAP_LCD_GWLUT	(CFG_MBAR + 0x000ACC00)
#define MMAP_USBHW	(CFG_MBAR + 0x000B0000)
#define MMAP_USBCAPS	(CFG_MBAR + 0x000B0100)
#define MMAP_USBEHCI	(CFG_MBAR + 0x000B0140)
#define MMAP_USBOTG	(CFG_MBAR + 0x000B01A0)
#define MMAP_SDRAM	(CFG_MBAR + 0x000B8000)
#define MMAP_SSI	(CFG_MBAR + 0x000BC000)
#define MMAP_PLL	(CFG_MBAR + 0x000C0000)

#include <asm/coldfire/crossbar.h>
#include <asm/coldfire/dspi.h>
#include <asm/coldfire/edma.h>
#include <asm/coldfire/flexbus.h>
#include <asm/coldfire/lcd.h>
#include <asm/coldfire/ssi.h>

/* Interrupt Controller (INTC) */
typedef struct int0_ctrl {
	u32 iprh0;		/* 0x00 Pending Register High */
	u32 iprl0;		/* 0x04 Pending Register Low */
	u32 imrh0;		/* 0x08 Mask Register High */
	u32 imrl0;		/* 0x0C Mask Register Low */
	u32 frch0;		/* 0x10 Force Register High */
	u32 frcl0;		/* 0x14 Force Register Low */
	u16 res1;		/* 0x18 - 0x19 */
	u16 icfg0;		/* 0x1A Configuration Register */
	u8 simr0;		/* 0x1C Set Interrupt Mask */
	u8 cimr0;		/* 0x1D Clear Interrupt Mask */
	u8 clmask0;		/* 0x1E Current Level Mask */
	u8 slmask;		/* 0x1F Saved Level Mask */
	u32 res2[8];		/* 0x20 - 0x3F */
	u8 icr0[64];		/* 0x40 - 0x7F Control registers */
	u32 res3[24];		/* 0x80 - 0xDF */
	u8 swiack0;		/* 0xE0 Software Interrupt ack */
	u8 res4[3];		/* 0xE1 - 0xE3 */
	u8 Lniack0_1;		/* 0xE4 Level n interrupt ack */
	u8 res5[3];		/* 0xE5 - 0xE7 */
	u8 Lniack0_2;		/* 0xE8 Level n interrupt ack */
	u8 res6[3];		/* 0xE9 - 0xEB */
	u8 Lniack0_3;		/* 0xEC Level n interrupt ack */
	u8 res7[3];		/* 0xED - 0xEF */
	u8 Lniack0_4;		/* 0xF0 Level n interrupt ack */
	u8 res8[3];		/* 0xF1 - 0xF3 */
	u8 Lniack0_5;		/* 0xF4 Level n interrupt ack */
	u8 res9[3];		/* 0xF5 - 0xF7 */
	u8 Lniack0_6;		/* 0xF8 Level n interrupt ack */
	u8 resa[3];		/* 0xF9 - 0xFB */
	u8 Lniack0_7;		/* 0xFC Level n interrupt ack */
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
	u16 res1;		/* 0x18 */
	u16 icfg1;		/* 0x1A Configuration Register */
	u8 simr1;		/* 0x1C Set Interrupt Mask */
	u8 cimr1;		/* 0x1D Clear Interrupt Mask */
	u16 res2;		/* 0x1E - 0x1F */
	u32 res3[8];		/* 0x20 - 0x3F */
	u8 icr1[64];		/* 0x40 - 0x7F */
	u32 res4[24];		/* 0x80 - 0xDF */
	u8 swiack1;		/* 0xE0 Software Interrupt ack */
	u8 res5[3];		/* 0xE1 - 0xE3 */
	u8 Lniack1_1;		/* 0xE4 Level n interrupt ack */
	u8 res6[3];		/* 0xE5 - 0xE7 */
	u8 Lniack1_2;		/* 0xE8 Level n interrupt ack */
	u8 res7[3];		/* 0xE9 - 0xEB */
	u8 Lniack1_3;		/* 0xEC Level n interrupt ack */
	u8 res8[3];		/* 0xED - 0xEF */
	u8 Lniack1_4;		/* 0xF0 Level n interrupt ack */
	u8 res9[3];		/* 0xF1 - 0xF3 */
	u8 Lniack1_5;		/* 0xF4 Level n interrupt ack */
	u8 resa[3];		/* 0xF5 - 0xF7 */
	u8 Lniack1_6;		/* 0xF8 Level n interrupt ack */
	u8 resb[3];		/* 0xF9 - 0xFB */
	u8 Lniack1_7;		/* 0xFC Level n interrupt ack */
	u8 resc[3];		/* 0xFD - 0xFF */
} int1_t;

/* Global Interrupt Acknowledge (IACK) */
typedef struct iack {
	u8 resv0[0xE0];
	u8 gswiack;
	u8 resv1[0x3];
	u8 gl1iack;
	u8 resv2[0x3];
	u8 gl2iack;
	u8 resv3[0x3];
	u8 gl3iack;
	u8 resv4[0x3];
	u8 gl4iack;
	u8 resv5[0x3];
	u8 gl5iack;
	u8 resv6[0x3];
	u8 gl6iack;
	u8 resv7[0x3];
	u8 gl7iack;
} iack_t;

/* Edge Port Module (EPORT) */
typedef struct eport {
	u16 eppar;
	u8 epddr;
	u8 epier;
	u8 epdr;
	u8 eppdr;
	u8 epfr;
} eport_t;

/* Reset Controller Module (RCM) */
typedef struct rcm {
	u8 rcr;
	u8 rsr;
} rcm_t;

/* Chip Configuration Module (CCM) */
typedef struct ccm {
	u16 ccr;		/* Chip Configuration (Rd-only) */
	u16 resv1;
	u16 rcon;		/* Reset Configuration (Rd-only) */
	u16 cir;		/* Chip Identification (Rd-only) */
	u32 resv2;
	u16 misccr;		/* Miscellaneous Control */
	u16 cdr;		/* Clock Divider */
	u16 uocsr;		/* USB On-the-Go Controller Status */
	u16 resv4;
	u16 sbfsr;		/* Serial Boot Status */
	u16 sbfcr;		/* Serial Boot Control */
} ccm_t;

/* General Purpose I/O Module (GPIO) */
typedef struct gpio {
	/* Port Output Data Registers */
	u8 podr_be;		/* 0x00 */
	u8 podr_cs;		/* 0x01 */
	u8 podr_fbctl;		/* 0x02 */
	u8 podr_i2c;		/* 0x03 */
	u8 rsvd1;		/* 0x04 */
	u8 podr_uart;		/* 0x05 */
	u8 podr_dspi;		/* 0x06 */
	u8 podr_timer;		/* 0x07 */
	u8 podr_lcdctl;		/* 0x08 */
	u8 podr_lcddatah;	/* 0x09 */
	u8 podr_lcddatam;	/* 0x0A */
	u8 podr_lcddatal;	/* 0x0B */

	/* Port Data Direction Registers */
	u8 pddr_be;		/* 0x0C */
	u8 pddr_cs;		/* 0x0D */
	u8 pddr_fbctl;		/* 0x0E */
	u8 pddr_i2c;		/* 0x0F */
	u8 rsvd2;		/* 0x10 */
	u8 pddr_uart;		/* 0x11 */
	u8 pddr_dspi;		/* 0x12 */
	u8 pddr_timer;		/* 0x13 */
	u8 pddr_lcdctl;		/* 0x14 */
	u8 pddr_lcddatah;	/* 0x15 */
	u8 pddr_lcddatam;	/* 0x16 */
	u8 pddr_lcddatal;	/* 0x17 */

	/* Port Pin Data/Set Data Registers */
	u8 ppdsdr_be;		/* 0x18 */
	u8 ppdsdr_cs;		/* 0x19 */
	u8 ppdsdr_fbctl;	/* 0x1A */
	u8 ppdsdr_i2c;		/* 0x1B */
	u8 rsvd3;		/* 0x1C */
	u8 ppdsdr_uart;		/* 0x1D */
	u8 ppdsdr_dspi;		/* 0x1E */
	u8 ppdsdr_timer;	/* 0x1F */
	u8 ppdsdr_lcdctl;	/* 0x20 */
	u8 ppdsdr_lcddatah;	/* 0x21 */
	u8 ppdsdr_lcddatam;	/* 0x22 */
	u8 ppdsdr_lcddatal;	/* 0x23 */

	/* Port Clear Output Data Registers */
	u8 pclrr_be;		/* 0x24 */
	u8 pclrr_cs;		/* 0x25 */
	u8 pclrr_fbctl;		/* 0x26 */
	u8 pclrr_i2c;		/* 0x27 */
	u8 rsvd4;		/* 0x28 */
	u8 pclrr_uart;		/* 0x29 */
	u8 pclrr_dspi;		/* 0x2A */
	u8 pclrr_timer;		/* 0x2B */
	u8 pclrr_lcdctl;	/* 0x2C */
	u8 pclrr_lcddatah;	/* 0x2D */
	u8 pclrr_lcddatam;	/* 0x2E */
	u8 pclrr_lcddatal;	/* 0x2F */

	/* Pin Assignment Registers */
	u8 par_be;		/* 0x30 */
	u8 par_cs;		/* 0x31 */
	u8 par_fbctl;		/* 0x32 */
	u8 par_i2c;		/* 0x33 */
	u16 par_uart;		/* 0x34 */
	u8 par_dspi;		/* 0x36 */
	u8 par_timer;		/* 0x37 */
	u8 par_lcdctl;		/* 0x38 */
	u8 par_irq;		/* 0x39 */
	u16 rsvd6;		/* 0x3A - 0x3B */
	u32 par_lcdh;		/* 0x3C */
	u32 par_lcdl;		/* 0x40 */

	/* Mode select control registers */
	u8 mscr_fb;		/* 0x44 */
	u8 mscr_sdram;		/* 0x45 */

	u16 rsvd7;		/* 0x46 - 0x47 */
	u8 dscr_dspi;		/* 0x48 */
	u8 dscr_timer;		/* 0x49 */
	u8 dscr_i2c;		/* 0x4A */
	u8 dscr_lcd;		/* 0x4B */
	u8 dscr_debug;		/* 0x4C */
	u8 dscr_clkrst;		/* 0x4D */
	u8 dscr_irq;		/* 0x4E */
	u8 dscr_uart;		/* 0x4F */
} gpio_t;

/* SDRAM Controller (SDRAMC) */
typedef struct sdramc {
	u32 sdmr;		/* Mode/Extended Mode */
	u32 sdcr;		/* Control */
	u32 sdcfg1;		/* Configuration 1 */
	u32 sdcfg2;		/* Chip Select */
	u8 resv0[0x100];
	u32 sdcs0;		/* Mode/Extended Mode */
	u32 sdcs1;		/* Mode/Extended Mode */
} sdramc_t;

/* Phase Locked Loop (PLL) */
typedef struct pll {
	u32 pcr;		/* PLL Control */
	u32 psr;		/* PLL Status */
} pll_t;

/* System Control Module register  */
typedef struct scm1 {
	u32 mpr;		/* 0x00 Master Privilege */
	u32 rsvd1[7];
	u32 pacra;		/* 0x20 */
	u32 pacrb;		/* 0x24 */
	u32 pacrc;		/* 0x28 */
	u32 pacrd;		/* 0x2C */
	u32 rsvd2[4];
	u32 pacre;		/* 0x40 */
	u32 pacrf;		/* 0x44 */
	u32 pacrg;		/* 0x48 */
	u32 rsvd3;
	u32 pacri;		/* 0x50 */
} scm1_t;

typedef struct scm2_ctrl {
	u8 res1[3];		/* 0x00 - 0x02 */
	u8 wcr;			/* 0x03 wakeup control */
	u16 res2;		/* 0x04 - 0x05 */
	u16 cwcr;		/* 0x06 Core Watchdog Control */
	u8 res3[3];		/* 0x08 - 0x0A */
	u8 cwsr;		/* 0x0B Core Watchdog Service */
	u8 res4[2];		/* 0x0C - 0x0D */
	u8 scmisr;		/* 0x0F Interrupt Status */
	u32 res5;		/* 0x20 */
	u32 bcr;		/* 0x24 Burst Configuration */
} scm2_t;

typedef struct scm3_ctrl {
	u32 cfadr;		/* 0x00 Core Fault Address */
	u8 res7;		/* 0x04 */
	u8 cfier;		/* 0x05 Core Fault Interrupt Enable */
	u8 cfloc;		/* 0x06 Core Fault Location */
	u8 cfatr;		/* 0x07 Core Fault Attributes */
	u32 cfdtr;		/* 0x08 Core Fault Data */
} scm3_t;

typedef struct rtcex {
	u32 rsvd1[3];
	u32 gocu;
	u32 gocl;
} rtcex_t;
#endif				/* __IMMAP_5227X__ */
