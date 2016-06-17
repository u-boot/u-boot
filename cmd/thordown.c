/*
 * cmd_thordown.c -- USB TIZEN "THOR" Downloader gadget
 *
 * Copyright (C) 2013 Lukasz Majewski <l.majewski@samsung.com>
 * All rights reserved.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <thor.h>
#include <dfu.h>
#include <g_dnl.h>
#include <usb.h>

int do_thor_down(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	if (argc < 4)
		return CMD_RET_USAGE;

	char *usb_controller = argv[1];
	char *interface = argv[2];
	char *devstring = argv[3];

	int ret;

	puts("TIZEN \"THOR\" Downloader\n");

	ret = dfu_init_env_entities(interface, devstring);
	if (ret)
		goto done;

	int controller_index = simple_strtoul(usb_controller, NULL, 0);
	ret = board_usb_init(controller_index, USB_INIT_DEVICE);
	if (ret) {
		error("USB init failed: %d", ret);
		ret = CMD_RET_FAILURE;
		goto exit;
	}

	g_dnl_register("usb_dnl_thor");

	ret = thor_init();
	if (ret) {
		error("THOR DOWNLOAD failed: %d", ret);
		ret = CMD_RET_FAILURE;
		goto exit;
	}

	ret = thor_handle();
	if (ret) {
		error("THOR failed: %d", ret);
		ret = CMD_RET_FAILURE;
		goto exit;
	}

exit:
	g_dnl_unregister();
	board_usb_cleanup(controller_index, USB_INIT_DEVICE);
done:
	dfu_free_entities();

	return ret;
}

U_BOOT_CMD(thordown, CONFIG_SYS_MAXARGS, 1, do_thor_down,
	   "TIZEN \"THOR\" downloader",
	   "<USB_controller> <interface> <dev>\n"
	   "  - device software upgrade via LTHOR TIZEN dowload\n"
	   "    program via <USB_controller> on device <dev>,\n"
	   "	attached to interface <interface>\n"
);
