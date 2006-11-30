/*
 * (C) Copyright 2004-2006 Freescale Semiconductor, Inc.
 *
 * MPC83xx Internal Memory Map
 *
 * History :
 * 20060601: Daveliu (daveliu@freescale.com)
 *	     TanyaJiang (tanya.jiang@freescale.com)
 *	     Unified variable names for mpc83xx
 * 2005	   : Mandy Lavi (mandy.lavi@freescale.com)
 *	     support for mpc8360e
 * 2004	   : Eran Liberty (liberty@freescale.com)
 *	     Initialized for mpc8349
 *	     based on:
 *	     MPC8260 Internal Memory Map
 *	     Copyright (c) 1999 Dan Malek (dmalek@jlc.net)
 *	     MPC85xx Internal Memory Map
 *	     Copyright(c) 2002,2003 Motorola Inc.
 *	     Xianghua Xiao (x.xiao@motorola.com)
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
 *
 */
#ifndef __IMMAP_83xx__
#define __IMMAP_83xx__

#include <config.h>
#include <asm/types.h>
#include <asm/fsl_i2c.h>

/*
 * Local Access Window.
 */
typedef struct law83xx {
	u32 bar;		/* LBIU local access window base address register */
/* Identifies the 20 most-significant address bits of the base of local
 * access window n. The specified base address should be aligned to the
 * window size, as defined by LBLAWARn[SIZE].
 */
#define LAWBAR_BAR	   0xFFFFF000
#define LAWBAR_RES	     ~(LAWBAR_BAR)
	u32 ar;			/* LBIU local access window attribute register */
} law83xx_t;

/*
 * System configuration registers.
 */
typedef struct sysconf83xx {
	u32 immrbar;		/* Internal memory map base address register */
	u8 res0[0x04];
	u32 altcbar;		/* Alternate configuration base address register */
/* Identifies the12 most significant address bits of an alternate base
 * address used for boot sequencer configuration accesses.
 */
#define ALTCBAR_BASE_ADDR     0xFFF00000
#define ALTCBAR_RES	      ~(ALTCBAR_BASE_ADDR)	/* Reserved. Write has no effect, read returns 0. */
	u8 res1[0x14];
	law83xx_t lblaw[4];	/* LBIU local access window */
	u8 res2[0x20];
	law83xx_t pcilaw[2];	/* PCI local access window */
	u8 res3[0x30];
	law83xx_t ddrlaw[2];	/* DDR local access window */
	u8 res4[0x50];
	u32 sgprl;		/* System General Purpose Register Low */
	u32 sgprh;		/* System General Purpose Register High */
	u32 spridr;		/* System Part and Revision ID Register */
#define SPRIDR_PARTID	      0xFFFF0000	/* Part Identification. */
#define SPRIDR_REVID	      0x0000FFFF	/* Revision Identification. */
	u8 res5[0x04];
	u32 spcr;		/* System Priority Configuration Register */
#define SPCR_PCIHPE   0x10000000	/* PCI Highest Priority Enable. */
#define SPCR_PCIHPE_SHIFT	(31-3)
#define SPCR_PCIPR    0x03000000	/* PCI bridge system bus request priority. */
#define SPCR_PCIPR_SHIFT	(31-7)
#define SPCR_OPT      0x00800000	/* Optimize */
#define SPCR_TBEN     0x00400000	/* E300 PowerPC core time base unit enable. */
#define SPCR_TBEN_SHIFT		(31-9)
#define SPCR_COREPR   0x00300000	/* E300 PowerPC Core system bus request priority. */
#define SPCR_COREPR_SHIFT	(31-11)
#if defined (CONFIG_MPC8349)
#define SPCR_TSEC1DP  0x00003000	/* TSEC1 data priority. */
#define SPCR_TSEC1DP_SHIFT	(31-19)
#define SPCR_TSEC1BDP 0x00000C00	/* TSEC1 buffer descriptor priority. */
#define SPCR_TSEC1BDP_SHIFT	(31-21)
#define SPCR_TSEC1EP  0x00000300	/* TSEC1 emergency priority. */
#define SPCR_TSEC1EP_SHIFT	(31-23)
#define SPCR_TSEC2DP  0x00000030	/* TSEC2 data priority. */
#define SPCR_TSEC2DP_SHIFT	(31-27)
#define SPCR_TSEC2BDP 0x0000000C	/* TSEC2 buffer descriptor priority. */
#define SPCR_TSEC2BDP_SHIFT	(31-29)
#define SPCR_TSEC2EP  0x00000003	/* TSEC2 emergency priority. */
#define SPCR_TSEC2EP_SHIFT	(31-31)
#define SPCR_RES      ~(SPCR_PCIHPE | SPCR_PCIPR | SPCR_TBEN | SPCR_COREPR \
			| SPCR_TSEC1DP | SPCR_TSEC1BDP | SPCR_TSEC1EP \
			| SPCR_TSEC2DP | SPCR_TSEC2BDP | SPCR_TSEC2EP)
#elif defined (CONFIG_MPC8360)
#define SPCR_RES      ~(SPCR_PCIHPE|SPCR_PCIPR|SPCR_OPT|SPCR_TBEN|SPCR_COREPR)
#endif
	u32 sicrl;		/* System General Purpose Register Low */
#if defined (CONFIG_MPC8349)
#define SICRL_LDP_A   0x80000000
#define SICRL_USB1    0x40000000
#define SICRL_USB0    0x20000000
#define SICRL_UART    0x0C000000
#define SICRL_GPIO1_A 0x02000000
#define SICRL_GPIO1_B 0x01000000
#define SICRL_GPIO1_C 0x00800000
#define SICRL_GPIO1_D 0x00400000
#define SICRL_GPIO1_E 0x00200000
#define SICRL_GPIO1_F 0x00180000
#define SICRL_GPIO1_G 0x00040000
#define SICRL_GPIO1_H 0x00020000
#define SICRL_GPIO1_I 0x00010000
#define SICRL_GPIO1_J 0x00008000
#define SICRL_GPIO1_K 0x00004000
#define SICRL_GPIO1_L 0x00003000
#define SICRL_RES ~(SICRL_LDP_A | SICRL_USB0 | SICRL_USB1 | SICRL_UART \
			| SICRL_GPIO1_A | SICRL_GPIO1_B | SICRL_GPIO1_C \
			| SICRL_GPIO1_D | SICRL_GPIO1_E | SICRL_GPIO1_F \
			| SICRL_GPIO1_G | SICRL_GPIO1_H | SICRL_GPIO1_I \
			| SICRL_GPIO1_J | SICRL_GPIO1_K | SICRL_GPIO1_L )
#elif defined (CONFIG_MPC8360)
#define SICRL_LDP_A   0xC0000000
#define SICRL_LCLK_1  0x10000000
#define SICRL_LCLK_2  0x08000000
#define SICRL_SRCID_A 0x03000000
#define SICRL_IRQ_CKSTP_A 0x00C00000
#define SICRL_RES     ~(SICRL_LDP_A | SICRL_LCLK_1 | SICRL_LCLK_2 | \
			SICRL_SRCID_A | SICRL_IRQ_CKSTP_A)
#endif
	u32 sicrh;		/* System General Purpose Register High */
#define SICRH_DDR     0x80000000
#if defined (CONFIG_MPC8349)
#define SICRH_TSEC1_A 0x10000000
#define SICRH_TSEC1_B 0x08000000
#define SICRH_TSEC1_C 0x04000000
#define SICRH_TSEC1_D 0x02000000
#define SICRH_TSEC1_E 0x01000000
#define SICRH_TSEC1_F 0x00800000
#define SICRH_TSEC2_A 0x00400000
#define SICRH_TSEC2_B 0x00200000
#define SICRH_TSEC2_C 0x00100000
#define SICRH_TSEC2_D 0x00080000
#define SICRH_TSEC2_E 0x00040000
#define SICRH_TSEC2_F 0x00020000
#define SICRH_TSEC2_G 0x00010000
#define SICRH_TSEC2_H 0x00008000
#define SICRH_GPIO2_A 0x00004000
#define SICRH_GPIO2_B 0x00002000
#define SICRH_GPIO2_C 0x00001000
#define SICRH_GPIO2_D 0x00000800
#define SICRH_GPIO2_E 0x00000400
#define SICRH_GPIO2_F 0x00000200
#define SICRH_GPIO2_G 0x00000180
#define SICRH_GPIO2_H 0x00000060
#define SICRH_TSOBI1  0x00000002
#define SICRH_TSOBI2  0x00000001
#define SICRH_RES     ~(  SICRH_DDR | SICRH_TSEC1_A | SICRH_TSEC1_B \
			| SICRH_TSEC1_C | SICRH_TSEC1_D | SICRH_TSEC1_E \
			| SICRH_TSEC1_F | SICRH_TSEC2_A | SICRH_TSEC2_B \
			| SICRH_TSEC2_C | SICRH_TSEC2_D | SICRH_TSEC2_E \
			| SICRH_TSEC2_F | SICRH_TSEC2_G | SICRH_TSEC2_H \
			| SICRH_GPIO2_A | SICRH_GPIO2_B | SICRH_GPIO2_C \
			| SICRH_GPIO2_D | SICRH_GPIO2_E | SICRH_GPIO2_F \
			| SICRH_GPIO2_G | SICRH_GPIO2_H | SICRH_TSOBI1 \
			| SICRH_TSOBI2)
#elif defined (CONFIG_MPC8360)
#define SICRH_SECONDARY_DDR 0x40000000
#define SICRH_SDDROE   0x02000000	/* SDDRIOE bit from reset configuration word high. */
#define SICRH_UC1EOBI  0x00000004	/* UCC1 Ethernet Output Buffer Impedance. */
#define SICRH_UC2E1OBI 0x00000002	/* UCC2 Ethernet pin option 1 Output Buffer Impedance. */
#define SICRH_UC2E2OBI 0x00000001	/* UCC2 Ethernet pin option 2 Output Buffer Impedance. */
#define SICRH_RES     ~(SICRH_DDR | SICRH_SECONDARY_DDR | SICRH_SDDROE | \
			SICRH_UC2E1OBI | SICRH_UC2E2OBI | SICRH_UC2E2OBI)
#endif
	u8 res6[0xE4];
} sysconf83xx_t;

/*
 * Watch Dog Timer (WDT) Registers
 */
typedef struct wdt83xx {
	u8 res0[4];
	u32 swcrr;		/* System watchdog control register */
	u32 swcnr;		/* System watchdog count register */
#define SWCNR_SWCN 0x0000FFFF Software Watchdog Count Field.
#define SWCNR_RES  ~(SWCNR_SWCN)
	u8 res1[2];
	u16 swsrr;		/* System watchdog service register */
#define SWSRR_WS 0x0000FFFF	/* Software Watchdog Service Field. */
	u8 res2[0xF0];
} wdt83xx_t;

/*
 * RTC/PIT Module Registers
 */
typedef struct rtclk83xx {
	u32 cnr;		/* control register */
#define CNR_CLEN 0x00000080	/* Clock Enable Control Bit  */
#define CNR_CLIN 0x00000040	/* Input Clock Control Bit  */
#define CNR_AIM	 0x00000002	/* Alarm Interrupt Mask Bit  */
#define CNR_SIM	 0x00000001	/* Second Interrupt Mask Bit  */
#define CNR_RES	 ~(CNR_CLEN | CNR_CLIN | CNR_AIM | CNR_SIM)
	u32 ldr;		/* load register */
#define LDR_CLDV 0xFFFFFFFF	/* Contains the 32-bit value to be
				 * loaded in a 32-bit RTC counter.*/
	u32 psr;		/* prescale register */
#define PSR_PRSC 0xFFFFFFFF	/*  RTC Prescaler bits. */
	u32 ctr;		/* Counter value field register */
#define CRT_CNTV 0xFFFFFFFF	/* RTC Counter value field. */
	u32 evr;		/* event register */
#define RTEVR_SIF  0x00000001	/* Second Interrupt Flag Bit  */
#define RTEVR_AIF  0x00000002	/* Alarm Interrupt Flag Bit  */
#define RTEVR_RES ~(RTEVR_SIF | RTEVR_AIF)
#define PTEVR_PIF  0x00000001	/* Periodic interrupt flag bit. */
#define PTEVR_RES ~(PTEVR_PIF)
	u32 alr;		/* alarm register */
	u8 res0[0xE8];
} rtclk83xx_t;

/*
 * Global timper module
 */

typedef struct gtm83xx {
	u8 cfr1;		/* Timer1/2 Configuration  */
#define CFR1_PCAS 0x80		/* Pair Cascade mode  */
#define CFR1_BCM  0x40		/* Backward compatible mode  */
#define CFR1_STP2 0x20		/* Stop timer  */
#define CFR1_RST2 0x10		/* Reset timer	*/
#define CFR1_GM2  0x08		/* Gate mode for pin 2	*/
#define CFR1_GM1  0x04		/* Gate mode for pin 1	*/
#define CFR1_STP1 0x02		/* Stop timer  */
#define CFR1_RST1 0x01		/* Reset timer	*/
#define CFR1_RES ~(CFR1_PCAS | CFR1_STP2 | CFR1_RST2 | CFR1_GM2 |\
		 CFR1_GM1 | CFR1_STP1 | CFR1_RST1)
	u8 res0[3];
	u8 cfr2;		/* Timer3/4 Configuration  */
#define CFR2_PCAS 0x80		/* Pair Cascade mode  */
#define CFR2_SCAS 0x40		/* Super Cascade mode  */
#define CFR2_STP4 0x20		/* Stop timer  */
#define CFR2_RST4 0x10		/* Reset timer	*/
#define CFR2_GM4  0x08		/* Gate mode for pin 4	*/
#define CFR2_GM3  0x04		/* Gate mode for pin 3	*/
#define CFR2_STP3 0x02		/* Stop timer  */
#define CFR2_RST3 0x01		/* Reset timer	*/
	u8 res1[10];
	u16 mdr1;		/* Timer1 Mode Register	 */
