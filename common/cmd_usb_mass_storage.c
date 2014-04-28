/*
 * Copyright (C) 2011 Samsung Electronics
 * Lukasz Majewski <l.majewski@samsung.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <errno.h>
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

	unsigned int dev_num = simple_strtoul(mmc_devstring, NULL, 0);

	struct ums *ums = ums_init(dev_num);
	if (!ums)
		return CMD_RET_FAILURE;

	unsigned int controller_index = (unsigned int)(simple_strtoul(
					usb_controller,	NULL, 0));
	if (board_usb_init(controller_index, USB_INIT_DEVICE)) {
		error("Couldn't init USB controller.");
		return CMD_RET_FAILURE;
	}

	int rc = fsg_init(ums);
	if (rc) {
		error("fsg_init failed");
		return CMD_RET_FAILURE;
	}

	g_dnl_register("usb_dnl_ums");

	/* Timeout unit: seconds */
	int cable_ready_timeout = UMS_CABLE_READY_TIMEOUT;

	if (!g_dnl_board_usb_cable_connected()) {
		/*
		 * Won't execute if we don't know whether the cable is
		 * connected.
		 */
		puts("Please connect USB cable.\n");

		while (!g_dnl_board_usb_cable_connected()) {
			if (ctrlc()) {
				puts("\rCTRL+C - Operation aborted.\n");
				goto exit;
			}
			if (!cable_ready_timeout) {
				puts("\rUSB cable not detected.\n" \
				     "Command exit.\n");
				goto exit;
			}

			printf("\rAuto exit in: %.2d s.", cable_ready_timeout);
			mdelay(1000);
			cable_ready_timeout--;
		}
		puts("\r\n");
	}

	while (1) {
		usb_gadget_handle_interrupts();

		rc = fsg_main_thread(NULL);
		if (rc) {
			/* Check I/O error */
			if (rc == -EIO)
				printf("\rCheck USB cable connection\n");

			/* Check CTRL+C */
			if (rc == -EPIPE)
				printf("\rCTRL+C - Operation aborted\n");

			goto exit;
		}
	}
exit:
	g_dnl_unregister();
	return CMD_RET_SUCCESS;
}

U_BOOT_CMD(ums, CONFIG_SYS_MAXARGS, 1, do_usb_mass_storage,
	"Use the UMS [User Mass Storage]",
	"ums <USB_controller> <mmc_dev>  e.g. ums 0 0"
);
