/*
 * (C) Copyright 2007 DENX Software Engineering
 *
 * MPC512x Internal Memory Map
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
 * Based on the MPC83xx header.
 */

#ifndef __IMMAP_512x__
#define __IMMAP_512x__

#include <asm/types.h>

typedef struct law512x {
	u32 bar;	/* Base Addr Register */
	u32 ar;		/* Attributes Register */
} law512x_t;

/*
 * System configuration registers
 */
typedef struct sysconf512x {
	u32 immrbar;		/* Internal memory map base address register */
	u8 res0[0x1c];
	u32 lpbaw;		/* LP Boot Access Window */
	u32 lpcs0aw;		/* LP CS0 Access Window */
	u32 lpcs1aw;		/* LP CS1 Access Window */
	u32 lpcs2aw;		/* LP CS2 Access Window */
	u32 lpcs3aw;		/* LP CS3 Access Window */
	u32 lpcs4aw;		/* LP CS4 Access Window */
	u32 lpcs5aw;		/* LP CS5 Access Window */
	u32 lpcs6aw;		/* LP CS6 Access Window */
	u32 lpcs7aw;		/* LP CS7 Access Window */
	u8 res1[0x1c];
	law512x_t pcilaw[3];	/* PCI Local Access Window 0-2 Registers */
	u8 res2[0x28];
	law512x_t ddrlaw;	/* DDR Local Access Window */
	u8 res3[0x18];
	u32 mbxbar;		/* MBX Base Address */
	u32 srambar;		/* SRAM Base Address */
	u32 nfcbar;		/* NFC Base Address */
	u8 res4[0x34];
	u32 spridr;		/* System Part and Revision ID Register */
	u32 spcr;		/* System Priority Configuration Register */
	u8 res5[0xf8];
} sysconf512x_t;

/*
 * Watch Dog Timer (WDT) Registers
 */
typedef struct wdt512x {
	u8 res0[4];
	u32 swcrr;		/* System watchdog control register */
	u32 swcnr;		/* System watchdog count register */
	u8 res1[2];
	u16 swsrr;		/* System watchdog service register */
	u8 res2[0xF0];
} wdt512x_t;

/*
 * RTC Module Registers
 */
typedef struct rtclk512x {
	u8 fixme[0x100];
} rtclk512x_t;

/*
 * General Purpose Timer
 */
typedef struct gpt512x {
	u8 fixme[0x100];
} gpt512x_t;

/*
 * Integrated Programmable Interrupt Controller
 */
typedef struct ipic512x {
	u8 fixme[0x100];
} ipic512x_t;

/*
 * System Arbiter Registers
 */
typedef struct arbiter512x {
	u32 acr;		/* Arbiter Configuration Register */
	u32 atr;		/* Arbiter Timers Register */
	u32 ater;		/* Arbiter Transfer Error Register */
	u32 aer;		/* Arbiter Event Register */
	u32 aidr;		/* Arbiter Interrupt Definition Register */
	u32 amr;		/* Arbiter Mask Register */
	u32 aeatr;		/* Arbiter Event Attributes Register */
	u32 aeadr;		/* Arbiter Event Address Register */
	u32 aerr;		/* Arbiter Event Response Register */
	u8 res1[0xDC];
} arbiter512x_t;

/*
 * Reset Module
 */
typedef struct reset512x {
	u32 rcwl;		/* Reset Configuration Word Low Register */
	u32 rcwh;		/* Reset Configuration Word High Register */
	u8 res0[8];
	u32 rsr;		/* Reset Status Register */
	u32 rmr;		/* Reset Mode Register */
	u32 rpr;		/* Reset protection Register */
	u32 rcr;		/* Reset Control Register */
	u32 rcer;		/* Reset Control Enable Register */
	u8 res1[0xDC];
} reset512x_t;

/*
 * Clock Module
 */
typedef struct clk512x {
	u32 spmr;		/* System PLL Mode Register */
	u32 sccr[2];		/* System Clock Control Registers */
	u32 scfr[2];		/* System Clock Frequency Registers */
	u8 res0[4];
	u32 bcr;		/* Bread Crumb Register */
	u32 pscccr[12];		/* PSC0-11 Clock Control Registers */
	u32 spccr;		/* SPDIF Clock Control Registers */
	u32 cccr;		/* CFM Clock Control Registers */
	u32 dccr;		/* DIU Clock Control Registers */
	u8 res1[0xa8];
} clk512x_t;

