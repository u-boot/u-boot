/*
 * Copyright (C) 2014-2015 Samsung Electronics
 * Przemyslaw Marczak <p.marczak@samsung.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */
#include <common.h>
#include <errno.h>
#include <dm.h>
#include <dm/uclass-internal.h>
#include <power/pmic.h>

#define LIMIT_SEQ	3
#define LIMIT_DEVNAME	20

static struct udevice *currdev;

static int failed(const char *getset, const char *thing,
		  const char *for_dev, int ret)
{
	printf("Can't %s %s %s.\nError: %d (%s)\n", getset, thing, for_dev,
						    ret, errno_str(ret));
	return CMD_RET_FAILURE;
}

static int pmic_dev_get(bool list_only, int get_seq, struct udevice **devp)
{
	struct udevice *dev;
	int ret;

	if (devp)
		*devp = NULL;

	for (ret = uclass_first_device(UCLASS_PMIC, &dev); dev;
	     ret = uclass_next_device(&dev)) {
		if (list_only) {
			printf("|%*d | %-*.*s| %-*.*s| %s @ %d\n",
			       LIMIT_SEQ, dev->seq,
			       LIMIT_DEVNAME, LIMIT_DEVNAME, dev->name,
			       LIMIT_DEVNAME, LIMIT_DEVNAME, dev->parent->name,
			       dev_get_uclass_name(dev->parent),
			       dev->parent->seq);
			continue;
		}

		if (dev->seq == get_seq) {
			if (devp)
				*devp = dev;
			else
				return -EINVAL;

			return 0;
		}
	}

	if (list_only)
		return ret;

	return -ENODEV;
}

static int do_dev(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	int seq, ret = -ENODEV;

	switch (argc) {
	case 2:
		seq = simple_strtoul(argv[1], NULL, 0);
		ret = uclass_get_device_by_seq(UCLASS_PMIC, seq, &currdev);
		if (ret && (ret = pmic_dev_get(false, seq, &currdev)))
			goto failed;
	case 1:
		if (!currdev)
			goto failed;

		printf("dev: %d @ %s\n", currdev->seq, currdev->name);
	}

	return CMD_RET_SUCCESS;
failed:
	return failed("get", "the", "device", ret);
}

static int do_list(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	struct udevice *dev;
	int ret;

	printf("|%*s | %-*.*s| %-*.*s| %s @ %s\n",
	       LIMIT_SEQ, "Seq",
	       LIMIT_DEVNAME, LIMIT_DEVNAME, "Name",
	       LIMIT_DEVNAME, LIMIT_DEVNAME, "Parent name",
	       "Parent uclass", "seq");

	for (ret = uclass_first_device(UCLASS_PMIC, &dev); dev;
	     ret = uclass_next_device(&dev)) {
		printf("|%*d | %-*.*s| %-*.*s| %s @ %d\n",
		       LIMIT_SEQ, dev->seq,
		       LIMIT_DEVNAME, LIMIT_DEVNAME, dev->name,
		       LIMIT_DEVNAME, LIMIT_DEVNAME, dev->parent->name,
		       dev_get_uclass_name(dev->parent), dev->parent->seq);
	}

	if (ret)
		return CMD_RET_FAILURE;

	return CMD_RET_SUCCESS;
}

static int do_dump(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	struct udevice *dev;
	uint8_t value;
	uint reg;
	int ret;

	if (!currdev)
		return failed("get", "current", "device", -ENODEV);

	dev = currdev;

	printf("Dump pmic: %s registers\n", dev->name);

	for (reg = 0; reg < pmic_reg_count(dev); reg++) {
		ret = pmic_read(dev, reg, &value, 1);
		if (ret)
			return failed("read", dev->name, "register", ret);

		if (!(reg % 16))
			printf("\n0x%02x: ", reg);

		printf("%2.2x ", value);
	}
	printf("\n");

	return CMD_RET_SUCCESS;
}

static int do_read(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	struct udevice *dev;
	int regs, ret;
	uint8_t value;
	uint reg;

	if (!currdev)
		return failed("get", "current", "device", -ENODEV);

	dev = currdev;

	if (argc != 2)
		return CMD_RET_USAGE;

	reg = simple_strtoul(argv[1], NULL, 0);
	regs = pmic_reg_count(dev);
	if (reg > regs) {
		printf("Pmic max reg: %d\n", regs);
		return failed("read", "given", "address", -EFAULT);
	}

	ret = pmic_read(dev, reg, &value, 1);
	if (ret)
		return failed("read", dev->name, "register", ret);

	printf("0x%02x: 0x%2.2x\n", reg, value);

	return CMD_RET_SUCCESS;
}

static int do_write(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	struct udevice *dev;
	int regs, ret;
	uint8_t value;
	uint reg;

	if (!currdev)
		return failed("get", "current", "device", -ENODEV);

	dev = currdev;

	if (argc != 3)
		return CMD_RET_USAGE;

	reg = simple_strtoul(argv[1], NULL, 0);
	regs = pmic_reg_count(dev);
	if (reg > regs) {
		printf("Pmic max reg: %d\n", regs);
		return failed("write", "given", "address", -EFAULT);
	}

	value = simple_strtoul(argv[2], NULL, 0);

	ret = pmic_write(dev, reg, &value, 1);
	if (ret)
		return failed("write", dev->name, "register", ret);

	return CMD_RET_SUCCESS;
}

static cmd_tbl_t subcmd[] = {
	U_BOOT_CMD_MKENT(dev, 2, 1, do_dev, "", ""),
	U_BOOT_CMD_MKENT(list, 1, 1, do_list, "", ""),
	U_BOOT_CMD_MKENT(dump, 1, 1, do_dump, "", ""),
	U_BOOT_CMD_MKENT(read, 2, 1, do_read, "", ""),
	U_BOOT_CMD_MKENT(write, 3, 1, do_write, "", ""),
};

static int do_pmic(cmd_tbl_t *cmdtp, int flag, int argc,
			char * const argv[])
{
	cmd_tbl_t *cmd;

	argc--;
	argv++;

	cmd = find_cmd_tbl(argv[0], subcmd, ARRAY_SIZE(subcmd));
	if (cmd == NULL || argc > cmd->maxargs)
		return CMD_RET_USAGE;

	return cmd->cmd(cmdtp, flag, argc, argv);
}

U_BOOT_CMD(pmic, CONFIG_SYS_MAXARGS, 1, do_pmic,
	" operations",
	"list          - list pmic devices\n"
	"pmic dev [id]      - show or [set] operating pmic device\n"
	"pmic dump          - dump registers\n"
	"pmic read address  - read byte of register at address\n"
	"pmic write address - write byte to register at address\n"
);