#define MDR_SPS	 0xff00		/* Secondary Prescaler value  */
#define MDR_CE	 0x00c0		/* Capture edge and enable interrupt  */
#define MDR_OM	 0x0020		/* Output mode	*/
#define MDR_ORI	 0x0010		/* Output reference interrupt enable  */
#define MDR_FRR	 0x0008		/* Free run/restart  */
#define MDR_ICLK 0x0006		/* Input clock source for the timer  */
#define MDR_GE	 0x0001		/* Gate enable	*/
	u16 mdr2;		/* Timer2 Mode Register	 */
	u16 rfr1;		/* Timer1 Reference Register  */
	u16 rfr2;		/* Timer2 Reference Register  */
	u16 cpr1;		/* Timer1 Capture Register  */
	u16 cpr2;		/* Timer2 Capture Register  */
	u16 cnr1;		/* Timer1 Counter Register  */
	u16 cnr2;		/* Timer2 Counter Register  */
	u16 mdr3;		/* Timer3 Mode Register	 */
	u16 mdr4;		/* Timer4 Mode Register	 */
	u16 rfr3;		/* Timer3 Reference Register  */
	u16 rfr4;		/* Timer4 Reference Register  */
	u16 cpr3;		/* Timer3 Capture Register  */
	u16 cpr4;		/* Timer4 Capture Register  */
	u16 cnr3;		/* Timer3 Counter Register  */
	u16 cnr4;		/* Timer4 Counter Register  */
	u16 evr1;		/* Timer1 Event Register  */
	u16 evr2;		/* Timer2 Event Register  */
	u16 evr3;		/* Timer3 Event Register  */
	u16 evr4;		/* Timer4 Event Register  */
#define GTEVR_REF 0x0002	/* Output reference event  */
#define GTEVR_CAP 0x0001	/* Counter Capture event   */
#define GTEVR_RES ~(EVR_CAP|EVR_REF)
	u16 psr1;		/* Timer1 Prescaler Register  */
	u16 psr2;		/* Timer2 Prescaler Register  */
	u16 psr3;		/* Timer3 Prescaler Register  */
	u16 psr4;		/* Timer4 Prescaler Register  */
#define GTPSR_PPS  0x00FF	/* Primary Prescaler Bits. */
#define GTPSR_RES  ~(GTPSR_PPS)
	u8 res[0xC0];
} gtm83xx_t;

/*
 * Integrated Programmable Interrupt Controller
 */
typedef struct ipic83xx {
	u32 sicfr;		/*  System Global Interrupt Configuration Register (SICFR)  */
#define SICFR_HPI  0x7f000000	/*  Highest Priority Interrupt	*/
#define SICFR_MPSB 0x00400000	/*  Mixed interrupts Priority Scheme for group B  */
#define SICFR_MPSA 0x00200000	/*  Mixed interrupts Priority Scheme for group A  */
#define SICFR_IPSD 0x00080000	/*  Internal interrupts Priority Scheme for group D  */
#define SICFR_IPSA 0x00010000	/*  Internal interrupts Priority Scheme for group A  */
#define SICFR_HPIT 0x00000300	/*  HPI priority position IPIC output interrupt Type  */
#define SICFR_RES ~(SICFR_HPI|SICFR_MPSB|SICFR_MPSA|SICFR_IPSD|SICFR_IPSA|SICFR_HPIT)
	u32 sivcr;		/*  System Global Interrupt Vector Register (SIVCR)  */
#define SICVR_IVECX 0xfc000000	/*  Interrupt vector (for CE compatibility purpose only not used in 8349 IPIC implementation)  */
#define SICVR_IVEC  0x0000007f	/*  Interrupt vector  */
#define SICVR_RES ~(SICVR_IVECX|SICVR_IVEC)
	u32 sipnr_h;		/*  System Internal Interrupt Pending Register - High (SIPNR_H)	 */
#if defined (CONFIG_MPC8349)
#define SIIH_TSEC1TX 0x80000000 /*  TSEC1 Tx interrupt	*/
#define SIIH_TSEC1RX 0x40000000 /*  TSEC1 Rx interrupt	*/
#define SIIH_TSEC1ER 0x20000000 /*  TSEC1 Eror interrupt  */
#define SIIH_TSEC2TX 0x10000000 /*  TSEC2 Tx interrupt	*/
#define SIIH_TSEC2RX 0x08000000 /*  TSEC2 Rx interrupt	*/
#define SIIH_TSEC2ER 0x04000000 /*  TSEC2 Eror interrupt  */
#define SIIH_USB2DR  0x02000000 /*  USB2 DR interrupt  */
#define SIIH_USB2MPH 0x01000000 /*  USB2 MPH interrupt	*/
#endif
#if defined (CONFIG_MPC8360)
#define SIIH_H_QE_H   0x80000000	/*  QE high interrupt */
#define SIIH_H_QE_L   0x40000000	/*  QE low interrupt */
#endif
#define SIIH_UART1   0x00000080 /*  UART1 interrupt  */
#define SIIH_UART2   0x00000040 /*  UART2 interrupt  */
#define SIIH_SEC     0x00000020 /*  SEC interrupt  */
#define SIIH_I2C1    0x00000004 /*  I2C1 interrupt  */
#define SIIH_I2C2    0x00000002 /*  I2C2 interrupt  */
#if defined (CONFIG_MPC8349)
#define SIIH_SPI     0x00000001 /*  SPI interrupt  */
#define SIIH_RES	~(SIIH_TSEC1TX | SIIH_TSEC1RX | SIIH_TSEC1ER \
			| SIIH_TSEC2TX | SIIH_TSEC2RX | SIIH_TSEC2ER \
			| SIIH_USB2DR | SIIH_USB2MPH | SIIH_UART1 \
			| SIIH_UART2 | SIIH_SEC | SIIH_I2C1 \
			| SIIH_I2C2 | SIIH_SPI)
#endif
#if defined (CONFIG_MPC8360)
#define SIIH_RES       ~(SIIH_H_QE_H | SIIH_H_QE_L | SIIH_H_UART1 | \
			SIIH_H_UART2| SIIH_H_SEC  | SIIH_H_I2C1 |SIIH_H_I2C2)
#endif
	u32 sipnr_l;		/*  System Internal Interrupt Pending Register - Low (SIPNR_L)	*/
#define SIIL_RTCS  0x80000000	/*  RTC SECOND interrupt  */
#define SIIL_PIT   0x40000000	/*  PIT interrupt  */
#define SIIL_PCI1  0x20000000	/*  PCI1 interrupt  */
#if defined (CONFIG_MPC8349)
#define SIIL_PCI2  0x10000000	/*  PCI2 interrupt  */
#endif
#define SIIL_RTCA  0x08000000	/*  RTC ALARM interrupt	 */
#define SIIL_MU	   0x04000000	/*  Message Unit interrupt  */
#define SIIL_SBA   0x02000000	/*  System Bus Arbiter interrupt  */
#define SIIL_DMA   0x01000000	/*  DMA interrupt  */
#define SIIL_GTM4  0x00800000	/*  GTM4 interrupt  */
#define SIIL_GTM8  0x00400000	/*  GTM8 interrupt  */
#if defined (CONFIG_MPC8349)
#define SIIL_GPIO1 0x00200000	/*  GPIO1 interrupt  */
#define SIIL_GPIO2 0x00100000	/*  GPIO2 interrupt  */
#endif
#if defined (CONFIG_MPC8360)
#define SIIL_QEP   0x00200000	/*  QE ports interrupt	*/
#define SIIL_SDDR  0x00100000	/*  SDDR interrupt  */
#endif
#define SIIL_DDR   0x00080000	/*  DDR interrupt  */
#define SIIL_LBC   0x00040000	/*  LBC interrupt  */
#define SIIL_GTM2  0x00020000	/*  GTM2 interrupt  */
#define SIIL_GTM6  0x00010000	/*  GTM6 interrupt  */
#define SIIL_PMC   0x00008000	/*  PMC interrupt  */
#define SIIL_GTM3  0x00000800	/*  GTM3 interrupt  */
#define SIIL_GTM7  0x00000400	/*  GTM7 interrupt  */
#define SIIL_GTM1  0x00000020	/*  GTM1 interrupt  */
#define SIIL_GTM5  0x00000010	/*  GTM5 interrupt  */
#define SIIL_DPTC  0x00000001	/*  DPTC interrupt (!!! Invisible for user !!!)	 */
#if defined (CONFIG_MPC8349)
#define SIIL_RES	~(SIIL_RTCS | SIIL_PIT | SIIL_PCI1 | SIIL_PCI2 \
			| SIIL_RTCA | SIIL_MU | SIIL_SBA | SIIL_DMA \
			| SIIL_GTM4 | SIIL_GTM8 | SIIL_GPIO1 | SIIL_GPIO2 \
			| SIIL_DDR | SIIL_LBC | SIIL_GTM2 | SIIL_GTM6 \
			| SIIL_PMC |SIIL_GTM3 | SIIL_GTM7 | SIIL_GTM1 \
			| SIIL_GTM5 |SIIL_DPTC )
#endif
#if defined (CONFIG_MPC8360)
#define SIIL_RES	~(SIIL_RTCS  |SIIL_PIT	|SIIL_PCI1 |SIIL_RTCALR \
			|SIIL_MU |SIIL_SBA  |SIIL_DMA  |SIIL_GTM4 |SIIL_GTM8 \
			|SIIL_QEP | SIIL_SDDR| SIIL_DDR	 |SIIL_LBC  |SIIL_GTM2 \
			|SIIL_GTM6 |SIIL_PMC  |SIIL_GTM3 |SIIL_GTM7 |SIIL_GTM1 \
			|SIIL_GTM5 )
#endif
	u32 siprr_a;		/*  System Internal Interrupt Group A Priority Register (PRR)  */
	u8 res0[8];
	u32 siprr_d;		/*  System Internal Interrupt Group D Priority Register (PRR)  */
	u32 simsr_h;		/*  System Internal Interrupt Mask Register - High (SIIH)  */
	u32 simsr_l;		/*  System Internal Interrupt Mask Register - Low (SIIL)  */
	u8 res1[4];
	u32 sepnr;		/*  System External Interrupt Pending Register (SEI)  */
	u32 smprr_a;		/*  System Mixed Interrupt Group A Priority Register (PRR)  */
	u32 smprr_b;		/*  System Mixed Interrupt Group B Priority Register (PRR)  */
#define PRR_0 0xe0000000	/* Priority Register, Position 0 programming */
#define PRR_1 0x1c000000	/* Priority Register, Position 1 programming */
#define PRR_2 0x03800000	/* Priority Register, Position 2 programming */
#define PRR_3 0x00700000	/* Priority Register, Position 3 programming */
#define PRR_4 0x0000e000	/* Priority Register, Position 4 programming */
#define PRR_5 0x00001c00	/* Priority Register, Position 5 programming */
#define PRR_6 0x00000380	/* Priority Register, Position 6 programming */
#define PRR_7 0x00000070	/* Priority Register, Position 7 programming */
#define PRR_RES ~(PRR_0|PRR_1|PRR_2|PRR_3|PRR_4|PRR_5|PRR_6|PRR_7)
	u32 semsr;		/*  System External Interrupt Mask Register (SEI)  */
#define SEI_IRQ0  0x80000000	/*  IRQ0 external interrupt  */
#define SEI_IRQ1  0x40000000	/*  IRQ1 external interrupt  */
#define SEI_IRQ2  0x20000000	/*  IRQ2 external interrupt  */
#define SEI_IRQ3  0x10000000	/*  IRQ3 external interrupt  */
#define SEI_IRQ4  0x08000000	/*  IRQ4 external interrupt  */
#define SEI_IRQ5  0x04000000	/*  IRQ5 external interrupt  */
#define SEI_IRQ6  0x02000000	/*  IRQ6 external interrupt  */
#define SEI_IRQ7  0x01000000	/*  IRQ7 external interrupt  */
#define SEI_SIRQ0 0x00008000	/*  SIRQ0 external interrupt  */
#define SEI_RES		~( SEI_IRQ0 | SEI_IRQ1 | SEI_IRQ2 | SEI_IRQ3 \
			| SEI_IRQ4 | SEI_IRQ5 | SEI_IRQ6 | SEI_IRQ7 \
			| SEI_SIRQ0)
	u32 secnr;		/*  System External Interrupt Control Register (SECNR) */
#define SECNR_MIXB0T 0xc0000000 /*  MIXB0 priority position IPIC output interrupt type	*/
#define SECNR_MIXB1T 0x30000000 /*  MIXB1 priority position IPIC output interrupt type	*/
#define SECNR_MIXA0T 0x00c00000 /*  MIXA0 priority position IPIC output interrupt type	*/
#define SECNR_SYSA1T 0x00300000 /*  MIXA1 priority position IPIC output interrupt type	*/
#define SECNR_EDI0   0x00008000 /*  IRQ0 external interrupt edge/level detect  */
#define SECNR_EDI1   0x00004000 /*  IRQ1 external interrupt edge/level detect  */
#define SECNR_EDI2   0x00002000 /*  IRQ2 external interrupt edge/level detect  */
#define SECNR_EDI3   0x00001000 /*  IRQ3 external interrupt edge/level detect  */
#define SECNR_EDI4   0x00000800 /*  IRQ4 external interrupt edge/level detect  */
#define SECNR_EDI5   0x00000400 /*  IRQ5 external interrupt edge/level detect  */
#define SECNR_EDI6   0x00000200 /*  IRQ6 external interrupt edge/level detect  */
#define SECNR_EDI7   0x00000100 /*  IRQ7 external interrupt edge/level detect  */
#define SECNR_RES	~( SECNR_MIXB0T | SECNR_MIXB1T | SECNR_MIXA0T \
			| SECNR_SYSA1T | SECNR_EDI0 | SECNR_EDI1 \
			| SECNR_EDI2 | SECNR_EDI3 | SECNR_EDI4 \
			| SECNR_EDI5 | SECNR_EDI6 | SECNR_EDI7)
	u32 sersr;		/*  System Error Status Register (SERR)	 */
	u32 sermr;		/*  System Error Mask Register (SERR)  */
