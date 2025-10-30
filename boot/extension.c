// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2025 Köry Maincent <kory.maincent@bootlin.com>
 */

#include <bootdev.h>
#include <command.h>
#include <env.h>
#include <extension_board.h>
#include <fdt_support.h>
#include <malloc.h>
#include <mapmem.h>

LIST_HEAD(extension_list);

int extension_apply(struct extension *extension)
{
	ulong extrasize, overlay_addr;
	struct fdt_header *blob;
	char *overlay_cmd;

	if (!working_fdt) {
		printf("No FDT memory address configured. Please configure\n"
		       "the FDT address via \"fdt addr <address>\" command.\n");
		return CMD_RET_FAILURE;
	}

	overlay_cmd = env_get("extension_overlay_cmd");
	if (!overlay_cmd) {
		printf("Environment extension_overlay_cmd is missing\n");
		return CMD_RET_FAILURE;
	}

	overlay_addr = env_get_hex("extension_overlay_addr", 0);
	if (!overlay_addr) {
		printf("Environment extension_overlay_addr is missing\n");
		return CMD_RET_FAILURE;
	}

	env_set("extension_overlay_name", extension->overlay);
	if (run_command(overlay_cmd, 0) != 0)
		return CMD_RET_FAILURE;

	extrasize = env_get_hex("filesize", 0);
	if (!extrasize)
		return CMD_RET_FAILURE;

	fdt_shrink_to_minimum(working_fdt, extrasize);

	blob = map_sysmem(overlay_addr, 0);
	if (!fdt_valid(&blob))
		return CMD_RET_FAILURE;

	/* apply method prints messages on error */
	if (fdt_overlay_apply_verbose(working_fdt, blob))
		return CMD_RET_FAILURE;

	return CMD_RET_SUCCESS;
}

int extension_scan(bool show)
{
	struct extension *extension, *next;
	int extension_num;

	list_for_each_entry_safe(extension, next, &extension_list, list) {
		list_del(&extension->list);
		free(extension);
	}
	extension_num = extension_board_scan(&extension_list);
	if (show && extension_num >= 0)
		printf("Found %d extension board(s).\n", extension_num);

	/* either the number of extensions, or -ve for error */
	return extension_num;
}

static int extension_bootdev_hunt(struct bootdev_hunter *info, bool show)
{
	int ret;

	ret = env_set_hex("extension_overlay_addr",
			  env_get_hex("fdtoverlay_addr_r", 0));
	if (ret)
		return log_msg_ret("env", ret);

	ret = extension_scan(show);
	if (ret < 0)
		return log_msg_ret("ext", ret);

	return 0;
}

/* extensions should have a uclass - for now we use UCLASS_SIMPLE_BUS uclass */
BOOTDEV_HUNTER(extension_bootdev_hunter) = {
	.prio		= BOOTDEVP_1_PRE_SCAN,
	.uclass		= UCLASS_SIMPLE_BUS,
	.hunt		= extension_bootdev_hunt,
};
