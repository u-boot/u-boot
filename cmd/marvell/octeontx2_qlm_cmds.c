// SPDX-License-Identifier:    GPL-2.0
/*
 * https://spdx.org/licenses
 *
 * Copyright (C) 2019 Marvell International Ltd.
 */

#include <common.h>
#include <command.h>

extern int cgx_intf_prbs(u8 qlm, u8 mode, u32 time, u32 lane);
extern int cgx_intf_display_eye(u8 qlm, u8 lane);
extern int cgx_intf_display_serdes(u8 qlm, u8 lane);

static int do_prbs(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	ulong time;
	ulong mode;
	ulong qlm;
	ulong lane;

	if (argc == 5) {
		qlm = simple_strtoul(argv[1], NULL, 10);
		lane = simple_strtoul(argv[2], NULL, 10);
		mode = simple_strtoul(argv[3], NULL, 10);
		time = simple_strtoul(argv[4], NULL, 10);
	} else {
		return CMD_RET_USAGE;
	}

	cgx_intf_prbs(qlm, mode, time, lane);

	return CMD_RET_SUCCESS;
}

U_BOOT_CMD(prbs,    5,    1,     do_prbs,
	   "command to run PRBS on slected QLM",
	   "<qlm> <lane> <mode> <time> \n"
	   "    - run PRBS with pattern indicated by 'mode' on selected 'qlm'\n"
	   "      PRBS will be enabled by 'time' seconds\n"
	   "      PRBS is performed on a particular 'lane'"
);

static int do_eye(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	ulong lane;
	ulong qlm;

	if (argc != 3)
		return CMD_RET_USAGE;

	qlm = simple_strtoul(argv[1], NULL, 10);
	lane = simple_strtoul(argv[2], NULL, 10);

	cgx_intf_display_eye(qlm, lane);

	return CMD_RET_SUCCESS;
}

U_BOOT_CMD(eye,    3,    1,     do_eye,
	   "command to display eye on slected QLM",
	   "<qlm> <lane>\n"
	   "    - run eye by 'lane'  'qlm'\n"
);

static int do_serdes(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	ulong lane;
	ulong qlm;

	if (argc != 3)
		return CMD_RET_USAGE;

	qlm = simple_strtoul(argv[1], NULL, 10);
	lane = simple_strtoul(argv[2], NULL, 10);

	cgx_intf_display_serdes(qlm, lane);

	return CMD_RET_SUCCESS;
}

U_BOOT_CMD(serdes,    3,    1,     do_serdes,
	   "command to display serdes state of a lane in a given QLM",
	   "<qlm> <lane>\n"
	   "    - display serdes state of 'lane' in 'qlm'\n"
);
