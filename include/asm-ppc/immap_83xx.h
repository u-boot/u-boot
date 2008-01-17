/*
 * (C) Copyright 2004-2007 Freescale Semiconductor, Inc.
 *
 * MPC83xx Internal Memory Map
 *
 * Contributors:
 *	Dave Liu <daveliu@freescale.com>
 *	Tanya Jiang <tanya.jiang@freescale.com>
 *	Mandy Lavi <mandy.lavi@freescale.com>
 *	Eran Liberty <liberty@freescale.com>
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
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 *
 */
#ifndef __IMMAP_83xx__
#define __IMMAP_83xx__

#include <asm/types.h>
#include <asm/fsl_i2c.h>
#include <asm/mpc8xxx_spi.h>

/*
 * Local Access Window
 */
typedef struct law83xx {
	u32 bar;		/* LBIU local access window base address register */
	u32 ar;			/* LBIU local access window attribute register */
} law83xx_t;

/*
 * System configuration registers
 */
typedef struct sysconf83xx {
	u32 immrbar;		/* Internal memory map base address register */
	u8 res0[0x04];
	u32 altcbar;		/* Alternate configuration base address register */
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
	u8 res5[0x04];
	u32 spcr;		/* System Priority Configuration Register */
	u32 sicrl;		/* System I/O Configuration Register Low */
	u32 sicrh;		/* System I/O Configuration Register High */
	u8 res6[0x0C];
	u32 ddrcdr;		/* DDR Control Driver Register */
	u32 ddrdsr;		/* DDR Debug Status Register */
	u32 obir;		/* Output Buffer Impedance Register */
	u8 res7[0xCC];
} sysconf83xx_t;

/*
 * Watch Dog Timer (WDT) Registers
 */
typedef struct wdt83xx {
	u8 res0[4];
	u32 swcrr;		/* System watchdog control register */
	u32 swcnr;		/* System watchdog count register */
	u8 res1[2];
	u16 swsrr;		/* System watchdog service register */
	u8 res2[0xF0];
} wdt83xx_t;

/*
 * RTC/PIT Module Registers
 */
typedef struct rtclk83xx {
	u32 cnr;		/* control register */
	u32 ldr;		/* load register */
	u32 psr;		/* prescale register */
	u32 ctr;		/* counter value field register */
	u32 evr;		/* event register */
	u32 alr;		/* alarm register */
	u8 res0[0xE8];
} rtclk83xx_t;

/*
 * Global timer module
 */
typedef struct gtm83xx {
	u8 cfr1;		/* Timer1/2 Configuration */
	u8 res0[3];
	u8 cfr2;		/* Timer3/4 Configuration */
	u8 res1[10];
	u16 mdr1;		/* Timer1 Mode Register */
	u16 mdr2;		/* Timer2 Mode Register */
	u16 rfr1;		/* Timer1 Reference Register */
	u16 rfr2;		/* Timer2 Reference Register */
	u16 cpr1;		/* Timer1 Capture Register */
	u16 cpr2;		/* Timer2 Capture Register */
	u16 cnr1;		/* Timer1 Counter Register */
	u16 cnr2;		/* Timer2 Counter Register */
	u16 mdr3;		/* Timer3 Mode Register */
	u16 mdr4;		/* Timer4 Mode Register */
	u16 rfr3;		/* Timer3 Reference Register */
	u16 rfr4;		/* Timer4 Reference Register */
	u16 cpr3;		/* Timer3 Capture Register */
	u16 cpr4;		/* Timer4 Capture Register */
	u16 cnr3;		/* Timer3 Counter Register */
	u16 cnr4;		/* Timer4 Counter Register */
	u16 evr1;		/* Timer1 Event Register */
	u16 evr2;		/* Timer2 Event Register */
	u16 evr3;		/* Timer3 Event Register */
	u16 evr4;		/* Timer4 Event Register */
	u16 psr1;		/* Timer1 Prescaler Register */
	u16 psr2;		/* Timer2 Prescaler Register */
	u16 psr3;		/* Timer3 Prescaler Register */
	u16 psr4;		/* Timer4 Prescaler Register */
	u8 res[0xC0];
} gtm83xx_t;

/*
 * Integrated Programmable Interrupt Controller
 */
