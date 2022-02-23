/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2018 Marek Behun <marek.behun@nic.cz>
 */

#ifndef _MVEBU_MBOX_H
#define _MVEBU_MBOX_H

enum mbox_cmd {
	MBOX_CMD_GET_RANDOM	= 1,
	MBOX_CMD_BOARD_INFO,
	MBOX_CMD_ECDSA_PUB_KEY,
	MBOX_CMD_HASH,
	MBOX_CMD_SIGN,
	MBOX_CMD_VERIFY,

	MBOX_CMD_OTP_READ,
	MBOX_CMD_OTP_WRITE,
};

int mbox_do_cmd(enum mbox_cmd cmd, u32 *in, int nout);

#endif
