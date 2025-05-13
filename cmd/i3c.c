// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2025 Altera Corporation <www.altera.com>
 */

#include <bootretry.h>
#include <cli.h>
#include <command.h>
#include <console.h>
#include <dm.h>
#include <dw-i3c.h>
#include <edid.h>
#include <errno.h>
#include <hexdump.h>
#include <log.h>
#include <malloc.h>
#include <asm/byteorder.h>
#include <linux/compiler.h>
#include <linux/delay.h>
#include <u-boot/crc.h>
#include <linux/i3c/master.h>
#include <linux/printk.h>
#include <linux/types.h>

static struct udevice *currdev;
static struct udevice *prevdev;
static struct dw_i3c_master *master;

static void low_to_high_bytes(void *data, size_t size)
{
	u8 *byte_data = data;
	size_t start = 0;
	size_t end = size - 1;

	while (start < end) {
		u8 temp = byte_data[start];

		byte_data[start] = byte_data[end];
		byte_data[end] = temp;
		start++;
		end--;
	}
}

static int handle_i3c_select(const char *name)
{
	struct uclass *uc;
	struct udevice *dev_list;
	int ret = uclass_get_device_by_name(UCLASS_I3C, name, &currdev);

	if (ret) {
		currdev = prevdev;
		if (!currdev) {
			ret = uclass_get(UCLASS_I3C, &uc);
			if (ret)
				return CMD_RET_FAILURE;

			uclass_foreach_dev(dev_list, uc)
				printf("%s (%s)\n", dev_list->name, dev_list->driver->name);

			printf("i3c: Host controller not initialized: %s\n", name);
			return CMD_RET_FAILURE;
		}
	} else {
		master = dev_get_priv(currdev);
		printf("i3c: Current controller: %s\n", currdev->name);
		prevdev = currdev;
	}

	return CMD_RET_SUCCESS;
}

static int handle_i3c_list(void)
{
	struct uclass *uc;
	struct udevice *dev_list;
	int ret = uclass_get(UCLASS_I3C, &uc);

	if (ret)
		return CMD_RET_FAILURE;

	uclass_foreach_dev(dev_list, uc)
		printf("%s (%s)\n", dev_list->name, dev_list->driver->name);

	return CMD_RET_SUCCESS;
}

static int handle_i3c_current(void)
{
	if (!currdev)
		printf("i3c: No current controller selected\n");
	else
		printf("i3c: Current controller: %s\n", currdev->name);

	return CMD_RET_SUCCESS;
}

static int handle_i3c_device_list(void)
{
	if (!master) {
		printf("i3c: No controller active\n");
		return CMD_RET_FAILURE;
	}

	for (int i = 0; i < master->num_i3cdevs; i++) {
		struct i3c_device_info *info = &master->i3cdev[i]->info;

		printf("Device %d:\n", i);
		printf("  Static Address  : 0x%02X\n", info->static_addr);
		printf("  Dynamic Address : 0x%X\n", info->dyn_addr);
		printf("  PID             : %016llx\n", info->pid);
		printf("  BCR             : 0x%X\n", info->bcr);
		printf("  DCR             : 0x%X\n", info->dcr);
		printf("  Max Read DS     : 0x%X\n", info->max_read_ds);
		printf("  Max Write DS    : 0x%X\n", info->max_write_ds);
		printf("\n");
	}

	return CMD_RET_SUCCESS;
}