#define SERR_IRQ0 0x80000000	/*  IRQ0 MCP request  */
#define SERR_WDT  0x40000000	/*  WDT MCP request  */
#define SERR_SBA  0x20000000	/*  SBA MCP request  */
#if defined (CONFIG_MPC8349)
#define SERR_DDR  0x10000000	/*  DDR MCP request  */
#define SERR_LBC  0x08000000	/*  LBC MCP request  */
#define SERR_PCI1 0x04000000	/*  PCI1 MCP request  */
#define SERR_PCI2 0x02000000	/*  PCI2 MCP request  */
#endif
#if defined (CONFIG_MPC8360)
#define SERR_CIEE 0x10000000	/*  CIEE MCP request  */
#define SERR_CMEE 0x08000000	/*  CMEEMCP request  */
#define SERR_PCI  0x04000000	/*  PCI MCP request  */
#endif
#define SERR_MU	  0x01000000	/*  MU MCP request  */
#define SERR_RNC  0x00010000	/*  MU MCP request (!!! Non-visible for users !!!)  */
#if defined (CONFIG_MPC8349)
#define SERR_RES	~( SERR_IRQ0 | SERR_WDT | SERR_SBA | SERR_DDR \
			|SERR_LBC | SERR_PCI1 | SERR_PCI2 | SERR_MU \
			|SERR_RNC )
#elif defined (CONFIG_MPC8360)
#define SERR_RES	~( SERR_IRQ0|SERR_WDT |SERR_SBA |SERR_CIEE\
			|SERR_CMEE|SERR_PCI|SERR_MU)
#endif
	u32 sercr;		/*  System Error Control Register  (SERCR)  */
#define SERCR_MCPR 0x00000001	/*  MCP Route  */
#define SERCR_RES ~(SERCR_MCPR)
	u8 res2[4];
	u32 sifcr_h;		/*  System Internal Interrupt Force Register - High (SIIH)  */
	u32 sifcr_l;		/*  System Internal Interrupt Force Register - Low (SIIL)  */
	u32 sefcr;		/*  System External Interrupt Force Register (SEI)  */
	u32 serfr;		/*  System Error Force Register (SERR)	*/
	u32 scvcr;		/* System Critical Interrupt Vector Register */
#define SCVCR_CVECX	0xFC000000	/* Backward (MPC8260) compatible
					   critical interrupt vector. */
#define SCVCR_CVEC	0x0000007F	/* Critical interrupt vector */
#define SCVCR_RES	~(SCVCR_CVECX|SCVCR_CVEC)
	u32 smvcr;		/* System Management Interrupt Vector Register */
#define SMVCR_CVECX	0xFC000000	/* Backward (MPC8260) compatible
					   critical interrupt vector. */
#define SMVCR_CVEC	0x0000007F	/* Critical interrupt vector */
#define SMVCR_RES	~(SMVCR_CVECX|SMVCR_CVEC)
	u8 res3[0x98];
} ipic83xx_t;

/*
 * System Arbiter Registers
 */
typedef struct arbiter83xx {
	u32 acr;		/* Arbiter Configuration Register */
#define ACR_COREDIS    0x10000000	/* Core disable. */
#define ACR_COREDIS_SHIFT		(31-7)
#define ACR_PIPE_DEP   0x00070000	/* Pipeline depth (number of outstanding transactions). */
#define ACR_PIPE_DEP_SHIFT		(31-15)
#define ACR_PCI_RPTCNT 0x00007000	/* PCI repeat count. */
#define ACR_PCI_RPTCNT_SHIFT		(31-19)
#define ACR_RPTCNT     0x00000700	/* Repeat count. */
#define ACR_RPTCNT_SHIFT		(31-23)
#define ACR_APARK      0x00000030	/* Address parking. */
#define ACR_APARK_SHIFT			(31-27)
#define ACR_PARKM	   0x0000000F	/* Parking master. */
#define ACR_PARKM_SHIFT			(31-31)
#define ACR_RES ~(ACR_COREDIS|ACR_PIPE_DEP|ACR_PCI_RPTCNT|ACR_RPTCNT|ACR_APARK|ACR_PARKM)
	u32 atr;		/* Arbiter Timers Register */
#define ATR_DTO 0x00FF0000	/* Data time out. */
#define ATR_ATO 0x000000FF	/* Address time out. */
#define ATR_RES ~(ATR_DTO|ATR_ATO)
	u8 res[4];
	u32 aer;		/* Arbiter Event Register (AE) */
	u32 aidr;		/* Arbiter Interrupt Definition Register (AE) */
	u32 amr;		/* Arbiter Mask Register (AE) */
	u32 aeatr;		/* Arbiter Event Attributes Register */
#define AEATR_EVENT   0x07000000	/* Event type. */
#define AEATR_MSTR_ID 0x001F0000	/* Master Id. */
#define AEATR_TBST    0x00000800	/* Transfer burst. */
#define AEATR_TSIZE   0x00000700	/* Transfer Size. */
#define AEATR_TTYPE	  0x0000001F	/* Transfer Type. */
#define AEATR_RES ~(AEATR_EVENT|AEATR_MSTR_ID|AEATR_TBST|AEATR_TSIZE|AEATR_TTYPE)
	u32 aeadr;		/* Arbiter Event Address Register */
	u32 aerr;		/* Arbiter Event Response Register (AE) */
#define AE_ETEA 0x00000020	/* Transfer error. */
#define AE_RES_ 0x00000010	/* Reserved transfer type. */
#define AE_ECW	0x00000008	/* External control word transfer type. */
#define AE_AO	0x00000004	/* Address Only transfer type. */
#define AE_DTO	0x00000002	/* Data time out. */
#define AE_ATO	0x00000001	/* Address time out. */
#define AE_RSRV ~(AE_ETEA|AE_RES_|AE_ECW|AE_AO|AE_DTO|AE_ATO)
	u8 res1[0xDC];
} arbiter83xx_t;

/*
 * Reset Module
 */
typedef struct reset83xx {
	u32 rcwl;		/* RCWL Register  */
#define RCWL_LBIUCM  0x80000000 /* LBIUCM  */
#define RCWL_LBIUCM_SHIFT    31
#define RCWL_DDRCM   0x40000000 /* DDRCM  */
#define RCWL_DDRCM_SHIFT     30
#if defined (CONFIG_MPC8349)
#define RCWL_SVCOD   0x30000000 /* SVCOD  */
#endif
#define RCWL_SPMF    0x0f000000 /* SPMF	 */
#define RCWL_SPMF_SHIFT	     24
#define RCWL_COREPLL 0x007F0000 /* COREPLL  */
#define RCWL_COREPLL_SHIFT   16
#define RCWL_CEVCOD  0x000000C0 /* CEVCOD  */
#define RCWL_CEPDF   0x00000020 /* CEPDF  */
#define RCWL_CEPDF_SHIFT      5
#define RCWL_CEPMF   0x0000001F /* CEPMF  */
#define RCWL_CEPMF_SHIFT      0
#if defined (CONFIG_MPC8349)
#define RCWL_RES ~(RCWL_LBIUCM|RCWL_DDRCM|RCWL_SVCOD|RCWL_SPMF|RCWL_COREPLL|RCWL_CEVCOD|RCWL_CEPDF|RCWL_CEPMF)
#elif defined (CONFIG_MPC8360)
#define RCWL_RES ~(RCWL_LBIUCM|RCWL_DDRCM|RCWL_SPMF|RCWL_COREPLL|RCWL_CEPDF|RCWL_CEPMF)
#endif
	u32 rcwh;		/* RCHL Register  */
#define RCWH_PCIHOST 0x80000000 /* PCIHOST  */
#define RCWH_PCIHOST_SHIFT   31
#if defined (CONFIG_MPC8349)
#define RCWH_PCI64   0x40000000 /* PCI64  */
#define RCWH_PCI1ARB 0x20000000 /* PCI1ARB  */
#define RCWH_PCI2ARB 0x10000000 /* PCI2ARB  */
#elif defined (CONFIG_MPC8360)
#define RCWH_PCIARB   0x20000000	/* PCI internal arbiter mode. */
#define RCWH_PCICKDRV 0x10000000	/* PCI clock output drive. */
#endif
#define RCWH_COREDIS 0x08000000 /* COREDIS  */
#define RCWH_BMS     0x04000000 /* BMS	*/
#define RCWH_BOOTSEQ 0x03000000 /* BOOTSEQ  */
#define RCWH_SWEN    0x00800000 /* SWEN	 */
#define RCWH_ROMLOC  0x00700000 /* ROMLOC  */
#if defined (CONFIG_MPC8349)
#define RCWH_TSEC1M  0x0000c000 /* TSEC1M  */
#define RCWH_TSEC2M  0x00003000 /* TSEC2M  */
#define RCWH_TPR     0x00000100 /* TPR	*/
#elif defined (CONFIG_MPC8360)
#define RCWH_SDDRIOE  0x00000010	/* Secondary DDR IO Enable.  */
#endif
#define RCWH_TLE     0x00000008 /* TLE	*/
#define RCWH_LALE    0x00000004 /* LALE	 */
#if defined (CONFIG_MPC8349)
#define RCWH_RES	~(RCWH_PCIHOST | RCWH_PCI64 | RCWH_PCI1ARB \
			| RCWH_PCI2ARB | RCWH_COREDIS | RCWH_BMS \
			| RCWH_BOOTSEQ | RCWH_SWEN | RCWH_ROMLOC \
			| RCWH_TSEC1M | RCWH_TSEC2M | RCWH_TPR \
			| RCWH_TLE | RCWH_LALE)
#elif defined (CONFIG_MPC8360)
#define RCWH_RES	~(RCWH_PCIHOST|RCWH_PCIARB|RCWH_PCICKDRV \
			|RCWH_COREDIS|RCWH_BMS|RCWH_BOOTSEQ|RCWH_SWEN \
			|RCWH_SDDRIOE |RCWH_TLE)
#endif
	u8 res0[8];
	u32 rsr;		/* Reset status Register  */
#define RSR_RSTSRC 0xE0000000	/* Reset source	 */
#define RSR_RSTSRC_SHIFT   29
#define RSR_BSF	   0x00010000	/* Boot seq. fail  */
#define RSR_BSF_SHIFT	   16
#define RSR_SWSR   0x00002000	/* software soft reset	*/
#define RSR_SWSR_SHIFT	   13
#define RSR_SWHR   0x00001000	/* software hard reset	*/
#define RSR_SWHR_SHIFT	   12
#define RSR_JHRS   0x00000200	/* jtag hreset	*/
#define RSR_JHRS_SHIFT	    9
#define RSR_JSRS   0x00000100	/* jtag sreset status  */
#define RSR_JSRS_SHIFT	    8
#define RSR_CSHR   0x00000010	/* checkstop reset status  */
#define RSR_CSHR_SHIFT	    4
#define RSR_SWRS   0x00000008	/* software watchdog reset status  */
#define RSR_SWRS_SHIFT	    3
#define RSR_BMRS   0x00000004	/* bus monitop reset status  */
#define RSR_BMRS_SHIFT	    2
#define RSR_SRS	   0x00000002	/* soft reset status  */
#define RSR_SRS_SHIFT	    1
#define RSR_HRS	   0x00000001	/* hard reset status  */
#define RSR_HRS_SHIFT	    0
#define RSR_RES ~(RSR_RSTSRC | RSR_BSF | RSR_SWSR | RSR_SWHR | RSR_JHRS | RSR_JSRS | RSR_CSHR | RSR_SWRS | RSR_BMRS | RSR_SRS | RSR_HRS)
	u32 rmr;		/* Reset mode Register	*/
#define RMR_CSRE   0x00000001	/* checkstop reset enable  */
#define RMR_CSRE_SHIFT	    0
#define RMR_RES ~(RMR_CSRE)
	u32 rpr;		/* Reset protection Register  */
	u32 rcr;		/* Reset Control Register  */
#define RCR_SWHR 0x00000002	/* software hard reset	*/
#define RCR_SWSR 0x00000001	/* software soft reset	*/
#define RCR_RES ~(RCR_SWHR | RCR_SWSR)
	u32 rcer;		/* Reset Control Enable Register  */
#define RCER_CRE 0x00000001	/* software hard reset	*/
#define RCER_RES ~(RCER_CRE)
	u8 res1[0xDC];
} reset83xx_t;

typedef struct clk83xx {
	u32 spmr;		/* system PLL mode Register  */
#define SPMR_LBIUCM  0x80000000 /* LBIUCM  */
#define SPMR_DDRCM   0x40000000 /* DDRCM  */
#if defined (CONFIG_MPC8349)
#define SPMR_SVCOD   0x30000000 /* SVCOD  */
#endif
#define SPMR_SPMF    0x0F000000 /* SPMF	 */
#define SPMR_CKID    0x00800000 /* CKID	 */
#define SPMR_CKID_SHIFT 23
#define SPMR_COREPLL 0x007F0000 /* COREPLL  */
#define SPMR_CEVCOD  0x000000C0 /* CEVCOD  */
#define SPMR_CEPDF   0x00000020 /* CEPDF  */
#define SPMR_CEPMF   0x0000001F /* CEPMF  */
#if defined (CONFIG_MPC8349)
#define SPMR_RES	~(SPMR_LBIUCM | SPMR_DDRCM | SPMR_SVCOD \
			| SPMR_SPMF | SPMR_CKID | SPMR_COREPLL \
			| SPMR_CEVCOD | SPMR_CEPDF | SPMR_CEPMF)
#elif defined (CONFIG_MPC8360)
#define SPMR_RES	~(SPMR_LBIUCM | SPMR_DDRCM | SPMR_SPMF \
			| SPMR_CKID | SPMR_COREPLL | SPMR_CEVCOD \
			| SPMR_CEPDF | SPMR_CEPMF)
#endif
	u32 occr;		/* output clock control Register  */
