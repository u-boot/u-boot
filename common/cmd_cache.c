/*
 * (C) Copyright 2000
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

/*
 * Cache support: switch on or off, get status
 */
#include <common.h>
#include <command.h>

static int on_off (const char *);

int do_icache ( cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	switch (argc) {
	case 2:			/* on / off	*/
		switch (on_off(argv[1])) {
		case 0:	icache_disable();
			break;
		case 1:	icache_enable ();
			break;
		}
		/* FALL TROUGH */
	case 1:			/* get status */
		printf ("Instruction Cache is %s\n",
			icache_status() ? "ON" : "OFF");
		return 0;
	default:
		return cmd_usage(cmdtp);
	}
	return 0;
}

int do_dcache ( cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	switch (argc) {
	case 2:			/* on / off	*/
		switch (on_off(argv[1])) {
		case 0:	dcache_disable();
			break;
		case 1:	dcache_enable ();
			break;
		}
		/* FALL TROUGH */
	case 1:			/* get status */
		printf ("Data (writethrough) Cache is %s\n",
			dcache_status() ? "ON" : "OFF");
		return 0;
	default:
		return cmd_usage(cmdtp);
	}
	return 0;

}

static int on_off (const char *s)
{
	if (strcmp(s, "on") == 0) {
		return (1);
	} else if (strcmp(s, "off") == 0) {
		return (0);
	}
	return (-1);
}


U_BOOT_CMD(
	icache,   2,   1,     do_icache,
	"enable or disable instruction cache",
	"[on, off]\n"
	"    - enable or disable instruction cache"
);

U_BOOT_CMD(
	dcache,   2,   1,     do_dcache,
	"enable or disable data cache",
	"[on, off]\n"
	"    - enable or disable data (writethrough) cache"
);
