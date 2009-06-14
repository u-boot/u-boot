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

#include <common.h>
#include <command.h>

static int mmc_nspi (const char *);

int do_dataflash_mmc_mux (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	switch (argc) {
	case 2:			/* on / off	*/
		switch (mmc_nspi (argv[1])) {
		case 0:	AT91F_SelectSPI ();
			break;
		case 1:	AT91F_SelectMMC ();
			break;
		}
	case 1:			/* get status */
		printf ("Mux is configured to be %s\n",
			AT91F_GetMuxStatus () ? "MMC" : "SPI");
		return 0;
	default:
		printf ("Usage:\n%s\n", cmdtp->usage);
		return 1;
	}
	return 0;
}

static int mmc_nspi (const char *s)
{
	if (strcmp (s, "mmc") == 0) {
		return 1;
	} else if (strcmp (s, "spi") == 0) {
		return 0;
	}
	return -1;
}

U_BOOT_CMD(
	dataflash_mmc_mux, 2, 1, do_dataflash_mmc_mux,
	"dataflash_mmc_mux\t- enable or disable MMC or SPI\n",
	"[mmc, spi]\n"
	"    - enable or disable MMC or SPI"
);