/*
 * Power Management Control Module
 */
typedef struct pmc512x {
	u8 fixme[0x100];
} pmc512x_t;

/*
 * General purpose I/O module
 */
typedef struct gpio512x {
	u8 fixme[0x100];
} gpio512x_t;

/*
 * DDR Memory Controller Memory Map
 */
typedef struct ddr512x {
	u32 ddr_sys_config;	/* System Configuration Register */
	u32 ddr_time_config0;	/* Timing Configuration Register */
	u32 ddr_time_config1;	/* Timing Configuration Register */
	u32 ddr_time_config2;	/* Timing Configuration Register */
	u32 ddr_command;	/* Command Register */
	u32 ddr_compact_command;	/* Compact Command Register */
	u32 self_refresh_cmd_0;	/* Enter/Exit Self Refresh Registers */
	u32 self_refresh_cmd_1;	/* Enter/Exit Self Refresh Registers */
	u32 self_refresh_cmd_2;	/* Enter/Exit Self Refresh Registers */
	u32 self_refresh_cmd_3;	/* Enter/Exit Self Refresh Registers */
	u32 self_refresh_cmd_4;	/* Enter/Exit Self Refresh Registers */
	u32 self_refresh_cmd_5;	/* Enter/Exit Self Refresh Registers */
	u32 self_refresh_cmd_6;	/* Enter/Exit Self Refresh Registers */
	u32 self_refresh_cmd_7;	/* Enter/Exit Self Refresh Registers */
	u32 DQS_config_offset_count;	/* DQS Config Offset Count */
	u32 DQS_config_offset_time;	/* DQS Config Offset Time */
	u32 DQS_delay_status;	/* DQS Delay Status */
	u32 res0[0xF];
	u32 prioman_config1;	/* Priority Manager Configuration */
	u32 prioman_config2;	/* Priority Manager Configuration */
	u32 hiprio_config;	/* High Priority Configuration */
	u32 lut_table0_main_upper;	/* LUT0 Main Upper */
	u32 lut_table1_main_upper;	/* LUT1 Main Upper */
	u32 lut_table2_main_upper;	/* LUT2 Main Upper */
	u32 lut_table3_main_upper;	/* LUT3 Main Upper */
	u32 lut_table4_main_upper;	/* LUT4 Main Upper */
	u32 lut_table0_main_lower;	/* LUT0 Main Lower */
	u32 lut_table1_main_lower;	/* LUT1 Main Lower */
	u32 lut_table2_main_lower;	/* LUT2 Main Lower */
	u32 lut_table3_main_lower;	/* LUT3 Main Lower */
	u32 lut_table4_main_lower;	/* LUT4 Main Lower */
	u32 lut_table0_alternate_upper;	/* LUT0 Alternate Upper */
	u32 lut_table1_alternate_upper; /* LUT1 Alternate Upper */
	u32 lut_table2_alternate_upper; /* LUT2 Alternate Upper */
	u32 lut_table3_alternate_upper; /* LUT3 Alternate Upper */
	u32 lut_table4_alternate_upper; /* LUT4 Alternate Upper */
	u32 lut_table0_alternate_lower; /* LUT0 Alternate Lower */
	u32 lut_table1_alternate_lower; /* LUT1 Alternate Lower */
	u32 lut_table2_alternate_lower; /* LUT2 Alternate Lower */
	u32 lut_table3_alternate_lower; /* LUT3 Alternate Lower */
	u32 lut_table4_alternate_lower; /* LUT4 Alternate Lower */
	u32 performance_monitor_config;
	u32 event_time_counter;
	u32 event_time_preset;
	u32 performance_monitor1_address_low;
	u32 performance_monitor2_address_low;
	u32 performance_monitor1_address_hi;
	u32 performance_monitor2_address_hi;
	u32 res1[2];
	u32 performance_monitor1_read_counter;
	u32 performance_monitor2_read_counter;
	u32 performance_monitor1_write_counter;
	u32 performance_monitor2_write_counter;
	u32 granted_ack_counter0;
	u32 granted_ack_counter1;
	u32 granted_ack_counter2;
	u32 granted_ack_counter3;
	u32 granted_ack_counter4;
	u32 cumulative_wait_counter0;
	u32 cumulative_wait_counter1;
	u32 cumulative_wait_counter2;
	u32 cumulative_wait_counter3;
	u32 cumulative_wait_counter4;
	u32 summed_priority_counter0;
	u32 summed_priority_counter1;
	u32 summed_priority_counter2;
	u32 summed_priority_counter3;
	u32 summed_priority_counter4;
	u32 res2[0x3AD];
} ddr512x_t;


