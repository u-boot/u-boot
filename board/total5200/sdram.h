/*
 * (C) Copyright 2004
 * Mark Jonas, Freescale Semiconductor, mark.jonas@freescale.com.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

typedef struct {
	ulong ddr;
	ulong mode;
	ulong emode;
	ulong control;
	ulong config1;
	ulong config2;
	ulong tapdelay;
} sdram_conf_t;

long int mpc5xxx_sdram_init (sdram_conf_t *sdram_conf);
