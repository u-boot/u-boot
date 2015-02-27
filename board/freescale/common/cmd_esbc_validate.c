/*
 * Copyright 2015 Freescale Semiconductor, Inc.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <command.h>
#include <fsl_validate.h>

static int do_esbc_validate(cmd_tbl_t *cmdtp, int flag, int argc,
				char * const argv[])
{
	if (argc < 2)
		return cmd_usage(cmdtp);

	return fsl_secboot_validate(cmdtp, flag, argc, argv);
}

/***************************************************/
static char esbc_validate_help_text[] =
	"esbc_validate hdr_addr <hash_val> - Validates signature using\n"
	"                          RSA verification\n"
	"                          $hdr_addr Address of header of the image\n"
	"                          to be validated.\n"
	"                          $hash_val -Optional\n"
	"                          It provides Hash of public/srk key to be\n"
	"                          used to verify signature.\n";

U_BOOT_CMD(
	esbc_validate,	3,	0,	do_esbc_validate,
	"Validates signature on a given image using RSA verification",
	esbc_validate_help_text
);
