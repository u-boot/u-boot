/*
 * Copyright (C) 2019 Marvell International Ltd.
 *
 * SPDX-License-Identifier:    GPL-2.0
 * https://spdx.org/licenses
 */

#include <common.h>
#include <command.h>
#include <dm.h>
#include <dm/uclass-internal.h>
#include <net.h>

extern int cgx_intf_set_fec(struct udevice *ethdev, int type);
extern int cgx_intf_get_fec(struct udevice *ethdev);
extern int cgx_intf_get_phy_mod_type(struct udevice *ethdev);
extern int cgx_intf_set_phy_mod_type(struct udevice *ethdev, int type);
extern int cgx_intf_set_mode(struct udevice *ethdev, int mode);
extern int cgx_intf_set_an_lbk(struct udevice *ethdev, int enable);
extern int cgx_intf_set_ignore(struct udevice *ethdev, int cgxid, int lmacid,
			       int ignore);
extern int cgx_intf_get_ignore(struct udevice *ethdev, int cgxid, int lmacid);
extern int cgx_intf_get_mode(struct udevice *ethdev);
extern void nix_print_mac_info(struct udevice *dev);

static int do_ethlist(cmd_tbl_t *cmdtp, int flag, int argc,
		      char *const argv[])
{
	struct udevice *dev;

	for (uclass_find_first_device(UCLASS_ETH, &dev); dev;
	     uclass_find_next_device(&dev)) {
		printf("eth%d [%s]", dev->seq, dev->name);
		if (!strncmp(dev->name, "rvu_", 4))
			nix_print_mac_info(dev);
		printf("\n");
	}
	return 0;
}

U_BOOT_CMD(
	ethlist, 1, 1, do_ethlist, "Display ethernet interface list",
	"Prints all detected ethernet interfaces with below format\n"
	"ethX [device name] [LMAC info for RVU PF devices]\n"
);

static int do_ethparam_common(cmd_tbl_t *cmdtp, int flag, int argc,
			      char *const argv[])
{
	const char *cmd;
	char *endp;
	const char *devname;
	int ret = CMD_RET_USAGE, arg, cgx = -1, lmac = -1;
	struct udevice *dev = NULL;

	if (argc < 2)
		return ret;

	cmd = argv[0];

	if (strncmp(argv[1], "eth", 3)) {
		if (strcmp(cmd, "set_ignore") == 0) {
			if (argc != 4)
				return ret;
			cgx = simple_strtol(argv[1], &endp, 0);
			if (cgx < 0 || cgx > 4)
				return ret;
			lmac = simple_strtol(argv[2], &endp, 0);
			if (lmac < 0 || lmac > 3)
				return ret;
		} else if (strcmp(cmd, "get_ignore") == 0) {
			if (argc != 3)
				return ret;
			cgx = simple_strtol(argv[1], &endp, 0);
			if (cgx < 0 || cgx > 4)
				return ret;
			lmac = simple_strtol(argv[2], &endp, 0);
			if (lmac < 0 || lmac > 3)
				return ret;
		} else {
			return ret;
		}
	} else {
		devname = argv[1];
		dev = eth_get_dev_by_name(devname);
		if (!dev) {
			printf("device interface %s not found\n", devname);
			return CMD_RET_FAILURE;
		}
		if (strncmp(dev->name, "rvu_", 4)) {
			printf("Invalid eth interface choose RVU PF device\n");
			return CMD_RET_FAILURE;
		}
	}

	if (strcmp(cmd, "set_fec") == 0) {
		if (argc < 3)
			return CMD_RET_FAILURE;
		arg = simple_strtol(argv[2], &endp, 0);
		if (arg < 0 || arg > 2)
			return ret;
		ret = cgx_intf_set_fec(dev, arg);
	} else if (strcmp(cmd, "get_fec") == 0) {
		ret = cgx_intf_get_fec(dev);
	} else if (strcmp(cmd, "set_an_lbk") == 0) {
		if (argc < 3)
			return CMD_RET_FAILURE;
		arg = simple_strtol(argv[2], &endp, 0);
		if (arg < 0 || arg > 1)
			return CMD_RET_USAGE;
		ret = cgx_intf_set_an_lbk(dev, arg);
	} else if (strcmp(cmd, "set_ignore") == 0) {
		if (dev)
			arg = simple_strtol(argv[2], &endp, 0);
		else
			arg = simple_strtol(argv[3], &endp, 0);
		if (arg < 0 || arg > 1)
			return ret;
		ret = cgx_intf_set_ignore(dev, cgx, lmac, arg);
	} else if (strcmp(cmd, "get_ignore") == 0) {
		ret = cgx_intf_get_ignore(dev, cgx, lmac);
	} else if (strcmp(cmd, "set_phymod") == 0) {
		if (argc < 3)
			return CMD_RET_FAILURE;
		arg = simple_strtol(argv[2], &endp, 0);
		if (arg < 0 || arg > 1)
			return ret;
		ret = cgx_intf_set_phy_mod_type(dev, arg);
	} else if (strcmp(cmd, "get_phymod") == 0) {
		ret = cgx_intf_get_phy_mod_type(dev);
	} else if (strcmp(cmd, "get_mode") == 0) {
		ret = cgx_intf_get_mode(dev);
	} else if (strcmp(cmd, "set_mode") == 0) {
		if (argc < 3)
			return CMD_RET_FAILURE;
		arg = simple_strtol(argv[2], &endp, 0);
		if (arg < 0 || arg > 6)
			return ret;
		ret = cgx_intf_set_mode(dev, arg);
	}
	return (ret == 0) ? CMD_RET_SUCCESS : CMD_RET_FAILURE;
}

