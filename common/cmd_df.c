/*
 * Command for accessing DataFlash.
 *
 * Copyright (C) 2008 Atmel Corporation
 */
#include <common.h>
#include <df.h>

static int do_df(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
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
	return cmd_usage(cmdtp);
}

U_BOOT_CMD(
	sf,	2,	1,	do_serial_flash,
	"Serial flash sub-system",
	"probe [bus:]cs		- init flash device on given SPI bus and CS")