/*
 * DMA/Messaging Unit
 */
typedef struct dma512x {
	u8 fixme[0x1800];
} dma512x_t;

/*
 * PCI Software Configuration Registers
 */
typedef struct pciconf512x {
	u32 config_address;
	u32 config_data;
	u32 int_ack;
	u8 res[116];
} pciconf512x_t;

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
} pot512x_t;

/*
 * Sequencer
 */
typedef struct ios512x {
	pot512x_t pot[6];
	u8 res0[0x60];
	u32 pmcr;
	u8 res1[4];
	u32 dtcr;
	u8 res2[4];
} ios512x_t;

/*
 * PCI Controller
 */
typedef struct pcictrl512x {
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
} pcictrl512x_t;


/*
 * MSCAN
 */
typedef struct mscan512x {
	u8 fixme[0x100];
} mscan512x_t;

/*
 * BDLC
 */
typedef struct bdlc512x {
	u8 fixme[0x100];
} bdlc512x_t;

/*
 * SDHC
 */
typedef struct sdhc512x {
	u8 fixme[0x100];
} sdhc512x_t;

/*
 * SPDIF
 */
typedef struct spdif512x {
	u8 fixme[0x100];
} spdif512x_t;

/*
 * I2C
 */
typedef struct i2c512x_dev {
	volatile u32 madr;		/* I2Cn + 0x00 */
	volatile u32 mfdr;		/* I2Cn + 0x04 */
	volatile u32 mcr;		/* I2Cn + 0x08 */
	volatile u32 msr;		/* I2Cn + 0x0C */
	volatile u32 mdr;		/* I2Cn + 0x10 */
	u8 res0[0x0C];
} i2c512x_dev_t;

typedef struct i2c512x {
	i2c512x_dev_t dev[3];
	volatile u32 icr;
	volatile u32 mifr;
	u8 res0[0x98];
} i2c512x_t;

/*
 * AXE
 */
typedef struct axe512x {
	u8 fixme[0x100];
} axe512x_t;

/*
 * DIU
 */
typedef struct diu512x {
	u8 fixme[0x100];
} diu512x_t;

/*
 * CFM
 */
typedef struct cfm512x {
	u8 fixme[0x100];
} cfm512x_t;

/*
 * FEC
 */
typedef struct fec512x {
	u8 fixme[0x800];
} fec512x_t;

/*
 * ULPI
 */
typedef struct ulpi512x {
	u8 fixme[0x600];
} ulpi512x_t;

/*
 * UTMI
 */
typedef struct utmi512x {
	u8 fixme[0x3000];
} utmi512x_t;

/*
 * PCI DMA
 */
typedef struct pcidma512x {
	u8 fixme[0x300];
} pcidma512x_t;

/*
 * IO Control
 */
typedef struct ioctrl512x {
	u32 regs[0x400];
} ioctrl512x_t;

/*
 * IIM
 */
typedef struct iim512x {
	u8 fixme[0x1000];
} iim512x_t;

/*
 * LPC
 */
