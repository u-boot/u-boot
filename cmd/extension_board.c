// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2021
 * Köry Maincent, Bootlin, <kory.maincent@bootlin.com>
 */

#include <alist.h>
#include <exports.h>
#include <command.h>
#include <extension_board.h>

static int do_extension_list(struct cmd_tbl *cmdtp, int flag,
			     int argc, char *const argv[])
{
#if CONFIG_IS_ENABLED(SUPPORT_DM_EXTENSION_SCAN)
	struct alist *dm_extension_list;
#endif
	struct extension *extension;
	int i = 0;

#if CONFIG_IS_ENABLED(SUPPORT_DM_EXTENSION_SCAN)
	dm_extension_list = dm_extension_get_list();

	if (!alist_get_ptr(dm_extension_list, 0)) {
		printf("No extension registered - Please run \"extension scan\"\n");
		return CMD_RET_SUCCESS;
	}

	alist_for_each(extension, dm_extension_list) {
		printf("Extension %d: %s\n", i++, extension->name);
		printf("\tManufacturer: \t\t%s\n", extension->owner);
		printf("\tVersion: \t\t%s\n", extension->version);
		printf("\tDevicetree overlay: \t%s\n", extension->overlay);
		printf("\tOther information: \t%s\n", extension->other);
	}
#else
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
#endif
	return CMD_RET_SUCCESS;
}

static int do_extension_scan(struct cmd_tbl *cmdtp, int flag,
			     int argc, char *const argv[])
{
	int extension_num;

#if CONFIG_IS_ENABLED(SUPPORT_DM_EXTENSION_SCAN)
	extension_num = dm_extension_scan();
	if (extension_num == -ENODEV)
		extension_num = 0;
	else if (extension_num < 0)
		return CMD_RET_FAILURE;

	printf("Found %d extension board(s).\n", extension_num);
#else
	extension_num = extension_scan(true);
	if (extension_num < 0 && extension_num != -ENODEV)
		return CMD_RET_FAILURE;
#endif

	return CMD_RET_SUCCESS;
}

static int do_extension_apply(struct cmd_tbl *cmdtp, int flag,
			      int argc, char *const argv[])
{
#if !CONFIG_IS_ENABLED(SUPPORT_DM_EXTENSION_SCAN)
	struct extension *extension = NULL;
	struct list_head *entry;
	int i = 0;
#endif
	int extension_id, ret;

	if (argc < 2)
		return CMD_RET_USAGE;

	if (strcmp(argv[1], "all") == 0) {
		ret = CMD_RET_FAILURE;
#if CONFIG_IS_ENABLED(SUPPORT_DM_EXTENSION_SCAN)
		if (dm_extension_apply_all())
			return CMD_RET_FAILURE;
#else
		list_for_each_entry(extension, &extension_list, list) {
			ret = extension_apply(extension);
			if (ret != CMD_RET_SUCCESS)
				break;
		}
#endif
	} else {
		extension_id = simple_strtol(argv[1], NULL, 10);
#if CONFIG_IS_ENABLED(SUPPORT_DM_EXTENSION_SCAN)
		if (dm_extension_apply(extension_id))
			return CMD_RET_FAILURE;
#else
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
#endif
	}

	return CMD_RET_SUCCESS;
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