#define OCCR_PCICOE0 0x80000000 /* PCICOE0  */
#define OCCR_PCICOE1 0x40000000 /* PCICOE1  */
#define OCCR_PCICOE2 0x20000000 /* PCICOE2  */
#if defined (CONFIG_MPC8349)
#define OCCR_PCICOE3 0x10000000 /* PCICOE3  */
#define OCCR_PCICOE4 0x08000000 /* PCICOE4  */
#define OCCR_PCICOE5 0x04000000 /* PCICOE5  */
#define OCCR_PCICOE6 0x02000000 /* PCICOE6  */
#define OCCR_PCICOE7 0x01000000 /* PCICOE7  */
#endif
#define OCCR_PCICD0  0x00800000 /* PCICD0  */
#define OCCR_PCICD1  0x00400000 /* PCICD1  */
#define OCCR_PCICD2  0x00200000 /* PCICD2  */
#if defined (CONFIG_MPC8349)
#define OCCR_PCICD3  0x00100000 /* PCICD3  */
#define OCCR_PCICD4  0x00080000 /* PCICD4  */
#define OCCR_PCICD5  0x00040000 /* PCICD5  */
#define OCCR_PCICD6  0x00020000 /* PCICD6  */
#define OCCR_PCICD7  0x00010000 /* PCICD7  */
#define OCCR_PCI1CR  0x00000002 /* PCI1CR  */
#define OCCR_PCI2CR  0x00000001 /* PCI2CR  */
#define OCCR_RES	~(OCCR_PCICOE0 | OCCR_PCICOE1 | OCCR_PCICOE2 \
			| OCCR_PCICOE3 | OCCR_PCICOE4 | OCCR_PCICOE5 \
			| OCCR_PCICOE6 | OCCR_PCICOE7 | OCCR_PCICD0 \
			| OCCR_PCICD1 | OCCR_PCICD2  | OCCR_PCICD3 \
			| OCCR_PCICD4  | OCCR_PCICD5 | OCCR_PCICD6  \
			| OCCR_PCICD7  | OCCR_PCI1CR  | OCCR_PCI2CR )
#endif
#if defined (CONFIG_MPC8360)
#define OCCR_PCICR	0x00000002	/* PCI clock rate  */
#define OCCR_RES	~(OCCR_PCICOE0|OCCR_PCICOE1|OCCR_PCICOE2 \
			|OCCR_PCICD0|OCCR_PCICD1|OCCR_PCICD2|OCCR_PCICR )
#endif
	u32 sccr;		/* system clock control Register  */
#if defined (CONFIG_MPC8349)
#define SCCR_TSEC1CM  0xc0000000	/* TSEC1CM  */
#define SCCR_TSEC1CM_SHIFT 30
#define SCCR_TSEC2CM  0x30000000	/* TSEC2CM  */
#define SCCR_TSEC2CM_SHIFT 28
#endif
#define SCCR_ENCCM    0x03000000	/* ENCCM  */
#define SCCR_ENCCM_SHIFT 24
#if defined (CONFIG_MPC8349)
#define SCCR_USBMPHCM 0x00c00000	/* USBMPHCM  */
#define SCCR_USBMPHCM_SHIFT 22
#define SCCR_USBDRCM  0x00300000	/* USBDRCM  */
#define SCCR_USBDRCM_SHIFT 20
#endif
#define SCCR_PCICM    0x00010000	/* PCICM  */
#if defined (CONFIG_MPC8349)
#define SCCR_RES	~( SCCR_TSEC1CM | SCCR_TSEC2CM | SCCR_ENCCM \
			| SCCR_USBMPHCM | SCCR_USBDRCM | SCCR_PCICM)
#endif
#if defined (CONFIG_MPC8360)
#define SCCR_RES	~(SCCR_ENCCM | SCCR_PCICM)
#endif
	u8 res0[0xF4];
} clk83xx_t;

/*
 * Power Management Control Module
 */
typedef struct pmc83xx {
	u32 pmccr;		/* PMC Configuration Register  */
#define PMCCR_SLPEN 0x00000001	/* System Low Power Enable  */
#define PMCCR_DLPEN 0x00000002	/* DDR SDRAM Low Power Enable  */
#if defined (CONFIG_MPC8360)
#define PMCCR_SDLPEN 0x00000004 /* Secondary DDR SDRAM Low Power Enable	 */
#define PMCCR_RES ~(PMCCR_SLPEN | PMCCR_DLPEN | PMCCR_SDLPEN)
#elif defined (CONFIG_MPC8349)
#define PMCCR_RES    ~(PMCCR_SLPEN | PMCCR_DLPEN)
#endif
	u32 pmcer;		/* PMC Event Register  */
#define PMCER_PMCI  0x00000001	/* PMC Interrupt  */
#define PMCER_RES ~(PMCER_PMCI)
	u32 pmcmr;		/* PMC Mask Register  */
#define PMCMR_PMCIE 0x0001	/* PMC Interrupt Enable	 */
#define PMCMR_RES ~(PMCMR_PMCIE)
	u8 res0[0xF4];
} pmc83xx_t;

#if defined (CONFIG_MPC8349)
/*
 * general purpose I/O module
 */
typedef struct gpio83xx {
	u32 dir;		/* direction register */
	u32 odr;		/* open drain register */
	u32 dat;		/* data register */
	u32 ier;		/* interrupt event register */
	u32 imr;		/* interrupt mask register */
	u32 icr;		/* external interrupt control register */
	u8 res0[0xE8];
} gpio83xx_t;
#endif

#if defined (CONFIG_MPC8360)
/*
 * QE Ports Interrupts Registers
 */
typedef struct qepi83xx {
	u8 res0[0xC];
	u32 qepier;		/* QE Ports Interrupt Event Register */
#define QEPIER_PA15 0x80000000
#define QEPIER_PA16 0x40000000
#define QEPIER_PA29 0x20000000
#define QEPIER_PA30 0x10000000
#define QEPIER_PB3  0x08000000
#define QEPIER_PB5  0x04000000
#define QEPIER_PB12 0x02000000
#define QEPIER_PB13 0x01000000
#define QEPIER_PB26 0x00800000
#define QEPIER_PB27 0x00400000
#define QEPIER_PC27 0x00200000
#define QEPIER_PC28 0x00100000
#define QEPIER_PC29 0x00080000
#define QEPIER_PD12 0x00040000
#define QEPIER_PD13 0x00020000
#define QEPIER_PD16 0x00010000
#define QEPIER_PD17 0x00008000
#define QEPIER_PD26 0x00004000
#define QEPIER_PD27 0x00002000
#define QEPIER_PE12 0x00001000
#define QEPIER_PE13 0x00000800
#define QEPIER_PE24 0x00000400
#define QEPIER_PE25 0x00000200
#define QEPIER_PE26 0x00000100
#define QEPIER_PE27 0x00000080
#define QEPIER_PE31 0x00000040
#define QEPIER_PF20 0x00000020
#define QEPIER_PG31 0x00000010
#define QEPIER_RES ~(QEPIER_PA15|QEPIER_PA16|QEPIER_PA29|QEPIER_PA30|QEPIER_PB3 \
		   |QEPIER_PB5|QEPIER_PB12|QEPIER_PB13|QEPIER_PB26|QEPIER_PB27 \
		   |QEPIER_PC27|QEPIER_PC28|QEPIER_PC29|QEPIER_PD12|QEPIER_PD13 \
		   |QEPIER_PD16|QEPIER_PD17|QEPIER_PD26|QEPIER_PD27|QEPIER_PE12 \
		   |QEPIER_PE13|QEPIER_PE24|QEPIER_PE25|QEPIER_PE26|QEPIER_PE27 \
		   |QEPIER_PE31|QEPIER_PF20|QEPIER_PG31)
	u32 qepimr;		/* QE Ports Interrupt Mask Register */
#define QEPIMR_PA15 0x80000000
#define QEPIMR_PA16 0x40000000
#define QEPIMR_PA29 0x20000000
#define QEPIMR_PA30 0x10000000
#define QEPIMR_PB3  0x08000000
#define QEPIMR_PB5  0x04000000
#define QEPIMR_PB12 0x02000000
#define QEPIMR_PB13 0x01000000
#define QEPIMR_PB26 0x00800000
#define QEPIMR_PB27 0x00400000
#define QEPIMR_PC27 0x00200000
#define QEPIMR_PC28 0x00100000
#define QEPIMR_PC29 0x00080000
#define QEPIMR_PD12 0x00040000
#define QEPIMR_PD13 0x00020000
#define QEPIMR_PD16 0x00010000
#define QEPIMR_PD17 0x00008000
#define QEPIMR_PD26 0x00004000
#define QEPIMR_PD27 0x00002000
#define QEPIMR_PE12 0x00001000
#define QEPIMR_PE13 0x00000800
#define QEPIMR_PE24 0x00000400
#define QEPIMR_PE25 0x00000200
#define QEPIMR_PE26 0x00000100
#define QEPIMR_PE27 0x00000080
#define QEPIMR_PE31 0x00000040
#define QEPIMR_PF20 0x00000020
#define QEPIMR_PG31 0x00000010
#define QEPIMR_RES ~(QEPIMR_PA15|QEPIMR_PA16|QEPIMR_PA29|QEPIMR_PA30|QEPIMR_PB3 \
		   |QEPIMR_PB5|QEPIMR_PB12|QEPIMR_PB13|QEPIMR_PB26|QEPIMR_PB27 \
		   |QEPIMR_PC27|QEPIMR_PC28|QEPIMR_PC29|QEPIMR_PD12|QEPIMR_PD13 \
		   |QEPIMR_PD16|QEPIMR_PD17|QEPIMR_PD26|QEPIMR_PD27|QEPIMR_PE12 \
		   |QEPIMR_PE13|QEPIMR_PE24|QEPIMR_PE25|QEPIMR_PE26|QEPIMR_PE27 \
		   |QEPIMR_PE31|QEPIMR_PF20|QEPIMR_PG31)
	u32 qepicr;		/* QE Ports Interrupt Control Register */
#define QEPICR_PA15 0x80000000
#define QEPICR_PA16 0x40000000
#define QEPICR_PA29 0x20000000
#define QEPICR_PA30 0x10000000
#define QEPICR_PB3  0x08000000
#define QEPICR_PB5  0x04000000
#define QEPICR_PB12 0x02000000
#define QEPICR_PB13 0x01000000
#define QEPICR_PB26 0x00800000
#define QEPICR_PB27 0x00400000
#define QEPICR_PC27 0x00200000
#define QEPICR_PC28 0x00100000
#define QEPICR_PC29 0x00080000
#define QEPICR_PD12 0x00040000
#define QEPICR_PD13 0x00020000
#define QEPICR_PD16 0x00010000
#define QEPICR_PD17 0x00008000
#define QEPICR_PD26 0x00004000
#define QEPICR_PD27 0x00002000
#define QEPICR_PE12 0x00001000
#define QEPICR_PE13 0x00000800
#define QEPICR_PE24 0x00000400
#define QEPICR_PE25 0x00000200
#define QEPICR_PE26 0x00000100
#define QEPICR_PE27 0x00000080
#define QEPICR_PE31 0x00000040
#define QEPICR_PF20 0x00000020
#define QEPICR_PG31 0x00000010
#define QEPICR_RES ~(QEPICR_PA15|QEPICR_PA16|QEPICR_PA29|QEPICR_PA30|QEPICR_PB3 \
		   |QEPICR_PB5|QEPICR_PB12|QEPICR_PB13|QEPICR_PB26|QEPICR_PB27 \
		   |QEPICR_PC27|QEPICR_PC28|QEPICR_PC29|QEPICR_PD12|QEPICR_PD13 \
		   |QEPICR_PD16|QEPICR_PD17|QEPICR_PD26|QEPICR_PD27|QEPICR_PE12 \
		   |QEPICR_PE13|QEPICR_PE24|QEPICR_PE25|QEPICR_PE26|QEPICR_PE27 \
		   |QEPICR_PE31|QEPICR_PF20|QEPICR_PG31)
	u8 res1[0xE8];
} qepi83xx_t;

/*
 * general purpose I/O module
 */
typedef struct gpio_n {
	u32 podr;		/* Open Drain Register */
	u32 pdat;		/* Data Register */
	u32 dir1;		/* direction register 1 */
	u32 dir2;		/* direction register 2 */
	u32 ppar1;		/* Pin Assignment Register 1 */
	u32 ppar2;		/* Pin Assignment Register 2 */
} gpio_n_t;

typedef struct gpio83xx {
	gpio_n_t ioport[0x7];
	u8 res0[0x358];
} gpio83xx_t;

/*
 * QE Secondary Bus Access Windows
 */

typedef struct qesba83xx {
	u32 lbmcsar;		/* Local bus memory controller start address */
#define LBMCSAR_SA	0x000FFFFF	/* 20 most-significant bits of the start address */
#define LBMCSAR_RES	~(LBMCSAR_SA)
	u32 sdmcsar;		/* Secondary DDR memory controller start address */
#define SDMCSAR_SA	0x000FFFFF	/* 20 most-significant bits of the start address */
#define SDMCSAR_RES	~(SDMCSAR_SA)
	u8 res0[0x38];
	u32 lbmcear;		/* Local bus memory controller end address */
#define LBMCEAR_EA	0x000FFFFF	/* 20 most-significant bits of the end address */
#define LBMCEAR_RES	~(LBMCEAR_EA)
	u32 sdmcear;		/* Secondary DDR memory controller end address */
#define SDMCEAR_EA	0x000FFFFF	/* 20 most-significant bits of the end address */
#define SDMCEAR_RES	~(SDMCEAR_EA)
	u8 res1[0x38];
	u32 lbmcar;		/* Local bus memory controller attributes  */
#define LBMCAR_WEN	0x00000001	/* Forward transactions to the QE local bus */
#define LBMCAR_RES	~(LBMCAR_WEN)
	u32 sdmcar;		/* Secondary DDR memory controller attributes */
#define SDMCAR_WEN	0x00000001	/* Forward transactions to the second DDR bus */
#define SDMCAR_RES	~(SDMCAR_WEN)
	u8 res2[0x778];
} qesba83xx_t;
#endif

/*
 * DDR Memory Controller Memory Map
 */
typedef struct ddr_cs_bnds {
	u32 csbnds;
#define CSBNDS_SA 0x00FF0000
#define CSBNDS_SA_SHIFT	   8
#define CSBNDS_EA 0x000000FF
#define CSBNDS_EA_SHIFT	  24
	u8 res0[4];
} ddr_cs_bnds_t;

