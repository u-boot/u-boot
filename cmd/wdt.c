// SPDX-License-Identifier: GPL-2.0+
/*
 * Watchdog commands
 *
 * Copyright (c) 2019 Michael Walle <michael@walle.cc>
 */

#include <command.h>
#include <dm.h>
#include <wdt.h>

static int currdev_seq = -1;

static int do_wdt_list(struct cmd_tbl *cmdtp, int flag, int argc,
		       char *const argv[])
{
	struct udevice *dev;
	struct uclass *uc;
	int ret;

	ret = uclass_get(UCLASS_WDT, &uc);
	if (ret)
		return CMD_RET_FAILURE;

	uclass_foreach_dev(dev, uc)
		printf("%s (%s)\n", dev->name, dev->driver->name);

	return CMD_RET_SUCCESS;
}

static int do_wdt_dev(struct cmd_tbl *cmdtp, int flag, int argc,
		      char *const argv[])
{
	struct udevice *currdev;

	if (argc > 1) {
		if (uclass_get_device_by_name(UCLASS_WDT, argv[1], &currdev)) {
			currdev_seq = -1;
			printf("Can't get the watchdog timer: %s\n", argv[1]);
			return CMD_RET_FAILURE;
		}
		currdev_seq = dev_seq(currdev);
	} else {
		if (uclass_get_device_by_seq(UCLASS_WDT, currdev_seq, &currdev)) {
			currdev_seq = -1;
			printf("No watchdog timer device set!\n");
			return CMD_RET_FAILURE;
		}
		printf("dev: %s\n", currdev->name);
	}

	return CMD_RET_SUCCESS;
}

static int check_currdev(struct udevice **devp)
{
	if (uclass_get_device_by_seq(UCLASS_WDT, currdev_seq, devp)) {
		currdev_seq = -1;
		printf("No device set, use 'wdt dev' first\n");
		return CMD_RET_FAILURE;
	}
	return 0;
}

static int do_wdt_start(struct cmd_tbl *cmdtp, int flag, int argc,
			char *const argv[])
{
	int ret;
	u64 timeout;
	ulong flags = 0;
	struct udevice *currdev;

	if (argc < 2)
		return CMD_RET_USAGE;

	if (check_currdev(&currdev))
		return CMD_RET_FAILURE;

	timeout = simple_strtoull(argv[1], NULL, 0);
	if (argc > 2)
		flags = simple_strtoul(argv[2], NULL, 0);

	ret = wdt_start(currdev, timeout, flags);
	if (ret == -ENOSYS) {
		printf("Starting watchdog timer not supported.\n");
		return CMD_RET_FAILURE;
	} else if (ret) {
		printf("Starting watchdog timer failed (%d)\n", ret);
		return CMD_RET_FAILURE;
	}

	return CMD_RET_SUCCESS;
}

static int do_wdt_stop(struct cmd_tbl *cmdtp, int flag, int argc,
		       char *const argv[])
{
	int ret;
	struct udevice *currdev;

	if (check_currdev(&currdev))
		return CMD_RET_FAILURE;

	ret = wdt_stop(currdev);
	if (ret == -ENOSYS) {
		printf("Stopping watchdog timer not supported.\n");
		return CMD_RET_FAILURE;
	} else if (ret) {
		printf("Stopping watchdog timer failed (%d)\n", ret);
		return CMD_RET_FAILURE;
	}

	return CMD_RET_SUCCESS;
}

static int do_wdt_reset(struct cmd_tbl *cmdtp, int flag, int argc,
			char *const argv[])
{
	int ret;
	struct udevice *currdev;

	if (check_currdev(&currdev))
		return CMD_RET_FAILURE;

	ret = wdt_reset(currdev);
	if (ret == -ENOSYS) {
		printf("Resetting watchdog timer not supported.\n");
		return CMD_RET_FAILURE;
	} else if (ret) {
		printf("Resetting watchdog timer failed (%d)\n", ret);
		return CMD_RET_FAILURE;
	}

	return CMD_RET_SUCCESS;
}

static int do_wdt_gettimeout(struct cmd_tbl *cmdtp, int flag, int argc,
			     char *const argv[])
{
	int ret;
	u64 timeout_ms;
	struct udevice *currdev;

	if (check_currdev(&currdev))
		return CMD_RET_FAILURE;

	ret = wdt_get_timeout(currdev, &timeout_ms);
	if (ret == -ENOSYS)
		printf("Getting watchdog timeout not supported.\n");
	else if (ret == -ENODATA)
		printf("Watchdog not running.\n");
	else if (ret)
		printf("Getting watchdog timeout failed (%d)\n", ret);
	else
		printf("Timeout: %llums\n", timeout_ms);

	return ret ? CMD_RET_FAILURE : CMD_RET_SUCCESS;
}

static int do_wdt_expire(struct cmd_tbl *cmdtp, int flag, int argc,
			 char *const argv[])
{
	int ret;
	ulong flags = 0;
	struct udevice *currdev;

	if (check_currdev(&currdev))
		return CMD_RET_FAILURE;

	if (argc > 1)
		flags = simple_strtoul(argv[1], NULL, 0);

	ret = wdt_expire_now(currdev, flags);
	if (ret == -ENOSYS) {
		printf("Expiring watchdog timer not supported.\n");
		return CMD_RET_FAILURE;
	} else if (ret) {
		printf("Expiring watchdog timer failed (%d)\n", ret);
		return CMD_RET_FAILURE;
	}

	return CMD_RET_SUCCESS;
}

U_BOOT_LONGHELP(wdt,
	"list - list watchdog devices\n"
	"wdt dev [<name>] - get/set current watchdog device\n"
	"wdt start <timeout ms> [flags] - start watchdog timer\n"
	"wdt stop - stop watchdog timer\n"
	"wdt reset - reset watchdog timer\n"
	"wdt gettimeout - get device timeout in ms\n"
	"wdt expire [flags] - expire watchdog timer immediately\n");

U_BOOT_CMD_WITH_SUBCMDS(wdt, "Watchdog sub-system", wdt_help_text,
	U_BOOT_SUBCMD_MKENT(list, 1, 1, do_wdt_list),
	U_BOOT_SUBCMD_MKENT(dev, 2, 1, do_wdt_dev),
	U_BOOT_SUBCMD_MKENT(start, 3, 1, do_wdt_start),
	U_BOOT_SUBCMD_MKENT(stop, 1, 1, do_wdt_stop),
	U_BOOT_SUBCMD_MKENT(reset, 1, 1, do_wdt_reset),
	U_BOOT_SUBCMD_MKENT(gettimeout, 1, 1, do_wdt_gettimeout),
	U_BOOT_SUBCMD_MKENT(expire, 2, 1, do_wdt_expire));