U_BOOT_CMD(
	set_fec, 3, 1, do_ethparam_common,
	"Modify fec type for selected ethernet interface",
	"Example - set_fec <ethX> [type]\n"
	"Set FEC type for any of RVU PF based network interfaces\n"
	"- where type - 0 [NO FEC] 1 [BASER_FEC] 2 [RS_FEC]\n"
	"Use 'ethlist' command to display network interface names\n"
);

U_BOOT_CMD(
	get_fec, 2, 1, do_ethparam_common,
	"Display fec type for selected ethernet interface",
	"Example - get_fec <ethX>\n"
	"Get FEC type for any of RVU PF based network interfaces\n"
	"Use 'ethlist' command to display network interface names\n"
);

U_BOOT_CMD(set_an_lbk, 3, 1, do_ethparam_common,
	   "Set or clear Auto-neg loopback for ethernet interface",
	   "Example - set_an_lbk <ethX> [value]\n"
	   "0 [clear] or 1 [set]\n"
	   "Use 'ethlist' command to display network interface names\n"
);

U_BOOT_CMD(set_ignore, 4, 1, do_ethparam_common,
	   "Set or clear ignore param in persist storage for eth interface",
	   "Example - set_ignore <ethX> [value]\n"
	   "0 [clear ignore] or 1 [set ignore]\n"
	   "Example - set_ignore <cgx#> <lmac#> [value]\n"
	   "For CGX0 LMAC3 - set_ignore 0 3 [value]\n"
	   "0 [clear ignore] or 1 [set ignore]\n"
	   "Helps to ignore all persist settings for selected ethernet\n"
	   "interface on next boot\n"
	   "Use 'ethlist' command to display network interface names\n"
);

U_BOOT_CMD(get_ignore, 3, 1, do_ethparam_common,
	   "Display ignore param in persist storage for ethernet interface",
	   "Example - get_ignore <ethX> or\n"
	   "Example - get_ignore <cgx#> <lmac#>\n"
	   "For CGX0 LMAC3 - get_ignore 0 3\n"
	   "Use 'ethlist' command to display network interface names\n"
);

U_BOOT_CMD(
	set_phymod, 3, 1, do_ethparam_common,
	"Modify line side phy-mod type for selected ethernet interface",
	"Example - set_phymod <ethX> [type]\n"
	"Set PHY MOD type for any of RVU PF based network interfaces\n"
	"Currently, only 50G_R mode supports type 0 [NRZ] or 1 [PAM4]\n"
	"Use 'ethlist' command to display network interface names\n"
);

U_BOOT_CMD(
	get_phymod, 2, 1, do_ethparam_common,
	"Display line side phy-mod type for selected ethernet interface",
	"Example - get_phymod <ethX>\n"
	"Get PHY MOD type for any of RVU PF based network interfaces\n"
	"Use 'ethlist' command to display network interface names\n"
);

U_BOOT_CMD(get_mode, 2, 1, do_ethparam_common,
	   "Display Interface mode for selected ethernet interface",
	   "Example - get_mode <ethX>\n"
	   "Use 'ethlist' command to display network interface names\n"
);

U_BOOT_CMD(set_mode, 3, 1, do_ethparam_common,
	   "Modify Interface mode for selected ethernet interface",
	   "Example - set_mode <ethX> [mode]\n"
	   "Change mode of selected network interface\n"
	   "\n"
	   "mode encoding -\n"
	   "	0 - 10G_C2C\n"
	   "	1 - 10G_C2M\n"
	   "	2 - 10G_KR\n"
	   "	3 - 25G_C2C\n"
	   "	4 - 25G_2_C2C\n"
	   "	5 - 50G_C2C\n"
	   "	6 - 50G_4_C2C\n"
	   "Use 'ethlist' command to display network interface names\n"
);

