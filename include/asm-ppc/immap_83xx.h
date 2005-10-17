/*
 * MPC8349 Internal Memory Map
 * Copyright (c) 2004 Freescale Semiconductor.
 * Eran Liberty (liberty@freescale.com)
 *
 * based on:
 * - MPC8260 Internal Memory Map
 *   Copyright (c) 1999 Dan Malek (dmalek@jlc.net)
 * - MPC85xx Internal Memory Map
 *   Copyright(c) 2002,2003 Motorola Inc.
 *   Xianghua Xiao (x.xiao@motorola.com)
 */
#ifndef __IMMAP_8349__
#define __IMMAP_8349__

#include <asm/types.h>
#include <asm/i2c.h>

/*
 * Local Access Window.
 */
typedef struct law8349 {
	u32 bar; /* LBIU local access window base address register */
/* Identifies the 20 most-significant address bits of the base of local
 * access window n. The specified base address should be aligned to the
 * window size, as defined by LBLAWARn[SIZE].
 */
#define LAWBAR_BAR         0xFFFFF000
#define LAWBAR_RES	     ~(LAWBAR_BAR)
	u32 ar; /* LBIU local access window attribute register */
} law8349_t;

/*
 * System configuration registers.
 */
typedef struct sysconf8349 {
	u32 immrbar; /* Internal memory map base address register */
	u8 res0[0x04];
	u32 altcbar; /* Alternate configuration base address register */
/* Identifies the12 most significant address bits of an alternate base
 * address used for boot sequencer configuration accesses.
 */
#define ALTCBAR_BASE_ADDR     0xFFF00000
#define ALTCBAR_RES           ~(ALTCBAR_BASE_ADDR) /* Reserved. Write has no effect, read returns 0. */
	u8 res1[0x14];
	law8349_t lblaw[4]; /* LBIU local access window */
	u8 res2[0x20];
	law8349_t pcilaw[2]; /* PCI local access window */
	u8 res3[0x30];
	law8349_t ddrlaw[2]; /* DDR local access window */
	u8 res4[0x50];
	u32 sgprl; /* System General Purpose Register Low */
	u32 sgprh; /* System General Purpose Register High */
	u32 spridr; /* System Part and Revision ID Register */
#define SPRIDR_PARTID         0xFFFF0000 /* Part Identification. */
#define SPRIDR_REVID          0x0000FFFF /* Revision Identification. */
	u8 res5[0x04];
	u32 spcr; /* System Priority Configuration Register */
#define SPCR_PCIHPE   0x10000000 /* PCI Highest Priority Enable. */
#define SPCR_PCIPR    0x03000000 /* PCI bridge system bus request priority. */
#define SPCR_TBEN     0x00400000 /* E300 PowerPC core time base unit enable. */
#define SPCR_COREPR   0x00300000 /* E300 PowerPC Core system bus request priority. */
#define SPCR_TSEC1DP  0x00003000 /* TSEC1 data priority. */
#define SPCR_TSEC1BDP 0x00000C00 /* TSEC1 buffer descriptor priority. */
#define SPCR_TSEC1EP  0x00000300 /* TSEC1 emergency priority. */
#define SPCR_TSEC2DP  0x00000030 /* TSEC2 data priority. */
#define SPCR_TSEC2BDP 0x0000000C /* TSEC2 buffer descriptor priority. */
#define SPCR_TSEC2EP  0x00000003 /* TSEC2 emergency priority. */
#define SPCR_RES      ~(SPCR_PCIHPE | SPCR_PCIPR | SPCR_TBEN | SPCR_COREPR \
			| SPCR_TSEC1DP | SPCR_TSEC1BDP | SPCR_TSEC1EP \
			| SPCR_TSEC2DP | SPCR_TSEC2BDP | SPCR_TSEC2EP)
	u32 sicrl; /* System General Purpose Register Low */
#define SICRL_LDP_A   0x80000000
#define SICRL_USB0    0x40000000
#define SICRL_USB1    0x20000000
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
	u32 sicrh; /* System General Purpose Register High */
#define SICRH_DDR     0x80000000
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
#define SICRh_RES     ~(  SICRH_DDR | SICRH_TSEC1_A | SICRH_TSEC1_B \
			| SICRH_TSEC1_C | SICRH_TSEC1_D | SICRH_TSEC1_E \
			| SICRH_TSEC1_F | SICRH_TSEC2_A | SICRH_TSEC2_B \
			| SICRH_TSEC2_C | SICRH_TSEC2_D | SICRH_TSEC2_E \
			| SICRH_TSEC2_F | SICRH_TSEC2_G | SICRH_TSEC2_H \
			| SICRH_GPIO2_A | SICRH_GPIO2_B | SICRH_GPIO2_C \
			| SICRH_GPIO2_D | SICRH_GPIO2_E | SICRH_GPIO2_F \
			| SICRH_GPIO2_G | SICRH_GPIO2_H | SICRH_TSOBI1 \
			| SICRH_TSOBI2)
	u8 res6[0xE4];
} sysconf8349_t;

/*
 * Watch Dog Timer (WDT) Registers
 */
typedef struct wdt8349 {
	u8 res0[4];
	u32 swcrr; /* System watchdog control register */
	u32 swcnr; /* System watchdog count register */
#define SWCNR_SWCN 0x0000FFFF Software Watchdog Count Field.
#define SWCNR_RES  ~(SWCNR_SWCN)
	u8 res1[2];
	u16 swsrr; /* System watchdog service register */
	u8 res2[0xF0];
} wdt8349_t;

/*
 * RTC/PIT Module Registers
 */
typedef struct rtclk8349 {
	u32 cnr; /* control register */
#define CNR_CLEN 0x00000080 /* Clock Enable Control Bit  */
#define CNR_CLIN 0x00000040 /* Input Clock Control Bit  */
#define CNR_AIM  0x00000002 /* Alarm Interrupt Mask Bit  */
#define CNR_SIM  0x00000001 /* Second Interrupt Mask Bit  */
#define CNR_RES  ~(CNR_CLEN | CNR_CLIN | CNR_AIM | CNR_SIM)
	u32 ldr; /* load register */
	u32 psr; /* prescale register */
	u32 ctr; /* register */
	u32 evr; /* event register */
#define RTEVR_SIF  0x00000001 /* Second Interrupt Flag Bit  */
#define RTEVR_AIF  0x00000002 /* Alarm Interrupt Flag Bit  */
#define RTEVR_RES  ~(EVR_SIF | EVR_AIF)
	u32 alr; /* alarm register */
	u8 res0[0xE8];
} rtclk8349_t;

/*
 * Global timper module
 */

