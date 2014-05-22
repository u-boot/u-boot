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
#include <part.h>
#include <usb.h>
#include <usb_mass_storage.h>

static int ums_read_sector(struct ums *ums_dev,
			   ulong start, lbaint_t blkcnt, void *buf)
{
	block_dev_desc_t *block_dev = ums_dev->block_dev;
	lbaint_t blkstart = start + ums_dev->start_sector;
	int dev_num = block_dev->dev;

	return block_dev->block_read(dev_num, blkstart, blkcnt, buf);
}

static int ums_write_sector(struct ums *ums_dev,
			    ulong start, lbaint_t blkcnt, const void *buf)
{
	block_dev_desc_t *block_dev = ums_dev->block_dev;
	lbaint_t blkstart = start + ums_dev->start_sector;
	int dev_num = block_dev->dev;

	return block_dev->block_write(dev_num, blkstart, blkcnt, buf);
}

static struct ums ums_dev = {
	.read_sector = ums_read_sector,
	.write_sector = ums_write_sector,
	.name = "UMS disk",
};

struct ums *ums_init(const char *devtype, const char *devnum)
{
	block_dev_desc_t *block_dev;
	int ret;

	ret = get_device(devtype, devnum, &block_dev);
	if (ret < 0)
		return NULL;

	/* f_mass_storage.c assumes SECTOR_SIZE sectors */
	if (block_dev->blksz != SECTOR_SIZE)
		return NULL;

	ums_dev.block_dev = block_dev;
	ums_dev.start_sector = 0;
	ums_dev.num_sectors = block_dev->lba;

	printf("UMS: disk start sector: %#x, count: %#x\n",
	       ums_dev.start_sector, ums_dev.num_sectors);

	return &ums_dev;
}

int do_usb_mass_storage(cmd_tbl_t *cmdtp, int flag,
			       int argc, char * const argv[])
{
	const char *usb_controller;
	const char *devtype;
	const char *devnum;
	struct ums *ums;
	unsigned int controller_index;
	int rc;
	int cable_ready_timeout __maybe_unused;

	if (argc < 3)
		return CMD_RET_USAGE;

	usb_controller = argv[1];
	if (argc >= 4) {
		devtype = argv[2];
		devnum  = argv[3];
	} else {
		devtype = "mmc";
		devnum  = argv[2];
	}

	ums = ums_init(devtype, devnum);
	if (!ums)
		return CMD_RET_FAILURE;

	controller_index = (unsigned int)(simple_strtoul(
				usb_controller,	NULL, 0));
	if (board_usb_init(controller_index, USB_INIT_DEVICE)) {
		error("Couldn't init USB controller.");
		return CMD_RET_FAILURE;
	}

	rc = fsg_init(ums);
	if (rc) {
		error("fsg_init failed");
		return CMD_RET_FAILURE;
	}

	rc = g_dnl_register("usb_dnl_ums");
	if (rc) {
		error("g_dnl_register failed");
		return CMD_RET_FAILURE;
	}

	/* Timeout unit: seconds */
	cable_ready_timeout = UMS_CABLE_READY_TIMEOUT;

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

U_BOOT_CMD(ums, 4, 1, do_usb_mass_storage,
	"Use the UMS [User Mass Storage]",
	"ums <USB_controller> [<devtype>] <devnum>  e.g. ums 0 mmc 0\n"
	"    devtype defaults to mmc"
);
