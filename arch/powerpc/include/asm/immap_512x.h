/*
 * (C) Copyright 2007-2009 DENX Software Engineering
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
#if defined(CONFIG_E300)
#include <asm/e300.h>
#endif

/*
 * System reset offset (PowerPC standard)
 */
#define EXC_OFF_SYS_RESET	0x0100
#define	_START_OFFSET		EXC_OFF_SYS_RESET

#define SPR_5121E		0x80180000

/*
 * IMMRBAR - Internal Memory Register Base Address
 */
#define CONFIG_DEFAULT_IMMR	0xFF400000	/* Default IMMR base address */
#define IMMRBAR			0x0000		/* Register offset to immr */
#define IMMRBAR_BASE_ADDR	0xFFF00000	/* Base address mask */
#define IMMRBAR_RES		~(IMMRBAR_BASE_ADDR)


#ifndef __ASSEMBLY__
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

#define LAWBAR_BAR	0xFFFFF000	/* Base address mask */

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

/* RSR - Reset Status Register */
#define RSR_SWSR	0x00002000	/* software soft reset */
#define RSR_SWHR	0x00001000	/* software hard reset */
#define RSR_JHRS	0x00000200	/* jtag hreset */
#define RSR_JSRS	0x00000100	/* jtag sreset status */
#define RSR_CSHR	0x00000010	/* checkstop reset status */
#define RSR_SWRS	0x00000008	/* software watchdog reset status */
#define RSR_BMRS	0x00000004	/* bus monitop reset status */
#define RSR_SRS		0x00000002	/* soft reset status */
#define RSR_HRS		0x00000001	/* hard reset status */
#define RSR_RES		~(RSR_SWSR | RSR_SWHR |\
			 RSR_JHRS | RSR_JSRS | RSR_CSHR | RSR_SWRS |\
			 RSR_BMRS | RSR_SRS | RSR_HRS)

/* RMR - Reset Mode Register */
#define RMR_CSRE	0x00000001	/* checkstop reset enable */
#define RMR_CSRE_SHIFT	0
#define RMR_RES		(~(RMR_CSRE))

/* RCR - Reset Control Register */
#define RCR_SWHR	0x00000002	/* software hard reset */
#define RCR_SWSR	0x00000001	/* software soft reset */
#define RCR_RES		(~(RCR_SWHR | RCR_SWSR))

/* RCER - Reset Control Enable Register */
#define RCER_CRE	0x00000001	/* software hard reset */
#define RCER_RES	(~(RCER_CRE))

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
	u32 spccr;		/* SPDIF Clock Control Register */
	u32 cccr;		/* CFM Clock Control Register */
	u32 dccr;		/* DIU Clock Control Register */
	u32 msccr[4];		/* MSCAN1-4 Clock Control Registers */
	u8 res1[0x98];
} clk512x_t;

/* SPMR - System PLL Mode Register */
#define SPMR_SPMF		0x0F000000
#define SPMR_SPMF_SHIFT		24
#define SPMR_CPMF		0x000F0000
#define SPMR_CPMF_SHIFT		16

/* System Clock Control Register 1 commands */
#define CLOCK_SCCR1_CFG_EN		0x80000000
#define CLOCK_SCCR1_LPC_EN		0x40000000
#define CLOCK_SCCR1_NFC_EN		0x20000000
#define CLOCK_SCCR1_PATA_EN		0x10000000
#define CLOCK_SCCR1_PSC_EN(cn)		(0x08000000 >> (cn))
#define CLOCK_SCCR1_PSCFIFO_EN		0x00008000
#define CLOCK_SCCR1_SATA_EN		0x00004000
#define CLOCK_SCCR1_FEC_EN		0x00002000
#define CLOCK_SCCR1_TPR_EN		0x00001000
#define CLOCK_SCCR1_PCI_EN		0x00000800
#define CLOCK_SCCR1_DDR_EN		0x00000400

/* System Clock Control Register 2 commands */
#define CLOCK_SCCR2_DIU_EN		0x80000000
#define CLOCK_SCCR2_AXE_EN		0x40000000
#define CLOCK_SCCR2_MEM_EN		0x20000000
#define CLOCK_SCCR2_USB1_EN		0x10000000
#define CLOCK_SCCR2_USB2_EN		0x08000000
#define CLOCK_SCCR2_I2C_EN		0x04000000
#define CLOCK_SCCR2_BDLC_EN		0x02000000
#define CLOCK_SCCR2_SDHC_EN		0x01000000
#define CLOCK_SCCR2_SPDIF_EN		0x00800000
#define CLOCK_SCCR2_MBX_BUS_EN		0x00400000
#define CLOCK_SCCR2_MBX_EN		0x00200000
#define CLOCK_SCCR2_MBX_3D_EN		0x00100000
#define CLOCK_SCCR2_IIM_EN		0x00080000