typedef struct gtm8349 {
	u8    cfr1; /* Timer1/2 Configuration  */
#define CFR1_PCAS 0x80 /* Pair Cascade mode  */
#define CFR1_BCM  0x40  /* Backward compatible mode  */
#define CFR1_STP2 0x20 /* Stop timer  */
#define CFR1_RST2 0x10 /* Reset timer  */
#define CFR1_GM2  0x08 /* Gate mode for pin 2  */
#define CFR1_GM1  0x04 /* Gate mode for pin 1  */
#define CFR1_STP1 0x02 /* Stop timer  */
#define CFR1_RST1 0x01 /* Reset timer  */
	u8    res0[3];
	u8    cfr2; /* Timer3/4 Configuration  */
#define CFR2_PCAS 0x80 /* Pair Cascade mode  */
#define CFR2_SCAS 0x40 /* Super Cascade mode  */
#define CFR2_STP4 0x20 /* Stop timer  */
#define CFR2_RST4 0x10 /* Reset timer  */
#define CFR2_GM4  0x08 /* Gate mode for pin 4  */
#define CFR2_GM3  0x04 /* Gate mode for pin 3  */
#define CFR2_STP3 0x02 /* Stop timer  */
#define CFR2_RST3 0x01 /* Reset timer  */
	u8    res1[10];
	u16   mdr1; /* Timer1 Mode Register  */
#define MDR_SPS  0xff00 /* Secondary Prescaler value  */
#define MDR_CE   0x00c0 /* Capture edge and enable interrupt  */
#define MDR_OM   0x0020 /* Output mode  */
#define MDR_ORI  0x0010 /* Output reference interrupt enable  */
#define MDR_FRR  0x0008 /* Free run/restart  */
#define MDR_ICLK 0x0006 /* Input clock source for the timer  */
#define MDR_GE   0x0001 /* Gate enable  */
	u16   mdr2; /* Timer2 Mode Register  */
	u16   rfr1; /* Timer1 Reference Register  */
	u16   rfr2; /* Timer2 Reference Register  */
	u16   cpr1; /* Timer1 Capture Register  */
	u16   cpr2; /* Timer2 Capture Register  */
	u16   cnr1; /* Timer1 Counter Register  */
	u16   cnr2; /* Timer2 Counter Register  */
	u16   mdr3; /* Timer3 Mode Register  */
	u16   mdr4; /* Timer4 Mode Register  */
	u16   rfr3; /* Timer3 Reference Register  */
	u16   rfr4; /* Timer4 Reference Register  */
	u16   cpr3; /* Timer3 Capture Register  */
	u16   cpr4; /* Timer4 Capture Register  */
	u16   cnr3; /* Timer3 Counter Register  */
	u16   cnr4; /* Timer4 Counter Register  */
	u16   evr1; /* Timer1 Event Register  */
	u16   evr2; /* Timer2 Event Register  */
	u16   evr3; /* Timer3 Event Register  */
	u16   evr4; /* Timer4 Event Register  */
#define GTEVR_REF 0x0002 /* Output reference event  */
#define GTEVR_CAP 0x0001 /* Counter Capture event   */
#define GTEVR_RES ~(EVR_CAP|EVR_REF)
	u16   psr1; /* Timer1 Prescaler Register  */
	u16   psr2; /* Timer2 Prescaler Register  */
	u16   psr3; /* Timer3 Prescaler Register  */
	u16   psr4; /* Timer4 Prescaler Register  */
	u8    res[0xC0];
} gtm8349_t;

/*
 * Integrated Programmable Interrupt Controller
 */
typedef struct ipic8349 {
	u32    sicfr; /*  System Global Interrupt Configuration Register (SICFR)  */
#define SICFR_HPI  0x7f000000 /*  Highest Priority Interrupt  */
#define SICFR_MPSB 0x00400000 /*  Mixed interrupts Priority Scheme for group B  */
#define SICFR_MPSA 0x00200000 /*  Mixed interrupts Priority Scheme for group A  */
#define SICFR_IPSD 0x00080000 /*  Internal interrupts Priority Scheme for group D  */
#define SICFR_IPSA 0x00010000 /*  Internal interrupts Priority Scheme for group A  */
#define SICFR_HPIT 0x00000300 /*  HPI priority position IPIC output interrupt Type  */
#define SICFR_RES ~(SICFR_HPI|SICFR_MPSB|SICFR_MPSA|SICFR_IPSD|SICFR_IPSA|SICFR_HPIT)
	u32    sivcr; /*  System Global Interrupt Vector Register (SIVCR)  */
#define SICVR_IVECX 0xfc000000 /*  Interrupt vector (for CE compatibility purpose only not used in 8349 IPIC implementation)  */
#define SICVR_IVEC  0x0000007f /*  Interrupt vector  */
#define SICVR_RES ~(SICVR_IVECX|SICVR_IVEC)
	u32    sipnr_h; /*  System Internal Interrupt Pending Register - High (SIPNR_H)  */
#define SIIH_TSEC1TX 0x80000000 /*  TSEC1 Tx interrupt  */
#define SIIH_TSEC1RX 0x40000000 /*  TSEC1 Rx interrupt  */
#define SIIH_TSEC1ER 0x20000000 /*  TSEC1 Eror interrupt  */
#define SIIH_TSEC2TX 0x10000000 /*  TSEC2 Tx interrupt  */
#define SIIH_TSEC2RX 0x08000000 /*  TSEC2 Rx interrupt  */
#define SIIH_TSEC2ER 0x04000000 /*  TSEC2 Eror interrupt  */
#define SIIH_USB2DR  0x02000000 /*  USB2 DR interrupt  */
#define SIIH_USB2MPH 0x01000000 /*  USB2 MPH interrupt  */
#define SIIH_UART1   0x00000080 /*  UART1 interrupt  */
#define SIIH_UART2   0x00000040 /*  UART2 interrupt  */
#define SIIH_SEC     0x00000020 /*  SEC interrupt  */
#define SIIH_I2C1    0x00000004 /*  I2C1 interrupt  */
#define SIIH_I2C2    0x00000002 /*  I2C1 interrupt  */
#define SIIH_SPI     0x00000001 /*  SPI interrupt  */
#define SIIH_RES	~(SIIH_TSEC1TX | SIIH_TSEC1RX | SIIH_TSEC1ER \
			| SIIH_TSEC2TX | SIIH_TSEC2RX | SIIH_TSEC2ER \
			| SIIH_USB2DR | SIIH_USB2MPH | SIIH_UART1 \
			| SIIH_UART2 | SIIH_SEC | SIIH_I2C1 \
			| SIIH_I2C2 | SIIH_SPI)
	u32    sipnr_l; /*  System Internal Interrupt Pending Register - Low (SIPNR_L)  */