typedef struct ipic83xx {
	u32 sicfr;		/* System Global Interrupt Configuration Register */
	u32 sivcr;		/* System Global Interrupt Vector Register */
	u32 sipnr_h;		/* System Internal Interrupt Pending Register - High */
	u32 sipnr_l;		/* System Internal Interrupt Pending Register - Low */
	u32 siprr_a;		/* System Internal Interrupt Group A Priority Register */
	u8 res0[8];
	u32 siprr_d;		/* System Internal Interrupt Group D Priority Register */
	u32 simsr_h;		/* System Internal Interrupt Mask Register - High */
	u32 simsr_l;		/* System Internal Interrupt Mask Register - Low */
	u8 res1[4];
	u32 sepnr;		/* System External Interrupt Pending Register */
	u32 smprr_a;		/* System Mixed Interrupt Group A Priority Register */
	u32 smprr_b;		/* System Mixed Interrupt Group B Priority Register */
	u32 semsr;		/* System External Interrupt Mask Register */
	u32 secnr;		/* System External Interrupt Control Register */
	u32 sersr;		/* System Error Status Register */
	u32 sermr;		/* System Error Mask Register */
	u32 sercr;		/* System Error Control Register */
	u8 res2[4];
	u32 sifcr_h;		/* System Internal Interrupt Force Register - High */
	u32 sifcr_l;		/* System Internal Interrupt Force Register - Low */
	u32 sefcr;		/* System External Interrupt Force Register */
	u32 serfr;		/* System Error Force Register */
	u32 scvcr;		/* System Critical Interrupt Vector Register */
	u32 smvcr;		/* System Management Interrupt Vector Register */
	u8 res3[0x98];
} ipic83xx_t;

/*
 * System Arbiter Registers
 */
typedef struct arbiter83xx {
	u32 acr;		/* Arbiter Configuration Register */
	u32 atr;		/* Arbiter Timers Register */
	u8 res[4];
	u32 aer;		/* Arbiter Event Register */
	u32 aidr;		/* Arbiter Interrupt Definition Register */
	u32 amr;		/* Arbiter Mask Register */
	u32 aeatr;		/* Arbiter Event Attributes Register */
	u32 aeadr;		/* Arbiter Event Address Register */
	u32 aerr;		/* Arbiter Event Response Register */
	u8 res1[0xDC];
} arbiter83xx_t;

/*
 * Reset Module
 */
typedef struct reset83xx {
	u32 rcwl;		/* Reset Configuration Word Low Register */
	u32 rcwh;		/* Reset Configuration Word High Register */
	u8 res0[8];
	u32 rsr;		/* Reset Status Register */
	u32 rmr;		/* Reset Mode Register */
	u32 rpr;		/* Reset protection Register */
	u32 rcr;		/* Reset Control Register */
	u32 rcer;		/* Reset Control Enable Register */
	u8 res1[0xDC];
} reset83xx_t;

/*
 * Clock Module
 */
typedef struct clk83xx {
	u32 spmr;		/* system PLL mode Register */
	u32 occr;		/* output clock control Register */
	u32 sccr;		/* system clock control Register */
	u8 res0[0xF4];
} clk83xx_t;

/*
 * Power Management Control Module
 */
typedef struct pmc83xx {
	u32 pmccr;		/* PMC Configuration Register */
	u32 pmcer;		/* PMC Event Register */
	u32 pmcmr;		/* PMC Mask Register */
	u32 pmccr1;		/* PMC Configuration Register 1 */
	u32 pmccr2;		/* PMC Configuration Register 2 */
	u8 res0[0xEC];
} pmc83xx_t;

/*
 * General purpose I/O module
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

/*
 * QE Ports Interrupts Registers
 */
typedef struct qepi83xx {
	u8 res0[0xC];
	u32 qepier;		/* QE Ports Interrupt Event Register */
	u32 qepimr;		/* QE Ports Interrupt Mask Register */
	u32 qepicr;		/* QE Ports Interrupt Control Register */
	u8 res1[0xE8];
} qepi83xx_t;

/*
 * QE Parallel I/O Ports
 */
typedef struct gpio_n {
	u32 podr;		/* Open Drain Register */
	u32 pdat;		/* Data Register */
	u32 dir1;		/* direction register 1 */
	u32 dir2;		/* direction register 2 */
	u32 ppar1;		/* Pin Assignment Register 1 */
	u32 ppar2;		/* Pin Assignment Register 2 */
} gpio_n_t;

