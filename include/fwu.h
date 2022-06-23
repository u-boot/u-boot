/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright (c) 2022, Linaro Limited
 */

#if !defined _FWU_H_
#define _FWU_H_

#include <blk.h>
#include <efi.h>

#include <linux/types.h>

struct fwu_mdata;
struct udevice;

struct fwu_mdata_gpt_blk_priv {
	struct udevice *blk_dev;
};

/**
 * @mdata_check: check the validity of the FWU metadata partitions
 * @get_mdata() - Get a FWU metadata copy
 * @update_mdata() - Update the FWU metadata copy
 */
struct fwu_mdata_ops {
	/**
	 * mdata_check() - Check if the FWU metadata is valid
	 * @dev:	FWU device
	 *
	 * Validate both copies of the FWU metadata. If one of the copies
	 * has gone bad, restore it from the other bad copy.
	 *
	 * Return: 0 if OK, -ve on error
	 */
	int (*mdata_check)(struct udevice *dev);

	/**
	 * get_mdata() - Get a FWU metadata copy
	 * @dev:	FWU device
	 * @mdata:	Pointer to FWU metadata
	 *
	 * Get a valid copy of the FWU metadata.
	 *
	 * Return: 0 if OK, -ve on error
	 */
	int (*get_mdata)(struct udevice *dev, struct fwu_mdata *mdata);

	/**
	 * fwu_update_mdata() - Update the FWU metadata
	 * @dev:	FWU device
	 * @mdata:	Copy of the FWU metadata
	 *
	 * Update the FWU metadata structure by writing to the
	 * FWU metadata partitions.
	 *
	 * Return: 0 if OK, -ve on error
	 */
	int (*update_mdata)(struct udevice *dev, struct fwu_mdata *mdata);
};

#define FWU_MDATA_VERSION	0x1

/*
* GUID value defined in the FWU specification for identification
* of the FWU metadata partition.
*/
#define FWU_MDATA_GUID \
	EFI_GUID(0x8a7a84a0, 0x8387, 0x40f6, 0xab, 0x41, \
		 0xa8, 0xb9, 0xa5, 0xa6, 0x0d, 0x23)

/**
 * fwu_get_mdata() - Get a FWU metadata copy
 * @dev: FWU metadata device
 * @mdata: Copy of the FWU metadata
 *
 * Get a valid copy of the FWU metadata.
 *
 * Note: This function is to be called first when modifying any fields
 * in the metadata. The sequence of calls to modify any field in the
 * metadata would  be 1) fwu_get_mdata 2) Modify metadata, followed by
 * 3) fwu_update_mdata
 *
 * Return: 0 if OK, -ve on error
 *
 */
int fwu_get_mdata(struct udevice *dev, struct fwu_mdata *mdata);

/**
 * fwu_update_mdata() - Update the FWU metadata
 * @dev: FWU metadata device
 * @mdata: Copy of the FWU metadata
 *
 * Update the FWU metadata structure by writing to the
 * FWU metadata partitions.
 *
 * Note: This function is not to be called directly to update the
 * metadata fields. The sequence of function calls should be
 * 1) fwu_get_mdata() 2) Modify the medata fields 3) fwu_update_mdata()
 *
 * Return: 0 if OK, -ve on error
 *
 */
int fwu_update_mdata(struct udevice *dev, struct fwu_mdata *mdata);

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
int fwu_get_active_index(u32 *active_idxp);

/**
 * fwu_update_active_index() - Update active_index from the FWU metadata
 * @active_idx: active_index value to be updated
 *
 * Update the active_index field in the FWU metadata
 *
 * Return: 0 if OK, -ve on error
 *
 */
int fwu_update_active_index(uint active_idx);

/**
 * fwu_get_image_index() - Get the Image Index to be used for capsule update
 * @image_type_id: pointer to the image GUID as passed in the capsule
 * @update_bank: Bank to which the update is to be made
 * @image_index: The Image Index for the image
 *
 * Based on the GUID value passed in the capsule, along with the bank to which the
 * image needs to be updated, get the Image Index value which will be used for the
 * capsule update.
 *
 * Currently, the capsule update driver uses the DFU framework for
 * the updates. This function gets the DFU alt number which is to
 * be used as the Image Index
 *
 * Return: 0 if OK, -ve on error
 *
 */
int fwu_get_image_index(efi_guid_t *image_type_id, u32 update_bank,
			u8 *image_index);

/**
 * fwu_mdata_check() - Check if the FWU metadata is valid
 * @dev: FWU metadata device
 *
 * Validate both copies of the FWU metadata. If one of the copies
 * has gone bad, restore it from the other bad copy.
 *
 * Return: 0 if OK, -ve on error
 *
 */
int fwu_mdata_check(struct udevice *dev);

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
 * fwu_verify_mdata() - Verify the FWU metadata
 * @mdata: FWU metadata structure
 * @pri_part: FWU metadata partition is primary or secondary
 *
 * Verify the FWU metadata by computing the CRC32 for the metadata
 * structure and comparing it against the CRC32 value stored as part
 * of the structure.
 *
 * Return: 0 if OK, -ve on error
 *
 */
int fwu_verify_mdata(struct fwu_mdata *mdata, bool pri_part);

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
 * Return: 1 if OK, 0 on error
 *
 */
u8 fwu_update_checks_pass(void);

#endif /* _FWU_H_ */