typedef struct ddr83xx {
	ddr_cs_bnds_t csbnds[4];	    /**< Chip Select x Memory Bounds */
	u8 res0[0x60];
	u32 cs_config[4];	/**< Chip Select x Configuration */
#define CSCONFIG_EN	    0x80000000
#define CSCONFIG_AP	    0x00800000
#define CSCONFIG_ROW_BIT    0x00000700
#define CSCONFIG_ROW_BIT_12 0x00000000
#define CSCONFIG_ROW_BIT_13 0x00000100
#define CSCONFIG_ROW_BIT_14 0x00000200
#define CSCONFIG_COL_BIT    0x00000007
#define CSCONFIG_COL_BIT_8  0x00000000
#define CSCONFIG_COL_BIT_9  0x00000001
#define CSCONFIG_COL_BIT_10 0x00000002
#define CSCONFIG_COL_BIT_11 0x00000003
	u8 res1[0x78];
	u32 timing_cfg_1;	/**< SDRAM Timing Configuration 1 */
#define TIMING_CFG1_PRETOACT 0x70000000
#define TIMING_CFG1_PRETOACT_SHIFT   28
#define TIMING_CFG1_ACTTOPRE 0x0F000000
#define TIMING_CFG1_ACTTOPRE_SHIFT   24
#define TIMING_CFG1_ACTTORW  0x00700000
#define TIMING_CFG1_ACTTORW_SHIFT    20
#define TIMING_CFG1_CASLAT   0x00070000
#define TIMING_CFG1_CASLAT_SHIFT     16
#define TIMING_CFG1_REFREC   0x0000F000
#define TIMING_CFG1_REFREC_SHIFT     12
#define TIMING_CFG1_WRREC    0x00000700
#define TIMING_CFG1_WRREC_SHIFT	      8
#define TIMING_CFG1_ACTTOACT 0x00000070
#define TIMING_CFG1_ACTTOACT_SHIFT    4
#define TIMING_CFG1_WRTORD   0x00000007
#define TIMING_CFG1_WRTORD_SHIFT      0
#define TIMING_CFG1_CASLAT_20 0x00030000	/* CAS latency = 2.0 */
#define TIMING_CFG1_CASLAT_25 0x00040000	/* CAS latency = 2.5 */

	u32 timing_cfg_2;	/**< SDRAM Timing Configuration 2 */
#define TIMING_CFG2_CPO		  0x0F000000
#define TIMING_CFG2_CPO_SHIFT		  24
#define TIMING_CFG2_ACSM	  0x00080000
#define TIMING_CFG2_WR_DATA_DELAY 0x00001C00
#define TIMING_CFG2_WR_DATA_DELAY_SHIFT	  10
#define TIMING_CFG2_CPO_DEF	  0x00000000	/* default (= CASLAT + 1) */

	u32 sdram_cfg;		/**< SDRAM Control Configuration */
#define SDRAM_CFG_MEM_EN     0x80000000
#define SDRAM_CFG_SREN	     0x40000000
#define SDRAM_CFG_ECC_EN     0x20000000
#define SDRAM_CFG_RD_EN	     0x10000000
#define SDRAM_CFG_SDRAM_TYPE 0x03000000
#define SDRAM_CFG_SDRAM_TYPE_SHIFT   24
#define SDRAM_CFG_DYN_PWR    0x00200000
#define SDRAM_CFG_32_BE	     0x00080000
#define SDRAM_CFG_8_BE	     0x00040000
#define SDRAM_CFG_NCAP	     0x00020000
#define SDRAM_CFG_2T_EN	     0x00008000
#define SDRAM_CFG_SDRAM_TYPE_DDR 0x02000000

	u8 res2[4];
	u32 sdram_mode;		/**< SDRAM Mode Configuration */
#define SDRAM_MODE_ESD 0xFFFF0000
#define SDRAM_MODE_ESD_SHIFT   16
#define SDRAM_MODE_SD  0x0000FFFF
#define SDRAM_MODE_SD_SHIFT	0
#define DDR_MODE_EXT_MODEREG	0x4000	/* select extended mode reg */
#define DDR_MODE_EXT_OPMODE	0x3FF8	/* operating mode, mask */
#define DDR_MODE_EXT_OP_NORMAL	0x0000	/* normal operation */
#define DDR_MODE_QFC		0x0004	/* QFC / compatibility, mask */
#define DDR_MODE_QFC_COMP	0x0000	/* compatible to older SDRAMs */
#define DDR_MODE_WEAK		0x0002	/* weak drivers */
#define DDR_MODE_DLL_DIS	0x0001	/* disable DLL */
#define DDR_MODE_CASLAT		0x0070	/* CAS latency, mask */
#define DDR_MODE_CASLAT_15	0x0010	/* CAS latency 1.5 */
#define DDR_MODE_CASLAT_20	0x0020	/* CAS latency 2 */
#define DDR_MODE_CASLAT_25	0x0060	/* CAS latency 2.5 */
#define DDR_MODE_CASLAT_30	0x0030	/* CAS latency 3 */
#define DDR_MODE_BTYPE_SEQ	0x0000	/* sequential burst */
#define DDR_MODE_BTYPE_ILVD	0x0008	/* interleaved burst */
#define DDR_MODE_BLEN_2		0x0001	/* burst length 2 */
#define DDR_MODE_BLEN_4		0x0002	/* burst length 4 */
#define DDR_REFINT_166MHZ_7US	1302	/* exact value for 7.8125 µs */
#define DDR_BSTOPRE	256	/* use 256 cycles as a starting point */
#define DDR_MODE_MODEREG	0x0000	/* select mode register */

	u8 res3[8];
	u32 sdram_interval;	/**< SDRAM Interval Configuration */
#define SDRAM_INTERVAL_REFINT  0x3FFF0000
#define SDRAM_INTERVAL_REFINT_SHIFT    16
#define SDRAM_INTERVAL_BSTOPRE 0x00003FFF
#define SDRAM_INTERVAL_BSTOPRE_SHIFT	0
	u8 res9[8];
	u32 sdram_clk_cntl;
#define DDR_SDRAM_CLK_CNTL_SS_EN		0x80000000
#define DDR_SDRAM_CLK_CNTL_CLK_ADJUST_025	0x01000000
#define DDR_SDRAM_CLK_CNTL_CLK_ADJUST_05	0x02000000
#define DDR_SDRAM_CLK_CNTL_CLK_ADJUST_075	0x03000000
#define DDR_SDRAM_CLK_CNTL_CLK_ADJUST_1		0x04000000

	u8 res4[0xCCC];
	u32 data_err_inject_hi; /**< Memory Data Path Error Injection Mask High */
	u32 data_err_inject_lo; /**< Memory Data Path Error Injection Mask Low */
	u32 ecc_err_inject;	/**< Memory Data Path Error Injection Mask ECC */
#define ECC_ERR_INJECT_EMB			(0x80000000>>22)	/* ECC Mirror Byte */
#define ECC_ERR_INJECT_EIEN			(0x80000000>>23)	/* Error Injection Enable */
#define ECC_ERR_INJECT_EEIM			(0xff000000>>24)	/* ECC Erroe Injection Enable */
#define ECC_ERR_INJECT_EEIM_SHIFT		0
	u8 res5[0x14];
	u32 capture_data_hi;	/**< Memory Data Path Read Capture High */
	u32 capture_data_lo;	/**< Memory Data Path Read Capture Low */
	u32 capture_ecc;	/**< Memory Data Path Read Capture ECC */
#define CAPTURE_ECC_ECE				(0xff000000>>24)
#define CAPTURE_ECC_ECE_SHIFT			0
	u8 res6[0x14];
	u32 err_detect;		/**< Memory Error Detect */
#define ECC_ERROR_DETECT_MME			(0x80000000>>0) /* Multiple Memory Errors */
#define ECC_ERROR_DETECT_MBE			(0x80000000>>28)	/* Multiple-Bit Error */
#define ECC_ERROR_DETECT_SBE			(0x80000000>>29)	/* Single-Bit ECC Error Pickup */
#define ECC_ERROR_DETECT_MSE			(0x80000000>>31)	/* Memory Select Error */
	u32 err_disable;	/**< Memory Error Disable */
#define ECC_ERROR_DISABLE_MBED			(0x80000000>>28)	/* Multiple-Bit ECC Error Disable */
#define ECC_ERROR_DISABLE_SBED			(0x80000000>>29)	/* Sinle-Bit ECC Error disable */
#define ECC_ERROR_DISABLE_MSED			(0x80000000>>31)	/* Memory Select Error Disable */
#define ECC_ERROR_ENABLE			~(ECC_ERROR_DISABLE_MSED|ECC_ERROR_DISABLE_SBED|ECC_ERROR_DISABLE_MBED)
	u32 err_int_en;		/**< Memory Error Interrupt Enable */
#define ECC_ERR_INT_EN_MBEE			(0x80000000>>28)	/* Multiple-Bit ECC Error Interrupt Enable */
#define ECC_ERR_INT_EN_SBEE			(0x80000000>>29)	/* Single-Bit ECC Error Interrupt Enable */
#define ECC_ERR_INT_EN_MSEE			(0x80000000>>31)	/* Memory Select Error Interrupt Enable */
#define ECC_ERR_INT_DISABLE			~(ECC_ERR_INT_EN_MBEE|ECC_ERR_INT_EN_SBEE|ECC_ERR_INT_EN_MSEE)
	u32 capture_attributes; /**< Memory Error Attributes Capture */
#define ECC_CAPT_ATTR_BNUM			(0xe0000000>>1) /* Data Beat Num */
#define ECC_CAPT_ATTR_BNUM_SHIFT		28
#define ECC_CAPT_ATTR_TSIZ			(0xc0000000>>6) /* Transaction Size */
#define ECC_CAPT_ATTR_TSIZ_FOUR_DW		0
#define ECC_CAPT_ATTR_TSIZ_ONE_DW		1
#define ECC_CAPT_ATTR_TSIZ_TWO_DW		2
#define ECC_CAPT_ATTR_TSIZ_THREE_DW		3
#define ECC_CAPT_ATTR_TSIZ_SHIFT		24
#define ECC_CAPT_ATTR_TSRC			(0xf8000000>>11)	/* Transaction Source */
#define ECC_CAPT_ATTR_TSRC_E300_CORE_DT		0x0
#define ECC_CAPT_ATTR_TSRC_E300_CORE_IF		0x2
#define ECC_CAPT_ATTR_TSRC_TSEC1		0x4
#define ECC_CAPT_ATTR_TSRC_TSEC2		0x5
#define ECC_CAPT_ATTR_TSRC_USB			(0x06|0x07)
#define ECC_CAPT_ATTR_TSRC_ENCRYPT		0x8
#define ECC_CAPT_ATTR_TSRC_I2C			0x9
#define ECC_CAPT_ATTR_TSRC_JTAG			0xA
#define ECC_CAPT_ATTR_TSRC_PCI1			0xD
#define ECC_CAPT_ATTR_TSRC_PCI2			0xE
#define ECC_CAPT_ATTR_TSRC_DMA			0xF
#define ECC_CAPT_ATTR_TSRC_SHIFT		16
#define ECC_CAPT_ATTR_TTYP			(0xe0000000>>18)	/* Transaction Type */
#define ECC_CAPT_ATTR_TTYP_WRITE		0x1
#define ECC_CAPT_ATTR_TTYP_READ			0x2
#define ECC_CAPT_ATTR_TTYP_R_M_W		0x3
#define ECC_CAPT_ATTR_TTYP_SHIFT		12
#define ECC_CAPT_ATTR_VLD			(0x80000000>>31)	/* Valid */
	u32 capture_address;	/**< Memory Error Address Capture */
	u32 capture_ext_address;/**< Memory Error Extended Address Capture */
	u32 err_sbe;		/**< Memory Single-Bit ECC Error Management */
#define ECC_ERROR_MAN_SBET			(0xff000000>>8) /* Single-Bit Error Threshold 0..255 */
#define ECC_ERROR_MAN_SBET_SHIFT		16
#define ECC_ERROR_MAN_SBEC			(0xff000000>>24)	/* Single Bit Error Counter 0..255 */
#define ECC_ERROR_MAN_SBEC_SHIFT		0
	u8 res7[0xA4];
	u32 debug_reg;
	u8 res8[0xFC];
} ddr83xx_t;

/*
 * I2C1 Controller
 */

/*
 * DUART
 */
typedef struct duart83xx {
	u8 urbr_ulcr_udlb; /**< combined register for URBR, UTHR and UDLB */
	u8 uier_udmb;	   /**< combined register for UIER and UDMB */
	u8 uiir_ufcr_uafr; /**< combined register for UIIR, UFCR and UAFR */
	u8 ulcr;	/**< line control register */
	u8 umcr;	/**< MODEM control register */
	u8 ulsr;	/**< line status register */
	u8 umsr;	/**< MODEM status register */
	u8 uscr;	/**< scratch register */
	u8 res0[8];
	u8 udsr;	/**< DMA status register */
	u8 res1[3];
	u8 res2[0xEC];
} duart83xx_t;

/*
 * Local Bus Controller Registers
 */
typedef struct lbus_bank {
	u32 br;		    /**< Base Register	*/
	u32 or;		    /**< Base Register	*/
} lbus_bank_t;

typedef struct lbus83xx {
	lbus_bank_t bank[8];
	u8 res0[0x28];
	u32 mar;		/**< UPM Address Register */
	u8 res1[0x4];
	u32 mamr;		/**< UPMA Mode Register */
	u32 mbmr;		/**< UPMB Mode Register */
	u32 mcmr;		/**< UPMC Mode Register */
	u8 res2[0x8];
	u32 mrtpr;		/**< Memory Refresh Timer Prescaler Register */
	u32 mdr;		/**< UPM Data Register */
	u8 res3[0x8];
	u32 lsdmr;		/**< SDRAM Mode Register */
	u8 res4[0x8];
	u32 lurt;		/**< UPM Refresh Timer */
	u32 lsrt;		/**< SDRAM Refresh Timer */
	u8 res5[0x8];
	u32 ltesr;		/**< Transfer Error Status Register */
	u32 ltedr;		/**< Transfer Error Disable Register */
	u32 lteir;		/**< Transfer Error Interrupt Register */
	u32 lteatr;		/**< Transfer Error Attributes Register */
	u32 ltear;		/**< Transfer Error Address Register */
	u8 res6[0xC];
	u32 lbcr;		/**< Configuration Register */
#define LBCR_LDIS  0x80000000
#define LBCR_LDIS_SHIFT	   31
#define LBCR_BCTLC 0x00C00000
#define LBCR_BCTLC_SHIFT   22
#define LBCR_LPBSE 0x00020000
#define LBCR_LPBSE_SHIFT   17
#define LBCR_EPAR  0x00010000
#define LBCR_EPAR_SHIFT	   16
#define LBCR_BMT   0x0000FF00
#define LBCR_BMT_SHIFT	    8
	u32 lcrr;		/**< Clock Ratio Register */
#define LCRR_DBYP    0x80000000
#define LCRR_DBYP_SHIFT	     31
#define LCRR_BUFCMDC 0x30000000
#define LCRR_BUFCMDC_SHIFT   28
#define LCRR_ECL     0x03000000
#define LCRR_ECL_SHIFT	     24
#define LCRR_EADC    0x00030000
#define LCRR_EADC_SHIFT	     16
#define LCRR_CLKDIV  0x0000000F
#define LCRR_CLKDIV_SHIFT     0

	u8 res7[0x28];
	u8 res8[0xF00];
} lbus83xx_t;