typedef struct qegpio83xx {
	gpio_n_t ioport[0x7];
	u8 res0[0x358];
} qepio83xx_t;

/*
 * QE Secondary Bus Access Windows
 */
typedef struct qesba83xx {
	u32 lbmcsar;		/* Local bus memory controller start address */
	u32 sdmcsar;		/* Secondary DDR memory controller start address */
	u8 res0[0x38];
	u32 lbmcear;		/* Local bus memory controller end address */
	u32 sdmcear;		/* Secondary DDR memory controller end address */
	u8 res1[0x38];
	u32 lbmcar;		/* Local bus memory controller attributes */
	u32 sdmcar;		/* Secondary DDR memory controller attributes */
	u8 res2[0x378];
} qesba83xx_t;

/*
 * DDR Memory Controller Memory Map
 */
typedef struct ddr_cs_bnds {
	u32 csbnds;
	u8 res0[4];
} ddr_cs_bnds_t;

typedef struct ddr83xx {
	ddr_cs_bnds_t csbnds[4];/* Chip Select x Memory Bounds */
	u8 res0[0x60];
	u32 cs_config[4];	/* Chip Select x Configuration */
	u8 res1[0x70];
	u32 timing_cfg_3;	/* SDRAM Timing Configuration 3 */
	u32 timing_cfg_0;	/* SDRAM Timing Configuration 0 */
	u32 timing_cfg_1;	/* SDRAM Timing Configuration 1 */
	u32 timing_cfg_2;	/* SDRAM Timing Configuration 2 */
	u32 sdram_cfg;		/* SDRAM Control Configuration */
	u32 sdram_cfg2;		/* SDRAM Control Configuration 2 */
	u32 sdram_mode;		/* SDRAM Mode Configuration */
	u32 sdram_mode2;	/* SDRAM Mode Configuration 2 */
	u32 sdram_md_cntl;	/* SDRAM Mode Control */
	u32 sdram_interval;	/* SDRAM Interval Configuration */
	u32 ddr_data_init;	/* SDRAM Data Initialization */
	u8 res2[4];
	u32 sdram_clk_cntl;	/* SDRAM Clock Control */
	u8 res3[0x14];
	u32 ddr_init_addr;	/* DDR training initialization address */
	u32 ddr_init_ext_addr;	/* DDR training initialization extended address */
	u8 res4[0xAA8];
	u32 ddr_ip_rev1;	/* DDR IP block revision 1 */
	u32 ddr_ip_rev2;	/* DDR IP block revision 2 */
	u8 res5[0x200];
	u32 data_err_inject_hi;	/* Memory Data Path Error Injection Mask High */
	u32 data_err_inject_lo;	/* Memory Data Path Error Injection Mask Low */
	u32 ecc_err_inject;	/* Memory Data Path Error Injection Mask ECC */
	u8 res6[0x14];
	u32 capture_data_hi;	/* Memory Data Path Read Capture High */
	u32 capture_data_lo;	/* Memory Data Path Read Capture Low */
	u32 capture_ecc;	/* Memory Data Path Read Capture ECC */
	u8 res7[0x14];
	u32 err_detect;		/* Memory Error Detect */
	u32 err_disable;	/* Memory Error Disable */
	u32 err_int_en;		/* Memory Error Interrupt Enable */
	u32 capture_attributes;	/* Memory Error Attributes Capture */
	u32 capture_address;	/* Memory Error Address Capture */
	u32 capture_ext_address;/* Memory Error Extended Address Capture */
	u32 err_sbe;		/* Memory Single-Bit ECC Error Management */
	u8 res8[0xA4];
	u32 debug_reg;
	u8 res9[0xFC];
} ddr83xx_t;

/*
 * DUART
 */
typedef struct duart83xx {
	u8 urbr_ulcr_udlb;	/* combined register for URBR, UTHR and UDLB */
	u8 uier_udmb;		/* combined register for UIER and UDMB */
	u8 uiir_ufcr_uafr;	/* combined register for UIIR, UFCR and UAFR */
	u8 ulcr;		/* line control register */
	u8 umcr;		/* MODEM control register */
	u8 ulsr;		/* line status register */
	u8 umsr;		/* MODEM status register */
	u8 uscr;		/* scratch register */
	u8 res0[8];
	u8 udsr;		/* DMA status register */
	u8 res1[3];
	u8 res2[0xEC];
} duart83xx_t;

