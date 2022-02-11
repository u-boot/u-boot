// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2022, Linaro Limited
 */

#define LOG_CATEGORY UCLASS_FWU_MDATA

#include <common.h>
#include <dm.h>
#include <efi_loader.h>
#include <fwu.h>
#include <fwu_mdata.h>
#include <log.h>
#include <malloc.h>

#include <linux/errno.h>
#include <linux/types.h>
#include <u-boot/crc.h>

#define IMAGE_ACCEPT_SET	BIT(0)
#define IMAGE_ACCEPT_CLEAR	BIT(1)

static int fwu_get_dev_ops(struct udevice **dev,
			   const struct fwu_mdata_ops **ops)
{
	int ret;

	ret = uclass_get_device(UCLASS_FWU_MDATA, 0, dev);
	if (ret) {
		log_debug("Cannot find fwu device\n");
		return ret;
	}

	if ((*ops = device_get_ops(*dev)) == NULL) {
		log_debug("Cannot get fwu device ops\n");
		return -ENOSYS;
	}

	return 0;
}

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
int fwu_verify_mdata(struct fwu_mdata *mdata, bool pri_part)
{
	u32 calc_crc32;
	void *buf;

	buf = &mdata->version;
	calc_crc32 = crc32(0, buf, sizeof(*mdata) - sizeof(u32));

	if (calc_crc32 != mdata->crc32) {
		log_err("crc32 check failed for %s FWU metadata partition\n",
			pri_part ? "primary" : "secondary");
		return -1;
	}

	return 0;
}

/**
 * fwu_get_active_index() - Get active_index from the FWU metadata
 * @active_idx: active_index value to be read
 *
 * Read the active_index field from the FWU metadata and place it in
 * the variable pointed to be the function argument.
 *
 * Return: 0 if OK, -ve on error
 *
 */
int fwu_get_active_index(u32 *active_idx)
{
	int ret;
	struct fwu_mdata *mdata = NULL;

	ret = fwu_get_mdata(&mdata);
	if (ret < 0) {
		log_err("Unable to get valid FWU metadata\n");
		goto out;
	}

	/*
	 * Found the FWU metadata partition, now read the active_index
	 * value
	 */
	*active_idx = mdata->active_index;
	if (*active_idx > CONFIG_FWU_NUM_BANKS - 1) {
		log_err("Active index value read is incorrect\n");
		ret = -EINVAL;
	}

out:
	free(mdata);

	return ret;
}

/**
 * fwu_update_active_index() - Update active_index from the FWU metadata
 * @active_idx: active_index value to be updated
 *
 * Update the active_index field in the FWU metadata
 *
 * Return: 0 if OK, -ve on error
 *
 */
int fwu_update_active_index(u32 active_idx)
{
	int ret;
	struct fwu_mdata *mdata = NULL;

	if (active_idx > CONFIG_FWU_NUM_BANKS - 1) {
		log_err("Active index value to be updated is incorrect\n");
		return -1;
	}

	ret = fwu_get_mdata(&mdata);
	if (ret < 0) {
		log_err("Unable to get valid FWU metadata\n");
		goto out;
	}

	/*
	 * Update the active index and previous_active_index fields
	 * in the FWU metadata
	 */
	mdata->previous_active_index = mdata->active_index;
	mdata->active_index = active_idx;

	/*
	 * Now write this updated FWU metadata to both the
	 * FWU metadata partitions
	 */
	ret = fwu_update_mdata(mdata);
	if (ret < 0) {
		log_err("Failed to update FWU metadata partitions\n");
		ret = -EIO;
	}

out:
	free(mdata);

	return ret;
}

/**
 * fwu_get_image_alt_num() - Get the dfu alt number to be used for capsule update
 * @image_type_id: pointer to the image guid as passed in the capsule
 * @update_bank: Bank to which the update is to be made
 * @alt_num: The alt_num for the image
 *
 * Based on the guid value passed in the capsule, along with the bank to which the
 * image needs to be updated, get the dfu alt number which will be used for the
 * capsule update
 *
 * Return: 0 if OK, -ve on error
 *
 */
int fwu_get_image_alt_num(efi_guid_t *image_type_id, u32 update_bank,
			  int *alt_num)
{
	int ret, i;
	efi_guid_t *image_guid;
	struct udevice *dev = NULL;
	struct fwu_mdata *mdata = NULL;
	struct fwu_image_entry *img_entry;
	const struct fwu_mdata_ops *ops = NULL;
	struct fwu_image_bank_info *img_bank_info;

	ret = fwu_get_dev_ops(&dev, &ops);
	if (ret)
		return ret;

	ret = fwu_get_mdata(&mdata);
	if (ret) {
		log_err("Unable to get valid FWU metadata\n");
		goto out;
	}

	/*
	 * The FWU metadata has been read. Now get the image_uuid for the
	 * image with the update_bank.
	 */
	for (i = 0; i < CONFIG_FWU_NUM_IMAGES_PER_BANK; i++) {
		if (!guidcmp(image_type_id,
			     &mdata->img_entry[i].image_type_uuid)) {
			img_entry = &mdata->img_entry[i];
			img_bank_info = &img_entry->img_bank_info[update_bank];
			image_guid = &img_bank_info->image_uuid;
			ret = fwu_plat_get_alt_num(dev, image_guid, alt_num);
			break;
		}
	}

	if (i == CONFIG_FWU_NUM_IMAGES_PER_BANK) {
		log_err("Partition with the image type %pUs not found\n",
			image_type_id);
		ret = -EINVAL;
		goto out;
	}

	if (!ret) {
		log_debug("alt_num %d for partition %pUs\n",
			  *alt_num, image_guid);
	} else {
		log_err("alt_num not found for partition with GUID %pUs\n",
			image_guid);
		ret = -EINVAL;
	}

out:
	free(mdata);

	return ret;
}