#if defined (CONFIG_MPC8349)
/*
 * Serial Peripheral Interface
 */
typedef struct spi83xx {
	u32 mode;     /**< mode register  */
	u32 event;    /**< event register */
	u32 mask;     /**< mask register  */
	u32 com;      /**< command register */
	u8 res0[0x10];
	u32 tx;	      /**< transmit register */
	u32 rx;	      /**< receive register */
	u8 res1[0xD8];
} spi83xx_t;
#endif

/*
 * DMA/Messaging Unit
 */
typedef struct dma83xx {
	u32 res0[0xC];		/* 0x0-0x29 reseverd */
	u32 omisr;		/* 0x30 Outbound message interrupt status register */
	u32 omimr;		/* 0x34 Outbound message interrupt mask register */
	u32 res1[0x6];		/* 0x38-0x49 reserved */

	u32 imr0;		/* 0x50 Inbound message register 0 */
	u32 imr1;		/* 0x54 Inbound message register 1 */
	u32 omr0;		/* 0x58 Outbound message register 0 */
	u32 omr1;		/* 0x5C Outbound message register 1 */

	u32 odr;		/* 0x60 Outbound doorbell register */
	u32 res2;		/* 0x64-0x67 reserved */
	u32 idr;		/* 0x68 Inbound doorbell register */
	u32 res3[0x5];		/* 0x6C-0x79 reserved */

	u32 imisr;		/* 0x80 Inbound message interrupt status register */
	u32 imimr;		/* 0x84 Inbound message interrupt mask register */
	u32 res4[0x1E];		/* 0x88-0x99 reserved */

	u32 dmamr0;		/* 0x100 DMA 0 mode register */
	u32 dmasr0;		/* 0x104 DMA 0 status register */
	u32 dmacdar0;		/* 0x108 DMA 0 current descriptor address register */
	u32 res5;		/* 0x10C reserved */
	u32 dmasar0;		/* 0x110 DMA 0 source address register */
	u32 res6;		/* 0x114 reserved */
	u32 dmadar0;		/* 0x118 DMA 0 destination address register */
	u32 res7;		/* 0x11C reserved */
	u32 dmabcr0;		/* 0x120 DMA 0 byte count register */
	u32 dmandar0;		/* 0x124 DMA 0 next descriptor address register */
	u32 res8[0x16];		/* 0x128-0x179 reserved */

	u32 dmamr1;		/* 0x180 DMA 1 mode register */
	u32 dmasr1;		/* 0x184 DMA 1 status register */
	u32 dmacdar1;		/* 0x188 DMA 1 current descriptor address register */
	u32 res9;		/* 0x18C reserved */
	u32 dmasar1;		/* 0x190 DMA 1 source address register */
	u32 res10;		/* 0x194 reserved */
	u32 dmadar1;		/* 0x198 DMA 1 destination address register */
	u32 res11;		/* 0x19C reserved */
	u32 dmabcr1;		/* 0x1A0 DMA 1 byte count register */
	u32 dmandar1;		/* 0x1A4 DMA 1 next descriptor address register */
	u32 res12[0x16];	/* 0x1A8-0x199 reserved */

	u32 dmamr2;		/* 0x200 DMA 2 mode register */
	u32 dmasr2;		/* 0x204 DMA 2 status register */
	u32 dmacdar2;		/* 0x208 DMA 2 current descriptor address register */
	u32 res13;		/* 0x20C reserved */
	u32 dmasar2;		/* 0x210 DMA 2 source address register */
	u32 res14;		/* 0x214 reserved */
	u32 dmadar2;		/* 0x218 DMA 2 destination address register */
	u32 res15;		/* 0x21C reserved */
	u32 dmabcr2;		/* 0x220 DMA 2 byte count register */
	u32 dmandar2;		/* 0x224 DMA 2 next descriptor address register */
	u32 res16[0x16];	/* 0x228-0x279 reserved */

	u32 dmamr3;		/* 0x280 DMA 3 mode register */
	u32 dmasr3;		/* 0x284 DMA 3 status register */
	u32 dmacdar3;		/* 0x288 DMA 3 current descriptor address register */
	u32 res17;		/* 0x28C reserved */
	u32 dmasar3;		/* 0x290 DMA 3 source address register */
	u32 res18;		/* 0x294 reserved */
	u32 dmadar3;		/* 0x298 DMA 3 destination address register */
	u32 res19;		/* 0x29C reserved */
	u32 dmabcr3;		/* 0x2A0 DMA 3 byte count register */
	u32 dmandar3;		/* 0x2A4 DMA 3 next descriptor address register */

	u32 dmagsr;		/* 0x2A8 DMA general status register */
	u32 res20[0x15];	/* 0x2AC-0x2FF reserved */
} dma83xx_t;

/* DMAMRn bits */
#define DMA_CHANNEL_START			(0x00000001)	/* Bit - DMAMRn CS */
#define DMA_CHANNEL_TRANSFER_MODE_DIRECT	(0x00000004)	/* Bit - DMAMRn CTM */
#define DMA_CHANNEL_SOURCE_ADRESSS_HOLD_EN	(0x00001000)	/* Bit - DMAMRn SAHE */
#define DMA_CHANNEL_SOURCE_ADDRESS_HOLD_1B	(0x00000000)	/* 2Bit- DMAMRn SAHTS 1byte */
#define DMA_CHANNEL_SOURCE_ADDRESS_HOLD_2B	(0x00004000)	/* 2Bit- DMAMRn SAHTS 2bytes */
#define DMA_CHANNEL_SOURCE_ADDRESS_HOLD_4B	(0x00008000)	/* 2Bit- DMAMRn SAHTS 4bytes */
#define DMA_CHANNEL_SOURCE_ADDRESS_HOLD_8B	(0x0000c000)	/* 2Bit- DMAMRn SAHTS 8bytes */
#define DMA_CHANNEL_SNOOP			(0x00010000)	/* Bit - DMAMRn DMSEN */

/* DMASRn bits */
#define DMA_CHANNEL_BUSY			(0x00000004)	/* Bit - DMASRn CB */
#define DMA_CHANNEL_TRANSFER_ERROR		(0x00000080)	/* Bit - DMASRn TE */

/*
 * PCI Software Configuration Registers
 */
typedef struct pciconf83xx {
	u32 config_address;
#define PCI_CONFIG_ADDRESS_EN	0x80000000
#define PCI_CONFIG_ADDRESS_BN_SHIFT	16
#define PCI_CONFIG_ADDRESS_BN_MASK	0x00ff0000
#define PCI_CONFIG_ADDRESS_DN_SHIFT	11
#define PCI_CONFIG_ADDRESS_DN_MASK	0x0000f800
#define PCI_CONFIG_ADDRESS_FN_SHIFT	8
#define PCI_CONFIG_ADDRESS_FN_MASK	0x00000700
#define PCI_CONFIG_ADDRESS_RN_SHIFT	0
#define PCI_CONFIG_ADDRESS_RN_MASK	0x000000fc
	u32 config_data;
	u32 int_ack;
	u8 res[116];
} pciconf83xx_t;

/*
 * PCI Outbound Translation Register
 */
typedef struct pci_outbound_window {
	u32 potar;
	u8 res0[4];
	u32 pobar;
	u8 res1[4];
	u32 pocmr;
	u8 res2[4];
} pot83xx_t;

/*
 * Sequencer
 */
typedef struct ios83xx {
	pot83xx_t pot[6];
#define POTAR_TA_MASK	0x000fffff
#define POBAR_BA_MASK	0x000fffff
#define POCMR_EN	0x80000000
#define POCMR_IO	0x40000000	/* 0--memory space 1--I/O space */
#define POCMR_SE	0x20000000	/* streaming enable */
#define POCMR_DST	0x10000000	/* 0--PCI1 1--PCI2 */
#define POCMR_CM_MASK	0x000fffff
#define POCMR_CM_4G	0x00000000
#define POCMR_CM_2G	0x00080000
#define POCMR_CM_1G	0x000C0000
#define POCMR_CM_512M	0x000E0000
#define POCMR_CM_256M	0x000F0000
#define POCMR_CM_128M	0x000F8000
#define POCMR_CM_64M	0x000FC000
#define POCMR_CM_32M	0x000FE000
#define POCMR_CM_16M	0x000FF000
#define POCMR_CM_8M	0x000FF800
#define POCMR_CM_4M	0x000FFC00
#define POCMR_CM_2M	0x000FFE00
#define POCMR_CM_1M	0x000FFF00
#define POCMR_CM_512K	0x000FFF80
#define POCMR_CM_256K	0x000FFFC0
#define POCMR_CM_128K	0x000FFFE0
#define POCMR_CM_64K	0x000FFFF0
#define POCMR_CM_32K	0x000FFFF8
#define POCMR_CM_16K	0x000FFFFC
#define POCMR_CM_8K	0x000FFFFE
#define POCMR_CM_4K	0x000FFFFF
	u8 res0[0x60];
	u32 pmcr;
	u8 res1[4];
	u32 dtcr;
	u8 res2[4];
} ios83xx_t;

/*
 * PCI Controller Control and Status Registers
 */
typedef struct pcictrl83xx {
	u32 esr;
#define ESR_MERR	0x80000000
#define ESR_APAR	0x00000400
#define ESR_PCISERR	0x00000200
#define ESR_MPERR	0x00000100
#define ESR_TPERR	0x00000080
#define ESR_NORSP	0x00000040
#define ESR_TABT	0x00000020
	u32 ecdr;
#define ECDR_APAR	0x00000400
#define ECDR_PCISERR	0x00000200
#define ECDR_MPERR	0x00000100
#define ECDR_TPERR	0x00000080
#define ECDR_NORSP	0x00000040
#define ECDR_TABT	0x00000020
	u32 eer;
#define EER_APAR	0x00000400
#define EER_PCISERR	0x00000200
#define EER_MPERR	0x00000100
#define EER_TPERR	0x00000080
#define EER_NORSP	0x00000040
#define EER_TABT	0x00000020
	u32 eatcr;
#define EATCR_ERRTYPR_MASK	0x70000000
#define EATCR_ERRTYPR_APR	0x00000000	/* address parity error */
#define EATCR_ERRTYPR_WDPR	0x10000000	/* write data parity error */
#define EATCR_ERRTYPR_RDPR	0x20000000	/* read data parity error */
#define EATCR_ERRTYPR_MA	0x30000000	/* master abort */
#define EATCR_ERRTYPR_TA	0x40000000	/* target abort */
#define EATCR_ERRTYPR_SE	0x50000000	/* system error indication received */
#define EATCR_ERRTYPR_PEA	0x60000000	/* parity error indication received on a read */
#define EATCR_ERRTYPR_PEW	0x70000000	/* parity error indication received on a write */
#define EATCR_BN_MASK		0x0f000000	/* beat number */
#define EATCR_BN_1st		0x00000000
#define EATCR_BN_2ed		0x01000000
#define EATCR_BN_3rd		0x02000000
#define EATCR_BN_4th		0x03000000
#define EATCR_BN_5th		0x0400000
#define EATCR_BN_6th		0x05000000
#define EATCR_BN_7th		0x06000000
#define EATCR_BN_8th		0x07000000
#define EATCR_BN_9th		0x08000000
#define EATCR_TS_MASK		0x00300000	/* transaction size */
#define EATCR_TS_4		0x00000000
#define EATCR_TS_1		0x00100000
#define EATCR_TS_2		0x00200000
#define EATCR_TS_3		0x00300000
#define EATCR_ES_MASK		0x000f0000	/* error source */
#define EATCR_ES_EM		0x00000000	/* external master */
#define EATCR_ES_DMA		0x00050000
#define EATCR_CMD_MASK		0x0000f000
#if defined (CONFIG_MPC8349)
#define EATCR_HBE_MASK		0x00000f00	/* PCI high byte enable */
#endif
#define EATCR_BE_MASK		0x000000f0	/* PCI byte enable */
#if defined (CONFIG_MPC8349)
#define EATCR_HPB		0x00000004	/* high parity bit */
#endif
#define EATCR_PB		0x00000002	/* parity bit */
#define EATCR_VI		0x00000001	/* error information valid */
	u32 eacr;
	u32 eeacr;
#if defined (CONFIG_MPC8349)
	u32 edlcr;
	u32 edhcr;
#elif defined (CONFIG_MPC8360)
	u32 edcr;		/* was edlcr */
	u8 res_edcr[0x4];
#endif
	u32 gcr;
	u32 ecr;
	u32 gsr;
	u8 res0[12];
	u32 pitar2;
	u8 res1[4];
	u32 pibar2;
	u32 piebar2;
	u32 piwar2;
	u8 res2[4];
	u32 pitar1;
	u8 res3[4];
	u32 pibar1;
	u32 piebar1;
	u32 piwar1;
	u8 res4[4];
	u32 pitar0;
	u8 res5[4];
	u32 pibar0;
	u8 res6[4];
	u32 piwar0;
	u8 res7[132];
#define PITAR_TA_MASK		0x000fffff
#define PIBAR_MASK		0xffffffff
#define PIEBAR_EBA_MASK		0x000fffff
#define PIWAR_EN		0x80000000
#define PIWAR_PF		0x20000000
#define PIWAR_RTT_MASK		0x000f0000
#define PIWAR_RTT_NO_SNOOP	0x00040000
#define PIWAR_RTT_SNOOP		0x00050000
#define PIWAR_WTT_MASK		0x0000f000
#define PIWAR_WTT_NO_SNOOP	0x00004000
#define PIWAR_WTT_SNOOP		0x00005000
#define PIWAR_IWS_MASK	0x0000003F
#define PIWAR_IWS_4K	0x0000000B
#define PIWAR_IWS_8K	0x0000000C
#define PIWAR_IWS_16K	0x0000000D
#define PIWAR_IWS_32K	0x0000000E
#define PIWAR_IWS_64K	0x0000000F
#define PIWAR_IWS_128K	0x00000010
#define PIWAR_IWS_256K	0x00000011
#define PIWAR_IWS_512K	0x00000012
#define PIWAR_IWS_1M	0x00000013
#define PIWAR_IWS_2M	0x00000014
#define PIWAR_IWS_4M	0x00000015
#define PIWAR_IWS_8M	0x00000016
#define PIWAR_IWS_16M	0x00000017
#define PIWAR_IWS_32M	0x00000018
#define PIWAR_IWS_64M	0x00000019
#define PIWAR_IWS_128M	0x0000001A
#define PIWAR_IWS_256M	0x0000001B
#define PIWAR_IWS_512M	0x0000001C
#define PIWAR_IWS_1G	0x0000001D
#define PIWAR_IWS_2G	0x0000001E
} pcictrl83xx_t;

