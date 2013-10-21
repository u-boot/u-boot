/*
 * Copyright (C) 2011 Samsung Electronics
 * Lukasz Majewski <l.majewski@samsung.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <command.h>
#include <g_dnl.h>
#include <usb.h>
#include <usb_mass_storage.h>

int do_usb_mass_storage(cmd_tbl_t *cmdtp, int flag,
			       int argc, char * const argv[])
{
	if (argc < 3)
		return CMD_RET_USAGE;

	const char *usb_controller = argv[1];
	const char *mmc_devstring  = argv[2];

	unsigned int dev_num = (unsigned int)(simple_strtoul(mmc_devstring,
				NULL, 0));
	if (dev_num) {
		error("Set eMMC device to 0! - e.g. ums 0");
		goto fail;
	}

	unsigned int controller_index = (unsigned int)(simple_strtoul(
					usb_controller,	NULL, 0));
	if (board_usb_init(controller_index, USB_INIT_DEVICE)) {
		error("Couldn't init USB controller.");
		goto fail;
	}

	struct ums_board_info *ums_info = board_ums_init(dev_num, 0, 0);
	if (!ums_info) {
		error("MMC: %d -> NOT available", dev_num);
		goto fail;
	}

	int rc = fsg_init(ums_info);
	if (rc) {
		error("fsg_init failed");
		goto fail;
	}

	g_dnl_register("ums");

	while (1) {
		/* Handle control-c and timeouts */
		if (ctrlc()) {
			error("The remote end did not respond in time.");
			goto exit;
		}

		usb_gadget_handle_interrupts();
		/* Check if USB cable has been detached */
		if (fsg_main_thread(NULL) == EIO)
			goto exit;
	}
exit:
	g_dnl_unregister();
	return 0;

fail:
	return -1;
}

U_BOOT_CMD(ums, CONFIG_SYS_MAXARGS, 1, do_usb_mass_storage,
	"Use the UMS [User Mass Storage]",
	"<USB_controller> <mmc_dev>"
);
