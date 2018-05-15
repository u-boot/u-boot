// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2018 Bootlin
 * Author: Miquel Raynal <miquel.raynal@bootlin.com>
 */

#include <common.h>
#include <dm.h>
#include <tpm-common.h>
#include <tpm-v2.h>
#include "tpm-utils.h"

u32 tpm2_startup(enum tpm2_startup_types mode)
{
	const u8 command_v2[12] = {
		tpm_u16(TPM2_ST_NO_SESSIONS),
		tpm_u32(12),
		tpm_u32(TPM2_CC_STARTUP),
		tpm_u16(mode),
	};
	int ret;

	/*
	 * Note TPM2_Startup command will return RC_SUCCESS the first time,
	 * but will return RC_INITIALIZE otherwise.
	 */
	ret = tpm_sendrecv_command(command_v2, NULL, NULL);
	if (ret && ret != TPM2_RC_INITIALIZE)
		return ret;

	return 0;
}
