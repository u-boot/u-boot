// SPDX-License-Identifier: GPL-2.0+
/*
 * The 'kaslrseed' command takes bytes from the hardware random number
 * generator and uses them to set the kaslr-seed value in the chosen node.
 *
 * Copyright (c) 2021, Chris Morgan <macromorgan@hotmail.com>
 */

#include <command.h>
#include <dm.h>
#include <hexdump.h>
#include <malloc.h>
#include <rng.h>
#include <fdt_support.h>

static int do_kaslr_seed(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[])
{
	int err = CMD_RET_SUCCESS;

	printf("Notice: a /chosen/kaslr-seed is automatically added to the device-tree when booted via booti/bootm/bootz therefore using this command is likely no longer needed\n");

	if (!working_fdt) {
		printf("No FDT memory address configured. Please configure\n"
		       "the FDT address via \"fdt addr <address>\" command.\n"
		       "Aborting!\n");
		err = CMD_RET_FAILURE;
	} else {
		if (fdt_kaslrseed(working_fdt, true) < 0)
			err = CMD_RET_FAILURE;
	}

	return cmd_process_error(cmdtp, err);
}

U_BOOT_LONGHELP(kaslrseed,
	"\n"
	"  - append random bytes to chosen kaslr-seed node\n");

U_BOOT_CMD(
	kaslrseed, 1, 0, do_kaslr_seed,
	"feed bytes from the hardware random number generator to the kaslr-seed",
	kaslrseed_help_text
);
