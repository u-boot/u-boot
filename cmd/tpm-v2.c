// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2018 Bootlin
 * Author: Miquel Raynal <miquel.raynal@bootlin.com>
 */

#include <common.h>
#include <dm.h>
#include <log.h>
#include <tpm-common.h>
#include <tpm-v2.h>
#include "tpm-user-utils.h"

static cmd_tbl_t tpm2_commands[] = {
	U_BOOT_CMD_MKENT(info, 0, 1, do_tpm_info, "", ""),
	U_BOOT_CMD_MKENT(init, 0, 1, do_tpm_init, "", ""),
};

cmd_tbl_t *get_tpm_commands(unsigned int *size)
{
	*size = ARRAY_SIZE(tpm2_commands);

	return tpm2_commands;
}

U_BOOT_CMD(tpm, CONFIG_SYS_MAXARGS, 1, do_tpm, "Issue a TPMv2.x command",
"<command> [<arguments>]\n"
"\n"
"info\n"
"    Show information about the TPM.\n"
"init\n"
"    Initialize the software stack. Always the first command to issue.\n"
);