/*
 * Local Bus Controller Registers
 */
typedef struct lbus_bank {
	u32 br;			/* Base Register */
	u32 or;			/* Option Register */
} lbus_bank_t;

typedef struct lbus83xx {
	lbus_bank_t bank[8];
	u8 res0[0x28];
	u32 mar;		/* UPM Address Register */
	u8 res1[0x4];
	u32 mamr;		/* UPMA Mode Register */
	u32 mbmr;		/* UPMB Mode Register */
	u32 mcmr;		/* UPMC Mode Register */
	u8 res2[0x8];
	u32 mrtpr;		/* Memory Refresh Timer Prescaler Register */
	u32 mdr;		/* UPM Data Register */
	u8 res3[0x4];
	u32 lsor;		/* Special Operation Initiation Register */
	u32 lsdmr;		/* SDRAM Mode Register */
	u8 res4[0x8];
	u32 lurt;		/* UPM Refresh Timer */
	u32 lsrt;		/* SDRAM Refresh Timer */
	u8 res5[0x8];
	u32 ltesr;		/* Transfer Error Status Register */
	u32 ltedr;		/* Transfer Error Disable Register */
	u32 lteir;		/* Transfer Error Interrupt Register */
	u32 lteatr;		/* Transfer Error Attributes Register */
	u32 ltear;		/* Transfer Error Address Register */
	u8 res6[0xC];
	u32 lbcr;		/* Configuration Register */
	u32 lcrr;		/* Clock Ratio Register */
	u8 res7[0x8];
	u32 fmr;		/* Flash Mode Register */
	u32 fir;		/* Flash Instruction Register */
	u32 fcr;		/* Flash Command Register */
	u32 fbar;		/* Flash Block Addr Register */
	u32 fpar;		/* Flash Page Addr Register */
	u32 fbcr;		/* Flash Byte Count Register */
	u8 res8[0xF08];
} lbus83xx_t;

/*
 * Serial Peripheral Interface
 */
typedef struct spi83xx {
	u32 mode;		/* mode register */
	u32 event;		/* event register */
	u32 mask;		/* mask register */
	u32 com;		/* command register */
	u8 res0[0x10];
	u32 tx;			/* transmit register */
	u32 rx;			/* receive register */
	u8 res1[0xFD8];
} spi83xx_t;

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

/*
 * PCI Software Configuration Registers
 */
typedef struct pciconf83xx {
	u32 config_address;
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
	u32 ecdr;
	u32 eer;
	u32 eatcr;
	u32 eacr;
	u32 eeacr;
	u32 edlcr;
	u32 edhcr;
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
} pcictrl83xx_t;

/*
 * USB
 */
typedef struct usb83xx {
	u8 fixme[0x1000];
} usb83xx_t;

/*
 * TSEC
 */
typedef struct tsec83xx {
	u8 fixme[0x1000];
} tsec83xx_t;

/*
 * Security
 */
typedef struct security83xx {
	u8 fixme[0x10000];
} security83xx_t;

/*
 *  PCI Express
 */
typedef struct pex83xx {
	u8 fixme[0x1000];
} pex83xx_t;

/*
 * SATA
 */
typedef struct sata83xx {
	u8 fixme[0x1000];
} sata83xx_t;

/*
 * eSDHC
 */
typedef struct sdhc83xx {
	u8 fixme[0x1000];
} sdhc83xx_t;

/*
 * SerDes
 */
typedef struct serdes83xx {
	u8 fixme[0x100];
} serdes83xx_t;

/*
 * On Chip ROM
 */
typedef struct rom83xx {
	u8 mem[0x10000];
} rom83xx_t;

/*
 * TDM
 */
typedef struct tdm83xx {
	u8 fixme[0x200];
} tdm83xx_t;

/*
 * TDM DMAC
 */
typedef struct tdmdmac83xx {
	u8 fixme[0x2000];
} tdmdmac83xx_t;

