/*
 * (C) Copyright 2011 DENX Software Engineering,
 * Anatolij Gustschin <agust@denx.de>
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
#include <mpc5xxx.h>
#include <asm/io.h>

#define GPIO_USB1_0	0x00010000

static int cmd_disp(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	struct mpc5xxx_gpio *gpio = (struct mpc5xxx_gpio *)MPC5XXX_GPIO;

	if (argc < 2) {
		printf("%s\n",
		       in_be32(&gpio->simple_dvo) & GPIO_USB1_0 ? "on" : "off");
		return 0;
	}

	if (!strncmp(argv[1], "on", 2)) {
		setbits_be32(&gpio->simple_dvo, GPIO_USB1_0);
	} else if (!strncmp(argv[1], "off", 3)) {
		clrbits_be32(&gpio->simple_dvo, GPIO_USB1_0);
	} else {
		cmd_usage(cmdtp);
		return 1;
	}
	return 0;
}

U_BOOT_CMD(disp, 2, 1, cmd_disp,
		"disp [on/off] - switch display on/off",
		"\n    - print display on/off status\n"
		"on\n    - turn on\n"
		"off\n    - turn off\n"
);