#if defined (CONFIG_MPC8349)
/*
 * USB
 */
typedef struct usb83xx {
	u8 fixme[0x2000];
} usb83xx_t;

/*
 * TSEC
 */
typedef struct tsec83xx {
	u8 fixme[0x1000];
} tsec83xx_t;
#endif

/*
 * Security
 */
typedef struct security83xx {
	u8 fixme[0x10000];
} security83xx_t;

#if defined (CONFIG_MPC8360)
/*
 * iram
 */
typedef struct iram83xx {
	u32 iadd;		/* I-RAM address register */
	u32 idata;		/* I-RAM data register */
	u8 res0[0x78];
} iram83xx_t;

/*
 * Interrupt Controller
 */
typedef struct irq83xx {
	u32 cicr;		/* QE system interrupt configuration */
	u32 civec;		/* QE system interrupt vector register */
	u32 cripnr;		/* QE RISC interrupt pending register */
	u32 cipnr;		/*  QE system interrupt pending register */
	u32 cipxcc;		/* QE interrupt priority register */
	u32 cipycc;		/* QE interrupt priority register */
	u32 cipwcc;		/* QE interrupt priority register */
	u32 cipzcc;		/* QE interrupt priority register */
	u32 cimr;		/* QE system interrupt mask register */
	u32 crimr;		/* QE RISC interrupt mask register */
	u32 cicnr;		/* QE system interrupt control register */
	u8 res0[0x4];
	u32 ciprta;		/* QE system interrupt priority register for RISC tasks A */
	u32 ciprtb;		/* QE system interrupt priority register for RISC tasks B */
	u8 res1[0x4];
	u32 cricr;		/* QE system RISC interrupt control */
	u8 res2[0x20];
	u32 chivec;		/* QE high system interrupt vector */
	u8 res3[0x1C];
} irq83xx_t;

/*
 * Communications Processor
 */
typedef struct cp83xx {
	u32 cecr;		/* QE command register */
	u32 ceccr;		/* QE controller configuration register */
	u32 cecdr;		/* QE command data register */
	u8 res0[0xA];
	u16 ceter;		/* QE timer event register */
	u8 res1[0x2];
	u16 cetmr;		/* QE timers mask register */
	u32 cetscr;		/* QE time-stamp timer control register */
	u32 cetsr1;		/* QE time-stamp register 1 */
	u32 cetsr2;		/* QE time-stamp register 2 */
	u8 res2[0x8];
	u32 cevter;		/* QE virtual tasks event register */
	u32 cevtmr;		/* QE virtual tasks mask register */
	u16 cercr;		/* QE RAM control register */
	u8 res3[0x2];
	u8 res4[0x24];
	u16 ceexe1;		/* QE external request 1 event register */
	u8 res5[0x2];
	u16 ceexm1;		/* QE external request 1 mask register */
	u8 res6[0x2];
	u16 ceexe2;		/* QE external request 2 event register */
	u8 res7[0x2];
	u16 ceexm2;		/* QE external request 2 mask register */
	u8 res8[0x2];
	u16 ceexe3;		/* QE external request 3 event register */
	u8 res9[0x2];
	u16 ceexm3;		/* QE external request 3 mask register */
	u8 res10[0x2];
	u16 ceexe4;		/* QE external request 4 event register */
	u8 res11[0x2];
	u16 ceexm4;		/* QE external request 4 mask register */
	u8 res12[0x2];
	u8 res13[0x280];
} cp83xx_t;

/*
 * QE Multiplexer
 */

typedef struct qmx83xx {
	u32 cmxgcr;		/* CMX general clock route register */
	u32 cmxsi1cr_l;		/* CMX SI1 clock route low register */
	u32 cmxsi1cr_h;		/* CMX SI1 clock route high register */
	u32 cmxsi1syr;		/* CMX SI1 SYNC route register */
	u32 cmxucr1;		/* CMX UCC1, UCC3 clock route register */
	u32 cmxucr2;		/* CMX UCC5, UCC7 clock route register */
	u32 cmxucr3;		/* CMX UCC2, UCC4 clock route register */
	u32 cmxucr4;		/* CMX UCC6, UCC8 clock route register */
	u32 cmxupcr;		/* CMX UPC clock route register */
	u8 res0[0x1C];
} qmx83xx_t;

/*
* QE Timers
*/

typedef struct qet83xx {
	u8 gtcfr1;		/* Timer 1 and Timer 2 global configuration register */
	u8 res0[0x3];
	u8 gtcfr2;		/* Timer 3 and timer 4 global configuration register */
	u8 res1[0xB];
	u16 gtmdr1;		/* Timer 1 mode register */
	u16 gtmdr2;		/* Timer 2 mode register */
	u16 gtrfr1;		/* Timer 1 reference register */
	u16 gtrfr2;		/* Timer 2 reference register */
	u16 gtcpr1;		/* Timer 1 capture register */
	u16 gtcpr2;		/* Timer 2 capture register */
	u16 gtcnr1;		/* Timer 1 counter */
	u16 gtcnr2;		/* Timer 2 counter */
	u16 gtmdr3;		/* Timer 3 mode register */
	u16 gtmdr4;		/* Timer 4 mode register */
	u16 gtrfr3;		/* Timer 3 reference register */
	u16 gtrfr4;		/* Timer 4 reference register */
	u16 gtcpr3;		/* Timer 3 capture register */
	u16 gtcpr4;		/* Timer 4 capture register */
	u16 gtcnr3;		/* Timer 3 counter */
	u16 gtcnr4;		/* Timer 4 counter */
	u16 gtevr1;		/* Timer 1 event register */
	u16 gtevr2;		/* Timer 2 event register */
	u16 gtevr3;		/* Timer 3 event register */
	u16 gtevr4;		/* Timer 4 event register */
	u16 gtps;		/* Timer 1 prescale register */
	u8 res2[0x46];
} qet83xx_t;

/*
* spi
*/

typedef struct spi83xx {
	u8 res0[0x20];
	u32 spmode;		/* SPI mode register */
	u8 res1[0x2];
	u8 spie;		/* SPI event register */
	u8 res2[0x1];
	u8 res3[0x2];
	u8 spim;		/* SPI mask register */
	u8 res4[0x1];
	u8 res5[0x1];
	u8 spcom;		/* SPI command register	 */
	u8 res6[0x2];
	u32 spitd;		/* SPI transmit data register (cpu mode) */
	u32 spird;		/* SPI receive data register (cpu mode) */
	u8 res7[0x8];
} spi83xx_t;

/*
* mcc
*/

typedef struct mcc83xx {
	u32 mcce;		/* MCC event register */
	u32 mccm;		/* MCC mask register */
	u32 mccf;		/* MCC configuration register */
	u32 merl;		/* MCC emergency request level register */
	u8 res0[0xF0];
} mcc83xx_t;

/*
* brg
*/

typedef struct brg83xx {
	u32 brgc1;		/* BRG1 configuration register */
	u32 brgc2;		/* BRG2 configuration register */
	u32 brgc3;		/* BRG3 configuration register */
	u32 brgc4;		/* BRG4 configuration register */
	u32 brgc5;		/* BRG5 configuration register */
	u32 brgc6;		/* BRG6 configuration register */
	u32 brgc7;		/* BRG7 configuration register */
	u32 brgc8;		/* BRG8 configuration register */
	u32 brgc9;		/* BRG9 configuration register */
	u32 brgc10;		/* BRG10 configuration register */
	u32 brgc11;		/* BRG11 configuration register */
	u32 brgc12;		/* BRG12 configuration register */
	u32 brgc13;		/* BRG13 configuration register */
	u32 brgc14;		/* BRG14 configuration register */
	u32 brgc15;		/* BRG15 configuration register */
	u32 brgc16;		/* BRG16 configuration register */
	u8 res0[0x40];
} brg83xx_t;

/*
* USB
*/

typedef struct usb83xx {
	u8 usmod;		/* USB mode register */
	u8 usadd;		/* USB address register */
	u8 uscom;		/* USB command register */
	u8 res0[0x1];
	u16 usep0;		/* USB endpoint register 0 */
	u16 usep1;		/* USB endpoint register 1 */
	u16 usep2;		/* USB endpoint register 2 */
	u16 usep3;		/* USB endpoint register 3 */
	u8 res1[0x4];
	u16 usber;		/* USB event register */
	u8 res2[0x2];
	u16 usbmr;		/* USB mask register */
	u8 res3[0x1];
	u8 usbs;		/* USB status register */
	u32 ussft;		/* USB start of frame timer */
	u8 res4[0x24];
} usb83xx_t;

/*
* SI
*/

typedef struct si1_83xx {
	u16 siamr1;		/* SI1 TDMA mode register */
	u16 sibmr1;		/* SI1 TDMB mode register */
	u16 sicmr1;		/* SI1 TDMC mode register */
	u16 sidmr1;		/* SI1 TDMD mode register */
	u8 siglmr1_h;		/* SI1 global mode register high */
	u8 res0[0x1];
	u8 sicmdr1_h;		/* SI1 command register high */
	u8 res2[0x1];
	u8 sistr1_h;		/* SI1 status register high */
	u8 res3[0x1];
	u16 sirsr1_h;		/* SI1 RAM shadow address register high */
	u8 sitarc1;		/* SI1 RAM counter Tx TDMA */
	u8 sitbrc1;		/* SI1 RAM counter Tx TDMB */
	u8 sitcrc1;		/* SI1 RAM counter Tx TDMC */
	u8 sitdrc1;		/* SI1 RAM counter Tx TDMD */
	u8 sirarc1;		/* SI1 RAM counter Rx TDMA */
	u8 sirbrc1;		/* SI1 RAM counter Rx TDMB */
	u8 sircrc1;		/* SI1 RAM counter Rx TDMC */
	u8 sirdrc1;		/* SI1 RAM counter Rx TDMD */
	u8 res4[0x8];
	u16 siemr1;		/* SI1 TDME mode register 16 bits */
	u16 sifmr1;		/* SI1 TDMF mode register 16 bits */
	u16 sigmr1;		/* SI1 TDMG mode register 16 bits */
	u16 sihmr1;		/* SI1 TDMH mode register 16 bits */
	u8 siglmg1_l;		/* SI1 global mode register low 8 bits */
	u8 res5[0x1];
	u8 sicmdr1_l;		/* SI1 command register low 8 bits */
	u8 res6[0x1];
	u8 sistr1_l;		/* SI1 status register low 8 bits */
	u8 res7[0x1];
	u16 sirsr1_l;		/* SI1 RAM shadow address register low 16 bits */
	u8 siterc1;		/* SI1 RAM counter Tx TDME 8 bits */
	u8 sitfrc1;		/* SI1 RAM counter Tx TDMF 8 bits */
	u8 sitgrc1;		/* SI1 RAM counter Tx TDMG 8 bits */
	u8 sithrc1;		/* SI1 RAM counter Tx TDMH 8 bits */
	u8 sirerc1;		/* SI1 RAM counter Rx TDME 8 bits */
	u8 sirfrc1;		/* SI1 RAM counter Rx TDMF 8 bits */
	u8 sirgrc1;		/* SI1 RAM counter Rx TDMG 8 bits */
	u8 sirhrc1;		/* SI1 RAM counter Rx TDMH 8 bits */
	u8 res8[0x8];
	u32 siml1;		/* SI1 multiframe limit register */
	u8 siedm1;		/* SI1 extended diagnostic mode register */
	u8 res9[0xBB];
} si1_83xx_t;

/*
*  SI Routing Tables
*/

typedef struct sir83xx {
	u8 tx[0x400];
	u8 rx[0x400];
	u8 res0[0x800];
} sir83xx_t;

/*
* ucc
*/

typedef struct uslow {
	u32 gumr_l;		/* UCCx general mode register (low) */
	u32 gumr_h;		/* UCCx general mode register (high) */
	u16 upsmr;		/* UCCx protocol-specific mode register */
	u8 res0[0x2];
	u16 utodr;		/* UCCx transmit on demand register */
	u16 udsr;		/* UCCx data synchronization register */
	u16 ucce;		/* UCCx event register */
	u8 res1[0x2];
	u16 uccm;		/* UCCx mask register */
	u8 res2[0x1];
	u8 uccs;		/* UCCx status register */
	u8 res3[0x1E8];
} uslow_t;