#define SIIL_RTCS  0x80000000 /*  RTC SECOND interrupt  */
#define SIIL_PIT   0x40000000 /*  PIT interrupt  */
#define SIIL_PCI1  0x20000000 /*  PCI1 interrupt  */
#define SIIL_PCI2  0x10000000 /*  PCI2 interrupt  */
#define SIIL_RTCA  0x08000000 /*  RTC ALARM interrupt  */
#define SIIL_MU    0x04000000 /*  Message Unit interrupt  */
#define SIIL_SBA   0x02000000 /*  System Bus Arbiter interrupt  */
#define SIIL_DMA   0x01000000 /*  DMA interrupt  */
#define SIIL_GTM4  0x00800000 /*  GTM4 interrupt  */
#define SIIL_GTM8  0x00400000 /*  GTM8 interrupt  */
#define SIIL_GPIO1 0x00200000 /*  GPIO1 interrupt  */
#define SIIL_GPIO2 0x00100000 /*  GPIO2 interrupt  */
#define SIIL_DDR   0x00080000 /*  DDR interrupt  */
#define SIIL_LBC   0x00040000 /*  LBC interrupt  */
#define SIIL_GTM2  0x00020000 /*  GTM2 interrupt  */
#define SIIL_GTM6  0x00010000 /*  GTM6 interrupt  */
#define SIIL_PMC   0x00008000 /*  PMC interrupt  */
#define SIIL_GTM3  0x00000800 /*  GTM3 interrupt  */
#define SIIL_GTM7  0x00000400 /*  GTM7 interrupt  */
#define SIIL_GTM1  0x00000020 /*  GTM1 interrupt  */
#define SIIL_GTM5  0x00000010 /*  GTM5 interrupt  */
#define SIIL_DPTC  0x00000001 /*  DPTC interrupt (!!! Invisible for user !!!)  */
#define SIIL_RES	~(SIIL_RTCS | SIIL_PIT | SIIL_PCI1 | SIIL_PCI2 \
			| SIIL_RTCA | SIIL_MU | SIIL_SBA | SIIL_DMA \
			| SIIL_GTM4 | SIIL_GTM8 | SIIL_GPIO1 | SIIL_GPIO2 \
			| SIIL_DDR | SIIL_LBC | SIIL_GTM2 | SIIL_GTM6 \
			| SIIL_PMC |SIIL_GTM3 | SIIL_GTM7 | SIIL_GTM1 \
			| SIIL_GTM5 |SIIL_DPTC )
	u32    siprr_a; /*  System Internal Interrupt Group A Priority Register (PRR)  */
	u8     res0[8];
	u32    siprr_d; /*  System Internal Interrupt Group D Priority Register (PRR)  */
	u32    simsr_h; /*  System Internal Interrupt Mask Register - High (SIIH)  */
	u32    simsr_l; /*  System Internal Interrupt Mask Register - Low (SIIL)  */
	u8     res1[4];
	u32    sepnr;   /*  System External Interrupt Pending Register (SEI)  */
	u32    smprr_a; /*  System Mixed Interrupt Group A Priority Register (PRR)  */
	u32    smprr_b; /*  System Mixed Interrupt Group B Priority Register (PRR)  */
#define PRR_0 0xe0000000 /* Priority Register, Position 0 programming */
#define PRR_1 0x1c000000 /* Priority Register, Position 1 programming */
#define PRR_2 0x03800000 /* Priority Register, Position 2 programming */
#define PRR_3 0x00700000 /* Priority Register, Position 3 programming */
#define PRR_4 0x0000e000 /* Priority Register, Position 4 programming */
#define PRR_5 0x00001c00 /* Priority Register, Position 5 programming */
#define PRR_6 0x00000380 /* Priority Register, Position 6 programming */
#define PRR_7 0x00000070 /* Priority Register, Position 7 programming */
#define PRR_RES ~(PRR_0|PRR_1|PRR_2|PRR_3|PRR_4|PRR_5|PRR_6|PRR_7)
	u32    semsr; /*  System External Interrupt Mask Register (SEI)  */
#define SEI_IRQ0  0x80000000 /*  IRQ0 external interrupt  */
#define SEI_IRQ1  0x40000000 /*  IRQ1 external interrupt  */
#define SEI_IRQ2  0x20000000 /*  IRQ2 external interrupt  */
#define SEI_IRQ3  0x10000000 /*  IRQ3 external interrupt  */
#define SEI_IRQ4  0x08000000 /*  IRQ4 external interrupt  */
#define SEI_IRQ5  0x04000000 /*  IRQ5 external interrupt  */
#define SEI_IRQ6  0x02000000 /*  IRQ6 external interrupt  */
#define SEI_IRQ7  0x01000000 /*  IRQ7 external interrupt  */
#define SEI_SIRQ0 0x00008000 /*  SIRQ0 external interrupt  */
#define SEI_RES		~( SEI_IRQ0 | SEI_IRQ1 | SEI_IRQ2 | SEI_IRQ3 \
			| SEI_IRQ4 | SEI_IRQ5 | SEI_IRQ6 | SEI_IRQ7 \
			| SEI_SIRQ0)
	u32    secnr; /*  System External Interrupt Control Register (SECNR) */
#define SECNR_MIXB0T 0xc0000000 /*  MIXB0 priority position IPIC output interrupt type  */
#define SECNR_MIXB1T 0x30000000 /*  MIXB1 priority position IPIC output interrupt type  */
#define SECNR_MIXA0T 0x00c00000 /*  MIXA0 priority position IPIC output interrupt type  */
#define SECNR_SYSA1T 0x00300000 /*  MIXA1 priority position IPIC output interrupt type  */
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
	u32   sersr; /*  System Error Status Register (SERR)  */
	u32   sermr; /*  System Error Mask Register (SERR)  */
#define SERR_IRQ0 0x80000000 /*  IRQ0 MCP request  */
#define SERR_WDT  0x40000000 /*  WDT MCP request  */
#define SERR_SBA  0x20000000 /*  SBA MCP request  */
#define SERR_DDR  0x10000000 /*  DDR MCP request  */
#define SERR_LBC  0x08000000 /*  LBC MCP request  */
#define SERR_PCI1 0x04000000 /*  PCI1 MCP request  */
#define SERR_PCI2 0x02000000 /*  PCI2 MCP request  */
#define SERR_MU   0x01000000 /*  MU MCP request  */
#define SERR_RNC  0x00010000 /*  MU MCP request (!!! Non-visible for users !!!)  */
#define SERR_RES	~( SERR_IRQ0 | SERR_WDT | SERR_SBA | SERR_DDR \
			|SERR_LBC | SERR_PCI1 | SERR_PCI2 | SERR_MU \
			|SERR_RNC )
	u32    sercr; /*  System Error Control Register  (SERCR)  */