typedef struct lpc512x {
	u32	cs_cfg[8];	/* Chip Select N Configuration Registers
				   No dedicated entry for CS Boot as == CS0 */
	u32	cs_cr;		/* Chip Select Control Register */
	u32	cs_sr;		/* Chip Select Status Register */
	u32	cs_bcr;		/* Chip Select Burst Control Register */
	u32	cs_dccr;	/* Chip Select Deadcycle Control Register */
	u32	cs_hccr;	/* Chip Select Holdcycle Control Register */
	u8	res0[0xcc];
	u32	sclpc_psr;	/* SCLPC Packet Size Register */
	u32	sclpc_sar;	/* SCLPC Start Address Register */
	u32	sclpc_cr;	/* SCLPC Control Register */
	u32	sclpc_er;	/* SCLPC Enable Register */
	u32	sclpc_nar;	/* SCLPC NextAddress Register */
	u32	sclpc_sr;	/* SCLPC Status Register */
	u32	sclpc_bdr;	/* SCLPC Bytes Done Register */
	u32	emb_scr;	/* EMB Share Counter Register */
	u32	emb_pcr;	/* EMB Pause Control Register */
	u8	res1[0x1c];
	u32	lpc_fdwr;	/* LPC RX/TX FIFO Data Word Register */
	u32	lpc_fsr;	/* LPC RX/TX FIFO Status Register */
	u32	lpc_cr;		/* LPC RX/TX FIFO Control Register */
	u32	lpc_ar;		/* LPC RX/TX FIFO Alarm Register */
	u8	res2[0xb0];
} lpc512x_t;

/*
 * PATA
 */
typedef struct pata512x {
	u8 fixme[0x100];
} pata512x_t;

/*
 * PSC
 */
typedef struct psc512x {
	volatile u8	mode;		/* PSC + 0x00 */
	volatile u8	res0[3];
	union {				/* PSC + 0x04 */
		volatile u16	status;
		volatile u16	clock_select;
	} sr_csr;
#define psc_status	sr_csr.status
#define psc_clock_select sr_csr.clock_select
	volatile u16	res1;
	volatile u8	command;	/* PSC + 0x08 */
	volatile u8	res2[3];
	union {				/* PSC + 0x0c */
		volatile u8	buffer_8;
		volatile u16	buffer_16;
		volatile u32	buffer_32;
	} buffer;
#define psc_buffer_8	buffer.buffer_8
#define psc_buffer_16	buffer.buffer_16
#define psc_buffer_32	buffer.buffer_32
	union {				/* PSC + 0x10 */
		volatile u8	ipcr;
		volatile u8	acr;
	} ipcr_acr;
#define psc_ipcr	ipcr_acr.ipcr
#define psc_acr		ipcr_acr.acr
	volatile u8	res3[3];
	union {				/* PSC + 0x14 */
		volatile u16	isr;
		volatile u16	imr;
	} isr_imr;
#define psc_isr		isr_imr.isr
#define psc_imr		isr_imr.imr
	volatile u16	res4;
	volatile u8	ctur;		/* PSC + 0x18 */
	volatile u8	res5[3];
	volatile u8	ctlr;		/* PSC + 0x1c */
	volatile u8	res6[3];
	volatile u32	ccr;		/* PSC + 0x20 */
	volatile u8	res7[12];
	volatile u8	ivr;		/* PSC + 0x30 */
	volatile u8	res8[3];
	volatile u8	ip;		/* PSC + 0x34 */
	volatile u8	res9[3];
	volatile u8	op1;		/* PSC + 0x38 */
	volatile u8	res10[3];
	volatile u8	op0;		/* PSC + 0x3c */
	volatile u8	res11[3];
	volatile u32	sicr;		/* PSC + 0x40 */
	volatile u8	res12[60];
	volatile u32	tfcmd;		/* PSC + 0x80 */
	volatile u32	tfalarm;	/* PSC + 0x84 */
	volatile u32	tfstat;		/* PSC + 0x88 */
	volatile u32	tfintstat;	/* PSC + 0x8C */
	volatile u32	tfintmask;	/* PSC + 0x90 */
	volatile u32	tfcount;	/* PSC + 0x94 */
	volatile u16	tfwptr;		/* PSC + 0x98 */
	volatile u16	tfrptr;		/* PSC + 0x9A */
	volatile u32	tfsize;		/* PSC + 0x9C */
	volatile u8	res13[28];
	union {				/* PSC + 0xBC */
		volatile u8	buffer_8;
		volatile u16	buffer_16;
		volatile u32	buffer_32;
	} tfdata_buffer;
#define tfdata_8	tfdata_buffer.buffer_8
#define tfdata_16	tfdata_buffer.buffer_16
#define tfdata_32	tfdata_buffer.buffer_32

	volatile u32	rfcmd;		/* PSC + 0xC0 */
	volatile u32	rfalarm;	/* PSC + 0xC4 */
	volatile u32	rfstat;		/* PSC + 0xC8 */
	volatile u32	rfintstat;	/* PSC + 0xCC */
	volatile u32	rfintmask;	/* PSC + 0xD0 */
	volatile u32	rfcount;	/* PSC + 0xD4 */
	volatile u16	rfwptr;		/* PSC + 0xD8 */
	volatile u16	rfrptr;		/* PSC + 0xDA */
	volatile u32	rfsize;		/* PSC + 0xDC */
	volatile u8	res18[28];
	union {				/* PSC + 0xFC */
		volatile u8	buffer_8;
		volatile u16	buffer_16;
		volatile u32	buffer_32;
	} rfdata_buffer;
#define rfdata_8	rfdata_buffer.buffer_8
#define rfdata_16	rfdata_buffer.buffer_16
#define rfdata_32	rfdata_buffer.buffer_32
} psc512x_t;

