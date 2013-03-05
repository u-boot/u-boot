/*
 * Copyright (C) 2011 Samsung Electronics
 * Lukasz Majewski <l.majewski@samsung.com>
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

#include <errno.h>
#include <common.h>
#include <command.h>
#include <g_dnl.h>
#include <usb_mass_storage.h>

int do_usb_mass_storage(cmd_tbl_t *cmdtp, int flag,
			       int argc, char * const argv[])
{
	char *ep;
	unsigned int dev_num = 0, offset = 0, part_size = 0;
	int rc;

	struct ums_board_info *ums_info;
	static char *s = "ums";

	if (argc < 2) {
		printf("usage: ums <dev> - e.g. ums 0\n");
		return 0;
	}

	dev_num = (int)simple_strtoul(argv[1], &ep, 16);

	if (dev_num) {
		puts("\nSet eMMC device to 0! - e.g. ums 0\n");
		goto fail;
	}

	board_usb_init();
	ums_info = board_ums_init(dev_num, offset, part_size);

	if (!ums_info) {
		printf("MMC: %d -> NOT available\n", dev_num);
		goto fail;
	}
	rc = fsg_init(ums_info);
	if (rc) {
		printf("cmd ums: fsg_init failed\n");
		goto fail;
	}

	g_dnl_register(s);

	while (1) {
		/* Handle control-c and timeouts */
		if (ctrlc()) {
			printf("The remote end did not respond in time.\n");
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
	"ums - User Mass Storage Gadget"
);