#define SERCR_MCPR 0x00000001 /*  MCP Route  */
#define SERCR_RES ~(SERCR_MCPR)
	u8    res2[4];
	u32   sifcr_h; /*  System Internal Interrupt Force Register - High (SIIH)  */
	u32   sifcr_l; /*  System Internal Interrupt Force Register - Low (SIIL)  */
	u32   sefcr;   /*  System External Interrupt Force Register (SEI)  */
	u32   serfr;   /*  System Error Force Register (SERR)  */
	u8    res3[0xA0];
} ipic8349_t;

/*
 * System Arbiter Registers
 */
typedef struct arbiter8349 {
	u32 acr; /* Arbiter Configuration Register */
#define ACR_COREDIS    0x10000000 /* Core disable. */
#define ACR_PIPE_DEP   0x00070000 /* Pipeline depth (number of outstanding transactions). */
#define ACR_PCI_RPTCNT 0x00007000 /* PCI repeat count. */
#define ACR_RPTCNT     0x00000700 /* Repeat count. */
#define ACR_APARK      0x00000030 /* Address parking. */
#define ACR_PARKM	   0x0000000F /* Parking master. */
#define ACR_RES ~(ACR_COREDIS|ACR_PIPE_DEP|ACR_PCI_RPTCNT|ACR_RPTCNT|ACR_APARK|ACR_PARKM)
	u32 atr; /* Arbiter Timers Register */
#define ATR_DTO 0x00FF0000 /* Data time out. */
#define ATR_ATO	0x000000FF /* Address time out. */
#define ATR_RES ~(ATR_DTO|ATR_ATO)
	u8 res[4];
	u32 aer; /* Arbiter Event Register (AE)*/
	u32 aidr; /* Arbiter Interrupt Definition Register (AE) */
	u32 amr; /* Arbiter Mask Register (AE) */
	u32 aeatr; /* Arbiter Event Attributes Register */
#define AEATR_EVENT   0x07000000 /* Event type. */
#define AEATR_MSTR_ID 0x001F0000 /* Master Id. */
#define AEATR_TBST    0x00000800 /* Transfer burst. */
#define AEATR_TSIZE   0x00000700 /* Transfer Size. */
#define AEATR_TTYPE	  0x0000001F /* Transfer Type. */
#define AEATR_RES ~(AEATR_EVENT|AEATR_MSTR_ID|AEATR_TBST|AEATR_TSIZE|AEATR_TTYPE)
	u32 aeadr; /* Arbiter Event Address Register */
	u32 aerr; /* Arbiter Event Response Register (AE)*/
#define AE_ETEA 0x00000020 /* Transfer error. */
#define AE_RES_ 0x00000010 /* Reserved transfer type. */
#define AE_ECW  0x00000008 /* External control word transfer type. */
#define AE_AO   0x00000004 /* Address Only transfer type. */
#define AE_DTO  0x00000002 /* Data time out. */
#define AE_ATO	0x00000001 /* Address time out. */
#define AE_RSRV ~(AE_ETEA|AE_RES_|AE_ECW|AE_AO|AE_DTO|AE_ATO)
	u8 res1[0xDC];
} arbiter8349_t;

/*
 * Reset Module
 */
typedef struct reset8349 {
	u32    rcwl; /* RCWL Register  */
#define RCWL_LBIUCM  0x80000000 /* LBIUCM  */
#define RCWL_LBIUCM_SHIFT    31
#define RCWL_DDRCM   0x40000000 /* DDRCM  */
#define RCWL_DDRCM_SHIFT     30
#define RCWL_SVCOD   0x30000000 /* SVCOD  */
#define RCWL_SPMF    0x0f000000 /* SPMF  */
#define RCWL_SPMF_SHIFT      24
#define RCWL_COREPLL 0x007F0000 /* COREPLL  */
#define RCWL_COREPLL_SHIFT   16
#define RCWL_CEVCOD  0x000000C0 /* CEVCOD  */
#define RCWL_CEPDF   0x00000020 /* CEPDF  */
#define RCWL_CEPMF   0x0000001F /* CEPMF  */
#define RCWL_RES ~(RCWL_BIUCM|RCWL_DDRCM|RCWL_SVCOD|RCWL_SPMF|RCWL_COREPLL|RCWL_CEVCOD|RCWL_CEPDF|RCWL_CEPMF)
	u32    rcwh; /* RCHL Register  */
#define RCWH_PCIHOST 0x80000000 /* PCIHOST  */
#define RCWH_PCIHOST_SHIFT   31
#define RCWH_PCI64   0x40000000 /* PCI64  */
#define RCWH_PCI1ARB 0x20000000 /* PCI1ARB  */
#define RCWH_PCI2ARB 0x10000000 /* PCI2ARB  */
#define RCWH_COREDIS 0x08000000 /* COREDIS  */
#define RCWH_BMS     0x04000000 /* BMS  */
#define RCWH_BOOTSEQ 0x03000000 /* BOOTSEQ  */
#define RCWH_SWEN    0x00800000 /* SWEN  */
#define RCWH_ROMLOC  0x00700000 /* ROMLOC  */
#define RCWH_TSEC1M  0x0000c000 /* TSEC1M  */
#define RCWH_TSEC2M  0x00003000 /* TSEC2M  */
#define RCWH_TPR     0x00000100 /* TPR  */
#define RCWH_TLE     0x00000008 /* TLE  */
#define RCWH_LALE    0x00000004 /* LALE  */
#define RCWH_RES	~(RCWH_PCIHOST | RCWH_PCI64 | RCWH_PCI1ARB \
			| RCWH_PCI2ARB | RCWH_COREDIS | RCWH_BMS \
			| RCWH_BOOTSEQ | RCWH_SWEN | RCWH_ROMLOC \
			| RCWH_TSEC1M | RCWH_TSEC2M | RCWH_TPR \
			| RCWH_TLE | RCWH_LALE)
	u8     res0[8];
	u32    rsr; /* Reset status Register  */
