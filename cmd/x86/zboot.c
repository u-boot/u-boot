// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2011 The Chromium OS Authors.
 * (C) Copyright 2002
 * Daniel Engström, Omicron Ceti AB, <daniel@omicron.se>
 */

#define LOG_CATEGORY	LOGC_BOOT

#include <command.h>
#include <env.h>
#include <mapmem.h>
#include <vsprintf.h>
#include <asm/zimage.h>

static int do_zboot_start(struct cmd_tbl *cmdtp, int flag, int argc,
			  char *const argv[])
{
	ulong bzimage_addr = 0, bzimage_size, initrd_addr, initrd_size;
	const char *s, *cmdline;
	ulong base_addr;
	int i;

	log_debug("argc %d:", argc);
	for (i = 0; i < argc; i++)
		log_debug(" %s", argv[i]);
	log_debug("\n");

	/* argv[1] holds the address of the bzImage */
	s = cmd_arg1(argc, argv) ? : env_get("fileaddr");
	if (s)
		bzimage_addr = hextoul(s, NULL);
	bzimage_size = argc > 2 ? hextoul(argv[2], NULL) : 0;
	initrd_addr = argc > 3 ? hextoul(argv[3], NULL) : 0;
	initrd_size = argc > 4 ? hextoul(argv[4], NULL) : 0;
	base_addr = argc > 5 ? hextoul(argv[5], NULL) : 0;
	cmdline = argc > 6 ? env_get(argv[6]) : NULL;

	zboot_start(bzimage_addr, bzimage_size, initrd_addr, initrd_size,
		    base_addr, cmdline);

	return 0;
}

static int do_zboot_load(struct cmd_tbl *cmdtp, int flag, int argc,
			 char *const argv[])
{
	int ret;

	ret = zboot_load();
	if (ret)
		return ret;

	return 0;
}

static int do_zboot_setup(struct cmd_tbl *cmdtp, int flag, int argc,
			  char *const argv[])
{
	if (!state.base_ptr) {
		printf("base is not set: use 'zboot load' first\n");
		return CMD_RET_FAILURE;
	}
	if (zboot_setup()) {
		puts("Setting up boot parameters failed ...\n");
		return CMD_RET_FAILURE;
	}

	if (zboot_setup())
		return CMD_RET_FAILURE;

	return 0;
}

static int do_zboot_info(struct cmd_tbl *cmdtp, int flag, int argc,
			 char *const argv[])
{
	zboot_info();

	return 0;
}

static int do_zboot_go(struct cmd_tbl *cmdtp, int flag, int argc,
		       char *const argv[])
{
	int ret;

	ret = zboot_go();
	if (ret) {
		printf("Kernel returned! (err=%d)\n", ret);
		return CMD_RET_FAILURE;
	}

	return 0;
}

static int do_zboot_dump(struct cmd_tbl *cmdtp, int flag, int argc,
			 char *const argv[])
{
	struct boot_params *base_ptr = state.base_ptr;

	if (argc > 1)
		base_ptr = (void *)hextoul(argv[1], NULL);
	if (!base_ptr) {
		printf("No zboot setup_base\n");
		return CMD_RET_FAILURE;
	}
	zimage_dump(base_ptr, true);

	return 0;
}

/* Note: This defines the complete_zboot() function */
U_BOOT_SUBCMDS(zboot,
	U_BOOT_CMD_MKENT(start, 8, 1, do_zboot_start, "", ""),
	U_BOOT_CMD_MKENT(load, 1, 1, do_zboot_load, "", ""),
	U_BOOT_CMD_MKENT(setup, 1, 1, do_zboot_setup, "", ""),
	U_BOOT_CMD_MKENT(info, 1, 1, do_zboot_info, "", ""),
	U_BOOT_CMD_MKENT(go, 1, 1, do_zboot_go, "", ""),
	U_BOOT_CMD_MKENT(dump, 2, 1, do_zboot_dump, "", ""),
)

int do_zboot_states(struct cmd_tbl *cmdtp, int flag, int argc,
		    char *const argv[], int state_mask)
{
	int ret = 0;

	log_debug("state_mask %x\n", state_mask);
	if (state_mask & ZBOOT_STATE_START)
		ret = do_zboot_start(cmdtp, flag, argc, argv);
	if (!ret && (state_mask & ZBOOT_STATE_LOAD))
		ret = do_zboot_load(cmdtp, flag, argc, argv);
	if (!ret && (state_mask & ZBOOT_STATE_SETUP))
		ret = do_zboot_setup(cmdtp, flag, argc, argv);
	if (!ret && (state_mask & ZBOOT_STATE_INFO))
		ret = do_zboot_info(cmdtp, flag, argc, argv);
	if (!ret && (state_mask & ZBOOT_STATE_GO))
		ret = do_zboot_go(cmdtp, flag, argc, argv);
	if (ret)
		return ret;

	return 0;
}

int do_zboot_parent(struct cmd_tbl *cmdtp, int flag, int argc,
		    char *const argv[], int *repeatable)
{
	/* determine if we have a sub command */
	if (argc > 1) {
		char *endp;

		hextoul(argv[1], &endp);
		/*
		 * endp pointing to nul means that argv[1] was just a valid
		 * number, so pass it along to the normal processing
		 */
		if (*endp)
			return do_zboot(cmdtp, flag, argc, argv, repeatable);
	}

	do_zboot_states(cmdtp, flag, argc, argv, ZBOOT_STATE_START |
			ZBOOT_STATE_LOAD | ZBOOT_STATE_SETUP |
			ZBOOT_STATE_INFO | ZBOOT_STATE_GO);

	return CMD_RET_FAILURE;
}

U_BOOT_CMDREP_COMPLETE(
	zboot, 8, do_zboot_parent, "Boot bzImage",
	"[addr] [size] [initrd addr] [initrd size] [setup] [cmdline]\n"
	"      addr -        The optional starting address of the bzimage.\n"
	"                    If not set it defaults to the environment\n"
	"                    variable \"fileaddr\".\n"
	"      size -        The optional size of the bzimage. Defaults to\n"
	"                    zero.\n"
	"      initrd addr - The address of the initrd image to use, if any.\n"
	"      initrd size - The size of the initrd image to use, if any.\n"
	"      setup -       The address of the kernel setup region, if this\n"
	"                    is not at addr\n"
	"      cmdline -     Environment variable containing the kernel\n"
	"                    command line, to override U-Boot's normal\n"
	"                    cmdline generation\n"
	"\n"
	"Sub-commands to do part of the zboot sequence:\n"
	"\tstart [addr [arg ...]] - specify arguments\n"
	"\tload   - load OS image\n"
	"\tsetup  - set up table\n"
	"\tinfo   - show summary info\n"
	"\tgo     - start OS\n"
	"\tdump [addr]    - dump info (optional address of boot params)",
	complete_zboot
);
