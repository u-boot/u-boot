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
#include <config.h>
#include <command.h>

#if (CONFIG_COMMANDS & CFG_CMD_DTT)

#include <dtt.h>

int do_dtt (cmd_tbl_t * cmdtp, int flag, int argc, char *argv[])
{
	int i;
	unsigned char sensors[] = CONFIG_DTT_SENSORS;

	/*
	 * Loop through sensors, read
	 * temperature, and output it.
	 */
	for (i = 0; i < sizeof (sensors); i++) {
		printf ("DTT%d: %i C\n", i + 1, dtt_get_temp (sensors[i]));
	}

	return 0;
}	/* do_dtt() */

/***************************************************/

U_BOOT_CMD(
	  dtt,	1,	1,	do_dtt,
	  "dtt     - Digital Thermometer and Themostat\n",
	  "        - Read temperature from digital thermometer and thermostat.\n"
);

#endif /* CONFIG_COMMANDS & CFG_CMD_DTT */
