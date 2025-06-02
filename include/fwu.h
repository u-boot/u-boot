/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright (c) 2022, Linaro Limited
 */

#if !defined _FWU_H_
#define _FWU_H_

#include <blk.h>
#include <efi.h>
#include <fwu_mdata.h>
#include <u-boot/uuid.h>

#include <linux/types.h>

struct fwu_mdata;
struct udevice;

struct fwu_mdata_gpt_blk_priv {
	struct udevice *blk_dev;
};

struct fwu_mtd_image_info {
	u32 start, size;
	int bank_num, image_num;
	char uuidbuf[UUID_STR_LEN + 1];
};

struct fwu_mdata_mtd_priv {
	struct mtd_info *mtd;
	char pri_label[50];
	char sec_label[50];
	u32 pri_offset;
	u32 sec_offset;
	struct fwu_mtd_image_info *fwu_mtd_images;
};

struct fwu_data {
	uint32_t crc32;
	uint32_t version;
	uint32_t active_index;
	uint32_t previous_active_index;
	uint32_t metadata_size;
	uint32_t boot_index;
	uint32_t num_banks;
	uint32_t num_images;
	uint8_t  bank_state[4];
	bool     trial_state;

	struct fwu_mdata *fwu_mdata;

	struct fwu_image_entry fwu_images[CONFIG_FWU_NUM_IMAGES_PER_BANK];
};

struct fwu_mdata_ops {
	/**
	 * read_mdata() - Populate the asked FWU metadata copy
	 * @dev: FWU metadata device
	 * @mdata: Output FWU mdata read
	 * @primary: If primary or secondary copy of metadata is to be read
	 * @size: Size in bytes of the metadata to be read
	 *
	 * Return: 0 if OK, -ve on error
	 */
	int (*read_mdata)(struct udevice *dev, struct fwu_mdata *mdata,
			  bool primary, uint32_t size);

	/**
	 * write_mdata() - Write the given FWU metadata copy
	 * @dev: FWU metadata device
	 * @mdata: Copy of the FWU metadata to write
	 * @primary: If primary or secondary copy of metadata is to be written
	 * @size: Size in bytes of the metadata to be written
	 *
	 * Return: 0 if OK, -ve on error
	 */
	int (*write_mdata)(struct udevice *dev, struct fwu_mdata *mdata,
			   bool primary, uint32_t size);
};

#define FWU_IMAGE_ACCEPTED	0x1

#define FWU_BANK_INVALID	(uint8_t)0xFF
#define FWU_BANK_VALID		(uint8_t)0xFE
#define FWU_BANK_ACCEPTED	(uint8_t)0xFC

enum {
	PRIMARY_PART = 1,
	SECONDARY_PART,
	BOTH_PARTS,
};

/*
* GUID value defined in the FWU specification for identification
* of the FWU metadata partition.
*/
#define FWU_MDATA_GUID \
	EFI_GUID(0x8a7a84a0, 0x8387, 0x40f6, 0xab, 0x41, \
		 0xa8, 0xb9, 0xa5, 0xa6, 0x0d, 0x23)

/*
* GUID value defined in the Dependable Boot specification for
* identification of the revert capsule, used for reverting
* any image in the updated bank.
*/
#define FWU_OS_REQUEST_FW_REVERT_GUID \
	EFI_GUID(0xacd58b4b, 0xc0e8, 0x475f, 0x99, 0xb5, \
		 0x6b, 0x3f, 0x7e, 0x07, 0xaa, 0xf0)

/*
* GUID value defined in the Dependable Boot specification for
* identification of the accept capsule, used for accepting
* an image in the updated bank.
*/
#define FWU_OS_REQUEST_FW_ACCEPT_GUID \
	EFI_GUID(0x0c996046, 0xbcc0, 0x4d04, 0x85, 0xec, \
		 0xe1, 0xfc, 0xed, 0xf1, 0xc6, 0xf8)

/**
 * fwu_read_mdata() - Wrapper around fwu_mdata_ops.read_mdata()
 */
int fwu_read_mdata(struct udevice *dev, struct fwu_mdata *mdata,
		   bool primary, uint32_t size);

/**
 * fwu_write_mdata() - Wrapper around fwu_mdata_ops.write_mdata()
 */
int fwu_write_mdata(struct udevice *dev, struct fwu_mdata *mdata,
		    bool primary, uint32_t size);

/**
 * fwu_get_mdata() - Read, verify and return the FWU metadata
 *
 * Read both the metadata copies from the storage media, verify their checksum,
 * and ascertain that both copies match. If one of the copies has gone bad,
 * restore it from the good copy.
 *
 * Return: 0 if OK, -ve on error
 */
int fwu_get_mdata(struct fwu_mdata *mdata);

