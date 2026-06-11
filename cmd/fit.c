// SPDX-License-Identifier: GPL-2.0+
/*
 * FIT image utility commands
 *
 * Copyright (C) 2026 Daniel Golle <daniel@makrotopia.org>
 */

#include <command.h>
#include <image.h>
#include <mapmem.h>
#include <linux/libfdt.h>
#include <u-boot/uuid.h>

static int do_fit_setuuid(struct cmd_tbl *cmdtp, int flag,
			  int argc, char *const argv[])
{
	unsigned char uuid_bin[FIT_INSTALL_UUID_LEN];
	char uuid_str[UUID_STR_LEN + 1];
	ulong addr;
	void *fit;
	int ret;

	if (argc != 2)
		return CMD_RET_USAGE;

	addr = hextoul(argv[1], NULL);
	fit = map_sysmem(addr, 0);

	if (fdt_check_header(fit)) {
		printf("Bad FIT header at %#lx\n", addr);
		ret = CMD_RET_FAILURE;
		goto out;
	}

	gen_rand_uuid(uuid_bin);

	if (fdt_setprop_inplace(fit, 0, FIT_INSTALL_UUID_PROP,
				uuid_bin, FIT_INSTALL_UUID_LEN)) {
		printf("Cannot set %s (missing or wrong-size property)\n",
		       FIT_INSTALL_UUID_PROP);
		ret = CMD_RET_FAILURE;
		goto out;
	}

	uuid_bin_to_str(uuid_bin, uuid_str, UUID_STR_FORMAT_STD);
	printf("Set %s = %s\n", FIT_INSTALL_UUID_PROP, uuid_str);
	ret = CMD_RET_SUCCESS;
out:
	unmap_sysmem(fit);
	return ret;
}

U_BOOT_LONGHELP(fit,
	"setuuid <addr> - stamp random install-uuid into FIT at <addr>\n"
);

U_BOOT_CMD_WITH_SUBCMDS(fit, "FIT image utilities", fit_help_text,
			U_BOOT_SUBCMD_MKENT(setuuid, 2, 0, do_fit_setuuid));
