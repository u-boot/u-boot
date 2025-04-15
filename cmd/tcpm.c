// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2024 Collabora
 */

#include <command.h>
#include <errno.h>
#include <dm.h>
#include <dm/uclass-internal.h>
#include <usb/tcpm.h>

#define LIMIT_DEV	32
#define LIMIT_PARENT	20

static struct udevice *currdev;

static int do_dev(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[])
{
	int devnum, ret;

	switch (argc) {
	case 2:
		devnum = (int)dectoul(argv[1], NULL);
		ret = tcpm_get(devnum, &currdev);
		if (ret) {
			log_err("Can't get TCPM %d: %d (%s)!\n", devnum, ret, errno_str(ret));
			return CMD_RET_FAILURE;
		}
	case 1:
		if (!currdev) {
			log_err("TCPM device is not set!\n\n");
			return CMD_RET_USAGE;
		}

		printf("dev: %d @ %s\n", dev_seq(currdev), currdev->name);
	}

	return CMD_RET_SUCCESS;
}

static int do_list(struct cmd_tbl *cmdtp, int flag, int argc,
		   char *const argv[])
{
	struct udevice *dev;
	int ret, err = 0;

	printf("| ID | %-*.*s| %-*.*s| %s @ %s\n",
	       LIMIT_DEV, LIMIT_DEV, "Name",
	       LIMIT_PARENT, LIMIT_PARENT, "Parent name",
	       "Parent uclass", "seq");

	for (ret = uclass_first_device_check(UCLASS_TCPM, &dev); dev;
	     ret = uclass_next_device_check(&dev)) {
		if (ret)
			err = ret;

		printf("| %2d | %-*.*s| %-*.*s| %s @ %d | status: %i\n",
		       dev_seq(dev),
		       LIMIT_DEV, LIMIT_DEV, dev->name,
		       LIMIT_PARENT, LIMIT_PARENT, dev->parent->name,
		       dev_get_uclass_name(dev->parent), dev_seq(dev->parent),
		       ret);
	}

	if (err)
		return CMD_RET_FAILURE;

	return CMD_RET_SUCCESS;
}

int do_print_info(struct udevice *dev)
{
	enum typec_orientation orientation = tcpm_get_orientation(dev);
	const char *state = tcpm_get_state(dev);
	int pd_rev = tcpm_get_pd_rev(dev);
	int mv = tcpm_get_voltage(dev);
	int ma = tcpm_get_current(dev);
	enum typec_role pwr_role = tcpm_get_pwr_role(dev);
	enum typec_data_role data_role = tcpm_get_data_role(dev);
	bool connected = tcpm_is_connected(dev);

	if (!connected) {
		printf("TCPM State: %s\n", state);
		return 0;
	}

	printf("Orientation: %s\n", typec_orientation_name[orientation]);
	printf("PD Revision: %s\n", typec_pd_rev_name[pd_rev]);
	printf("Power Role:  %s\n", typec_role_name[pwr_role]);
	printf("Data Role:   %s\n", typec_data_role_name[data_role]);
	printf("Voltage:     %2d.%03d V\n", mv / 1000, mv % 1000);
	printf("Current:     %2d.%03d A\n", ma / 1000, ma % 1000);

	return 0;
}

static int do_info(struct cmd_tbl *cmdtp, int flag, int argc,
		   char *const argv[])
{
	if (!currdev) {
		printf("First, set the TCPM device!\n");
		return CMD_RET_USAGE;
	}

	return do_print_info(currdev);
}

static struct cmd_tbl subcmd[] = {
	U_BOOT_CMD_MKENT(dev, 2, 1, do_dev, "", ""),
	U_BOOT_CMD_MKENT(list, 1, 1, do_list, "", ""),
	U_BOOT_CMD_MKENT(info, 1, 1, do_info, "", ""),
};

static int do_tcpm(struct cmd_tbl *cmdtp, int flag, int argc,
		   char *const argv[])
{
	struct cmd_tbl *cmd;

	argc--;
	argv++;

	cmd = find_cmd_tbl(argv[0], subcmd, ARRAY_SIZE(subcmd));
	if (!cmd || argc > cmd->maxargs)
		return CMD_RET_USAGE;

	return cmd->cmd(cmdtp, flag, argc, argv);
}

 /**************************************************/

U_BOOT_CMD(tcpm, CONFIG_SYS_MAXARGS, 1, do_tcpm,
	"TCPM sub-system",
	"list          - list TCPM devices\n"
	"tcpm dev [ID]      - show or [set] operating TCPM device\n"
	"tcpm info          - dump information\n"
);
