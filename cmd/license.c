/*
 * (C) Copyright 2007 by OpenMoko, Inc.
 * Author: Harald Welte <laforge@openmoko.org>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>

/* COPYING is currently 15951 bytes in size */
#define LICENSE_MAX	20480

#include <command.h>
#include <malloc.h>
#include <license.h>

int do_license(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	char *tok, *dst = malloc(LICENSE_MAX);
	unsigned long len = LICENSE_MAX;

	if (!dst)
		return -1;

	if (gunzip(dst, LICENSE_MAX, license_gz, &len) != 0) {
		printf("Error uncompressing license text\n");
		free(dst);
		return -1;
	}
	puts(dst);
	free(dst);

	return 0;
}

U_BOOT_CMD(
	license, 1, 1, do_license,
	"print GPL license text",
	""
);
