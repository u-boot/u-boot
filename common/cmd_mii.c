/*
 * (C) Copyright 2001
 * Gerald Van Baren, Custom IDEAS, vanbaren@cideas.com
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
 * MII Utilities
 */

#include <common.h>
#include <command.h>
#include <cmd_mii.h>
#include <miiphy.h>

#if (CONFIG_COMMANDS & CFG_CMD_MII)

/*
 * Display values from last command.
 */
uint last_op;
uint last_addr;
uint last_data;
uint last_reg;

/*
 * MII read/write
 *
 * Syntax:
 *  mii read {addr} {reg}
 *  mii write {addr} {reg} {data}
 */

int do_mii (cmd_tbl_t * cmdtp, int flag, int argc, char *argv[])
{
	char		op;
	unsigned char	addr, reg;
	unsigned short	data;
	int		rcode = 0;

#ifdef CONFIG_MPC860
	mii_init ();
#endif

	/*
	 * We use the last specified parameters, unless new ones are
	 * entered.
	 */
	op   = last_op;
	addr = last_addr;
	data = last_data;
	reg  = last_reg;

	if ((flag & CMD_FLAG_REPEAT) == 0) {
		op = argv[1][0];
		if (argc >= 3)
			addr = simple_strtoul (argv[2], NULL, 16);
		if (argc >= 4)
			reg  = simple_strtoul (argv[3], NULL, 16);
		if (argc >= 5)
			data = simple_strtoul (argv[4], NULL, 16);
	}

	/*
	 * check info/read/write.
	 */
	if (op == 'i') {
		int j;
		unsigned int oui;
		unsigned char model;
		unsigned char rev;

		/*
		 * Look for any and all PHYs.  Valid addresses are 0..31.
		 */
		for (j = 0; j < 32; j++) {
			if (miiphy_info (j, &oui, &model, &rev) == 0) {
				printf ("PHY 0x%02X: "
					"OUI = 0x%04X, "
					"Model = 0x%02X, "
					"Rev = 0x%02X, "
					"%3dbaseT, %s\n",
					j, oui, model, rev,
					miiphy_speed (j) == _100BASET ? 100 : 10,
					miiphy_duplex (j) == FULL ? "FDX" : "HDX");
			}
		}
	} else if (op == 'r') {
		if (miiphy_read (addr, reg, &data) != 0) {
			printf ("Error reading from the PHY\n");
			rcode = 1;
		}
		printf ("%04X\n", data & 0x0000FFFF);
	} else if (op == 'w') {
		if (miiphy_write (addr, reg, data) != 0) {
			printf ("Error writing to the PHY\n");
			rcode = 1;
		}
	} else {
		printf ("Usage:\n%s\n", cmdtp->usage);
		return 1;
	}

	/*
	 * Save the parameters for repeats.
	 */
	last_op = op;
	last_addr = addr;
	last_data = data;

	return rcode;
}

#endif /* CFG_CMD_MII */