#define RSR_RSTSRC 0xE0000000 /* Reset source  */
#define RSR_RSTSRC_SHIFT   29
#define RSR_BSF    0x00010000 /* Boot seq. fail  */
#define RSR_BSF_SHIFT      16
#define RSR_SWSR   0x00002000 /* software soft reset  */
#define RSR_SWSR_SHIFT     13
#define RSR_SWHR   0x00001000 /* software hard reset  */
#define RSR_SWHR_SHIFT     12
#define RSR_JHRS   0x00000200 /* jtag hreset  */
#define RSR_JHRS_SHIFT      9
#define RSR_JSRS   0x00000100 /* jtag sreset status  */
#define RSR_JSRS_SHIFT      8
#define RSR_CSHR   0x00000010 /* checkstop reset status  */
#define RSR_CSHR_SHIFT      4
#define RSR_SWRS   0x00000008 /* software watchdog reset status  */
#define RSR_SWRS_SHIFT      3
#define RSR_BMRS   0x00000004 /* bus monitop reset status  */
#define RSR_BMRS_SHIFT      2
#define RSR_SRS    0x00000002 /* soft reset status  */
#define RSR_SRS_SHIFT       1
#define RSR_HRS    0x00000001 /* hard reset status  */
#define RSR_HRS_SHIFT       0
#define RSR_RES ~(RSR_RSTSRC | RSR_BSF | RSR_SWSR | RSR_SWHR | RSR_JHRS | RSR_JSRS | RSR_CSHR | RSR_SWRS | RSR_BMRS | RSR_SRS | RSR_HRS)
	u32    rmr; /* Reset mode Register  */
#define RMR_CSRE   0x00000001 /* checkstop reset enable  */
#define RMR_CSRE_SHIFT      0
#define RMR_RES ~(RMR_CSRE)
	u32    rpr; /* Reset protection Register  */
	u32    rcr; /* Reset Control Register  */
#define RCR_SWHR 0x00000002 /* software hard reset  */
#define RCR_SWSR 0x00000001 /* software soft reset  */
#define RCR_RES ~(RCR_SWHR | RCR_SWSR)
	u32    rcer; /* Reset Control Enable Register  */
#define RCER_CRE 0x00000001 /* software hard reset  */
#define RCER_RES ~(RCER_CRE)
	u8     res1[0xDC];
} reset8349_t;

typedef struct clk8349 {
	u32    spmr; /* system PLL mode Register  */
#define SPMR_LBIUCM  0x80000000 /* LBIUCM  */
#define SPMR_DDRCM   0x40000000 /* DDRCM  */
#define SPMR_SVCOD   0x30000000 /* SVCOD  */
#define SPMR_SPMF    0x0F000000 /* SPMF  */
#define SPMR_CKID    0x00800000 /* CKID  */
#define SPMR_CKID_SHIFT 23
#define SPMR_COREPLL 0x007F0000 /* COREPLL  */
#define SPMR_CEVCOD  0x000000C0 /* CEVCOD  */
#define SPMR_CEPDF   0x00000020 /* CEPDF  */
#define SPMR_CEPMF   0x0000001F /* CEPMF  */
#define SPMR_RES	~(SPMR_LBIUCM | SPMR_DDRCM | SPMR_SVCOD \
			| SPMR_SPMF | SPMR_CKID | SPMR_COREPLL \
			| SPMR_CEVCOD | SPMR_CEPDF | SPMR_CEPMF)
	u32    occr; /* output clock control Register  */
#define OCCR_PCICOE0 0x80000000 /* PCICOE0  */
#define OCCR_PCICOE1 0x40000000 /* PCICOE1  */
#define OCCR_PCICOE2 0x20000000 /* PCICOE2  */
#define OCCR_PCICOE3 0x10000000 /* PCICOE3  */
#define OCCR_PCICOE4 0x08000000 /* PCICOE4  */
#define OCCR_PCICOE5 0x04000000 /* PCICOE5  */
#define OCCR_PCICOE6 0x02000000 /* PCICOE6  */
#define OCCR_PCICOE7 0x01000000 /* PCICOE7  */
#define OCCR_PCICD0  0x00800000 /* PCICD0  */
#define OCCR_PCICD1  0x00400000 /* PCICD1  */
#define OCCR_PCICD2  0x00200000 /* PCICD2  */
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
	u32    sccr; /* system clock control Register  */
#define SCCR_TSEC1CM  0xc0000000 /* TSEC1CM  */
#define SCCR_TSEC1CM_SHIFT 30
#define SCCR_TSEC2CM  0x30000000 /* TSEC2CM  */
#define SCCR_TSEC2CM_SHIFT 28
#define SCCR_ENCCM    0x03000000 /* ENCCM  */
#define SCCR_ENCCM_SHIFT 24
#define SCCR_USBMPHCM 0x00c00000 /* USBMPHCM  */
#define SCCR_USBMPHCM_SHIFT 22
#define SCCR_USBDRCM  0x00300000 /* USBDRCM  */
#define SCCR_USBDRCM_SHIFT 20
#define SCCR_PCICM    0x00010000 /* PCICM  */
#define SCCR_RES	~( SCCR_TSEC1CM | SCCR_TSEC2CM | SCCR_ENCCM \
			| SCCR_USBMPHCM | SCCR_USBDRCM | SCCR_PCICM)
	u8     res0[0xF4];
} clk8349_t;

/*
 * Power Management Control Module
 */
typedef struct pmc8349 {
	u32    pmccr; /* PMC Configuration Register  */
#define PMCCR_SLPEN 0x00000001 /* System Low Power Enable  */
#define PMCCR_DLPEN 0x00000002 /* DDR SDRAM Low Power Enable  */
#define PMCCR_RES ~(PMCCR_SLPEN | PMCCR_DLPEN)
	u32    pmcer; /* PMC Event Register  */
#define PMCER_PMCI  0x00000001 /* PMC Interrupt  */
#define PMCER_RES ~(PMCER_PMCI)
	u32    pmcmr; /* PMC Mask Register  */
#define PMCMR_PMCIE 0x0001 /* PMC Interrupt Enable  */
#define PMCMR_RES ~(PMCMR_PMCIE)
	u8 res0[0xF4];
} pmc8349_t;


/*
 * general purpose I/O module
 */
typedef struct gpio8349 {
	u32 dir; /* direction register */
	u32 odr; /* open drain register */
	u32 dat; /* data register */
	u32 ier; /* interrupt event register */
	u32 imr; /* interrupt mask register */
	u32 icr; /* external interrupt control register */
	u8 res0[0xE8];
} gpio8349_t;

/*
 * DDR Memory Controller Memory Map
 */
typedef struct ddr_cs_bnds{
	u32 csbnds;
#define CSBNDS_SA 0x00FF0000
#define CSBNDS_SA_SHIFT    8
#define CSBNDS_EA 0x000000FF
#define CSBNDS_EA_SHIFT   24
	u8  res0[4];
} ddr_cs_bnds_t;

