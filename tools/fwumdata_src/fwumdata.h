/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright (c) 2025, Kory Maincent <kory.maincent@bootlin.com>
 */

#ifndef _FWUMDATA_H_
#define _FWUMDATA_H_

#include <linux/compiler_attributes.h>

/* Type definitions for U-Boot compatibility */
typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

/* FWU Constants */
#define FWU_IMAGE_ACCEPTED	0x1
#define FWU_BANK_INVALID	(uint8_t)0xFF
#define FWU_BANK_VALID		(uint8_t)0xFE
#define FWU_BANK_ACCEPTED	(uint8_t)0xFC
#define MAX_BANKS_V2		4

/* EFI GUID structure */
struct efi_guid {
	u32 time_high;
	u16 time_low;
	u16 reserved;
	u8  family;
	u8  node[7];
} __packed;

/* FWU Metadata structures */
struct fwu_image_bank_info {
	struct efi_guid  image_guid;
	u32 accepted;
	u32 reserved;
} __packed;

struct fwu_image_entry {
	struct efi_guid image_type_guid;
	struct efi_guid location_guid;
	struct fwu_image_bank_info img_bank_info[0]; /* Variable length */
} __packed;

struct fwu_fw_store_desc {
	u8  num_banks;
	u8  reserved;
	u16 num_images;
	u16 img_entry_size;
	u16 bank_info_entry_size;
	struct fwu_image_entry img_entry[0]; /* Variable length */
} __packed;

struct fwu_mdata {
	u32 crc32;
	u32 version;
	u32 active_index;
	u32 previous_active_index;
	/* Followed by image entries or fwu_mdata_ext */
} __packed;

struct fwu_mdata_ext { /* V2 only */
	u32 metadata_size;
	u16 desc_offset;
	u16 reserved1;
	u8  bank_state[4];
	u32 reserved2;
} __packed;

/* Metadata access helpers */
struct fwu_image_entry *fwu_get_image_entry(struct fwu_mdata *mdata,
					    int version, int num_banks,
					    int img_id)
{
	size_t offset;

	if (version == 1) {
		offset = sizeof(struct fwu_mdata) +
			(sizeof(struct fwu_image_entry) +
			 sizeof(struct fwu_image_bank_info) * num_banks) * img_id;
	} else {
		/* V2: skip fwu_fw_store_desc header */
		offset = sizeof(struct fwu_mdata) +
			 sizeof(struct fwu_mdata_ext) +
			 sizeof(struct fwu_fw_store_desc) +
			 (sizeof(struct fwu_image_entry) +
			  sizeof(struct fwu_image_bank_info) * num_banks) * img_id;
	}

	return (struct fwu_image_entry *)((char *)mdata + offset);
}

struct fwu_image_bank_info *fwu_get_bank_info(struct fwu_mdata *mdata,
					      int version, int num_banks,
					      int img_id, int bank_id)
{
	size_t offset;

	if (version == 1) {
		offset = sizeof(struct fwu_mdata) +
			 (sizeof(struct fwu_image_entry) +
			  sizeof(struct fwu_image_bank_info) * num_banks) * img_id +
			 sizeof(struct fwu_image_entry) +
			 sizeof(struct fwu_image_bank_info) * bank_id;
	} else {
		offset = sizeof(struct fwu_mdata) +
			 sizeof(struct fwu_mdata_ext) +
			 sizeof(struct fwu_fw_store_desc) +
			 (sizeof(struct fwu_image_entry) +
			  sizeof(struct fwu_image_bank_info) * num_banks) * img_id +
			 sizeof(struct fwu_image_entry) +
			 sizeof(struct fwu_image_bank_info) * bank_id;
	}

	return (struct fwu_image_bank_info *)((char *)mdata + offset);
}

struct fwu_fw_store_desc *fwu_get_fw_desc(struct fwu_mdata *mdata)
{
	size_t offset;

	offset = sizeof(struct fwu_mdata) +
		 sizeof(struct fwu_mdata_ext);

	return (struct fwu_fw_store_desc *)((char *)mdata + offset);
}

struct fwu_mdata_ext *fwu_get_fw_mdata_ext(struct fwu_mdata *mdata)
{
	size_t offset;

	offset = sizeof(struct fwu_mdata);

	return (struct fwu_mdata_ext *)((char *)mdata + offset);
}

#endif /* _FWUMDATA_H_ */
