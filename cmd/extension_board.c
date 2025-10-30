// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2021
 * Köry Maincent, Bootlin, <kory.maincent@bootlin.com>
 */

#include <exports.h>
#include <command.h>
#include <extension_board.h>

static int do_extension_list(struct cmd_tbl *cmdtp, int flag,
			     int argc, char *const argv[])
{
	struct extension *extension;
	int i = 0;

	if (list_empty(&extension_list)) {
		printf("No extension registered - Please run \"extension scan\"\n");
		return CMD_RET_SUCCESS;
	}

	list_for_each_entry(extension, &extension_list, list) {
		printf("Extension %d: %s\n", i++, extension->name);
		printf("\tManufacturer: \t\t%s\n", extension->owner);
		printf("\tVersion: \t\t%s\n", extension->version);
		printf("\tDevicetree overlay: \t%s\n", extension->overlay);
		printf("\tOther information: \t%s\n", extension->other);
	}
	return CMD_RET_SUCCESS;
}

static int do_extension_scan(struct cmd_tbl *cmdtp, int flag,
			     int argc, char *const argv[])
{
	int extension_num;

	extension_num = extension_scan(true);
	if (extension_num < 0)
		return CMD_RET_FAILURE;

	return CMD_RET_SUCCESS;
}

static int do_extension_apply(struct cmd_tbl *cmdtp, int flag,
			      int argc, char *const argv[])
{
	struct extension *extension = NULL;
	int i = 0, extension_id, ret;
	struct list_head *entry;

	if (argc < 2)
		return CMD_RET_USAGE;

	if (strcmp(argv[1], "all") == 0) {
		ret = CMD_RET_FAILURE;
		list_for_each_entry(extension, &extension_list, list) {
			ret = extension_apply(extension);
			if (ret != CMD_RET_SUCCESS)
				break;
		}
	} else {
		extension_id = simple_strtol(argv[1], NULL, 10);
		list_for_each(entry, &extension_list) {
			if (i == extension_id) {
				extension = list_entry(entry, struct extension,  list);
				break;
			}
			i++;
		}

		if (!extension) {
			printf("Wrong extension number\n");
			return CMD_RET_FAILURE;
		}

		ret = extension_apply(extension);
	}

	return ret;
}

static struct cmd_tbl cmd_extension[] = {
	U_BOOT_CMD_MKENT(scan, 1, 1, do_extension_scan, "", ""),
	U_BOOT_CMD_MKENT(list, 1, 0, do_extension_list, "", ""),
	U_BOOT_CMD_MKENT(apply, 2, 0, do_extension_apply, "", ""),
};

static int do_extensionops(struct cmd_tbl *cmdtp, int flag, int argc,
			   char *const argv[])
{
	struct cmd_tbl *cp;

	/* Drop the extension command */
	argc--;
	argv++;

	cp = find_cmd_tbl(argv[0], cmd_extension, ARRAY_SIZE(cmd_extension));
	if (cp)
		return cp->cmd(cmdtp, flag, argc, argv);

	return CMD_RET_USAGE;
}

U_BOOT_CMD(extension, 3, 1, do_extensionops,
	"Extension board management sub system",
	"scan - scan plugged extension(s) board(s)\n"
	"extension list - lists available extension(s) board(s)\n"
	"extension apply <extension number|all> - applies DT overlays corresponding to extension boards\n"
);