typedef struct ddr8349{
	ddr_cs_bnds_t csbnds[4];            /**< Chip Select x Memory Bounds */
	u8 res0[0x60];
	u32 cs_config[4];       /**< Chip Select x Configuration */
#define CSCONFIG_EN         0x80000000
#define CSCONFIG_AP         0x00800000
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
	u32 timing_cfg_1;       /**< SDRAM Timing Configuration 1 */
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
#define TIMING_CFG1_WRREC_SHIFT       8
#define TIMING_CFG1_ACTTOACT 0x00000070
#define TIMING_CFG1_ACTTOACT_SHIFT    4
#define TIMING_CFG1_WRTORD   0x00000007
#define TIMING_CFG1_WRTORD_SHIFT      0
#define TIMING_CFG1_CASLAT_20 0x00030000  /* CAS latency = 2.0 */
#define TIMING_CFG1_CASLAT_25 0x00040000  /* CAS latency = 2.5 */

	u32 timing_cfg_2;       /**< SDRAM Timing Configuration 2 */
#define TIMING_CFG2_CPO           0x0F000000
#define TIMING_CFG2_CPO_SHIFT             24
#define TIMING_CFG2_ACSM          0x00080000
#define TIMING_CFG2_WR_DATA_DELAY 0x00001C00
#define TIMING_CFG2_WR_DATA_DELAY_SHIFT   10
#define TIMING_CFG2_CPO_DEF       0x00000000  /* default (= CASLAT + 1) */

	u32 sdram_cfg;          /**< SDRAM Control Configuration */
#define SDRAM_CFG_MEM_EN     0x80000000
#define SDRAM_CFG_SREN       0x40000000
#define SDRAM_CFG_ECC_EN     0x20000000
#define SDRAM_CFG_RD_EN      0x10000000
#define SDRAM_CFG_SDRAM_TYPE 0x03000000
#define SDRAM_CFG_SDRAM_TYPE_SHIFT   24
#define SDRAM_CFG_DYN_PWR    0x00200000
#define SDRAM_CFG_32_BE      0x00080000
#define SDRAM_CFG_8_BE       0x00040000
#define SDRAM_CFG_NCAP       0x00020000
#define SDRAM_CFG_2T_EN      0x00008000
#define SDRAM_CFG_SDRAM_TYPE_DDR 0x02000000

	u8 res2[4];
	u32 sdram_mode;         /**< SDRAM Mode Configuration */
#define SDRAM_MODE_ESD 0xFFFF0000
#define SDRAM_MODE_ESD_SHIFT   16
#define SDRAM_MODE_SD  0x0000FFFF
#define SDRAM_MODE_SD_SHIFT     0
#define DDR_MODE_EXT_MODEREG    0x4000  /* select extended mode reg */
#define DDR_MODE_EXT_OPMODE     0x3FF8  /* operating mode, mask */
#define DDR_MODE_EXT_OP_NORMAL  0x0000  /* normal operation */
#define DDR_MODE_QFC            0x0004  /* QFC / compatibility, mask */
#define DDR_MODE_QFC_COMP       0x0000  /* compatible to older SDRAMs */
#define DDR_MODE_WEAK           0x0002  /* weak drivers */
#define DDR_MODE_DLL_DIS        0x0001  /* disable DLL */
#define DDR_MODE_CASLAT         0x0070  /* CAS latency, mask */
#define DDR_MODE_CASLAT_15      0x0010  /* CAS latency 1.5 */
#define DDR_MODE_CASLAT_20      0x0020  /* CAS latency 2 */
#define DDR_MODE_CASLAT_25      0x0060  /* CAS latency 2.5 */
#define DDR_MODE_CASLAT_30      0x0030  /* CAS latency 3 */
#define DDR_MODE_BTYPE_SEQ      0x0000  /* sequential burst */
#define DDR_MODE_BTYPE_ILVD     0x0008  /* interleaved burst */
#define DDR_MODE_BLEN_2         0x0001  /* burst length 2 */
#define DDR_MODE_BLEN_4         0x0002  /* burst length 4 */
#define DDR_REFINT_166MHZ_7US   1302        /* exact value for 7.8125 µs */
#define DDR_BSTOPRE     256     /* use 256 cycles as a starting point */
#define DDR_MODE_MODEREG        0x0000  /* select mode register */

	u8 res3[8];
	u32 sdram_interval;     /**< SDRAM Interval Configuration */
#define SDRAM_INTERVAL_REFINT  0x3FFF0000
#define SDRAM_INTERVAL_REFINT_SHIFT    16
#define SDRAM_INTERVAL_BSTOPRE 0x00003FFF
#define SDRAM_INTERVAL_BSTOPRE_SHIFT    0
	u8   res9[8];
	u32  sdram_clk_cntl;
#define DDR_SDRAM_CLK_CNTL_SS_EN		0x80000000
#define DDR_SDRAM_CLK_CNTL_CLK_ADJUST_05	0x02000000

	u8 res4[0xCCC];
	u32 data_err_inject_hi; /**< Memory Data Path Error Injection Mask High */
	u32 data_err_inject_lo; /**< Memory Data Path Error Injection Mask Low */
	u32 ecc_err_inject;     /**< Memory Data Path Error Injection Mask ECC */
	u8 res5[0x14];
	u32 capture_data_hi;    /**< Memory Data Path Read Capture High */
	u32 capture_data_lo;    /**< Memory Data Path Read Capture Low */
	u32 capture_ecc;        /**< Memory Data Path Read Capture ECC */
	u8 res6[0x14];
	u32 err_detect;         /**< Memory Error Detect */
	u32 err_disable;        /**< Memory Error Disable */
	u32 err_int_en;         /**< Memory Error Interrupt Enable */
	u32 capture_attributes; /**< Memory Error Attributes Capture */
	u32 capture_address;    /**< Memory Error Address Capture */
	u32 capture_ext_address;/**< Memory Error Extended Address Capture */
	u32 err_sbe;            /**< Memory Single-Bit ECC Error Management */
	u8 res7[0xA4];
	u32 debug_reg;
	u8 res8[0xFC];
} ddr8349_t;

/*
 * I2C1 Controller
 */


/*
 * DUART
 */
typedef struct duart8349{
	u8 urbr_ulcr_udlb; /**< combined register for URBR, UTHR and UDLB */
	u8 uier_udmb;      /**< combined register for UIER and UDMB */
	u8 uiir_ufcr_uafr; /**< combined register for UIIR, UFCR and UAFR */
	u8 ulcr;        /**< line control register */
	u8 umcr;        /**< MODEM control register */
	u8 ulsr;        /**< line status register */
	u8 umsr;        /**< MODEM status register */
	u8 uscr;        /**< scratch register */
	u8 res0[8];
	u8 udsr;        /**< DMA status register */
	u8 res1[3];
	u8 res2[0xEC];
} duart8349_t;