/**
 * fwu_get_active_index() - Get active_index from the FWU metadata
 * @active_idxp: active_index value to be read
 *
 * Read the active_index field from the FWU metadata and place it in
 * the variable pointed to be the function argument.
 *
 * Return: 0 if OK, -ve on error
 *
 */
int fwu_get_active_index(uint *active_idxp);

/**
 * fwu_set_active_index() - Set active_index in the FWU metadata
 * @active_idx: active_index value to be set
 *
 * Update the active_index field in the FWU metadata
 *
 * Return: 0 if OK, -ve on error
 *
 */
int fwu_set_active_index(uint active_idx);

/**
 * fwu_get_dfu_alt_num() - Get the dfu_alt_num to be used for capsule update
 * @image_index:	The Image Index for the image
 * @alt_num:		pointer to store dfu_alt_num
 *
 * Currently, the capsule update driver uses the DFU framework for
 * the updates. This function gets the DFU alt number which is to
 * be used for capsule update.
 *
 * Return: 0 if OK, -ve on error
 *
 */
int fwu_get_dfu_alt_num(u8 image_index, u8 *alt_num);

/**
 * fwu_revert_boot_index() - Revert the active index in the FWU metadata
 *
 * Revert the active_index value in the FWU metadata, by swapping the values
 * of active_index and previous_active_index in both copies of the
 * FWU metadata.
 *
 * Return: 0 if OK, -ve on error
 *
 */
int fwu_revert_boot_index(void);

/**
 * fwu_accept_image() - Set the Acceptance bit for the image
 * @img_type_id: GUID of the image type for which the accepted bit is to be
 *               cleared
 * @bank: Bank of which the image's Accept bit is to be set
 *
 * Set the accepted bit for the image specified by the img_guid parameter. This
 * indicates acceptance of image for subsequent boots by some governing component
 * like OS(or firmware).
 *
 * Return: 0 if OK, -ve on error
 *
 */
int fwu_accept_image(efi_guid_t *img_type_id, u32 bank);

/**
 * fwu_clear_accept_image() - Clear the Acceptance bit for the image
 * @img_type_id: GUID of the image type for which the accepted bit is to be
 *               cleared
 * @bank: Bank of which the image's Accept bit is to be cleared
 *
 * Clear the accepted bit for the image type specified by the img_type_id parameter.
 * This function is called after the image has been updated. The accepted bit is
 * cleared to be set subsequently after passing the image acceptance criteria, by
 * either the OS(or firmware)
 *
 * Return: 0 if OK, -ve on error
 *
 */
int fwu_clear_accept_image(efi_guid_t *img_type_id, u32 bank);

/**
 * fwu_plat_get_alt_num() - Get the DFU Alt Num for the image from the platform
 * @dev: FWU device
 * @image_guid: Image GUID for which DFU alt number needs to be retrieved
 * @alt_num: Pointer to the alt_num
 *
 * Get the DFU alt number from the platform for the image specified by the
 * image GUID.
 *
 * Return: 0 if OK, -ve on error
 *
 */
int fwu_plat_get_alt_num(struct udevice *dev, efi_guid_t *image_guid,
			 u8 *alt_num);

/**
 * fwu_plat_get_update_index() - Get the value of the update bank
 * @update_idx: Bank number to which images are to be updated
 *
 * Get the value of the bank(partition) to which the update needs to be
 * made.
 *
 * Note: This is a weak function and platforms can override this with
 * their own implementation for selection of the update bank.
 *
 * Return: 0 if OK, -ve on error
 *
 */
int fwu_plat_get_update_index(uint *update_idx);

/**
 * fwu_plat_get_bootidx() - Get the value of the boot index
 * @boot_idx: Boot index value
 *
 * Get the value of the bank(partition) from which the platform
 * has booted. This value is passed to U-Boot from the earlier
 * stage bootloader which loads and boots all the relevant
 * firmware images
 *
 */
void fwu_plat_get_bootidx(uint *boot_idx);

/**
 * fwu_update_checks_pass() - Check if FWU update can be done
 *
 * Check if the FWU update can be executed. The updates are
 * allowed only when the platform is not in Trial State and
 * the boot time checks have passed
 *
 * Return: 1 if OK, 0 if checks do not pass
 *
 */
u8 fwu_update_checks_pass(void);

/**
 * fwu_empty_capsule_checks_pass() - Check if empty capsule can be processed
 *
 * Check if the empty capsule can be processed to either accept or revert
 * an earlier executed update. The empty capsules need to be processed
 * only when the platform is in Trial State and the boot time checks have
 * passed
 *
 * Return: 1 if OK, 0 if not to be allowed
 *
 */
u8 fwu_empty_capsule_checks_pass(void);

