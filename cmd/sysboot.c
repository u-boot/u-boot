// SPDX-License-Identifier: GPL-2.0+

#include <common.h>
#include <command.h>
#include <env.h>
#include <fs.h>
#include "pxe_utils.h"

static char *fs_argv[5];

static int do_get_ext2(cmd_tbl_t *cmdtp, const char *file_path, char *file_addr)
{
#ifdef CONFIG_CMD_EXT2
	fs_argv[0] = "ext2load";
	fs_argv[3] = file_addr;
	fs_argv[4] = (void *)file_path;

	if (!do_ext2load(cmdtp, 0, 5, fs_argv))
		return 1;
#endif
	return -ENOENT;
}

static int do_get_fat(cmd_tbl_t *cmdtp, const char *file_path, char *file_addr)
{
#ifdef CONFIG_CMD_FAT
	fs_argv[0] = "fatload";
	fs_argv[3] = file_addr;
	fs_argv[4] = (void *)file_path;

	if (!do_fat_fsload(cmdtp, 0, 5, fs_argv))
		return 1;
#endif
	return -ENOENT;
}

static int do_get_any(cmd_tbl_t *cmdtp, const char *file_path, char *file_addr)
{
#ifdef CONFIG_CMD_FS_GENERIC
	fs_argv[0] = "load";
	fs_argv[3] = file_addr;
	fs_argv[4] = (void *)file_path;

	if (!do_load(cmdtp, 0, 5, fs_argv, FS_TYPE_ANY))
		return 1;
#endif
	return -ENOENT;
}

/*
 * Boots a system using a local disk syslinux/extlinux file
 *
 * Returns 0 on success, 1 on error.
 */
static int do_sysboot(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	unsigned long pxefile_addr_r;
	struct pxe_menu *cfg;
	char *pxefile_addr_str;
	char *filename;
	int prompt = 0;

	is_pxe = false;

	if (argc > 1 && strstr(argv[1], "-p")) {
		prompt = 1;
		argc--;
		argv++;
	}

	if (argc < 4)
		return cmd_usage(cmdtp);

	if (argc < 5) {
		pxefile_addr_str = from_env("pxefile_addr_r");
		if (!pxefile_addr_str)
			return 1;
	} else {
		pxefile_addr_str = argv[4];
	}

	if (argc < 6) {
		filename = env_get("bootfile");
	} else {
		filename = argv[5];
		env_set("bootfile", filename);
	}

	if (strstr(argv[3], "ext2")) {
		do_getfile = do_get_ext2;
	} else if (strstr(argv[3], "fat")) {
		do_getfile = do_get_fat;
	} else if (strstr(argv[3], "any")) {
		do_getfile = do_get_any;
	} else {
		printf("Invalid filesystem: %s\n", argv[3]);
		return 1;
	}
	fs_argv[1] = argv[1];
	fs_argv[2] = argv[2];

	if (strict_strtoul(pxefile_addr_str, 16, &pxefile_addr_r) < 0) {
		printf("Invalid pxefile address: %s\n", pxefile_addr_str);
		return 1;
	}

	if (get_pxe_file(cmdtp, filename, pxefile_addr_r) < 0) {
		printf("Error reading config file\n");
		return 1;
	}

	cfg = parse_pxefile(cmdtp, pxefile_addr_r);

	if (!cfg) {
		printf("Error parsing config file\n");
		return 1;
	}

	if (prompt)
		cfg->prompt = 1;

	handle_pxe_menu(cmdtp, cfg);

	destroy_pxe_menu(cfg);

	return 0;
}

U_BOOT_CMD(sysboot, 7, 1, do_sysboot,
	   "command to get and boot from syslinux files",
	   "[-p] <interface> <dev[:part]> <ext2|fat|any> [addr] [filename]\n"
	   "    - load and parse syslinux menu file 'filename' from ext2, fat\n"
	   "      or any filesystem on 'dev' on 'interface' to address 'addr'"
);