/*
 * Local Bus Controller Registers
 */
typedef struct lbus_bank{
	u32 br;             /**< Base Register  */
	u32 or;             /**< Base Register  */
} lbus_bank_t;

typedef struct lbus8349 {
	lbus_bank_t bank[8];
	u8 res0[0x28];
	u32 mar;                /**< UPM Address Register */
	u8 res1[0x4];
	u32 mamr;               /**< UPMA Mode Register */
	u32 mbmr;               /**< UPMB Mode Register */
	u32 mcmr;               /**< UPMC Mode Register */
	u8 res2[0x8];
	u32 mrtpr;              /**< Memory Refresh Timer Prescaler Register */
	u32 mdr;                /**< UPM Data Register */
	u8 res3[0x8];
	u32 lsdmr;              /**< SDRAM Mode Register */
	u8 res4[0x8];
	u32 lurt;               /**< UPM Refresh Timer */
	u32 lsrt;               /**< SDRAM Refresh Timer */
	u8 res5[0x8];
	u32 ltesr;              /**< Transfer Error Status Register */
	u32 ltedr;              /**< Transfer Error Disable Register */
	u32 lteir;              /**< Transfer Error Interrupt Register */
	u32 lteatr;             /**< Transfer Error Attributes Register */
	u32 ltear;              /**< Transfer Error Address Register */
	u8 res6[0xC];
	u32 lbcr;               /**< Configuration Register */
#define LBCR_LDIS  0x80000000
#define LBCR_LDIS_SHIFT    31
#define LBCR_BCTLC 0x00C00000
#define LBCR_BCTLC_SHIFT   22
#define LBCR_LPBSE 0x00020000
#define LBCR_LPBSE_SHIFT   17
#define LBCR_EPAR  0x00010000
#define LBCR_EPAR_SHIFT    16
#define LBCR_BMT   0x0000FF00
#define LBCR_BMT_SHIFT      8
	u32 lcrr;               /**< Clock Ratio Register */
#define LCRR_DBYP    0x80000000
#define LCRR_DBYP_SHIFT      31
#define LCRR_BUFCMDC 0x30000000
#define LCRR_BUFCMDC_SHIFT   28
#define LCRR_ECL     0x03000000
#define LCRR_ECL_SHIFT       24
#define LCRR_EADC    0x00030000
#define LCRR_EADC_SHIFT      16
#define LCRR_CLKDIV  0x0000000F
#define LCRR_CLKDIV_SHIFT     0


	u8 res7[0x28];
	u8 res8[0xF00];
} lbus8349_t;

/*
 * Serial Peripheral Interface
 */
typedef struct spi8349
{
	u32 mode;     /**< mode register  */
	u32 event;    /**< event register */
	u32 mask;     /**< mask register  */
	u32 com;      /**< command register */
	u8 res0[0x10];
	u32 tx;       /**< transmit register */
	u32 rx;       /**< receive register */
	u8 res1[0xD8];
} spi8349_t;

typedef struct dma8349 {
	u8 fixme[0x300];
} dma8349_t;

/*
 * PCI Software Configuration Registers
 */
typedef struct pciconf8349 {
	u32	config_address;
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
	u8	res[116];
} pciconf8349_t;

/*
 * PCI Outbound Translation Register
 */
typedef struct pci_outbound_window {
	u32	potar;
	u8	res0[4];
	u32	pobar;
	u8	res1[4];
	u32	pocmr;
	u8	res2[4];
} pot8349_t;
/*
 * Sequencer
 */
typedef struct ios8349 {
	pot8349_t	pot[6];
#define POTAR_TA_MASK	0x000fffff
#define	POBAR_BA_MASK	0x000fffff
#define	POCMR_EN	0x80000000
#define	POCMR_IO	0x40000000	/* 0--memory space 1--I/O space */
#define	POCMR_SE	0x20000000	/* streaming enable */
#define	POCMR_DST	0x10000000	/* 0--PCI1 1--PCI2*/
#define	POCMR_CM_MASK	0x000fffff
#define	POCMR_CM_4G	0x00000000
#define	POCMR_CM_2G	0x00080000
#define	POCMR_CM_1G	0x000C0000
#define	POCMR_CM_512M	0x000E0000
#define	POCMR_CM_256M	0x000F0000
#define	POCMR_CM_128M	0x000F8000
#define	POCMR_CM_64M	0x000FC000
#define	POCMR_CM_32M	0x000FE000
#define	POCMR_CM_16M	0x000FF000
#define	POCMR_CM_8M	0x000FF800
#define	POCMR_CM_4M	0x000FFC00
#define	POCMR_CM_2M	0x000FFE00
#define	POCMR_CM_1M	0x000FFF00
#define	POCMR_CM_512K	0x000FFF80
#define	POCMR_CM_256K	0x000FFFC0
#define	POCMR_CM_128K	0x000FFFE0
#define	POCMR_CM_64K	0x000FFFF0
#define	POCMR_CM_32K	0x000FFFF8
#define	POCMR_CM_16K	0x000FFFFC
#define	POCMR_CM_8K	0x000FFFFE
#define	POCMR_CM_4K	0x000FFFFF
	u8	res0[0x60];
	u32	pmcr;
	u8	res1[4];
	u32	dtcr;
	u8	res2[4];
} ios8349_t;

/*
 * PCI Controller Control and Status Registers
 */