static int handle_i3c_write(int argc, char *const argv[])
{
	u32 mem_addr, num_bytes, dev_num_val;
	u8 device_num;
	u8 *data;
	int ret;

	if (argc < 5)
		return CMD_RET_USAGE;

	if (!currdev) {
		printf("i3c: No I3C controller selected\n");
		return CMD_RET_FAILURE;
	}

	mem_addr = hextoul(argv[2], NULL);
	num_bytes = hextoul(argv[3], NULL);
	dev_num_val = hextoul(argv[4], NULL);

	if (num_bytes == 0 || num_bytes > 4) {
		printf("i3c: Length must be between 1 and 4\n");
		return CMD_RET_USAGE;
	}

	if (dev_num_val > 0xFF) {
		printf("i3c: Device number 0x%x exceeds valid u8 range\n", dev_num_val);
		return CMD_RET_USAGE;
	}

	device_num = dev_num_val;
	data = malloc(num_bytes);

	if (!data) {
		printf("i3c: Memory allocation failed\n");
		return -ENOMEM;
	}

	memcpy(data, (void *)(uintptr_t)mem_addr, num_bytes);
	low_to_high_bytes(data, num_bytes);

	ret = dm_i3c_write(currdev, device_num, data, num_bytes);

	if (ret)
		printf("i3c: Write failed: %d\n", ret);

	free(data);
	return ret ? CMD_RET_FAILURE : CMD_RET_SUCCESS;
}

static int handle_i3c_read(int argc, char *const argv[])
{
	u32 mem_addr, read_len, dev_num_val;
	u8 device_num;
	u8 *rdata;
	int ret;

	if (argc < 5)
		return CMD_RET_USAGE;

	if (!currdev) {
		printf("i3c: No I3C controller selected\n");
		return CMD_RET_FAILURE;
	}

	mem_addr = hextoul(argv[2], NULL);
	read_len = hextoul(argv[3], NULL);
	dev_num_val = hextoul(argv[4], NULL);

	if (read_len == 0) {
		printf("i3c: Read length must be greater than 0\n");
		return CMD_RET_USAGE;
	}

	if (dev_num_val > 0xFF) {
		printf("i3c: Device number 0x%x exceeds valid u8 range\n", dev_num_val);
		return CMD_RET_USAGE;
	}

	device_num = dev_num_val;
	rdata = malloc(read_len);

	if (!rdata) {
		printf("i3c: Memory allocation failed\n");
		return -ENOMEM;
	}

	ret = dm_i3c_read(currdev, device_num, rdata, read_len);

	if (ret) {
		printf("i3c: Read failed: %d\n", ret);
		free(rdata);
		return CMD_RET_FAILURE;
	}

	memcpy((void *)(uintptr_t)mem_addr, rdata, read_len);
	print_hex_dump("i3c read: ", DUMP_PREFIX_OFFSET, 16, 1,
		       (void *)(uintptr_t)mem_addr, read_len, false);

	free(rdata);
	return CMD_RET_SUCCESS;
}

static bool is_i3c_subcommand(const char *cmd)
{
	return !strcmp(cmd, "write") ||
	       !strcmp(cmd, "read") ||
	       !strcmp(cmd, "device_list") ||
	       !strcmp(cmd, "list") ||
	       !strcmp(cmd, "current");
}

static int do_i3c(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[])
{
	if (argc < 2)
		return CMD_RET_USAGE;

	const char *subcmd = argv[1];

	if (!is_i3c_subcommand(subcmd))
		return handle_i3c_select(subcmd);

	if (!currdev) {
		printf("i3c: No I3C controller selected\n");
		return CMD_RET_FAILURE;
	}

	if (!strcmp(subcmd, "list"))
		return handle_i3c_list();
	else if (!strcmp(subcmd, "current"))
		return handle_i3c_current();
	else if (!strcmp(subcmd, "device_list"))
		return handle_i3c_device_list();
	else if (!strcmp(subcmd, "write"))
		return handle_i3c_write(argc, argv);
	else if (!strcmp(subcmd, "read"))
		return handle_i3c_read(argc, argv);

	return CMD_RET_USAGE;
}

U_BOOT_CMD(
	i3c, 5, 1, do_i3c,
	"access the system i3c",
	"i3c write <mem_addr> <length> <device_number> - write from memory to device\n"
	"i3c read <mem_addr> <length> <device_number> - read from device to memory\n"
	"i3c device_list - List valid target devices\n"
	"i3c <host_controller> - Select i3c controller\n"
	"i3c list - List all available i3c controllers\n"
	"i3c current - Show current i3c controller"
);
