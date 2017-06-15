/*
 * Freescale iMX51 ATA driver
 *
 * Copyright (C) 2010 Marek Vasut <marek.vasut@gmail.com>
 *
 * Based on code by:
 *	Mahesh Mahadevan <mahesh.mahadevan@freescale.com>
 *
 * Based on code from original FSL ATA driver, which is
 * part of eCos, the Embedded Configurable Operating System.
 * Copyright (C) 1998, 1999, 2000, 2001, 2002 Red Hat, Inc.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <command.h>
#include <config.h>
#include <asm/byteorder.h>
#include <asm/io.h>
#include <ide.h>

#include <asm/arch/imx-regs.h>
#include <asm/arch/clock.h>

/* MXC ATA register offsets */
struct mxc_ata_config_regs {
	u8	time_off;	/* 0x00 */
	u8	time_on;
	u8	time_1;
	u8	time_2w;
	u8	time_2r;
	u8	time_ax;
	u8	time_pio_rdx;
	u8	time_4;
	u8	time_9;
	u8	time_m;
	u8	time_jn;
	u8	time_d;
	u8	time_k;
	u8	time_ack;
	u8	time_env;
	u8	time_udma_rdx;
	u8	time_zah;	/* 0x10 */
	u8	time_mlix;
	u8	time_dvh;
	u8	time_dzfs;
	u8	time_dvs;
	u8	time_cvh;
	u8	time_ss;
	u8	time_cyc;
	u32	fifo_data_32;	/* 0x18 */
	u32	fifo_data_16;
	u32	fifo_fill;
	u32	ata_control;
	u32	interrupt_pending;
	u32	interrupt_enable;
	u32	interrupt_clear;
	u32	fifo_alarm;
};

struct mxc_data_hdd_regs {
	u32	drive_data;	/* 0xa0 */
	u32	drive_features;
	u32	drive_sector_count;
	u32	drive_sector_num;
	u32	drive_cyl_low;
	u32	drive_cyl_high;
	u32	drive_dev_head;
	u32	command;
	u32	status;
	u32	alt_status;
};

/* PIO timing table */
#define	NR_PIO_SPECS	5
static uint16_t pio_t1[NR_PIO_SPECS]	= { 70,  50,  30,  30,  25 };
static uint16_t pio_t2_8[NR_PIO_SPECS]	= { 290, 290, 290, 80,  70 };
static uint16_t pio_t4[NR_PIO_SPECS]	= { 30,  20,  15,  10,  10 };
static uint16_t pio_t9[NR_PIO_SPECS]	= { 20,  15,  10,  10,  10 };
static uint16_t pio_tA[NR_PIO_SPECS]	= { 50,  50,  50,  50,  50 };

#define	REG2OFF(reg)	((((uint32_t)reg) & 0x3) * 8)
static void set_ata_bus_timing(unsigned char mode)
{
	uint32_t T = 1000000000 / mxc_get_clock(MXC_IPG_CLK);

	struct mxc_ata_config_regs *ata_regs;
	ata_regs = (struct mxc_ata_config_regs *)CONFIG_SYS_ATA_BASE_ADDR;

	if (mode >= NR_PIO_SPECS)
		return;

	/* Write TIME_OFF/ON/1/2W */
	writeb(3, &ata_regs->time_off);
	writeb(3, &ata_regs->time_on);
	writeb((pio_t1[mode] + T) / T, &ata_regs->time_1);
	writeb((pio_t2_8[mode] + T) / T, &ata_regs->time_2w);

	/* Write TIME_2R/AX/RDX/4 */
	writeb((pio_t2_8[mode] + T) / T, &ata_regs->time_2r);
	writeb((pio_tA[mode] + T) / T + 2, &ata_regs->time_ax);
	writeb(1, &ata_regs->time_pio_rdx);
	writeb((pio_t4[mode] + T) / T, &ata_regs->time_4);

	/* Write TIME_9 ; the rest of timing registers is irrelevant for PIO */
	writeb((pio_t9[mode] + T) / T, &ata_regs->time_9);
}

int ide_preinit(void)
{
	struct mxc_ata_config_regs *ata_regs;
	ata_regs = (struct mxc_ata_config_regs *)CONFIG_SYS_ATA_BASE_ADDR;

	/* 46.3.3.4 @ FSL iMX51 manual */
	/* FIFO normal op., drive reset */
	writel(0x80, &ata_regs->ata_control);
	/* FIFO normal op., drive not reset */
	writel(0xc0, &ata_regs->ata_control);

	/* Configure the PIO timing */
	set_ata_bus_timing(CONFIG_MXC_ATA_PIO_MODE);

	/* 46.3.3.4 @ FSL iMX51 manual */
	/* Drive not reset, IORDY handshake */
	writel(0x41, &ata_regs->ata_control);

	return 0;
}