/* SCFR1 System Clock Frequency Register 1 */
#define SCFR1_IPS_DIV		0x3
#define SCFR1_IPS_DIV_MASK	0x03800000
#define SCFR1_IPS_DIV_SHIFT	23

#define SCFR1_PCI_DIV		0x6
#define SCFR1_PCI_DIV_MASK	0x00700000
#define SCFR1_PCI_DIV_SHIFT	20

#define SCFR1_LPC_DIV_MASK	0x00003800
#define SCFR1_LPC_DIV_SHIFT	11

/* SCFR2 System Clock Frequency Register 2 */
#define SCFR2_SYS_DIV		0xFC000000
#define SCFR2_SYS_DIV_SHIFT	26

/* SPCR - System Priority Configuration Register */
#define SPCR_TBEN	0x00400000	/* E300 core time base unit enable */

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
	u32 gpdir;
	u32 gpodr;
	u32 gpdat;
	u32 gpier;
	u32 gpimr;
	u32 gpicr1;
	u32 gpicr2;
	u8 res0[0xE4];
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

/* MDDRC SYS CFG and Timing CFG0 Registers */
#define MDDRC_SYS_CFG_EN	0xF0000000
#define MDDRC_SYS_CFG_CMD_MASK	0x10000000
#define MDDRC_REFRESH_ZERO_MASK	0x0000FFFF

/*
 * DDR Memory Controller Configuration settings
 */
typedef struct ddr512x_config {
	u32 ddr_sys_config;	/* System Configuration Register */
	u32 ddr_time_config0;	/* Timing Configuration Register */
	u32 ddr_time_config1;	/* Timing Configuration Register */
	u32 ddr_time_config2;	/* Timing Configuration Register */
} ddr512x_config_t;

typedef struct sdram_conf_s {
	unsigned long size;
	ddr512x_config_t cfg;
} sdram_conf_t;

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

/* POTAR - PCI Outbound Translation Address Register */
#define POTAR_TA_MASK	0x000fffff

/* POBAR - PCI Outbound Base Address Register */
#define POBAR_BA_MASK	0x000fffff

/* POCMR - PCI Outbound Comparision Mask Register */
#define POCMR_EN	0x80000000
#define POCMR_IO	0x40000000	/* 0-memory space 1-I/O space */
#define POCMR_PRE	0x20000000	/* prefetch enable */
#define POCMR_SBS	0x00100000	/* special byte swap enable */
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


/* PITAR - PCI Inbound Translation Address Register
 */
#define PITAR_TA_MASK	0x000fffff

/* PIBAR - PCI Inbound Base/Extended Address Register
 */
#define PIBAR_MASK	0xffffffff
#define PIEBAR_EBA_MASK	0x000fffff

/* PIWAR - PCI Inbound Windows Attributes Register
 */
#define PIWAR_EN	0x80000000
#define PIWAR_SBS	0x40000000
#define PIWAR_PF	0x20000000
#define PIWAR_RTT_MASK	0x000f0000
#define PIWAR_RTT_NO_SNOOP 0x00040000
#define PIWAR_RTT_SNOOP	0x00050000
#define PIWAR_WTT_MASK	0x0000f000
#define PIWAR_WTT_NO_SNOOP 0x00004000
#define PIWAR_WTT_SNOOP	0x00005000

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

/* Number of I2C buses */
#define I2C_BUS_CNT	3

typedef struct i2c512x {
	i2c512x_dev_t dev[I2C_BUS_CNT];
	volatile u32 icr;
	volatile u32 mifr;
	u8 res0[0x98];
} i2c512x_t;

/* I2Cn control register bits */
#define I2C_EN		0x80
#define I2C_IEN		0x40
#define I2C_STA		0x20
#define I2C_TX		0x10
#define I2C_TXAK	0x08
#define I2C_RSTA	0x04
#define I2C_INIT_MASK	(I2C_EN | I2C_STA | I2C_TX | I2C_RSTA)

