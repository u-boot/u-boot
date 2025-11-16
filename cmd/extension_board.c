// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2021
 * Köry Maincent, Bootlin, <kory.maincent@bootlin.com>
 */

#include <alist.h>
#include <exports.h>
#include <command.h>
#include <env.h>
#include <extension_board.h>

static int
cmd_extension_load_overlay_from_env(const struct extension *extension,
				    ulong *filesize)
{
	ulong size, overlay_addr;
	char *overlay_cmd;
	int ret;

	overlay_cmd = env_get("extension_overlay_cmd");
	if (!overlay_cmd) {
		printf("Environment extension_overlay_cmd is missing\n");
		return -EINVAL;
	}

	overlay_addr = env_get_hex("extension_overlay_addr", 0);
	if (!overlay_addr) {
		printf("Environment extension_overlay_addr is missing\n");
		return -EINVAL;
	}

	env_set("extension_overlay_name", extension->overlay);
	ret = run_command(overlay_cmd, 0);
	if (ret)
		return ret;

	size = env_get_hex("filesize", 0);
	if (!size)
		return -EINVAL;

	*filesize = size;
	return 0;
}

static int cmd_extension_apply(int extension_num)
{
	struct alist *extension_list = extension_get_list();
	const struct extension *extension;
	ulong size;
	int ret;

	if (!extension_list)
		return -ENODEV;

	extension = alist_get(extension_list, extension_num,
			      struct extension);
	if (!extension) {
		printf("Wrong extension number\n");
		return -ENODEV;
	}

	ret = cmd_extension_load_overlay_from_env(extension, &size);
	if (ret)
		return ret;

	return extension_apply(working_fdt, size);
}

static int cmd_extension_apply_all(void)
{
	struct alist *extension_list = extension_get_list();
	const struct extension *extension;
	int ret;

	if (!extension_list)
		return -ENODEV;

	alist_for_each(extension, extension_list) {
		ulong size;

		ret = cmd_extension_load_overlay_from_env(extension, &size);
		if (ret)
			return ret;

		ret = extension_apply(working_fdt, size);
		if (ret)
			return ret;
	}

	return 0;
}

static int do_extension_list(struct cmd_tbl *cmdtp, int flag,
			     int argc, char *const argv[])
{
	struct alist *extension_list;
	struct extension *extension;
	int i = 0;

	extension_list = extension_get_list();
	if (!extension_list) {
		printf("No extension device\n");
		return CMD_RET_FAILURE;
	}
	if (!alist_get_ptr(extension_list, 0)) {
		printf("No extension registered - Please run \"extension scan\"\n");
		return CMD_RET_SUCCESS;
	}

	alist_for_each(extension, extension_list) {
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

	extension_num = extension_scan();
	if (extension_num == -ENODEV)
		extension_num = 0;
	else if (extension_num < 0)
		return CMD_RET_FAILURE;

	printf("Found %d extension board(s).\n", extension_num);
	return CMD_RET_SUCCESS;
}

static int do_extension_apply(struct cmd_tbl *cmdtp, int flag,
			      int argc, char *const argv[])
{
	int extension_id;

	if (argc < 2)
		return CMD_RET_USAGE;

	if (!working_fdt) {
		printf("No FDT memory address configured. Please configure\n"
		       "the FDT address via \"fdt addr <address>\" command.\n");
		return -EINVAL;
	}

	if (strcmp(argv[1], "all") == 0) {
		if (cmd_extension_apply_all())
			return CMD_RET_FAILURE;
	} else {
		extension_id = simple_strtol(argv[1], NULL, 10);
		if (cmd_extension_apply(extension_id))
			return CMD_RET_FAILURE;
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