typedef struct pcictrl8349 {
	u32	esr;
#define ESR_MERR	0x80000000
#define ESR_APAR	0x00000400
#define	ESR_PCISERR	0x00000200
#define	ESR_MPERR	0x00000100
#define	ESR_TPERR	0x00000080
#define	ESR_NORSP	0x00000040
#define	ESR_TABT	0x00000020
	u32	ecdr;
#define ECDR_APAR	0x00000400
#define	ECDR_PCISERR	0x00000200
#define	ECDR_MPERR	0x00000100
#define	ECDR_TPERR	0x00000080
#define	ECDR_NORSP	0x00000040
#define	ECDR_TABT	0x00000020
	u32 eer;
#define EER_APAR	0x00000400
#define	EER_PCISERR	0x00000200
#define	EER_MPERR	0x00000100
#define	EER_TPERR	0x00000080
#define	EER_NORSP	0x00000040
#define	EER_TABT	0x00000020
	u32	eatcr;
#define	EATCR_ERRTYPR_MASK	0x70000000
#define	EATCR_ERRTYPR_APR	0x00000000	/* address parity error */
#define	EATCR_ERRTYPR_WDPR	0x10000000	/* write data parity error */
#define	EATCR_ERRTYPR_RDPR	0x20000000	/* read data parity error */
#define	EATCR_ERRTYPR_MA	0x30000000	/* master abort */
#define	EATCR_ERRTYPR_TA	0x40000000	/* target abort */
#define	EATCR_ERRTYPR_SE	0x50000000	/* system error indication received */
#define	EATCR_ERRTYPR_PEA	0x60000000	/* parity error indication received on a read */
#define	EATCR_ERRTYPR_PEW	0x70000000	/* parity error indication received on a write */
#define EATCR_BN_MASK		0x0f000000	/* beat number */
#define	EATCR_BN_1st		0x00000000
#define	EATCR_BN_2ed		0x01000000
#define	EATCR_BN_3rd		0x02000000
#define	EATCR_BN_4th		0x03000000
#define	EATCR_BN_5th		0x0400000
#define	EATCR_BN_6th		0x05000000
#define	EATCR_BN_7th		0x06000000
#define	EATCR_BN_8th		0x07000000
#define	EATCR_BN_9th		0x08000000
#define EATCR_TS_MASK		0x00300000	/* transaction size */
#define	EATCR_TS_4		0x00000000
#define	EATCR_TS_1		0x00100000
#define	EATCR_TS_2		0x00200000
#define	EATCR_TS_3		0x00300000
#define	EATCR_ES_MASK		0x000f0000	/* error source */
#define	EATCR_ES_EM		0x00000000	/* external master */
#define	EATCR_ES_DMA		0x00050000
#define	EATCR_CMD_MASK		0x0000f000
#define	EATCR_HBE_MASK		0x00000f00	/* PCI high byte enable*/
#define	EATCR_BE_MASK		0x000000f0	/* PCI byte enable */
#define	EATCR_HPB		0x00000004	/* high parity bit */
#define	EATCR_PB		0x00000002	/* parity bit*/
#define	EATCR_VI		0x00000001	/* error information valid */
	u32	eacr;
	u32	eeacr;
	u32	edlcr;
	u32	edhcr;
	u32	gcr;
	u32	ecr;
	u32	gsr;
	u8	res0[12];
	u32	pitar2;
	u8	res1[4];
	u32	pibar2;
	u32	piebar2;
	u32	piwar2;
	u8	res2[4];
	u32	pitar1;
	u8	res3[4];
	u32	pibar1;
	u32	piebar1;
	u32	piwar1;
	u8	res4[4];
	u32	pitar0;
	u8	res5[4];
	u32	pibar0;
	u8	res6[4];
	u32	piwar0;
	u8	res7[132];
#define PITAR_TA_MASK		0x000fffff
#define PIBAR_MASK		0xffffffff
#define PIEBAR_EBA_MASK		0x000fffff
#define PIWAR_EN		0x80000000
#define PIWAR_PF		0x20000000
#define	PIWAR_RTT_MASK		0x000f0000
#define	PIWAR_RTT_NO_SNOOP	0x00040000
#define PIWAR_RTT_SNOOP		0x00050000
#define	PIWAR_WTT_MASK		0x0000f000
#define	PIWAR_WTT_NO_SNOOP	0x00004000
#define PIWAR_WTT_SNOOP		0x00005000
#define	PIWAR_IWS_MASK	0x0000003F
#define	PIWAR_IWS_4K	0x0000000B
#define	PIWAR_IWS_8K	0x0000000C
#define	PIWAR_IWS_16K	0x0000000D
#define	PIWAR_IWS_32K	0x0000000E
#define	PIWAR_IWS_64K	0x0000000F
#define	PIWAR_IWS_128K	0x00000010
#define	PIWAR_IWS_256K	0x00000011
#define	PIWAR_IWS_512K	0x00000012
#define	PIWAR_IWS_1M	0x00000013
#define	PIWAR_IWS_2M	0x00000014
#define	PIWAR_IWS_4M	0x00000015
#define	PIWAR_IWS_8M	0x00000016
#define	PIWAR_IWS_16M	0x00000017
#define	PIWAR_IWS_32M	0x00000018
#define	PIWAR_IWS_64M	0x00000019
#define	PIWAR_IWS_128M	0x0000001A
#define	PIWAR_IWS_256M	0x0000001B
#define	PIWAR_IWS_512M	0x0000001C
#define	PIWAR_IWS_1G	0x0000001D
#define	PIWAR_IWS_2G	0x0000001E
} pcictrl8349_t;

/*
 * USB
 */
typedef struct usb8349 {
	u8 fixme[0x2000];
} usb8349_t;

/*
 * TSEC
 */
typedef struct tsec8349 {
	u8 fixme[0x1000];
} tsec8349_t;

/*
 * Security
 */
typedef struct security8349 {
	u8 fixme[0x10000];
} security8349_t;

typedef struct immap {
	sysconf8349_t sysconf; /* System configuration */
	wdt8349_t     wdt;     /* Watch Dog Timer (WDT) Registers */
	rtclk8349_t   rtc;     /* Real Time Clock Module Registers */
	rtclk8349_t   pit;     /* Periodic Interval Timer */
	gtm8349_t     gtm[2];  /* Global Timers Module */
	ipic8349_t    ipic;    /* Integrated Programmable Interrupt Controller */
	arbiter8349_t arbiter; /* System Arbiter Registers */
	reset8349_t   reset;   /* Reset Module */
	clk8349_t     clk;     /* System Clock Module */
	pmc8349_t     pmc;     /* Power Management Control Module */
	gpio8349_t    pgio[2]; /* general purpose I/O module */
	u8 res0[0x200];
	u8 DDL_DDR[0x100];
	u8 DDL_LBIU[0x100];
	u8 res1[0xE00];
	ddr8349_t     ddr;     /* DDR Memory Controller Memory */
	i2c_t     i2c[2];      /* I2C1 Controller */
	u8 res2[0x1300];
	duart8349_t   duart[2];/* DUART */
	u8 res3[0x900];
	lbus8349_t    lbus;    /* Local Bus Controller Registers */
	u8 res4[0x1000];
	spi8349_t     spi;     /* Serial Peripheral Interface */
	u8 res5[0xF00];
	dma8349_t     dma;     /* DMA */
	pciconf8349_t pci_conf[2];  /* PCI Software Configuration Registers */
	ios8349_t     ios;     /* Sequencer */
	pcictrl8349_t pci_ctrl[2];  /* PCI Controller Control and Status Registers */
	u8 res6[0x19900];
	usb8349_t     usb;
	tsec8349_t    tsec[2];
	u8 res7[0xA000];
	security8349_t security;
} immap_t;

#endif /* __IMMAP_8349__ */