/* I2Cn status register bits */
#define I2C_CF		0x80
#define I2C_AAS		0x40
#define I2C_BB		0x20
#define I2C_AL		0x10
#define I2C_SRW		0x04
#define I2C_IF		0x02
#define I2C_RXAK	0x01

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
	u32	fec_id;		/* FEC_ID register */
	u32	ievent;		/* Interrupt event register */
	u32	imask;		/* Interrupt mask register */
	u32	reserved_01;
	u32	r_des_active;	/* Receive ring updated flag */
	u32	x_des_active;	/* Transmit ring updated flag */
	u32	reserved_02[3];
	u32	ecntrl;		/* Ethernet control register */
	u32	reserved_03[6];
	u32	mii_data;	/* MII data register */
	u32	mii_speed;	/* MII speed register */
	u32	reserved_04[7];
	u32	mib_control;	/* MIB control/status register */
	u32	reserved_05[7];
	u32	r_cntrl;	/* Receive control register */
	u32	r_hash;		/* Receive hash */
	u32	reserved_06[14];
	u32	x_cntrl;	/* Transmit control register */
	u32	reserved_07[7];
	u32	paddr1;		/* Physical address low */
	u32	paddr2;		/* Physical address high + type field */
	u32	op_pause;	/* Opcode + pause duration */
	u32	reserved_08[10];
	u32	iaddr1;		/* Upper 32 bits of individual hash table */
	u32	iaddr2;		/* Lower 32 bits of individual hash table */
	u32	gaddr1;		/* Upper 32 bits of group hash table */
	u32	gaddr2;		/* Lower 32 bits of group hash table */
	u32	reserved_09[7];
	u32	x_wmrk;		/* Transmit FIFO watermark */
	u32	reserved_10;
	u32	r_bound;	/* End of RAM */
	u32	r_fstart;	/* Receive FIFO start address */
	u32	reserved_11[11];
	u32	r_des_start;	/* Beginning of receive descriptor ring */
	u32	x_des_start;	/* Pointer to beginning of transmit descriptor ring */
	u32	r_buff_size;	/* Receive buffer size */
	u32	reserved_12[26];
	u32	dma_control;	/* DMA control for IP bus, AMBA IF + DMA revision */
	u32	reserved_13[2];

	u32	mib[128];	/* MIB Block Counters */

	u32	fifo[256];	/*  used by FEC, can only be accessed by DMA */
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
	u32	io_control_mem;			/* MEM pad ctrl reg */
	u32	io_control_gp;			/* GP pad ctrl reg */
	u32	io_control_lpc_clk;		/* LPC_CLK pad ctrl reg */
	u32	io_control_lpc_oe;		/* LPC_OE pad ctrl reg */
	u32	io_control_lpc_rw;		/* LPC_R/W pad ctrl reg */
	u32	io_control_lpc_ack;		/* LPC_ACK pad ctrl reg */
	u32	io_control_lpc_cs0;		/* LPC_CS0 pad ctrl reg */
	u32	io_control_nfc_ce0;		/* NFC_CE0 pad ctrl reg */
	u32	io_control_lpc_cs1;		/* LPC_CS1 pad ctrl reg */
	u32	io_control_lpc_cs2;		/* LPC_CS2 pad ctrl reg */
	u32	io_control_lpc_ax03;		/* LPC_AX03 pad ctrl reg */
	u32	io_control_emb_ax02;		/* EMB_AX02 pad ctrl reg */
	u32	io_control_emb_ax01;		/* EMB_AX01 pad ctrl reg */
	u32	io_control_emb_ax00;		/* EMB_AX00 pad ctrl reg */
	u32	io_control_emb_ad31;		/* EMB_AD31 pad ctrl reg */
	u32	io_control_emb_ad30;		/* EMB_AD30 pad ctrl reg */
	u32	io_control_emb_ad29;		/* EMB_AD29 pad ctrl reg */
	u32	io_control_emb_ad28;		/* EMB_AD28 pad ctrl reg */
	u32	io_control_emb_ad27;		/* EMB_AD27 pad ctrl reg */
	u32	io_control_emb_ad26;		/* EMB_AD26 pad ctrl reg */
	u32	io_control_emb_ad25;		/* EMB_AD25 pad ctrl reg */
	u32	io_control_emb_ad24;		/* EMB_AD24 pad ctrl reg */
	u32	io_control_emb_ad23;		/* EMB_AD23 pad ctrl reg */
	u32	io_control_emb_ad22;		/* EMB_AD22 pad ctrl reg */
	u32	io_control_emb_ad21;		/* EMB_AD21 pad ctrl reg */
	u32	io_control_emb_ad20;		/* EMB_AD20 pad ctrl reg */
	u32	io_control_emb_ad19;		/* EMB_AD19 pad ctrl reg */
	u32	io_control_emb_ad18;		/* EMB_AD18 pad ctrl reg */
	u32	io_control_emb_ad17;		/* EMB_AD17 pad ctrl reg */
	u32	io_control_emb_ad16;		/* EMB_AD16 pad ctrl reg */
	u32	io_control_emb_ad15;		/* EMB_AD15 pad ctrl reg */
	u32	io_control_emb_ad14;		/* EMB_AD14 pad ctrl reg */
	u32	io_control_emb_ad13;		/* EMB_AD13 pad ctrl reg */
	u32	io_control_emb_ad12;		/* EMB_AD12 pad ctrl reg */
	u32	io_control_emb_ad11;		/* EMB_AD11 pad ctrl reg */
	u32	io_control_emb_ad10;		/* EMB_AD10 pad ctrl reg */
	u32	io_control_emb_ad09;		/* EMB_AD09 pad ctrl reg */
	u32	io_control_emb_ad08;		/* EMB_AD08 pad ctrl reg */
	u32	io_control_emb_ad07;		/* EMB_AD07 pad ctrl reg */
	u32	io_control_emb_ad06;		/* EMB_AD06 pad ctrl reg */
	u32	io_control_emb_ad05;		/* EMB_AD05 pad ctrl reg */
	u32	io_control_emb_ad04;		/* EMB_AD04 pad ctrl reg */
	u32	io_control_emb_ad03;		/* EMB_AD03 pad ctrl reg */
	u32	io_control_emb_ad02;		/* EMB_AD02 pad ctrl reg */
	u32	io_control_emb_ad01;		/* EMB_AD01 pad ctrl reg */
	u32	io_control_emb_ad00;		/* EMB_AD00 pad ctrl reg */
	u32	io_control_pata_ce1;		/* PATA_CE1 pad ctrl reg */
	u32	io_control_pata_ce2;		/* PATA_CE2 pad ctrl reg */
	u32	io_control_pata_isolate;	/* PATA_ISOLATE pad ctrl reg */
	u32	io_control_pata_ior;		/* PATA_IOR pad ctrl reg */
	u32	io_control_pata_iow;		/* PATA_IOW pad ctrl reg */
	u32	io_control_pata_iochrdy;	/* PATA_IOCHRDY pad ctrl reg */
	u32	io_control_pata_intrq;		/* PATA_INTRQ pad ctrl reg */
	u32	io_control_pata_drq;		/* PATA_DRQ pad ctrl reg */
	u32	io_control_pata_dack;		/* PATA_DACK pad ctrl reg */
	u32	io_control_nfc_wp;		/* NFC_WP pad ctrl reg */
	u32	io_control_nfc_rb;		/* NFC_RB pad ctrl reg */
	u32	io_control_nfc_ale;		/* NFC_ALE pad ctrl reg */
	u32	io_control_nfc_cle;		/* NFC_CLE pad ctrl reg */
	u32	io_control_nfc_we;		/* NFC_WE pad ctrl reg */
	u32	io_control_nfc_re;		/* NFC_RE pad ctrl reg */
	u32	io_control_pci_ad31;		/* PCI_AD31 pad ctrl reg */
	u32	io_control_pci_ad30;		/* PCI_AD30 pad ctrl reg */
	u32	io_control_pci_ad29;		/* PCI_AD29 pad ctrl reg */
	u32	io_control_pci_ad28;		/* PCI_AD28 pad ctrl reg */
	u32	io_control_pci_ad27;		/* PCI_AD27 pad ctrl reg */
	u32	io_control_pci_ad26;		/* PCI_AD26 pad ctrl reg */
	u32	io_control_pci_ad25;		/* PCI_AD25 pad ctrl reg */
	u32	io_control_pci_ad24;		/* PCI_AD24 pad ctrl reg */
	u32	io_control_pci_ad23;		/* PCI_AD23 pad ctrl reg */
	u32	io_control_pci_ad22;		/* PCI_AD22 pad ctrl reg */
	u32	io_control_pci_ad21;		/* PCI_AD21 pad ctrl reg */
	u32	io_control_pci_ad20;		/* PCI_AD20 pad ctrl reg */
	u32	io_control_pci_ad19;		/* PCI_AD19 pad ctrl reg */
	u32	io_control_pci_ad18;		/* PCI_AD18 pad ctrl reg */
	u32	io_control_pci_ad17;		/* PCI_AD17 pad ctrl reg */
	u32	io_control_pci_ad16;		/* PCI_AD16 pad ctrl reg */
	u32	io_control_pci_ad15;		/* PCI_AD15 pad ctrl reg */
	u32	io_control_pci_ad14;		/* PCI_AD14 pad ctrl reg */
	u32	io_control_pci_ad13;		/* PCI_AD13 pad ctrl reg */
	u32	io_control_pci_ad12;		/* PCI_AD12 pad ctrl reg */
	u32	io_control_pci_ad11;		/* PCI_AD11 pad ctrl reg */
	u32	io_control_pci_ad10;		/* PCI_AD10 pad ctrl reg */
	u32	io_control_pci_ad09;		/* PCI_AD09 pad ctrl reg */
	u32	io_control_pci_ad08;		/* PCI_AD08 pad ctrl reg */
	u32	io_control_pci_ad07;		/* PCI_AD07 pad ctrl reg */
	u32	io_control_pci_ad06;		/* PCI_AD06 pad ctrl reg */
	u32	io_control_pci_ad05;		/* PCI_AD05 pad ctrl reg */
	u32	io_control_pci_ad04;		/* PCI_AD04 pad ctrl reg */
	u32	io_control_pci_ad03;		/* PCI_AD03 pad ctrl reg */
	u32	io_control_pci_ad02;		/* PCI_AD02 pad ctrl reg */
	u32	io_control_pci_ad01;		/* PCI_AD01 pad ctrl reg */
	u32	io_control_pci_ad00;		/* PCI_AD00 pad ctrl reg */
	u32	io_control_pci_cbe0;		/* PCI_CBE0 pad ctrl reg */
	u32	io_control_pci_cbe1;		/* PCI_CBE1 pad ctrl reg */
	u32	io_control_pci_cbe2;		/* PCI_CBE2 pad ctrl reg */
	u32	io_control_pci_cbe3;		/* PCI_CBE3 pad ctrl reg */
	u32	io_control_pci_grant2;		/* PCI_GRANT2 pad ctrl reg */
	u32	io_control_pci_req2;		/* PCI_REQ2 pad ctrl reg */
	u32	io_control_pci_grant1;		/* PCI_GRANT1 pad ctrl reg */
	u32	io_control_pci_req1;		/* PCI_REQ1 pad ctrl reg */
	u32	io_control_pci_grant0;		/* PCI_GRANT0 pad ctrl reg */
	u32	io_control_pci_req0;		/* PCI_REQ0 pad ctrl reg */
	u32	io_control_pci_inta;		/* PCI_INTA pad ctrl reg */
	u32	io_control_pci_clk;		/* PCI_CLK pad ctrl reg */
	u32	io_control_pci_rst;		/* PCI_RST- pad ctrl reg */
	u32	io_control_pci_frame;		/* PCI_FRAME pad ctrl reg */
	u32	io_control_pci_idsel;		/* PCI_IDSEL pad ctrl reg */
	u32	io_control_pci_devsel;		/* PCI_DEVSEL pad ctrl reg */
	u32	io_control_pci_irdy;		/* PCI_IRDY pad ctrl reg */
	u32	io_control_pci_trdy;		/* PCI_TRDY pad ctrl reg */
	u32	io_control_pci_stop;		/* PCI_STOP pad ctrl reg */
	u32	io_control_pci_par;		/* PCI_PAR pad ctrl reg */
	u32	io_control_pci_perr;		/* PCI_PERR pad ctrl reg */
	u32	io_control_pci_serr;		/* PCI_SERR pad ctrl reg */
	u32	io_control_spdif_txclk;		/* SPDIF_TXCLK pad ctrl reg */
	u32	io_control_spdif_tx;		/* SPDIF_TX pad ctrl reg */
	u32	io_control_spdif_rx;		/* SPDIF_RX pad ctrl reg */
	u32	io_control_i2c0_scl;		/* I2C0_SCL pad ctrl reg */
	u32	io_control_i2c0_sda;		/* I2C0_SDA pad ctrl reg */
	u32	io_control_i2c1_scl;		/* I2C1_SCL pad ctrl reg */
	u32	io_control_i2c1_sda;		/* I2C1_SDA pad ctrl reg */
	u32	io_control_i2c2_scl;		/* I2C2_SCL pad ctrl reg */
	u32	io_control_i2c2_sda;		/* I2C2_SDA pad ctrl reg */
	u32	io_control_irq0;		/* IRQ0 pad ctrl reg */
	u32	io_control_irq1;		/* IRQ1 pad ctrl reg */
	u32	io_control_can1_tx;		/* CAN1_TX pad ctrl reg */
	u32	io_control_can2_tx;		/* CAN2_TX pad ctrl reg */
	u32	io_control_j1850_tx;		/* J1850_TX pad ctrl reg */
	u32	io_control_j1850_rx;		/* J1850_RX pad ctrl reg */
	u32	io_control_psc_mclk_in;		/* PSC_MCLK_IN pad ctrl reg */
	u32	io_control_psc0_0;		/* PSC0_0 pad ctrl reg */
	u32	io_control_psc0_1;		/* PSC0_1 pad ctrl reg */
	u32	io_control_psc0_2;		/* PSC0_2 pad ctrl reg */
	u32	io_control_psc0_3;		/* PSC0_3 pad ctrl reg */
	u32	io_control_psc0_4;		/* PSC0_4 pad ctrl reg */
	u32	io_control_psc1_0;		/* PSC1_0 pad ctrl reg */
	u32	io_control_psc1_1;		/* PSC1_1 pad ctrl reg */
	u32	io_control_psc1_2;		/* PSC1_2 pad ctrl reg */
	u32	io_control_psc1_3;		/* PSC1_3 pad ctrl reg */
	u32	io_control_psc1_4;		/* PSC1_4 pad ctrl reg */
	u32	io_control_psc2_0;		/* PSC2_0 pad ctrl reg */
	u32	io_control_psc2_1;		/* PSC2_1 pad ctrl reg */
	u32	io_control_psc2_2;		/* PSC2_2 pad ctrl reg */
	u32	io_control_psc2_3;		/* PSC2_3 pad ctrl reg */
	u32	io_control_psc2_4;		/* PSC2_4 pad ctrl reg */
	u32	io_control_psc3_0;		/* PSC3_0 pad ctrl reg */
	u32	io_control_psc3_1;		/* PSC3_1 pad ctrl reg */
	u32	io_control_psc3_2;		/* PSC3_2 pad ctrl reg */
	u32	io_control_psc3_3;		/* PSC3_3 pad ctrl reg */
	u32	io_control_psc3_4;		/* PSC3_4 pad ctrl reg */
	u32	io_control_psc4_0;		/* PSC4_0 pad ctrl reg */
	u32	io_control_psc4_1;		/* PSC4_1 pad ctrl reg */
	u32	io_control_psc4_2;		/* PSC4_2 pad ctrl reg */
	u32	io_control_psc4_3;		/* PSC4_3 pad ctrl reg */
	u32	io_control_psc4_4;		/* PSC4_4 pad ctrl reg */
	u32	io_control_psc5_0;		/* PSC5_0 pad ctrl reg */
	u32	io_control_psc5_1;		/* PSC5_1 pad ctrl reg */
	u32	io_control_psc5_2;		/* PSC5_2 pad ctrl reg */
	u32	io_control_psc5_3;		/* PSC5_3 pad ctrl reg */
	u32	io_control_psc5_4;		/* PSC5_4 pad ctrl reg */
	u32	io_control_psc6_0;		/* PSC6_0 pad ctrl reg */
	u32	io_control_psc6_1;		/* PSC6_1 pad ctrl reg */
	u32	io_control_psc6_2;		/* PSC6_2 pad ctrl reg */
	u32	io_control_psc6_3;		/* PSC6_3 pad ctrl reg */
	u32	io_control_psc6_4;		/* PSC6_4 pad ctrl reg */
	u32	io_control_psc7_0;		/* PSC7_0 pad ctrl reg */
	u32	io_control_psc7_1;		/* PSC7_1 pad ctrl reg */
	u32	io_control_psc7_2;		/* PSC7_2 pad ctrl reg */
	u32	io_control_psc7_3;		/* PSC7_3 pad ctrl reg */
	u32	io_control_psc7_4;		/* PSC7_4 pad ctrl reg */
	u32	io_control_psc8_0;		/* PSC8_0 pad ctrl reg */
	u32	io_control_psc8_1;		/* PSC8_1 pad ctrl reg */
	u32	io_control_psc8_2;		/* PSC8_2 pad ctrl reg */
	u32	io_control_psc8_3;		/* PSC8_3 pad ctrl reg */
	u32	io_control_psc8_4;		/* PSC8_4 pad ctrl reg */
	u32	io_control_psc9_0;		/* PSC9_0 pad ctrl reg */
	u32	io_control_psc9_1;		/* PSC9_1 pad ctrl reg */
	u32	io_control_psc9_2;		/* PSC9_2 pad ctrl reg */
	u32	io_control_psc9_3;		/* PSC9_3 pad ctrl reg */
	u32	io_control_psc9_4;		/* PSC9_4 pad ctrl reg */
	u32	io_control_psc10_0;		/* PSC10_0 pad ctrl reg */
	u32	io_control_psc10_1;		/* PSC10_1 pad ctrl reg */
	u32	io_control_psc10_2;		/* PSC10_2 pad ctrl reg */
	u32	io_control_psc10_3;		/* PSC10_3 pad ctrl reg */
	u32	io_control_psc10_4;		/* PSC10_4 pad ctrl reg */
	u32	io_control_psc11_0;		/* PSC11_0 pad ctrl reg */
	u32	io_control_psc11_1;		/* PSC11_1 pad ctrl reg */
	u32	io_control_psc11_2;		/* PSC11_2 pad ctrl reg */
	u32	io_control_psc11_3;		/* PSC11_3 pad ctrl reg */
	u32	io_control_psc11_4;		/* PSC11_4 pad ctrl reg */
	u32	io_control_ckstp_out;		/* CKSTP_OUT pad ctrl reg */
	u32	io_control_usb_phy_drvvbus;	/* USB2_DRVVBUS pad ctrl reg */
	u8	reserved[0x0cfc];		/* fill to 4096 bytes size */
} ioctrl512x_t;

