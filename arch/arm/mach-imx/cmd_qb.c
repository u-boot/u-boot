// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2024-2026 NXP
 */
#include <command.h>
#include <spl.h>
#include <stdlib.h>

#include <asm/mach-imx/boot_mode.h>
#include <asm/mach-imx/sys_proto.h>
#include <asm/mach-imx/qb.h>

static void parse_qb_args(int argc, char * const argv[],
			  const char **ifname, const char **dev)
{
	/* qb save/erase -> use boot device */
	if (argc < 2) {
		*ifname = "auto";
		return;
	}

	*ifname = argv[1];

	if (argc == 3)
		*dev = argv[2];
}

static int do_qb(struct cmd_tbl *cmdtp, int flag, int argc,
		 char * const argv[], bool save)
{
	const char *ifname, *dev;

	parse_qb_args(argc, argv, &ifname, &dev);

	if (imx_qb(ifname, dev, save))
		return CMD_RET_FAILURE;

	return CMD_RET_SUCCESS;
}

static int do_qb_check(struct cmd_tbl *cmdtp, int flag,
		       int argc, char * const argv[])
{
	return imx_qb_check() ? CMD_RET_SUCCESS : CMD_RET_FAILURE;
}

static int do_qb_save(struct cmd_tbl *cmdtp, int flag,
		      int argc, char * const argv[])
{
	return do_qb(cmdtp, flag, argc, argv, true);
}

static int do_qb_erase(struct cmd_tbl *cmdtp, int flag,
		       int argc, char * const argv[])
{
	return do_qb(cmdtp, flag, argc, argv, false);
}

static struct cmd_tbl cmd_qb[] = {
	U_BOOT_CMD_MKENT(check, 1, 1, do_qb_check, "", ""),
	U_BOOT_CMD_MKENT(save,  3, 1, do_qb_save,  "", ""),
	U_BOOT_CMD_MKENT(erase, 3, 1, do_qb_erase, "", ""),
};

static int do_qbops(struct cmd_tbl *cmdtp, int flag, int argc,
		    char *const argv[])
{
	struct cmd_tbl *cp;

	cp = find_cmd_tbl(argv[1], cmd_qb, ARRAY_SIZE(cmd_qb));

	/* Drop the qb command */
	argc--;
	argv++;

	if (!cp) {
		printf("qb: %s: command not found\n", argv[0] ? argv[0] : " ");
		return CMD_RET_USAGE;
	}

	if (argc > cp->maxargs) {
		printf("qb %s: too many arguments: %d > %d\n", cp->name,
		       argc - 1, cp->maxargs - 1);
		return CMD_RET_USAGE;
	}

	if (flag == CMD_FLAG_REPEAT && !cmd_is_repeatable(cp)) {
		printf("qb %s: repeat flag set but command is not repeatable\n",
		       cp->name);
		return CMD_RET_SUCCESS;
	}

	return cp->cmd(cmdtp, flag, argc, argv);
}

U_BOOT_CMD(
	qb, 4, 1, do_qbops,
	"DDR Quick Boot sub system",
	"check - check if quick boot data is stored in mem by training flow\n"
	"qb save [interface] [dev]  - save quick boot data in NVM => trigger quick boot flow\n"
	"qb erase [interface] [dev] - erase quick boot data from NVM => trigger training flow\n"
);