#if defined(CONFIG_MPC834X)
typedef struct immap {
	sysconf83xx_t		sysconf;	/* System configuration */
	wdt83xx_t		wdt;		/* Watch Dog Timer (WDT) Registers */
	rtclk83xx_t		rtc;		/* Real Time Clock Module Registers */
	rtclk83xx_t		pit;		/* Periodic Interval Timer */
	gtm83xx_t		gtm[2];		/* Global Timers Module */
	ipic83xx_t		ipic;		/* Integrated Programmable Interrupt Controller */
	arbiter83xx_t		arbiter;	/* System Arbiter Registers */
	reset83xx_t		reset;		/* Reset Module */
	clk83xx_t		clk;		/* System Clock Module */
	pmc83xx_t		pmc;		/* Power Management Control Module */
	gpio83xx_t		gpio[2];	/* General purpose I/O module */
	u8			res0[0x200];
	u8			dll_ddr[0x100];
	u8			dll_lbc[0x100];
	u8			res1[0xE00];
	ddr83xx_t		ddr;		/* DDR Memory Controller Memory */
	fsl_i2c_t		i2c[2];		/* I2C Controllers */
	u8			res2[0x1300];
	duart83xx_t		duart[2];	/* DUART */
	u8			res3[0x900];
	lbus83xx_t		lbus;		/* Local Bus Controller Registers */
	u8			res4[0x1000];
	spi8xxx_t		spi;		/* Serial Peripheral Interface */
	dma83xx_t		dma;		/* DMA */
	pciconf83xx_t		pci_conf[2];	/* PCI Software Configuration Registers */
	ios83xx_t		ios;		/* Sequencer */
	pcictrl83xx_t		pci_ctrl[2];	/* PCI Controller Control and Status Registers */
	u8			res5[0x19900];
	usb83xx_t		usb[2];
	tsec83xx_t		tsec[2];
	u8			res6[0xA000];
	security83xx_t		security;
	u8			res7[0xC0000];
} immap_t;

#elif defined(CONFIG_MPC8313)
typedef struct immap {
	sysconf83xx_t		sysconf;	/* System configuration */
	wdt83xx_t		wdt;		/* Watch Dog Timer (WDT) Registers */
	rtclk83xx_t		rtc;		/* Real Time Clock Module Registers */
	rtclk83xx_t		pit;		/* Periodic Interval Timer */
	gtm83xx_t		gtm[2];		/* Global Timers Module */
	ipic83xx_t		ipic;		/* Integrated Programmable Interrupt Controller */
	arbiter83xx_t		arbiter;	/* System Arbiter Registers */
	reset83xx_t		reset;		/* Reset Module */
	clk83xx_t		clk;		/* System Clock Module */
	pmc83xx_t		pmc;		/* Power Management Control Module */
	gpio83xx_t		gpio[1];	/* General purpose I/O module */
	u8			res0[0x1300];
	ddr83xx_t		ddr;		/* DDR Memory Controller Memory */
	fsl_i2c_t		i2c[2];		/* I2C Controllers */
	u8			res1[0x1300];
	duart83xx_t		duart[2];	/* DUART */
	u8			res2[0x900];
	lbus83xx_t		lbus;		/* Local Bus Controller Registers */
	u8			res3[0x1000];
	spi8xxx_t		spi;		/* Serial Peripheral Interface */
	dma83xx_t		dma;		/* DMA */
	pciconf83xx_t		pci_conf[1];	/* PCI Software Configuration Registers */
	u8			res4[0x80];
	ios83xx_t		ios;		/* Sequencer */
	pcictrl83xx_t		pci_ctrl[1];	/* PCI Controller Control and Status Registers */
	u8			res5[0x1aa00];
	usb83xx_t		usb[1];
	tsec83xx_t		tsec[2];
	u8			res6[0xA000];
	security83xx_t		security;
	u8			res7[0xC0000];
} immap_t;

