/*
 * cmd_dfu.c -- dfu command
 *
 * Copyright (C) 2012 Samsung Electronics
 * authors: Andrzej Pietrasiewicz <andrzej.p@samsung.com>
 *	    Lukasz Majewski <l.majewski@samsung.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <dfu.h>
#include <g_dnl.h>
#include <usb.h>

static int do_dfu(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	if (argc < 4)
		return CMD_RET_USAGE;

	char *usb_controller = argv[1];
	char *interface = argv[2];
	char *devstring = argv[3];

	int ret, i = 0;

	ret = dfu_init_env_entities(interface, simple_strtoul(devstring,
							      NULL, 10));
	if (ret)
		goto done;

	ret = CMD_RET_SUCCESS;
	if (argc > 4 && strcmp(argv[4], "list") == 0) {
		dfu_show_entities();
		goto done;
	}

	int controller_index = simple_strtoul(usb_controller, NULL, 0);
	board_usb_init(controller_index, USB_INIT_DEVICE);

	g_dnl_register("usb_dnl_dfu");
	while (1) {
		if (dfu_reset())
			/*
			 * This extra number of usb_gadget_handle_interrupts()
			 * calls is necessary to assure correct transmission
			 * completion with dfu-util
			 */
			if (++i == 10)
				goto exit;

		if (ctrlc())
			goto exit;

		usb_gadget_handle_interrupts();
	}
exit:
	g_dnl_unregister();
done:
	dfu_free_entities();

	if (dfu_reset())
		run_command("reset", 0);

	return ret;
}

U_BOOT_CMD(dfu, CONFIG_SYS_MAXARGS, 1, do_dfu,
	"Device Firmware Upgrade",
	"<USB_controller> <interface> <dev> [list]\n"
	"  - device firmware upgrade via <USB_controller>\n"
	"    on device <dev>, attached to interface\n"
	"    <interface>\n"
	"    [list] - list available alt settings\n"
);
