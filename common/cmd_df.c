/*
 * Command for accessing DataFlash.
 *
 * Copyright (C) 2008 Atmel Corporation
 */
#include <common.h>
#include <df.h>

static int do_df(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	const char *cmd;

	/* need at least two arguments */
	if (argc < 2)
		goto usage;

	cmd = argv[1];

	if (strcmp(cmd, "init") == 0) {
		df_init(0, 0, 1000000);
		return 0;
	}

	if (strcmp(cmd, "info") == 0) {
		df_show_info();
		return 0;
	}

usage:
	printf("Usage:\n%s\n", cmdtp->usage);
	return 1;
}

U_BOOT_CMD(
	sf,	2,	1,	do_serial_flash,
	"sf	- Serial flash sub-system\n",
	"probe [bus:]cs		- init flash device on given SPI bus and CS\n")