/*
 * FIFOC
 */
typedef struct fifoc512x {
	u32 fifoc_cmd;
	u32 fifoc_int;
	u32 fifoc_dma;
	u32 fifoc_axe;
	u32 fifoc_debug;
	u8 fixme[0xEC];
} fifoc512x_t;

/*
 * SATA
 */
typedef struct sata512x {
	u8 fixme[0x2000];
} sata512x_t;

typedef struct immap {
	sysconf512x_t		sysconf;	/* System configuration */
	u8			res0[0x700];
	wdt512x_t		wdt;		/* Watch Dog Timer (WDT) */
	rtclk512x_t		rtc;		/* Real Time Clock Module */
	gpt512x_t		gpt;		/* General Purpose Timer */
	ipic512x_t		ipic;		/* Integrated Programmable Interrupt Controller */
	arbiter512x_t		arbiter;	/* CSB Arbiter */
	reset512x_t		reset;		/* Reset Module */
	clk512x_t		clk;		/* Clock Module */
	pmc512x_t		pmc;		/* Power Management Control Module */
	gpio512x_t		gpio;		/* General purpose I/O module */
	u8			res1[0x100];
	mscan512x_t		mscan;		/* MSCAN */
	bdlc512x_t		bdlc;		/* BDLC */
	sdhc512x_t		sdhc;		/* SDHC */
	spdif512x_t		spdif;		/* SPDIF */
	i2c512x_t		i2c;		/* I2C Controllers */
	u8			res2[0x800];
	axe512x_t		axe;		/* AXE */
	diu512x_t		diu;		/* Display Interface Unit */
	cfm512x_t		cfm;		/* Clock Frequency Measurement */
	u8			res3[0x500];
	fec512x_t		fec;		/* Fast Ethernet Controller */
	ulpi512x_t		ulpi;		/* USB ULPI */
	u8			res4[0xa00];
	utmi512x_t		utmi;		/* USB UTMI */
	u8			res5[0x1000];
	pcidma512x_t		pci_dma;	/* PCI DMA */
	pciconf512x_t		pci_conf;	/* PCI Configuration */
	u8			res6[0x80];
	ios512x_t		ios;		/* PCI Sequencer */
	pcictrl512x_t		pci_ctrl;	/* PCI Controller Control and Status */
	u8			res7[0xa00];
	ddr512x_t		mddrc;		/* Multi-port DDR Memory Controller */
	ioctrl512x_t		io_ctrl;	/* IO Control */
	iim512x_t		iim;		/* IC Identification module */
	u8			res8[0x4000];
	lpc512x_t		lpc;		/* LocalPlus Controller */
	pata512x_t		pata;		/* Parallel ATA */
	u8			res9[0xd00];
	psc512x_t		psc[12];	/* PSCs */
	u8			res10[0x300];
	fifoc512x_t		fifoc;		/* FIFO Controller */
	u8			res11[0x2000];
	dma512x_t		dma;		/* DMA */
	u8			res12[0xa800];
	sata512x_t		sata;		/* Serial ATA */
	u8			res13[0xde000];
} immap_t;
#endif /* __IMMAP_512x__ */
