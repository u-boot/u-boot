/**
 * Copyright 2010 Freescale Semiconductor
 * Author: Timur Tabi <timur@freescale.com>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option)
 * any later version.
 *
 * This file provides support for the ngPIXIS, a board-specific FPGA used on
 * some Freescale reference boards.
 *
 * A "switch" is black rectangular block on the motherboard.  It contains
 * eight "bits".  The ngPIXIS has a set of memory-mapped registers (SWx) that
 * shadow the actual physical switches.  There is also another set of
 * registers (ENx) that tell the ngPIXIS which bits of SWx should actually be
 * used to override the values of the bits in the physical switches.
 *
 * The following macros need to be defined:
 *
 * PIXIS_BASE - The virtual address of the base of the PIXIS register map
 *
 * PIXIS_LBMAP_SWITCH - The switch number (i.e. the "x" in "SWx"). This value
 *    is used in the PIXIS_SW() macro to determine which offset in
 *    the PIXIS register map corresponds to the physical switch that controls
 *    the boot bank.
 *
 * PIXIS_LBMAP_MASK - A bit mask the defines which bits in SWx to use.
 *
 * PIXIS_LBMAP_SHIFT - The shift value that corresponds to PIXIS_LBMAP_MASK.
 *
 * PIXIS_LBMAP_ALTBANK - The value to program into SWx to tell the ngPIXIS to
 *    boot from the alternate bank.
 */

#include <common.h>
#include <command.h>
#include <watchdog.h>
#include <asm/cache.h>
#include <asm/io.h>

#include "ngpixis.h"

/*
 * Reset the board. This ignores the ENx registers.
 */
void pixis_reset(void)
{
	out_8(&pixis->rst, 0);

	while (1);
}

/*
 * Reset the board.  Like pixis_reset(), but it honors the ENx registers.
 */
void pixis_bank_reset(void)
{
	out_8(&pixis->vctl, 0);
	out_8(&pixis->vctl, 1);

	while (1);
}

/**
 * Set the boot bank to the power-on default bank
 */
void clear_altbank(void)
{
	/* Tell the ngPIXIS to use this the bits in the physical switch for the
	 * boot bank value, instead of the SWx register.  We need to be careful
	 * only to set the bits in SWx that correspond to the boot bank.
	 */
	clrbits_8(&PIXIS_EN(PIXIS_LBMAP_SWITCH), PIXIS_LBMAP_MASK);
}

/**
 * Set the boot bank to the alternate bank
 */
void set_altbank(void)
{
	/* Program the alternate bank number into the SWx register.
	 */
	clrsetbits_8(&PIXIS_SW(PIXIS_LBMAP_SWITCH), PIXIS_LBMAP_MASK,
		     PIXIS_LBMAP_ALTBANK);

	/* Tell the ngPIXIS to use this the bits in the SWx register for the
	 * boot bank value, instead of the physical switch.  We need to be
	 * careful only to set the bits in SWx that correspond to the boot bank.
	 */
	setbits_8(&PIXIS_EN(PIXIS_LBMAP_SWITCH), PIXIS_LBMAP_MASK);
}


int pixis_reset_cmd(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	unsigned int i;
	char *p_altbank = NULL;
	char *unknown_param = NULL;

	/* No args is a simple reset request.
	 */
	if (argc <= 1)
		pixis_reset();

	for (i = 1; i < argc; i++) {
		if (strcmp(argv[i], "altbank") == 0) {
			p_altbank = argv[i];
			continue;
		}

		unknown_param = argv[i];
	}

	if (unknown_param) {
		printf("Invalid option: %s\n", unknown_param);
		return 1;
	}

	if (p_altbank)
		set_altbank();
	else
		clear_altbank();

	pixis_bank_reset();

	/* Shouldn't be reached. */
	return 0;
}

U_BOOT_CMD(
	pixis_reset, CONFIG_SYS_MAXARGS, 1, pixis_reset_cmd,
	"Reset the board using the FPGA sequencer",
	"- hard reset to default bank\n"
	"pixis_reset altbank - reset to alternate bank\n"
	);
