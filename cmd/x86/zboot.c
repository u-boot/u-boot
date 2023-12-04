// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2011 The Chromium OS Authors.
 * (C) Copyright 2002
 * Daniel Engström, Omicron Ceti AB, <daniel@omicron.se>
 */

#include <command.h>
#include <mapmem.h>
#include <vsprintf.h>
#include <asm/zimage.h>

static void zboot_start(int argc, char *const argv[])
{
	const char *s;

	memset(&state, '\0', sizeof(state));
	if (argc >= 2) {
		/* argv[1] holds the address of the bzImage */
		s = argv[1];
	} else {
		s = env_get("fileaddr");
	}

	if (s)
		state.bzimage_addr = hextoul(s, NULL);

	if (argc >= 3) {
		/* argv[2] holds the size of the bzImage */
		state.bzimage_size = hextoul(argv[2], NULL);
	}

	if (argc >= 4)
		state.initrd_addr = hextoul(argv[3], NULL);
	if (argc >= 5)
		state.initrd_size = hextoul(argv[4], NULL);
	if (argc >= 6) {
		/*
		 * When the base_ptr is passed in, we assume that the image is
		 * already loaded at the address given by argv[1] and therefore
		 * the original bzImage is somewhere else, or not accessible.
		 * In any case, we don't need access to the bzImage since all
		 * the processing is assumed to be done.
		 *
		 * So set the base_ptr to the given address, use this arg as the
		 * load address and set bzimage_addr to 0 so we know that it
		 * cannot be proceesed (or processed again).
		 */
		state.base_ptr = (void *)hextoul(argv[5], NULL);
		state.load_address = state.bzimage_addr;
		state.bzimage_addr = 0;
	}
	if (argc >= 7)
		state.cmdline = env_get(argv[6]);
}

static int do_zboot_start(struct cmd_tbl *cmdtp, int flag, int argc,
			  char *const argv[])
{
	zboot_start(argc, argv);

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

static void zboot_info(void)
{
	printf("Kernel loaded at %08lx, setup_base=%p\n",
	       state.load_address, state.base_ptr);
}

static int do_zboot_info(struct cmd_tbl *cmdtp, int flag, int argc,
			 char *const argv[])
{
	zboot_info();

	return 0;
}

static int _zboot_go(void)
{
	int ret;

	ret = zboot_go();

	return ret;
}

static int do_zboot_go(struct cmd_tbl *cmdtp, int flag, int argc,
		       char *const argv[])
{
	int ret;

	ret = _zboot_go();
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
	int ret;

	if (flag & ZBOOT_STATE_START)
		ret = do_zboot_start(cmdtp, flag, argc, argv);
	if (!ret && (flag & ZBOOT_STATE_LOAD))
		ret = do_zboot_load(cmdtp, flag, argc, argv);
	if (!ret && (flag & ZBOOT_STATE_SETUP))
		ret = do_zboot_setup(cmdtp, flag, argc, argv);
	if (!ret && (flag & ZBOOT_STATE_INFO))
		ret = do_zboot_info(cmdtp, flag, argc, argv);
	if (!ret && (flag & ZBOOT_STATE_GO))
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
