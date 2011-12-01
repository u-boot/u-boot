/*
 * Copyright 2011 Freescale Semiconductor
 * Author: Shengzhou Liu <Shengzhou.Liu@freescale.com>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option)
 * any later version.
 *
 * This file provides support for the QIXIS of some Freescale reference boards.
 *
 */

#include <common.h>
#include <command.h>
#include <asm/io.h>
#include "qixis.h"

u8 qixis_read(unsigned int reg)
{
	void *p = (void *)QIXIS_BASE;

	return in_8(p + reg);
}

void qixis_write(unsigned int reg, u8 value)
{
	void *p = (void *)QIXIS_BASE;

	out_8(p + reg, value);
}

void qixis_reset(void)
{
	QIXIS_WRITE(rst_ctl, 0x83);
}

void qixis_bank_reset(void)
{
	QIXIS_WRITE(rcfg_ctl, 0x20);
	QIXIS_WRITE(rcfg_ctl, 0x21);
}

/* Set the boot bank to the power-on default bank0 */
void clear_altbank(void)
{
	u8 reg;

	reg = QIXIS_READ(brdcfg[0]);
	reg = reg & ~QIXIS_LBMAP_MASK;
	QIXIS_WRITE(brdcfg[0], reg);
}

/* Set the boot bank to the alternate bank */
void set_altbank(void)
{
	u8 reg;

	reg = QIXIS_READ(brdcfg[0]);
	reg = (reg & ~QIXIS_LBMAP_MASK) | QIXIS_LBMAP_ALTBANK;
	QIXIS_WRITE(brdcfg[0], reg);
}

#ifdef DEBUG
static void qixis_dump_regs(void)
{
	int i;

	printf("id	= %02x\n", QIXIS_READ(id));
	printf("arch	= %02x\n", QIXIS_READ(arch));
	printf("scver	= %02x\n", QIXIS_READ(scver));
	printf("model	= %02x\n", QIXIS_READ(model));
	printf("rst_ctl	= %02x\n", QIXIS_READ(rst_ctl));
	printf("aux	= %02x\n", QIXIS_READ(aux));
	for (i = 0; i < 16; i++)
		printf("brdcfg%02d = %02x\n", i, QIXIS_READ(brdcfg[i]));
	for (i = 0; i < 16; i++)
		printf("dutcfg%02d = %02x\n", i, QIXIS_READ(dutcfg[i]));
	printf("sclk	= %02x%02x%02x\n", QIXIS_READ(sclk[0]),
		QIXIS_READ(sclk[1]), QIXIS_READ(sclk[2]));
	printf("dclk	= %02x%02x%02x\n", QIXIS_READ(dclk[0]),
		QIXIS_READ(dclk[1]), QIXIS_READ(dclk[2]));
	printf("aux     = %02x\n", QIXIS_READ(aux));
	printf("watch	= %02x\n", QIXIS_READ(watch));
	printf("ctl_sys	= %02x\n", QIXIS_READ(ctl_sys));
	printf("rcw_ctl = %02x\n", QIXIS_READ(rcw_ctl));
	printf("present = %02x\n", QIXIS_READ(present));
	printf("clk_spd = %02x\n", QIXIS_READ(clk_spd));
	printf("stat_dut = %02x\n", QIXIS_READ(stat_dut));
	printf("stat_sys = %02x\n", QIXIS_READ(stat_sys));
	printf("stat_alrm = %02x\n", QIXIS_READ(stat_alrm));
	printf("ctl_sys2 = %02x\n", QIXIS_READ(ctl_sys2));
}
#endif

int qixis_reset_cmd(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	int i;

	if (argc <= 1) {
		clear_altbank();
		qixis_reset();
	} else if (strcmp(argv[1], "altbank") == 0) {
		set_altbank();
		qixis_bank_reset();
	} else if (strcmp(argv[1], "watchdog") == 0) {
		static char *period[9] = {"2s", "4s", "8s", "16s", "32s",
					  "1min", "2min", "4min", "8min"};
		u8 rcfg = QIXIS_READ(rcfg_ctl);

		if (argv[2] == NULL) {
			printf("qixis watchdog <watchdog_period>\n");
			return 0;
		}
		for (i = 0; i < ARRAY_SIZE(period); i++) {
			if (strcmp(argv[2], period[i]) == 0) {
				/* disable watchdog */
				QIXIS_WRITE(rcfg_ctl, rcfg & ~0x08);
				QIXIS_WRITE(watch, ((i<<2) - 1));
				QIXIS_WRITE(rcfg_ctl, rcfg);
				return 0;
			}
		}
	}

#ifdef DEBUG
	else if (strcmp(argv[1], "dump") == 0) {
		qixis_dump_regs();
		return 0;
	}
#endif

	else {
		printf("Invalid option: %s\n", argv[1]);
		return 1;
	}

	return 0;
}

U_BOOT_CMD(
	qixis_reset, CONFIG_SYS_MAXARGS, 1, qixis_reset_cmd,
	"Reset the board using the FPGA sequencer",
	"- hard reset to default bank\n"
	"qixis_reset altbank - reset to alternate bank\n"
	"qixis watchdog <watchdog_period> - set the watchdog period\n"
	"	period: 1s 2s 4s 8s 16s 32s 1min 2min 4min 8min\n"
#ifdef DEBUG
	"qixis_reset dump - display the QIXIS registers\n"
#endif
	);