/**
 * fwu_trial_state_ctr_start() - Start the Trial State counter
 *
 * Start the counter to identify the platform booting in the
 * Trial State. The counter is implemented as an EFI variable.
 *
 * Return: 0 if OK, -ve on error
 *
 */
int fwu_trial_state_ctr_start(void);

/**
 * fwu_gen_alt_info_from_mtd() - Parse dfu_alt_info from metadata in mtd
 * @buf: Buffer into which the dfu_alt_info is filled
 * @len: Maximum characters that can be written in buf
 * @mtd: Pointer to underlying MTD device
 *
 * Parse dfu_alt_info from metadata in mtd. Used for setting the env.
 *
 * Return: 0 if OK, -ve on error
 */
int fwu_gen_alt_info_from_mtd(char *buf, size_t len, struct mtd_info *mtd);

/**
 * fwu_mtd_get_alt_num() - Mapping of fwu_plat_get_alt_num for MTD device
 * @image_guid: Image GUID for which DFU alt number needs to be retrieved
 * @alt_num: Pointer to the alt_num
 * @mtd_dev: Name of mtd device instance
 *
 * To map fwu_plat_get_alt_num onto mtd based metadata implementation.
 *
 * Return: 0 if OK, -ve on error
 */
int fwu_mtd_get_alt_num(efi_guid_t *image_guid, u8 *alt_num, const char *mtd_dev);

/**
 * fwu_mdata_copies_allocate() - Allocate memory for metadata
 * @mdata_size: Size of the metadata structure
 *
 * Allocate memory for storing both the copies of the FWU metadata. The
 * copies are then used as a cache for storing FWU metadata contents.
 *
 * Return: 0 if OK, -ve on error
 */
int fwu_mdata_copies_allocate(u32 mdata_size);

/**
 * fwu_get_dev() - Return the FWU metadata device
 *
 * Return the pointer to the FWU metadata device.
 *
 * Return: Pointer to the FWU metadata dev
 */
struct udevice *fwu_get_dev(void);

/**
 * fwu_get_data() - Return the version agnostic FWU structure
 *
 * Return the pointer to the version agnostic FWU structure.
 *
 * Return: Pointer to the FWU data structure
 */
struct fwu_data *fwu_get_data(void);

/**
 * fwu_sync_mdata() - Update given meta-data partition(s) with the copy provided
 * @data: FWU Data structure
 * @part: Bitmask of FWU metadata partitions to be written to
 *
 * Return: 0 if OK, -ve on error
 */
int fwu_sync_mdata(struct fwu_mdata *mdata, int part);

/**
 * fwu_populate_mdata_image_info() - Populate the image information
 * of the metadata
 * @data: Version agnostic FWU metadata information
 *
 * Populate the image information in the FWU metadata by copying it
 * from the version agnostic structure. This is done before the
 * metadata gets written to the storage media.
 *
 * Return: None
 */
void fwu_populate_mdata_image_info(struct fwu_data *data);

/**
 * fwu_get_mdata_size() - Get the FWU metadata size
 * @mdata_size: Size of the metadata structure
 *
 * Get the size of the FWU metadata from the structure. This is later used
 * to allocate memory for the structure.
 *
 * Return: 0 if OK, -ve on error
 */
int fwu_get_mdata_size(uint32_t *mdata_size);

/**
 * fwu_state_machine_updates() - Update FWU state of the platform
 * @trial_state: Is platform transitioning into Trial State
 * @update_index: Bank number to which images have been updated
 *
 * On successful completion of updates, transition the platform to
 * either Trial State or Regular State.
 *
 * To transition the platform to Trial State, start the
 * TrialStateCtr counter, followed by setting the value of bank_state
 * field of the metadata to Valid state(applicable only in version 2
 * of metadata).
 *
 * In case, the platform is to transition directly to Regular State,
 * update the bank_state field of the metadata to Accepted
 * state(applicable only in version 2 of metadata).
 *
 * Return: 0 if OK, -ve on error
 */
int fwu_state_machine_updates(bool trial_state, uint32_t update_index);

/**
 * fwu_init() - FWU specific initialisations
 *
 * Carry out some FWU specific initialisations including allocation
 * of memory for the metadata copies, and reading the FWU metadata
 * copies into the allocated memory. The metadata fields are then
 * copied into a version agnostic structure.
 *
 * Return: 0 if OK, -ve on error
 */
int fwu_init(void);

/**
 * fwu_bank_accepted() - Has the bank been accepted
 * @data: Version agnostic FWU metadata information
 * @bank: Update bank to check
 *
 * Check in the given bank if all the images have been accepted.
 *
 * Return: true if all images accepted, false otherwise
 */
bool fwu_bank_accepted(struct fwu_data *data, uint32_t bank);

#endif /* _FWU_H_ */
