/*
 * (C) Copyright 2001
 * Erik Theisen, Wave 7 Optics, etheisen@mindspring.com
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <common.h>
#include <command.h>

#if (CONFIG_COMMANDS & CFG_CMD_BSP)

#include "vpd.h"

/* ======================================================================
 * Interpreter command to retrieve board specific Vital Product Data, "VPD"
 * ======================================================================
 */
int do_vpd (cmd_tbl_t * cmdtp, int flag, int argc, char *argv[])
{
	VPD vpd;			/* Board specific data struct */
	uchar dev_addr = CFG_DEF_EEPROM_ADDR;

	/* Validate usage */
	if (argc > 2) {
		printf ("Usage:\n%s\n", cmdtp->usage);
		return 1;
	}

	/* Passed in EEPROM address */
	if (argc == 2)
		dev_addr = (uchar) simple_strtoul (argv[1], NULL, 16);

	/* Read VPD and output it */
	if (!vpd_get_data (dev_addr, &vpd)) {
		vpd_print (&vpd);
		return 0;
	}

	return 1;
}

U_BOOT_CMD(
	  vpd,	2,	1,	do_vpd,
	  "vpd     - Read Vital Product Data\n",
	  "[dev_addr]\n"
	  "        - Read VPD Data from default address, or device address 'dev_addr'.\n"
);

#endif /* (CONFIG_COMMANDS & CFG_CMD_BSP) */