typedef struct ufast {
	u32 gumr;		/* UCCx general mode register */
	u32 upsmr;		/* UCCx protocol-specific mode register	 */
	u16 utodr;		/* UCCx transmit on demand register  */
	u8 res0[0x2];
	u16 udsr;		/* UCCx data synchronization register  */
	u8 res1[0x2];
	u32 ucce;		/* UCCx event register */
	u32 uccm;		/* UCCx mask register.	*/
	u8 uccs;		/* UCCx status register */
	u8 res2[0x7];
	u32 urfb;		/* UCC receive FIFO base  */
	u16 urfs;		/* UCC receive FIFO size  */
	u8 res3[0x2];
	u16 urfet;		/* UCC receive FIFO emergency threshold	 */
	u16 urfset;		/* UCC receive FIFO special emergency threshold	 */
	u32 utfb;		/* UCC transmit FIFO base */
	u16 utfs;		/* UCC transmit FIFO size  */
	u8 res4[0x2];
	u16 utfet;		/* UCC transmit FIFO emergency threshold */
	u8 res5[0x2];
	u16 utftt;		/* UCC transmit FIFO transmit threshold */
	u8 res6[0x2];
	u16 utpt;		/* UCC transmit polling timer */
	u32 urtry;		/* UCC retry counter register */
	u8 res7[0x4C];
	u8 guemr;		/* UCC general extended mode register */
	u8 res8[0x3];
	u8 res9[0x6C];
	u32 maccfg1;		/* Mac configuration register #1  */
	u32 maccfg2;		/* Mac configuration register #2  */
	u16 ipgifg;		/* Interframe gap register  */
	u8 res10[0x2];
	u32 hafdup;		/* Half-duplex register	 */
	u8 res11[0xC];
	u32 emtr;		/* Ethernet MAC test register  */
	u32 miimcfg;		/* MII mgmt configuration register  */
	u32 miimcom;		/* MII mgmt command register  */
	u32 miimadd;		/* MII mgmt address register  */
	u32 miimcon;		/* MII mgmt control register  */
	u32 miistat;		/* MII mgmt status register */
	u32 miimnd;		/* MII mgmt indication register */
	u32 ifctl;		/* Interface control register  */
	u32 ifstat;		/* Interface status register  */
	u32 macstnaddr1;	/* Station address part 1 register */
	u32 macstnaddr2;	/* Station address part 2 register */
	u8 res12[0x8];
	u32 uempr;		/* UCC Ethernet MAC parameter register */
	u32 utbipa;		/* UCC TBI address */
	u16 uescr;		/* UCC Ethernet statistics control register */
	u8 res13[0x26];
	u32 tx64;		/* Transmit and receive 64-byte frame counter */
	u32 tx127;		/* Transmit and receive 65- to 127-byte frame counter */
	u32 tx255;		/* Transmit and receive 128- to 255-byte frame counter */
	u32 rx64;		/* Receive and receive 64-byte frame counter */
	u32 rx127;		/* Receive and receive 65- to 127-byte frame counter */
	u32 rx255;		/* Receive and receive 128- to 255-byte frame counter */
	u32 txok;		/* Transmit good bytes counter */
	u32 txcf;		/* Transmit control frame counter */
	u32 tmca;		/* Transmit multicast control frame counter */
	u32 tbca;		/* Transmit broadcast packet counter */
	u32 rxfok;		/* Receive frame OK counter */
	u32 rbyt;		/* Receive good and bad bytes counter */
	u32 rxbok;		/* Receive bytes OK counter */
	u32 rmca;		/* Receive multicast packet counter */
	u32 rbca;		/* Receive broadcast packet counter */
	u32 scar;		/* Statistics carry register */
	u32 scam;		/* Statistics carry mask register */
	u8 res14[0x3C];
} ufast_t;

typedef struct ucc83xx {
	union {
		uslow_t slow;
		ufast_t fast;
	};
} ucc83xx_t;

/*
*  MultiPHY UTOPIA POS Controllers
*/

typedef struct upc83xx {
	u32 upgcr;		/* UTOPIA/POS general configuration register  */
#define UPGCR_PROTOCOL	0x80000000	/* protocol ul2 or pl2 */
#define UPGCR_TMS	0x40000000	/* Transmit master/slave mode */
#define UPGCR_RMS	0x20000000	/* Receive master/slave mode */
#define UPGCR_ADDR	0x10000000	/* Master MPHY Addr multiplexing: */
#define UPGCR_DIAG	0x01000000	/* Diagnostic mode */
	u32 uplpa;		/* UTOPIA/POS last PHY address */
	u32 uphec;		/* ATM HEC register */
	u32 upuc;		/* UTOPIA/POS UCC configuration */
	u32 updc1;		/* UTOPIA/POS device 1 configuration */
	u32 updc2;		/* UTOPIA/POS device 2 configuration  */
	u32 updc3;		/* UTOPIA/POS device 3 configuration */
	u32 updc4;		/* UTOPIA/POS device 4 configuration  */
	u32 upstpa;		/* UTOPIA/POS STPA threshold  */
	u8 res0[0xC];
	u32 updrs1_h;		/* UTOPIA/POS device 1 rate select  */
	u32 updrs1_l;		/* UTOPIA/POS device 1 rate select  */
	u32 updrs2_h;		/* UTOPIA/POS device 2 rate select  */
	u32 updrs2_l;		/* UTOPIA/POS device 2 rate select */
	u32 updrs3_h;		/* UTOPIA/POS device 3 rate select */
	u32 updrs3_l;		/* UTOPIA/POS device 3 rate select */
	u32 updrs4_h;		/* UTOPIA/POS device 4 rate select */
	u32 updrs4_l;		/* UTOPIA/POS device 4 rate select */
	u32 updrp1;		/* UTOPIA/POS device 1 receive priority low  */
	u32 updrp2;		/* UTOPIA/POS device 2 receive priority low  */
	u32 updrp3;		/* UTOPIA/POS device 3 receive priority low  */
	u32 updrp4;		/* UTOPIA/POS device 4 receive priority low  */
	u32 upde1;		/* UTOPIA/POS device 1 event */
	u32 upde2;		/* UTOPIA/POS device 2 event */
	u32 upde3;		/* UTOPIA/POS device 3 event */
	u32 upde4;		/* UTOPIA/POS device 4 event */
	u16 uprp1;
	u16 uprp2;
	u16 uprp3;
	u16 uprp4;
	u8 res1[0x8];
	u16 uptirr1_0;		/* Device 1 transmit internal rate 0 */
	u16 uptirr1_1;		/* Device 1 transmit internal rate 1 */
	u16 uptirr1_2;		/* Device 1 transmit internal rate 2 */
	u16 uptirr1_3;		/* Device 1 transmit internal rate 3 */
	u16 uptirr2_0;		/* Device 2 transmit internal rate 0 */
	u16 uptirr2_1;		/* Device 2 transmit internal rate 1 */
	u16 uptirr2_2;		/* Device 2 transmit internal rate 2 */
	u16 uptirr2_3;		/* Device 2 transmit internal rate 3 */
	u16 uptirr3_0;		/* Device 3 transmit internal rate 0 */
	u16 uptirr3_1;		/* Device 3 transmit internal rate 1 */
	u16 uptirr3_2;		/* Device 3 transmit internal rate 2 */
	u16 uptirr3_3;		/* Device 3 transmit internal rate 3 */
	u16 uptirr4_0;		/* Device 4 transmit internal rate 0 */
	u16 uptirr4_1;		/* Device 4 transmit internal rate 1 */
	u16 uptirr4_2;		/* Device 4 transmit internal rate 2 */
	u16 uptirr4_3;		/* Device 4 transmit internal rate 3 */
	u32 uper1;		/* Device 1 port enable register */
	u32 uper2;		/* Device 2 port enable register */
	u32 uper3;		/* Device 3 port enable register */
	u32 uper4;		/* Device 4 port enable register */
	u8 res2[0x150];
} upc83xx_t;

/*
* SDMA
*/

typedef struct sdma83xx {
	u32 sdsr;		/* Serial DMA status register */
	u32 sdmr;		/* Serial DMA mode register */
	u32 sdtr1;		/* SDMA system bus threshold register */
	u32 sdtr2;		/* SDMA secondary bus threshold register */
	u32 sdhy1;		/* SDMA system bus hysteresis register */
	u32 sdhy2;		/* SDMA secondary bus hysteresis register */
	u32 sdta1;		/* SDMA system bus address register */
	u32 sdta2;		/* SDMA secondary bus address register */
	u32 sdtm1;		/* SDMA system bus MSNUM register */
	u32 sdtm2;		/* SDMA secondary bus MSNUM register */
	u8 res0[0x10];
	u32 sdaqr;		/* SDMA address bus qualify register */
	u32 sdaqmr;		/* SDMA address bus qualify mask register */
	u8 res1[0x4];
	u32 sdwbcr;		/* SDMA CAM entries base register */
	u8 res2[0x38];
} sdma83xx_t;

/*
* Debug Space
*/

typedef struct dbg83xx {
	u32 bpdcr;		/* Breakpoint debug command register */
	u32 bpdsr;		/* Breakpoint debug status register */
	u32 bpdmr;		/* Breakpoint debug mask register */
	u32 bprmrr0;		/* Breakpoint request mode risc register 0 */
	u32 bprmrr1;		/* Breakpoint request mode risc register 1 */
	u8 res0[0x8];
	u32 bprmtr0;		/* Breakpoint request mode trb register 0 */
	u32 bprmtr1;		/* Breakpoint request mode trb register 1 */
	u8 res1[0x8];
	u32 bprmir;		/* Breakpoint request mode immediate register */
	u32 bprmsr;		/* Breakpoint request mode serial register */
	u32 bpemr;		/* Breakpoint exit mode register */
	u8 res2[0x48];
} dbg83xx_t;

/*
*  RISC Special Registers (Trap and Breakpoint)
*/

typedef struct rsp83xx {
	u8 fixme[0x100];
} rsp83xx_t;
#endif

typedef struct immap {
	sysconf83xx_t sysconf;	/* System configuration */
	wdt83xx_t wdt;		/* Watch Dog Timer (WDT) Registers */
	rtclk83xx_t rtc;	/* Real Time Clock Module Registers */
	rtclk83xx_t pit;	/* Periodic Interval Timer */
	gtm83xx_t gtm[2];	/* Global Timers Module */
	ipic83xx_t ipic;	/* Integrated Programmable Interrupt Controller */
	arbiter83xx_t arbiter;	/* System Arbiter Registers */
	reset83xx_t reset;	/* Reset Module */
	clk83xx_t clk;		/* System Clock Module */
	pmc83xx_t pmc;		/* Power Management Control Module */
#if defined (CONFIG_MPC8349)
	gpio83xx_t pgio[2];	/* general purpose I/O module */
#elif defined (CONFIG_MPC8360)
	qepi83xx_t qepi;	/* QE Ports Interrupts Registers */
#endif
	u8 res0[0x200];
#if defined (CONFIG_MPC8360)
	u8 DLL_LBDDR[0x100];
#endif
	u8 DDL_DDR[0x100];
	u8 DDL_LBIU[0x100];
#if defined (CONFIG_MPC8349)
	u8 res1[0xE00];
#elif defined (CONFIG_MPC8360)
	u8 res1[0x200];
	gpio83xx_t gpio;	/* General purpose I/O module */
	qesba83xx_t qesba;	/* QE Secondary Bus Access Windows */
#endif
	ddr83xx_t ddr;		/* DDR Memory Controller Memory */
	fsl_i2c_t i2c[2];	/* I2C Controllers */
	u8 res2[0x1300];
	duart83xx_t duart[2];	/* DUART */
#if defined (CONFIG_MPC8349)
	u8 res3[0x900];
	lbus83xx_t lbus;	/* Local Bus Controller Registers */
	u8 res4[0x1000];
	spi83xx_t spi;		/* Serial Peripheral Interface */
	u8 res5[0xF00];
#elif defined (CONFIG_MPC8360)
	u8 res3[0x900];
	lbus83xx_t lbus;	/* Local Bus Controller */
	u8 res4[0x2000];
#endif
	dma83xx_t dma;		/* DMA */
#if defined (CONFIG_MPC8349)
	pciconf83xx_t pci_conf[2];	/* PCI Software Configuration Registers */
	ios83xx_t ios;		/* Sequencer */
	pcictrl83xx_t pci_ctrl[2];	/* PCI Controller Control and Status Registers */
	u8 res6[0x19900];
	usb83xx_t usb;
	tsec83xx_t tsec[2];
	u8 res7[0xA000];
	security83xx_t security;
#elif defined (CONFIG_MPC8360)
	pciconf83xx_t pci_conf[1];	/* PCI Software Configuration Registers */
	u8 res_5[128];
	ios83xx_t ios;		/* Sequencer (IOS) */
	pcictrl83xx_t pci_ctrl[1];	/* PCI Controller Control and Status Registers */
	u8 res6[0x4A00];
	ddr83xx_t ddr_secondary;	/* Secondary DDR Memory Controller Memory Map */
	u8 res7[0x22000];
	security83xx_t security;
	u8 res8[0xC0000];
	iram83xx_t iram;	/* IRAM */
	irq83xx_t irq;		/* Interrupt Controller */
	cp83xx_t cp;		/* Communications Processor */
	qmx83xx_t qmx;		/* QE Multiplexer */
	qet83xx_t qet;		/* QE Timers */
	spi83xx_t spi[0x2];	/* spi	*/
	mcc83xx_t mcc;		/* mcc */
	brg83xx_t brg;		/* brg */
	usb83xx_t usb;		/* USB */
	si1_83xx_t si1;		/* SI */
	u8 res9[0x800];
	sir83xx_t sir;		/* SI Routing Tables  */
	ucc83xx_t ucc1;		/* ucc1 */
	ucc83xx_t ucc3;		/* ucc3 */
	ucc83xx_t ucc5;		/* ucc5 */
	ucc83xx_t ucc7;		/* ucc7 */
	u8 res10[0x600];
	upc83xx_t upc1;		/* MultiPHY UTOPIA POS Controller 1 */
	ucc83xx_t ucc2;		/* ucc2 */
	ucc83xx_t ucc4;		/* ucc4 */
	ucc83xx_t ucc6;		/* ucc6 */
	ucc83xx_t ucc8;		/* ucc8 */
	u8 res11[0x600];
	upc83xx_t upc2;		/* MultiPHY UTOPIA POS Controller 2 */
	sdma83xx_t sdma;	/* SDMA */
	dbg83xx_t dbg;		/* Debug Space */
	rsp83xx_t rsp[0x2];	/* RISC Special Registers (Trap and Breakpoint) */
	u8 res12[0x300];
	u8 res13[0x3A00];
	u8 res14[0x8000];	/* 0x108000 -  0x110000 */
	u8 res15[0xC000];	/* 0x110000 -  0x11C000 Multi-user RAM */
	u8 res16[0x24000];	/* 0x11C000 -  0x140000 */
	u8 res17[0xC0000];	/* 0x140000 -  0x200000 */
#endif
} immap_t;

#endif				/* __IMMAP_83xx__ */
