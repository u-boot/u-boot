/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright (c) 2022, Linaro Limited
 */

#if !defined _FWU_MDATA_H_
#define _FWU_MDATA_H_

#include <linux/compiler_attributes.h>
#include <efi.h>

/**
 * struct fwu_image_bank_info - firmware image information
 * @image_guid: Guid value of the image in this bank
 * @accepted: Acceptance status of the image
 * @reserved: Reserved
 *
 * The structure contains image specific fields which are
 * used to identify the image and to specify the image's
 * acceptance status
 */
struct fwu_image_bank_info {
	efi_guid_t  image_guid;
	uint32_t accepted;
	uint32_t reserved;
} __packed;

/**
 * struct fwu_image_entry - information for a particular type of image
 * @image_type_guid: Guid value for identifying the image type
 * @location_guid: Guid of the storage volume where the image is located
 * @img_bank_info: Array containing properties of images
 *
 * This structure contains information on various types of updatable
 * firmware images. Each image type then contains an array of image
 * information per bank.
 */
struct fwu_image_entry {
	efi_guid_t image_type_guid;
	efi_guid_t location_guid;
	struct fwu_image_bank_info img_bank_info[CONFIG_FWU_NUM_BANKS];
} __packed;

/**
 * struct fwu_fw_store_desc - FWU updatable image information
 * @num_banks: Number of firmware banks
 * @num_images: Number of images per bank
 * @img_entry_size: The size of the img_entry array
 * @bank_info_entry_size: The size of the img_bank_info array
 * @img_entry: Array of image entries each giving information on a image
 *
 * This image descriptor structure contains information on the number of
 * updatable banks and images per bank. It also gives the total sizes of
 * the fwu_image_entry and fwu_image_bank_info arrays. This structure is
 * only present in version 2 of the metadata structure.
 */
struct fwu_fw_store_desc {
	uint8_t  num_banks;
	uint8_t  reserved;
	uint16_t num_images;
	uint16_t img_entry_size;
	uint16_t bank_info_entry_size;

	struct fwu_image_entry img_entry[CONFIG_FWU_NUM_IMAGES_PER_BANK];
} __packed;

#if defined(CONFIG_FWU_MDATA_V1)
/**
 * struct fwu_mdata - FWU metadata structure for multi-bank updates
 * @crc32: crc32 value for the FWU metadata
 * @version: FWU metadata version
 * @active_index: Index of the bank currently used for booting images
 * @previous_active_inde: Index of the bank used before the current bank
 *                        being used for booting
 * @img_entry: Array of information on various firmware images that can
 *             be updated
 *
 * This structure is used to store all the needed information for performing
 * multi bank updates on the platform. This contains info on the bank being
 * used to boot along with the information needed for identification of
 * individual images
 */
struct fwu_mdata {
	uint32_t crc32;
	uint32_t version;
	uint32_t active_index;
	uint32_t previous_active_index;

	struct fwu_image_entry img_entry[CONFIG_FWU_NUM_IMAGES_PER_BANK];
} __packed;

#else /* CONFIG_FWU_MDATA_V1 */
/**
 * struct fwu_mdata - FWU metadata structure for multi-bank updates
 * @crc32: crc32 value for the FWU metadata
 * @version: FWU metadata version
 * @active_index: Index of the bank currently used for booting images
 * @previous_active_inde: Index of the bank used before the current bank
 *                        being used for booting
 * @metadata_size: Size of the entire metadata structure, including the
 *                 image descriptors
 * @desc_offset: The offset from the start of this structure where the
 *               image descriptor structure starts. 0 if absent
 * @bank_state: State of each bank, valid, invalid or accepted
 * @fw_desc: The structure describing the FWU updatable images
 *
 * This is the top level structure used to store all information for performing
 * multi bank updates on the platform. This contains info on the bank being
 * used to boot along with the information on state of individual banks.
 */
struct fwu_mdata {
	uint32_t crc32;
	uint32_t version;
	uint32_t active_index;
	uint32_t previous_active_index;
	uint32_t metadata_size;
	uint16_t desc_offset;
	uint16_t reserved1;
	uint8_t  bank_state[4];
	uint32_t reserved2;

	// struct fwu_fw_store_desc fw_desc;
} __packed;

#endif /* CONFIG_FWU_MDATA_V1 */

#endif /* _FWU_MDATA_H_ */
