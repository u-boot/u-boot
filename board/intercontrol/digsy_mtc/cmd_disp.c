/*
 * (C) Copyright 2011 DENX Software Engineering,
 * Anatolij Gustschin <agust@denx.de>
 *
 * SPDX-License-Identifier:	GPL-2.0+
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
