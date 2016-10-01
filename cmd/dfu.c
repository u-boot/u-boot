/*
 * cmd_dfu.c -- dfu command
 *
 * Copyright (C) 2015
 * Lukasz Majewski <l.majewski@majess.pl>
 *
 * Copyright (C) 2012 Samsung Electronics
 * authors: Andrzej Pietrasiewicz <andrzej.p@samsung.com>
 *	    Lukasz Majewski <l.majewski@samsung.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <watchdog.h>
#include <dfu.h>
#include <console.h>
#include <g_dnl.h>
#include <usb.h>
#include <net.h>

static int do_dfu(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{

	if (argc < 4)
		return CMD_RET_USAGE;

	char *usb_controller = argv[1];
	char *interface = argv[2];
	char *devstring = argv[3];

	int ret;
#ifdef CONFIG_DFU_TFTP
	unsigned long addr = 0;
	if (!strcmp(argv[1], "tftp")) {
		if (argc == 5)
			addr = simple_strtoul(argv[4], NULL, 0);

		return update_tftp(addr, interface, devstring);
	}
#endif

	ret = dfu_init_env_entities(interface, devstring);
	if (ret)
		goto done;

	ret = CMD_RET_SUCCESS;
	if (argc > 4 && strcmp(argv[4], "list") == 0) {
		dfu_show_entities();
		goto done;
	}

	int controller_index = simple_strtoul(usb_controller, NULL, 0);

	run_usb_dnl_gadget(controller_index, "usb_dnl_dfu");

done:
	dfu_free_entities();
	return ret;
}

U_BOOT_CMD(dfu, CONFIG_SYS_MAXARGS, 1, do_dfu,
	"Device Firmware Upgrade",
	"<USB_controller> <interface> <dev> [list]\n"
	"  - device firmware upgrade via <USB_controller>\n"
	"    on device <dev>, attached to interface\n"
	"    <interface>\n"
	"    [list] - list available alt settings\n"
#ifdef CONFIG_DFU_TFTP
	"dfu tftp <interface> <dev> [<addr>]\n"
	"  - device firmware upgrade via TFTP\n"
	"    on device <dev>, attached to interface\n"
	"    <interface>\n"
	"    [<addr>] - address where FIT image has been stored\n"
#endif
);