#elif defined(CONFIG_MPC8315)
typedef struct immap {
	sysconf83xx_t		sysconf;	/* System configuration */
	wdt83xx_t		wdt;		/* Watch Dog Timer (WDT) Registers */
	rtclk83xx_t		rtc;		/* Real Time Clock Module Registers */
	rtclk83xx_t		pit;		/* Periodic Interval Timer */
	gtm83xx_t		gtm[2];		/* Global Timers Module */
	ipic83xx_t		ipic;		/* Integrated Programmable Interrupt Controller */
	arbiter83xx_t		arbiter;	/* System Arbiter Registers */
	reset83xx_t		reset;		/* Reset Module */
	clk83xx_t		clk;		/* System Clock Module */
	pmc83xx_t		pmc;		/* Power Management Control Module */
	gpio83xx_t		gpio[1];	/* General purpose I/O module */
	u8			res0[0x1300];
	ddr83xx_t		ddr;		/* DDR Memory Controller Memory */
	fsl_i2c_t		i2c[2];		/* I2C Controllers */
	u8			res1[0x1300];
	duart83xx_t		duart[2];	/* DUART */
	u8			res2[0x900];
	lbus83xx_t		lbus;		/* Local Bus Controller Registers */
	u8			res3[0x1000];
	spi8xxx_t		spi;		/* Serial Peripheral Interface */
	dma83xx_t		dma;		/* DMA */
	pciconf83xx_t		pci_conf[1];	/* PCI Software Configuration Registers */
	u8			res4[0x80];
	ios83xx_t		ios;		/* Sequencer */
	pcictrl83xx_t		pci_ctrl[1];	/* PCI Controller Control and Status Registers */
	u8			res5[0xa00];
	pex83xx_t		pciexp[2];	/* PCI Express Controller */
	u8			res6[0xb000];
	tdm83xx_t		tdm;		/* TDM Controller */
	u8			res7[0x1e00];
	sata83xx_t		sata[2];	/* SATA Controller */
	u8			res8[0x9000];
	usb83xx_t		usb[1];		/* USB DR Controller */
	tsec83xx_t		tsec[2];
	u8			res9[0x6000];
	tdmdmac83xx_t		tdmdmac;	/* TDM DMAC */
	u8			res10[0x2000];
	security83xx_t		security;
	u8			res11[0xA3000];
	serdes83xx_t		serdes[1];	/* SerDes Registers */
	u8			res12[0x1CF00];
} immap_t;

#elif defined(CONFIG_MPC837X)
typedef struct immap {
	sysconf83xx_t		sysconf;	/* System configuration */
	wdt83xx_t		wdt;		/* Watch Dog Timer (WDT) Registers */
	rtclk83xx_t		rtc;		/* Real Time Clock Module Registers */
	rtclk83xx_t		pit;		/* Periodic Interval Timer */
	gtm83xx_t		gtm[2];		/* Global Timers Module */
	ipic83xx_t		ipic;		/* Integrated Programmable Interrupt Controller */
	arbiter83xx_t		arbiter;	/* System Arbiter Registers */
	reset83xx_t		reset;		/* Reset Module */
	clk83xx_t		clk;		/* System Clock Module */
	pmc83xx_t		pmc;		/* Power Management Control Module */
	gpio83xx_t		gpio[2];	/* General purpose I/O module */
	u8			res0[0x1200];
	ddr83xx_t		ddr;		/* DDR Memory Controller Memory */
	fsl_i2c_t		i2c[2];		/* I2C Controllers */
	u8			res1[0x1300];
	duart83xx_t		duart[2];	/* DUART */
	u8			res2[0x900];
	lbus83xx_t		lbus;		/* Local Bus Controller Registers */
	u8			res3[0x1000];
	spi8xxx_t		spi;		/* Serial Peripheral Interface */
	dma83xx_t		dma;		/* DMA */
	pciconf83xx_t		pci_conf[1];	/* PCI Software Configuration Registers */
	u8			res4[0x80];
	ios83xx_t		ios;		/* Sequencer */
	pcictrl83xx_t		pci_ctrl[1];	/* PCI Controller Control and Status Registers */
	u8			res5[0xa00];
	pex83xx_t		pciexp[2];	/* PCI Express Controller */
	u8			res6[0xd000];
	sata83xx_t		sata[4];	/* SATA Controller */
	u8			res7[0x7000];
	usb83xx_t		usb[1];		/* USB DR Controller */
	tsec83xx_t		tsec[2];
	u8			res8[0x8000];
	sdhc83xx_t		sdhc;		/* SDHC Controller */
	u8			res9[0x1000];
	security83xx_t		security;
	u8			res10[0xA3000];
	serdes83xx_t		serdes[2];	/* SerDes Registers */
	u8			res11[0xCE00];
	rom83xx_t		rom;		/* On Chip ROM */
} immap_t;

