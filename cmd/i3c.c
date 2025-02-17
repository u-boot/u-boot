// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright 2023 Intel Coporation.
 */

#include <bootretry.h>
#include <cli.h>
#include <command.h>
#include <console.h>
#include <dm.h>
#include <dw-i3c.h>
#include <edid.h>
#include <errno.h>
#include <log.h>
#include <malloc.h>
#include <asm/byteorder.h>
#include <linux/compiler.h>
#include <linux/delay.h>
#include <u-boot/crc.h>
#include <linux/i3c/master.h>

static struct udevice *currdev;
static struct udevice *prevdev;
static struct dw_i3c_master *master;

void low_to_high_bytes(void *data, size_t size)
{
	unsigned char *byte_data = (unsigned char *)data;
	size_t start = 0;
	size_t end = size - 1;

	while (start < end) {
		unsigned char temp = byte_data[start];

		byte_data[start] = byte_data[end];
		byte_data[end] = temp;
		start++;
		end--;
	}
}

/**
 * do_i3c() - Handle the "i3c" command-line command
 * @cmdtp:    Command data struct pointer
 * @flag:    Command flag
 * @argc:    Command-line argument count
 * @argv:    Array of command-line arguments
 *
 * Returns zero on success, CMD_RET_USAGE in case of misuse and negative
 * on error.
 */
static int do_i3c(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[])
{
	struct uclass *uc;
	int ret;
	struct udevice *dev_list;
	u32 length, bytes;
	u8 *data;
	u8 *rdata;
	u8 device_num;

	if (argc > 1) {
		ret = uclass_get_device_by_name(UCLASS_I3C, argv[1], &currdev);
		if (ret) {
			if (!strcmp(argv[1], "help"))
				return CMD_RET_USAGE;

			currdev = prevdev;
			if (!currdev) {
				ret = uclass_get(UCLASS_I3C, &uc);
				if (ret)
					return CMD_RET_FAILURE;

				uclass_foreach_dev(dev_list, uc)
					printf("%s (%s)\n", dev_list->name, dev_list->driver->name);

				printf("i3c master controller is not initialized: %s\n", argv[1]);
				return CMD_RET_FAILURE;
			}
		} else {
			master = dev_get_priv(currdev);
			printf("current dev: %s\n", currdev->name);
			prevdev = currdev;
		}
	} else {
		if (!currdev) {
			printf("No i3c device set!\n");
			return CMD_RET_FAILURE;
		}
		printf("dev: %s\n", currdev->name);
	}

	if (!strcmp(argv[1], "list")) {
		ret = uclass_get(UCLASS_I3C, &uc);
		if (ret)
			return CMD_RET_FAILURE;

		uclass_foreach_dev(dev_list, uc)
		printf("%s (%s)\n", dev_list->name, dev_list->driver->name);
	}

	if (!strcmp(argv[1], "current")) {
		if (!currdev)
			printf("no current dev!\n");
		else
			printf("current dev: %s\n", currdev->name);
	}

	if ((!strcmp(argv[1], "write")) && !(argv[2] == NULL) && !(argv[3] == NULL) &&
	    !(argv[4] == NULL)) {
		length = hextoul(argv[3], NULL);

		data = (u8 *)malloc(sizeof(u8) * length);
		if (!data) {
			debug("Memory allocation for data failed\n");
			return -ENOMEM;
		}

		device_num = hextoul(argv[4], NULL);
		bytes = hextoul(argv[2], NULL);

		for (int i = 0; i < length; i++)
			data[i] = (u8)((bytes >> (8 * i)) & 0xFF);

		low_to_high_bytes(data, length);
		ret = dm_i3c_write(currdev, device_num, data, length);
		if (ret) {
			ret = CMD_RET_FAILURE;
			goto write_out;
		}
	}

	if (!strcmp(argv[1], "read") && !(argv[2] == NULL) && !(argv[3] == NULL)) {
		length = hextoul(argv[2], NULL);

		rdata = (u8 *) malloc(sizeof(u8) * length);
		if (!rdata) {
			debug("Memory allocation for rdata failed\n");
			return -ENOMEM;
		}

		device_num = hextoul(argv[3], NULL);

		ret = dm_i3c_read(currdev, device_num, rdata, length);
		if (ret) {
			ret =  CMD_RET_FAILURE;
		} else {
			printf("Read buff: ");
			for (size_t i = 0; i < length; ++i)
				printf("%02x", rdata[i]);

			printf("\n");
		}

		goto read_out;
	}

	if (!strcmp(argv[1], "device_list")) {
		if (master) {
			for (int i = 0; i < master->num_i3cdevs; i++) {
				printf("device:%d\n", i);
				printf("dynamic address:0x%X\n", master->i3cdev[i]->info.dyn_addr);
				printf("PID:%llx\n", master->i3cdev[i]->info.pid);
				printf("BCR:%X\n", master->i3cdev[i]->info.bcr);
				printf("DCR:%X\n", master->i3cdev[i]->info.dcr);
				printf("Max Read DS:%X\n", master->i3cdev[i]->info.max_read_ds);
				printf("Max Write DS:%X\n", master->i3cdev[i]->info.max_write_ds);
				printf("\n");
			}
		}
	}

	return CMD_RET_SUCCESS;

read_out:
	free(rdata);
	return ret;

write_out:
	free(data);
	return ret;
}

U_BOOT_CMD(
	i3c,    5,    1,     do_i3c,
	"access the system i3c\n",
	"i3c write<data><length><device_number> - write to device.\n"
	"i3c read<length><device_number> - read from device.\n"
	"i3c device_list   - List valid target devices\n"
	"i3c <i3c name>   - Select i3c controller.\n"
	"i3c list   - List all the available i3c controller.\n"
	"i3c current   - print current i3c controller.\n"
);
