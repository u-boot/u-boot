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
#define AHAB_WRITE_SECURE_FUSE_REQ_CID	0x91
#define AHAB_FWD_LIFECYCLE_UP_REQ_CID   0x95
#define AHAB_READ_FUSE_REQ_CID	0x97
#define AHAB_GET_FW_VERSION_CID	0x9D
#define AHAB_RELEASE_RDC_REQ_CID   0xC4
#define AHAB_GET_FW_STATUS_CID   0xC5
#define AHAB_WRITE_FUSE_REQ_CID	0xD6
#define AHAB_CAAM_RELEASE_CID 0xD7
#define AHAB_GET_INFO_CID 0xDA

#define S400_MAX_MSG          255U

struct sentinel_msg {
	u8 version;
	u8 size;
	u8 command;
	u8 tag;
	u32 data[(S400_MAX_MSG - 1U)];
};

struct sentinel_get_info_data {
	u32 hdr;
	u32 soc;
	u32 lc;
	u32 uid[4];
	u32 sha256_rom_patch[8];
	u32 sha_fw[8];
};

int ahab_release_rdc(u8 core_id, u8 xrdc, u32 *response);
int ahab_auth_oem_ctnr(ulong ctnr_addr, u32 *response);
int ahab_release_container(u32 *response);
int ahab_verify_image(u32 img_id, u32 *response);
int ahab_forward_lifecycle(u16 life_cycle, u32 *response);
int ahab_write_fuse(u16 fuse_id, u32 fuse_val, bool lock, u32 *response);
int ahab_read_common_fuse(u16 fuse_id, u32 *fuse_words, u32 fuse_num, u32 *response);
int ahab_release_caam(u32 core_did, u32 *response);
int ahab_get_fw_version(u32 *fw_version, u32 *sha1, u32 *response);
int ahab_dump_buffer(u32 *buffer, u32 buffer_length);
int ahab_get_info(struct sentinel_get_info_data *info, u32 *response);
int ahab_get_fw_status(u32 *status, u32 *response);
int ahab_release_m33_trout(void);

#endif
