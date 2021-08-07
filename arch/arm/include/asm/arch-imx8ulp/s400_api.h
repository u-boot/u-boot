/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2021 NXP
 */

#ifndef __S400_API_H__
#define __S400_API_H__

#define AHAB_VERSION    0x6
#define AHAB_CMD_TAG    0x17
#define AHAB_RESP_TAG   0xe1

#define AHAB_LOG_CID            0x21
#define AHAB_AUTH_OEM_CTNR_CID  0x87
#define AHAB_VERIFY_IMG_CID     0x88
#define AHAB_RELEASE_CTNR_CID   0x89
#define AHAB_RELEASE_RDC_REQ_CID   0xC4

#define S400_MAX_MSG          8U

struct imx8ulp_s400_msg {
	u8 version;
	u8 size;
	u8 command;
	u8 tag;
	u32 data[(S400_MAX_MSG - 1U)];
};

int ahab_release_rdc(u8 core_id);
#endif