#elif defined(CONFIG_MPC8360)
typedef struct immap {
	sysconf83xx_t		sysconf;	/* System configuration */
	wdt83xx_t		wdt;		/* Watch Dog Timer (WDT) Registers */
	rtclk83xx_t		rtc;		/* Real Time Clock Module Registers */
	rtclk83xx_t		pit;		/* Periodic Interval Timer */
	u8			res0[0x200];
	ipic83xx_t		ipic;		/* Integrated Programmable Interrupt Controller */
	arbiter83xx_t		arbiter;	/* System Arbiter Registers */
	reset83xx_t		reset;		/* Reset Module */
	clk83xx_t		clk;		/* System Clock Module */
	pmc83xx_t		pmc;		/* Power Management Control Module */
	qepi83xx_t		qepi;		/* QE Ports Interrupts Registers */
	u8			res1[0x300];
	u8			dll_ddr[0x100];
	u8			dll_lbc[0x100];
	u8			res2[0x200];
	qepio83xx_t		qepio;		/* QE Parallel I/O ports */
	qesba83xx_t		qesba;		/* QE Secondary Bus Access Windows */
	u8			res3[0x400];
	ddr83xx_t		ddr;		/* DDR Memory Controller Memory */
	fsl_i2c_t		i2c[2];		/* I2C Controllers */
	u8			res4[0x1300];
	duart83xx_t		duart[2];	/* DUART */
	u8			res5[0x900];
	lbus83xx_t		lbus;		/* Local Bus Controller Registers */
	u8			res6[0x2000];
	dma83xx_t		dma;		/* DMA */
	pciconf83xx_t		pci_conf[1];	/* PCI Software Configuration Registers */
	u8			res7[128];
	ios83xx_t		ios;		/* Sequencer (IOS) */
	pcictrl83xx_t		pci_ctrl[1];	/* PCI Controller Control and Status Registers */
	u8			res8[0x4A00];
	ddr83xx_t		ddr_secondary;	/* Secondary DDR Memory Controller Memory Map */
	u8			res9[0x22000];
	security83xx_t		security;
	u8			res10[0xC0000];
	u8			qe[0x100000];	/* QE block */
} immap_t;

#elif defined(CONFIG_MPC832X)
typedef struct immap {
	sysconf83xx_t		sysconf;	/* System configuration */
	wdt83xx_t		wdt;		/* Watch Dog Timer (WDT) Registers */
	rtclk83xx_t		rtc;		/* Real Time Clock Module Registers */
	rtclk83xx_t		pit;		/* Periodic Interval Timer */
	gtm83xx_t		gtm[2];		/* Global Timers Module */
	ipic83xx_t		ipic;		/* Integrated Programmable Interrupt Controller */
	arbiter83xx_t		arbiter;	/* System Arbiter Registers */
	reset83xx_t		reset;		/* Reset Module */
	clk83xx_t		clk;		/* System Clock Module */
	pmc83xx_t		pmc;		/* Power Management Control Module */
	qepi83xx_t		qepi;		/* QE Ports Interrupts Registers */
	u8			res0[0x300];
	u8			dll_ddr[0x100];
	u8			dll_lbc[0x100];
	u8			res1[0x200];
	qepio83xx_t		qepio;		/* QE Parallel I/O ports */
	u8			res2[0x800];
	ddr83xx_t		ddr;		/* DDR Memory Controller Memory */
	fsl_i2c_t		i2c[2];		/* I2C Controllers */
	u8			res3[0x1300];
	duart83xx_t		duart[2];	/* DUART */
	u8			res4[0x900];
	lbus83xx_t		lbus;		/* Local Bus Controller Registers */
	u8			res5[0x2000];
	dma83xx_t		dma;		/* DMA */
	pciconf83xx_t		pci_conf[1];	/* PCI Software Configuration Registers */
	u8			res6[128];
	ios83xx_t		ios;		/* Sequencer (IOS) */
	pcictrl83xx_t		pci_ctrl[1];	/* PCI Controller Control and Status Registers */
	u8			res7[0x27A00];
	security83xx_t		security;
	u8			res8[0xC0000];
	u8			qe[0x100000];	/* QE block */
} immap_t;
#endif

#endif				/* __IMMAP_83xx__ */