/**
 * fwu_mdata_check() - Check if the FWU metadata is valid
 *
 * Validate both copies of the FWU metadata. If one of the copies
 * has gone bad, restore it from the other bad copy.
 *
 * Return: 0 if OK, -ve on error
 *
 */
int fwu_mdata_check(void)
{
	int ret;
	struct udevice *dev = NULL;
	const struct fwu_mdata_ops *ops = NULL;

	ret = fwu_get_dev_ops(&dev, &ops);
	if (ret)
		return ret;

	if (!ops->mdata_check) {
		log_err("mdata_check() method not defined\n");
		return -ENOSYS;
	}

	return ops->mdata_check(dev);
}

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
int fwu_revert_boot_index(void)
{
	int ret;
	u32 cur_active_index;
	struct fwu_mdata *mdata = NULL;

	ret = fwu_get_mdata(&mdata);
	if (ret < 0) {
		log_err("Unable to get valid FWU metadata\n");
		goto out;
	}

	/*
	 * Swap the active index and previous_active_index fields
	 * in the FWU metadata
	 */
	cur_active_index = mdata->active_index;
	mdata->active_index = mdata->previous_active_index;
	mdata->previous_active_index = cur_active_index;

	/*
	 * Now write this updated FWU metadata to both the
	 * FWU metadata partitions
	 */
	ret = fwu_update_mdata(mdata);
	if (ret < 0) {
		log_err("Failed to update FWU metadata partitions\n");
		ret = -EIO;
	}

out:
	free(mdata);

	return ret;
}

/**
 * fwu_set_clear_image_accept() - Set or Clear the Acceptance bit for the image
 * @img_type_id: Guid of the image type for which the accepted bit is to be
 *               set or cleared
 * @bank: Bank of which the image's Accept bit is to be set or cleared
 * @action: Action which specifies whether image's Accept bit is to be set or
 *          cleared
 *
 * Set/Clear the accepted bit for the image specified by the img_guid parameter.
 * This indicates acceptance or rejection of image for subsequent boots by some
 * governing component like OS(or firmware).
 *
 * Return: 0 if OK, -ve on error
 *
 */
static int fwu_set_clear_image_accept(efi_guid_t *img_type_id,
				      u32 bank, u8 action)
{
	int ret, i;
	u32 nimages;
	struct fwu_mdata *mdata = NULL;
	struct fwu_image_entry *img_entry;
	struct fwu_image_bank_info *img_bank_info;

	ret = fwu_get_mdata(&mdata);
	if (ret < 0) {
		log_err("Unable to get valid FWU metadata\n");
		goto out;
	}

	nimages = CONFIG_FWU_NUM_IMAGES_PER_BANK;
	img_entry = &mdata->img_entry[0];
	for (i = 0; i < nimages; i++) {
		if (!guidcmp(&img_entry[i].image_type_uuid, img_type_id)) {
			img_bank_info = &img_entry[i].img_bank_info[bank];
			if (action == IMAGE_ACCEPT_SET)
				img_bank_info->accepted |= FWU_IMAGE_ACCEPTED;
			else
				img_bank_info->accepted = 0;

			ret = fwu_update_mdata(mdata);
			goto out;
		}
	}

	/* Image not found */
	ret = -EINVAL;

out:
	free(mdata);

	return ret;
}

/**
 * fwu_accept_image() - Set the Acceptance bit for the image
 * @img_type_id: Guid of the image type for which the accepted bit is to be
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
int fwu_accept_image(efi_guid_t *img_type_id, u32 bank)
{
	return fwu_set_clear_image_accept(img_type_id, bank,
					  IMAGE_ACCEPT_SET);
}

/**
 * fwu_clear_accept_image() - Clear the Acceptance bit for the image
 * @img_type_id: Guid of the image type for which the accepted bit is to be
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
int fwu_clear_accept_image(efi_guid_t *img_type_id, u32 bank)
{
	return fwu_set_clear_image_accept(img_type_id, bank,
					  IMAGE_ACCEPT_CLEAR);
}

/**
 * fwu_get_mdata() - Get a FWU metadata copy
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
int fwu_get_mdata(struct fwu_mdata **mdata)
{
	int ret;
	struct udevice *dev = NULL;
	const struct fwu_mdata_ops *ops = NULL;

	ret = fwu_get_dev_ops(&dev, &ops);
	if (ret)
		return ret;

	if (!ops->get_mdata) {
		log_err("get_mdata() method not defined\n");
		return -ENOSYS;
	}

	return ops->get_mdata(dev, mdata);
}

/**
 * fwu_update_mdata() - Update the FWU metadata
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
int fwu_update_mdata(struct fwu_mdata *mdata)
{
	int ret;
	void *buf;
	struct udevice *dev = NULL;
	const struct fwu_mdata_ops *ops = NULL;

	ret = fwu_get_dev_ops(&dev, &ops);
	if (ret)
		return ret;

	if (!ops->update_mdata) {
		log_err("get_mdata() method not defined\n");
		return -ENOSYS;
	}

	/*
	 * Calculate the crc32 for the updated FWU metadata
	 * and put the updated value in the FWU metadata crc32
	 * field
	 */
	buf = &mdata->version;
	mdata->crc32 = crc32(0, buf, sizeof(*mdata) - sizeof(u32));

	return ops->update_mdata(dev, mdata);
}

UCLASS_DRIVER(fwu_mdata) = {
	.id		= UCLASS_FWU_MDATA,
	.name		= "fwu-mdata",
};