/* IO pin fields */
#define IO_PIN_FMUX(v)	((v) << 7)	/* pin function */
#define IO_PIN_HOLD(v)	((v) << 5)	/* hold time, pci only */
#define IO_PIN_PUD(v)	((v) << 4)	/* if PUE, 0=pull-down, 1=pull-up */
#define IO_PIN_PUE(v)	((v) << 3)	/* pull up/down enable */
#define IO_PIN_ST(v)	((v) << 2)	/* schmitt trigger */
#define IO_PIN_DS(v)	((v))		/* slew rate */

typedef struct iopin_t {
	int p_offset;		/* offset from IOCTL_MEM_OFFSET */
	int nr_pins;		/* number of pins to set this way */
	int bit_or;		/* or in the value instead of overwrite */
	u_long val;		/* value to write or or */
}iopin_t;

void iopin_initialize(iopin_t *,int);

/*
 * IIM
 */
typedef struct iim512x {
	u32 stat;		/* IIM status register */
	u32 statm;		/* IIM status IRQ mask */
	u32 err;		/* IIM errors register */
	u32 emask;		/* IIM error IRQ mask  */
	u32 fctl;		/* IIM fuse control register */
	u32 ua;			/* IIM upper address register */
	u32 la;			/* IIM lower address register */
	u32 sdat;		/* IIM explicit sense data */
	u8 res0[0x08];
	u32 prg_p;		/* IIM program protection register */
	u8 res1[0x10];
	u32 divide;		/* IIM divide factor register */
	u8 res2[0x7c0];
	u32 fbac0;		/* IIM fuse bank 0 prot (for Freescale use) */
	u32 fb0w0[0x1f];	/* IIM fuse bank 0 data (for Freescale use) */
	u8 res3[0x380];
	u32 fbac1;		/* IIM fuse bank 1 protection */
	u32 fb1w1[0x01f];	/* IIM fuse bank 1 data */
	u8 res4[0x380];
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
	u32	altr;		/* Address Latch Timing Register */
	u8	res0[0xc8];
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
	/* LOCAL Registers */
	u32 pata_time1;		/* Time register 1: PIO and tx timing parameter */
	u32 pata_time2;		/* Time register 2: PIO timing parameter */
	u32 pata_time3;		/* Time register 3: PIO and MDMA timing parameter */
	u32 pata_time4;		/* Time register 4: MDMA and UDMA timing parameter */
	u32 pata_time5;		/* Time register 5: UDMA timing parameter */
	u32 pata_time6;		/* Time register 6: UDMA timing parameter */
	u32 pata_fifo_data32;   /* 32bit wide dataport to/from FIFO */
	u32 pata_fifo_data16;   /* 16bit wide dataport to/from FIFO */
	u32 pata_fifo_fill;	/* FIFO filling in halfwords (READONLY)*/
	u32 pata_ata_control;   /* ATA Interface control register */
	u32 pata_irq_pending;   /* Interrupt pending register (READONLY) */
	u32 pata_irq_enable;	/* Interrupt enable register */
	u32 pata_irq_clear;	/* Interrupt clear register (WRITEONLY)*/
	u32 pata_fifo_alarm;	/* fifo alarm threshold */
	u32 res1[0x1A];
	/* DRIVE Registers */
	u32 pata_drive_data;	/* drive data register*/
	u32 pata_drive_features;/* drive features register */
	u32 pata_drive_sectcnt; /* drive sector count register */
	u32 pata_drive_sectnum; /* drive sector number register */
	u32 pata_drive_cyllow;  /* drive cylinder low register */
	u32 pata_drive_cylhigh; /* drive cylinder high register */
	u32 pata_drive_dev_head;/* drive device head register */
	u32 pata_drive_command; /* write = drive command, read = drive status reg */
	u32 res2[0x06];
	u32 pata_drive_alt_stat;/* write = drive control, read = drive alt status reg */
	u32 res3[0x09];
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

/* PSC FIFO Command values */
#define PSC_FIFO_RESET_SLICE		0x80
#define PSC_FIFO_ENABLE_SLICE		0x01

/* PSC FIFO Controller Command values */
#define FIFOC_ENABLE_CLOCK_GATE		0x01
#define FIFOC_DISABLE_CLOCK_GATE	0x00

/* PSC FIFO status */
#define PSC_FIFO_EMPTY			0x01

/* PSC Command values */
#define PSC_RX_ENABLE		0x01
#define PSC_RX_DISABLE		0x02
#define PSC_TX_ENABLE		0x04
#define PSC_TX_DISABLE		0x08
#define PSC_SEL_MODE_REG_1	0x10
#define PSC_RST_RX		0x20
#define PSC_RST_TX		0x30
#define PSC_RST_ERR_STAT	0x40
#define PSC_RST_BRK_CHG_INT	0x50
#define PSC_START_BRK		0x60
#define PSC_STOP_BRK		0x70

/* PSC status register bits */
#define PSC_SR_CDE		0x0080
#define PSC_SR_TXEMP		0x0800
#define PSC_SR_OE		0x1000
#define PSC_SR_PE		0x2000
#define PSC_SR_FE		0x4000
#define PSC_SR_RB		0x8000

/* PSC mode fields */
#define PSC_MODE_5_BITS		0x00
#define PSC_MODE_6_BITS		0x01
#define PSC_MODE_7_BITS		0x02
#define PSC_MODE_8_BITS		0x03
#define PSC_MODE_PAREVEN	0x00
#define PSC_MODE_PARODD		0x04
#define PSC_MODE_PARFORCE	0x08
#define PSC_MODE_PARNONE	0x10
#define PSC_MODE_ENTIMEOUT	0x20
#define PSC_MODE_RXRTS		0x80
#define PSC_MODE_1_STOPBIT	0x07

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
 * Centralized FIFO Controller has internal memory for all 12 PSCs FIFOs
 *
 * NOTE: individual PSC units are free to use whatever area (and size) of the
 * FIFOC internal memory, so make sure memory areas for FIFO slices used by
 * different PSCs do not overlap!
 *
 * Overall size of FIFOC memory is not documented in the MPC5121e RM, but
 * tests indicate that it is 1024 words total.
 *
 * *_TX_SIZE and *_RX_SIZE is the number of 4-byte words for FIFO slice.
 */
#define FIFOC_PSC0_TX_SIZE	0x04
#define FIFOC_PSC0_TX_ADDR	0x0
#define FIFOC_PSC0_RX_SIZE	0x04
#define FIFOC_PSC0_RX_ADDR	0x10

#define FIFOC_PSC1_TX_SIZE	0x04
#define FIFOC_PSC1_TX_ADDR	0x20
#define FIFOC_PSC1_RX_SIZE	0x04
#define FIFOC_PSC1_RX_ADDR	0x30

#define FIFOC_PSC2_TX_SIZE	0x04
#define FIFOC_PSC2_TX_ADDR	0x40
#define FIFOC_PSC2_RX_SIZE	0x04
#define FIFOC_PSC2_RX_ADDR	0x50

#define FIFOC_PSC3_TX_SIZE	0x04
#define FIFOC_PSC3_TX_ADDR	0x60
#define FIFOC_PSC3_RX_SIZE	0x04
#define FIFOC_PSC3_RX_ADDR	0x70

#define FIFOC_PSC4_TX_SIZE	0x04
#define FIFOC_PSC4_TX_ADDR	0x80
#define FIFOC_PSC4_RX_SIZE	0x04
#define FIFOC_PSC4_RX_ADDR	0x90

#define FIFOC_PSC5_TX_SIZE	0x04
#define FIFOC_PSC5_TX_ADDR	0xa0
#define FIFOC_PSC5_RX_SIZE	0x04
#define FIFOC_PSC5_RX_ADDR	0xb0

#define FIFOC_PSC6_TX_SIZE	0x04
#define FIFOC_PSC6_TX_ADDR	0xc0
#define FIFOC_PSC6_RX_SIZE	0x04
#define FIFOC_PSC6_RX_ADDR	0xd0

#define FIFOC_PSC7_TX_SIZE	0x04
#define FIFOC_PSC7_TX_ADDR	0xe0
#define FIFOC_PSC7_RX_SIZE	0x04
#define FIFOC_PSC7_RX_ADDR	0xf0

#define FIFOC_PSC8_TX_SIZE	0x04
#define FIFOC_PSC8_TX_ADDR	0x100
#define FIFOC_PSC8_RX_SIZE	0x04
#define FIFOC_PSC8_RX_ADDR	0x110

#define FIFOC_PSC9_TX_SIZE	0x04
#define FIFOC_PSC9_TX_ADDR	0x120
#define FIFOC_PSC9_RX_SIZE	0x04
#define FIFOC_PSC9_RX_ADDR	0x130

#define FIFOC_PSC10_TX_SIZE	0x04
#define FIFOC_PSC10_TX_ADDR	0x140
#define FIFOC_PSC10_RX_SIZE	0x04
#define FIFOC_PSC10_RX_ADDR	0x150

#define FIFOC_PSC11_TX_SIZE	0x04
#define FIFOC_PSC11_TX_ADDR	0x160
#define FIFOC_PSC11_RX_SIZE	0x04
#define FIFOC_PSC11_RX_ADDR	0x170

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

/* provide interface to get PATA base address */
static inline u32 get_pata_base (void)
{
	volatile immap_t *im = (immap_t *) CONFIG_SYS_IMMR;
	return (u32)(&im->pata);
}
#endif	/* __ASSEMBLY__ */

#endif /* __IMMAP_512x__ */
