/*
 * Copyright (C) 2019 Marvell International Ltd.
 *
 * SPDX-License-Identifier:    GPL-2.0
 * https://spdx.org/licenses
 */

#include <common.h>
#include <command.h>
#include <dm.h>
#include <asm/arch/smc.h>

extern void nix_get_cgx_lmac_id(struct udevice *dev, int *cgxid, int *lmacid);

static int do_mdio_dbg(cmd_tbl_t *cmdtp, int flag, int argc,
		       char *const argv[])
{
	const char *cmd;
	char *endp;
	const char *devname;
	int ret = -1;
	int mode, phyaddr, devaddr, regaddr, data, cgx, lmac, cgx_lmac;
	struct udevice *dev;

	if (argc < 7)
		return CMD_RET_USAGE;

	devaddr = -1;
	cmd = argv[1];
	devname = argv[2];

	dev = eth_get_dev_by_name(devname);
	if (!dev) {
		printf("device interface %s not found\n", devname);
		return CMD_RET_FAILURE;
	}
	if (strncmp(dev->name, "rvu_", 4)) {
		printf("Invalid eth interface, choose RVU PF device\n");
		return CMD_RET_FAILURE;
	}
	nix_get_cgx_lmac_id(dev, &cgx, &lmac);
	cgx_lmac = (lmac & 0xF) | ((cgx & 0xF) << 4);

	mode = simple_strtol(argv[3], &endp, 0);
	if (mode < 0 || mode > 1) {
		printf("Invalid clause selection\n");
		return CMD_RET_FAILURE;
	}
	phyaddr = simple_strtol(argv[4], &endp, 0);

	if (strcmp(cmd, "read") == 0) {
		if (mode)
			devaddr = simple_strtol(argv[5], &endp, 0);
		regaddr = simple_strtol(argv[6], &endp, 0);
		ret = smc_mdio_dbg_read(cgx_lmac, mode, phyaddr, devaddr,
					regaddr);
		printf("Read register 0x%x devad[%d] of PHY@0x%x => 0x%x\n",
		       regaddr, devaddr, phyaddr, ret);
	} else if (strcmp(cmd, "write") == 0) {
		if (mode)
			devaddr = simple_strtol(argv[5], &endp, 0);
		regaddr = simple_strtol(argv[6], &endp, 0);
		data = simple_strtol(argv[7], &endp, 0);
		ret = smc_mdio_dbg_write(cgx_lmac, mode, phyaddr, devaddr,
					 regaddr, data);
		printf("Write register 0x%x devad[%d] of PHY@0x%x <= 0x%x\n",
		       regaddr, devaddr, phyaddr, data);
	}
	return (ret == 0) ? CMD_RET_SUCCESS : CMD_RET_FAILURE;
}

U_BOOT_CMD(
	mdio_dbg,	8,	1,	do_mdio_dbg,
	"MDIO Debug utility commands",
	"mdio_dbg read <ethX> <mode> <addr> <devad> <reg>\n"
	" - read register of PHY@addr using <mode> at <devad>.<reg>\n"
	"mdio_dbg write <ethX> <mode> <addr> <devad> <reg> <data>\n"
	" - write register of PHY@addr using <mode> at <devad>.<reg>\n"
	"\n"
	" - mode 0 [CLAUSE22] and 1 [CLAUSE45]\n"
	" - devad should be -1 for clause22 and device address for clause45\n"
	"Use 'ethlist' command to display network interface names\n"
);
