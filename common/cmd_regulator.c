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
#include <power/regulator.h>

#define LIMIT_SEQ	3
#define LIMIT_DEVNAME	20
#define LIMIT_OFNAME	20
#define LIMIT_INFO	16

static struct udevice *currdev;

static int failed(const char *getset, const char *thing,
		  const char *for_dev, int ret)
{
	printf("Can't %s %s %s.\nError: %d (%s)\n", getset, thing, for_dev,
						    ret, errno_str(ret));
	return CMD_RET_FAILURE;
}

static int regulator_get(bool list_only, int get_seq, struct udevice **devp)
{
	struct dm_regulator_uclass_platdata *uc_pdata;
	struct udevice *dev;
	int ret;

	if (devp)
		*devp = NULL;

	for (ret = uclass_first_device(UCLASS_REGULATOR, &dev); dev;
	     ret = uclass_next_device(&dev)) {
		if (list_only) {
			uc_pdata = dev_get_uclass_platdata(dev);
			printf("|%*d | %*.*s @ %-*.*s| %s @ %s\n",
			       LIMIT_SEQ, dev->seq,
			       LIMIT_DEVNAME, LIMIT_DEVNAME, dev->name,
			       LIMIT_OFNAME, LIMIT_OFNAME, uc_pdata->name,
			       dev->parent->name,
			       dev_get_uclass_name(dev->parent));
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
	struct dm_regulator_uclass_platdata *uc_pdata;
	int seq, ret = -ENXIO;

	switch (argc) {
	case 2:
		seq = simple_strtoul(argv[1], NULL, 0);
		ret = uclass_get_device_by_seq(UCLASS_REGULATOR, seq, &currdev);
		if (ret && (ret = regulator_get(false, seq, &currdev)))
			goto failed;
	case 1:
		uc_pdata = dev_get_uclass_platdata(currdev);
		if (!uc_pdata)
			goto failed;

		printf("dev: %d @ %s\n", currdev->seq, uc_pdata->name);
	}

	return CMD_RET_SUCCESS;
failed:
	return failed("get", "the", "device", ret);
}

static int get_curr_dev_and_pl(struct udevice **devp,
			       struct dm_regulator_uclass_platdata **uc_pdata,
			       bool allow_type_fixed)
{
	*devp = NULL;
	*uc_pdata = NULL;

	if (!currdev)
		return failed("get", "current", "device", -ENODEV);

	*devp = currdev;

	*uc_pdata = dev_get_uclass_platdata(*devp);
	if (!*uc_pdata)
		return failed("get", "regulator", "platdata", -ENXIO);

	if (!allow_type_fixed && (*uc_pdata)->type == REGULATOR_TYPE_FIXED) {
		printf("Operation not allowed for fixed regulator!\n");
		return CMD_RET_FAILURE;
	}

	return CMD_RET_SUCCESS;
}

static int do_list(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	int ret;

	printf("|%*s | %*.*s @ %-*.*s| %s @ %s\n",
	       LIMIT_SEQ, "Seq",
	       LIMIT_DEVNAME, LIMIT_DEVNAME, "Name",
	       LIMIT_OFNAME, LIMIT_OFNAME, "fdtname",
	       "Parent", "uclass");

	ret = regulator_get(true, 0, NULL);
	if (ret)
		return CMD_RET_FAILURE;

	return CMD_RET_SUCCESS;
}

static int constraint(const char *name, int val, const char *val_name)
{
	printf("%-*s", LIMIT_INFO, name);
	if (val < 0) {
		printf(" %s (err: %d)\n", errno_str(val), val);
		return val;
	}

	if (val_name)
		printf(" %d (%s)\n", val, val_name);
	else
		printf(" %d\n", val);

	return 0;
}

static const char *get_mode_name(struct dm_regulator_mode *mode,
				 int mode_count,
				 int mode_id)
{
	while (mode_count--) {
		if (mode->id == mode_id)
			return mode->name;
		mode++;
	}

	return NULL;
}

static int do_info(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	struct udevice *dev;
	struct dm_regulator_uclass_platdata *uc_pdata;
	struct dm_regulator_mode *modes;
	const char *parent_uc;
	int mode_count;
	int ret;
	int i;

	ret = get_curr_dev_and_pl(&dev, &uc_pdata, true);
	if (ret)
		return ret;

	parent_uc = dev_get_uclass_name(dev->parent);

	printf("Uclass regulator dev %d info:\n", dev->seq);
	printf("%-*s %s @ %s\n%-*s %s\n%-*s %s\n%-*s\n",
	       LIMIT_INFO, "* parent:", dev->parent->name, parent_uc,
	       LIMIT_INFO, "* dev name:", dev->name,
	       LIMIT_INFO, "* fdt name:", uc_pdata->name,
	       LIMIT_INFO, "* constraints:");

	constraint("  - min uV:", uc_pdata->min_uV, NULL);
	constraint("  - max uV:", uc_pdata->max_uV, NULL);
	constraint("  - min uA:", uc_pdata->min_uA, NULL);
	constraint("  - max uA:", uc_pdata->max_uA, NULL);
	constraint("  - always on:", uc_pdata->always_on,
		   uc_pdata->always_on ? "true" : "false");
	constraint("  - boot on:", uc_pdata->boot_on,
		   uc_pdata->boot_on ? "true" : "false");

	mode_count = regulator_mode(dev, &modes);
	constraint("* op modes:", mode_count, NULL);

	for (i = 0; i < mode_count; i++, modes++)
		constraint("  - mode id:", modes->id, modes->name);

	return CMD_RET_SUCCESS;
}

static int do_status(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	struct dm_regulator_uclass_platdata *uc_pdata;
	int current, value, mode, ret;
	const char *mode_name = NULL;
	struct udevice *dev;
	bool enabled;

	ret = get_curr_dev_and_pl(&dev, &uc_pdata, true);
	if (ret)
		return ret;

	enabled = regulator_get_enable(dev);
	constraint(" * enable:", enabled, enabled ? "true" : "false");

	value = regulator_get_value(dev);
	constraint(" * value uV:", value, NULL);

	current = regulator_get_current(dev);
	constraint(" * current uA:", current, NULL);

	mode = regulator_get_mode(dev);
	mode_name = get_mode_name(uc_pdata->mode, uc_pdata->mode_count, mode);
	constraint(" * mode id:", mode, mode_name);

	return CMD_RET_SUCCESS;
}

static int do_value(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	struct udevice *dev;
	struct dm_regulator_uclass_platdata *uc_pdata;
	int value;
	int force;
	int ret;

	ret = get_curr_dev_and_pl(&dev, &uc_pdata, argc == 1);
	if (ret)
		return ret;

	if (argc == 1) {
		value = regulator_get_value(dev);
		if (value < 0)
			return failed("get", uc_pdata->name, "voltage", value);

		printf("%d uV\n", value);
		return CMD_RET_SUCCESS;
	}

	if (argc == 3)
		force = !strcmp("-f", argv[2]);
	else
		force = 0;

	value = simple_strtoul(argv[1], NULL, 0);
	if ((value < uc_pdata->min_uV || value > uc_pdata->max_uV) && !force) {
		printf("Value exceeds regulator constraint limits\n");
		return CMD_RET_FAILURE;
	}

	ret = regulator_set_value(dev, value);
	if (ret)
		return failed("set", uc_pdata->name, "voltage value", ret);

	return CMD_RET_SUCCESS;
}

static int do_current(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	struct udevice *dev;
	struct dm_regulator_uclass_platdata *uc_pdata;
	int current;
	int ret;

	ret = get_curr_dev_and_pl(&dev, &uc_pdata, argc == 1);
	if (ret)
		return ret;

	if (argc == 1) {
		current = regulator_get_current(dev);
		if (current < 0)
			return failed("get", uc_pdata->name, "current", current);

		printf("%d uA\n", current);
		return CMD_RET_SUCCESS;
	}

	current = simple_strtoul(argv[1], NULL, 0);
	if (current < uc_pdata->min_uA || current > uc_pdata->max_uA) {
		printf("Current exceeds regulator constraint limits\n");
		return CMD_RET_FAILURE;
	}

	ret = regulator_set_current(dev, current);
	if (ret)
		return failed("set", uc_pdata->name, "current value", ret);

	return CMD_RET_SUCCESS;
}

static int do_mode(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	struct udevice *dev;
	struct dm_regulator_uclass_platdata *uc_pdata;
	int new_mode;
	int mode;
	int ret;

	ret = get_curr_dev_and_pl(&dev, &uc_pdata, false);
	if (ret)
		return ret;

	if (argc == 1) {
		mode = regulator_get_mode(dev);
		if (mode < 0)
			return failed("get", uc_pdata->name, "mode", mode);

		printf("mode id: %d\n", mode);
		return CMD_RET_SUCCESS;
	}

	new_mode = simple_strtoul(argv[1], NULL, 0);

	ret = regulator_set_mode(dev, new_mode);
	if (ret)
		return failed("set", uc_pdata->name, "mode", ret);

	return CMD_RET_SUCCESS;
}

static int do_enable(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	struct udevice *dev;
	struct dm_regulator_uclass_platdata *uc_pdata;
	int ret;

	ret = get_curr_dev_and_pl(&dev, &uc_pdata, true);
	if (ret)
		return ret;

	ret = regulator_set_enable(dev, true);
	if (ret)
		return failed("enable", "regulator", uc_pdata->name, ret);

	return CMD_RET_SUCCESS;
}

static int do_disable(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	struct udevice *dev;
	struct dm_regulator_uclass_platdata *uc_pdata;
	int ret;

	ret = get_curr_dev_and_pl(&dev, &uc_pdata, true);
	if (ret)
		return ret;

	ret = regulator_set_enable(dev, false);
	if (ret)
		return failed("disable", "regulator", uc_pdata->name, ret);

	return CMD_RET_SUCCESS;
}

static cmd_tbl_t subcmd[] = {
	U_BOOT_CMD_MKENT(dev, 2, 1, do_dev, "", ""),
	U_BOOT_CMD_MKENT(list, 1, 1, do_list, "", ""),
	U_BOOT_CMD_MKENT(info, 2, 1, do_info, "", ""),
	U_BOOT_CMD_MKENT(status, 2, 1, do_status, "", ""),
	U_BOOT_CMD_MKENT(value, 3, 1, do_value, "", ""),
	U_BOOT_CMD_MKENT(current, 3, 1, do_current, "", ""),
	U_BOOT_CMD_MKENT(mode, 2, 1, do_mode, "", ""),
	U_BOOT_CMD_MKENT(enable, 1, 1, do_enable, "", ""),
	U_BOOT_CMD_MKENT(disable, 1, 1, do_disable, "", ""),
};

static int do_regulator(cmd_tbl_t *cmdtp, int flag, int argc,
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

U_BOOT_CMD(regulator, CONFIG_SYS_MAXARGS, 1, do_regulator,
	"uclass operations",
	"list         - list UCLASS regulator devices\n"
	"regulator dev [id]     - show or [set] operating regulator device\n"
	"regulator [info]       - print constraints info\n"
	"regulator [status]     - print operating status\n"
	"regulator [value] [-f] - print/[set] voltage value [uV] (force)\n"
	"regulator [current]    - print/[set] current value [uA]\n"
	"regulator [mode_id]    - print/[set] operating mode id\n"
	"regulator [enable]     - enable the regulator output\n"
	"regulator [disable]    - disable the regulator output\n"
);
