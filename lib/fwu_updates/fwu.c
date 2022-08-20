// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2022, Linaro Limited
 */

#include <dm.h>
#include <efi_loader.h>
#include <fwu.h>
#include <fwu_mdata.h>
#include <log.h>

#include <linux/errno.h>
#include <linux/types.h>
#include <u-boot/crc.h>

enum {
	IMAGE_ACCEPT_SET = 1,
	IMAGE_ACCEPT_CLEAR,
};

enum {
	PRIMARY_PART = 1,
	SECONDARY_PART,
	BOTH_PARTS,
};

static int fwu_get_dev_mdata(struct udevice **dev, struct fwu_mdata *mdata)
{
	int ret;

	ret = uclass_first_device_err(UCLASS_FWU_MDATA, dev);
	if (ret) {
		log_debug("Cannot find fwu device\n");
		return ret;
	}

	if (!mdata)
		return 0;

	ret = fwu_get_mdata(*dev, mdata);
	if (ret < 0)
		log_debug("Unable to get valid FWU metadata\n");

	return ret;
}

static int fwu_get_image_type_id(u8 *image_index, efi_guid_t *image_type_id)
{
	u8 index;
	int i;
	struct efi_fw_image *image;

	index = *image_index;
	image = update_info.images;
	for (i = 0; i < num_image_type_guids; i++) {
		if (index == image[i].image_index) {
			guidcpy(image_type_id, &image[i].image_type_id);
			return 0;
		}
	}

	return -ENOENT;
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
		log_debug("crc32 check failed for %s FWU metadata partition\n",
			  pri_part ? "primary" : "secondary");
		return -EINVAL;
	}

	return 0;
}

/**
 * fwu_check_mdata_validity() - Check for validity of the FWU metadata copies
 *
 * Read both the metadata copies from the storage media, verify their checksum,
 * and ascertain that both copies match. If one of the copies has gone bad,
 * restore it from the good copy.
 *
 * Return: 0 if OK, -ve on error
 *
 */
int fwu_check_mdata_validity(void)
{
	int ret;
	struct udevice *dev;
	struct fwu_mdata pri_mdata;
	struct fwu_mdata secondary_mdata;
	uint mdata_parts[2];
	uint valid_partitions, invalid_partitions;

	ret = fwu_get_dev_mdata(&dev, NULL);
	if (ret)
		return ret;

	/*
	 * Check if the platform has defined its own
	 * function to check the metadata partitions'
	 * validity. If so, that takes precedence.
	 */
	ret = fwu_mdata_check(dev);
	if (ret && ret != -ENOSYS)
		return ret;

	/*
	 * Two FWU metadata partitions are expected.
	 * If we don't have two, user needs to create
	 * them first
	 */
	valid_partitions = 0;
	ret = fwu_get_mdata_part_num(dev, mdata_parts);
	if (ret < 0) {
		log_debug("Error getting the FWU metadata partitions\n");
		return -ENOENT;
	}

	ret = fwu_read_mdata_partition(dev, &pri_mdata, mdata_parts[0]);
	if (!ret) {
		ret = fwu_verify_mdata(&pri_mdata, 1);
		if (!ret)
			valid_partitions |= PRIMARY_PART;
	}

	ret = fwu_read_mdata_partition(dev, &secondary_mdata, mdata_parts[1]);
	if (!ret) {
		ret = fwu_verify_mdata(&secondary_mdata, 0);
		if (!ret)
			valid_partitions |= SECONDARY_PART;
	}

	if (valid_partitions == (PRIMARY_PART | SECONDARY_PART)) {
		/*
		 * Before returning, check that both the
		 * FWU metadata copies are the same. If not,
		 * the FWU metadata copies need to be
		 * re-populated.
		 */
		if (!memcmp(&pri_mdata, &secondary_mdata,
			    sizeof(struct fwu_mdata))) {
			ret = 0;
		} else {
			log_debug("Both FWU metadata copies are valid but do not match. Please check!\n");
			ret = -1;
		}
		goto out;
	}

	if (!(valid_partitions & BOTH_PARTS)) {
		ret = -1;
		goto out;
	}

	invalid_partitions = valid_partitions ^ BOTH_PARTS;
	ret = fwu_write_mdata_partition(dev,
					(invalid_partitions == PRIMARY_PART) ?
					&secondary_mdata : &pri_mdata,
					(invalid_partitions == PRIMARY_PART) ?
					mdata_parts[0] : mdata_parts[1]);

	if (ret < 0)
		log_debug("Restoring %s FWU metadata partition failed\n",
			  (invalid_partitions == PRIMARY_PART) ?
			  "primary" : "secondary");

out:
	return ret;
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
int fwu_get_active_index(uint *active_idx)
{
	int ret;
	struct udevice *dev;
	struct fwu_mdata mdata = { 0 };

	ret = fwu_get_dev_mdata(&dev, &mdata);
	if (ret)
		return ret;

	/*
	 * Found the FWU metadata partition, now read the active_index
	 * value
	 */
	*active_idx = mdata.active_index;
	if (*active_idx >= CONFIG_FWU_NUM_BANKS) {
		log_debug("Active index value read is incorrect\n");
		ret = -EINVAL;
	}

	return ret;
}

/**
 * fwu_set_active_index() - Set active_index in the FWU metadata
 * @active_idx: active_index value to be set
 *
 * Update the active_index field in the FWU metadata
 *
 * Return: 0 if OK, -ve on error
 *
 */
int fwu_set_active_index(uint active_idx)
{
	int ret;
	struct udevice *dev;
	struct fwu_mdata mdata = { 0 };

	if (active_idx >= CONFIG_FWU_NUM_BANKS) {
		log_debug("Invalid active index value\n");
		return -EINVAL;
	}

	ret = fwu_get_dev_mdata(&dev, &mdata);
	if (ret)
		return ret;

	/*
	 * Update the active index and previous_active_index fields
	 * in the FWU metadata
	 */
	mdata.previous_active_index = mdata.active_index;
	mdata.active_index = active_idx;

	/*
	 * Now write this updated FWU metadata to both the
	 * FWU metadata partitions
	 */
	ret = fwu_update_mdata(dev, &mdata);
	if (ret) {
		log_debug("Failed to update FWU metadata partitions\n");
		ret = -EIO;
	}

	return ret;
}

/**
 * fwu_get_image_index() - Get the Image Index to be used for capsule update
 * @image_index: The Image Index for the image
 *
 * The FWU multi bank update feature computes the value of image_index at
 * runtime, based on the bank to which the image needs to be written to.
 * Derive the image_index value for the image.
 *
 * Currently, the capsule update driver uses the DFU framework for
 * the updates. This function gets the DFU alt number which is to
 * be used as the Image Index
 *
 * Return: 0 if OK, -ve on error
 *
 */
int fwu_get_image_index(u8 *image_index)
{
	int ret, i;
	u8 alt_num;
	uint update_bank;
	efi_guid_t *image_guid, image_type_id;
	struct udevice *dev;
	struct fwu_mdata mdata = { 0 };
	struct fwu_image_entry *img_entry;
	struct fwu_image_bank_info *img_bank_info;

	ret = fwu_get_dev_mdata(&dev, &mdata);
	if (ret)
		return ret;

	ret = fwu_plat_get_update_index(&update_bank);
	if (ret) {
		log_debug("Failed to get the FWU update bank\n");
		goto out;
	}

	ret = fwu_get_image_type_id(image_index, &image_type_id);
	if (ret) {
		log_debug("Unable to get image_type_id for image_index %u\n",
			  *image_index);
		goto out;
	}

	ret = -EINVAL;
	/*
	 * The FWU metadata has been read. Now get the image_uuid for the
	 * image with the update_bank.
	 */
	for (i = 0; i < CONFIG_FWU_NUM_IMAGES_PER_BANK; i++) {
		if (!guidcmp(&image_type_id,
			     &mdata.img_entry[i].image_type_uuid)) {
			img_entry = &mdata.img_entry[i];
			img_bank_info = &img_entry->img_bank_info[update_bank];
			image_guid = &img_bank_info->image_uuid;
			ret = fwu_plat_get_alt_num(dev, image_guid, &alt_num);
			if (ret) {
				log_debug("alt_num not found for partition with GUID %pUs\n",
					  image_guid);
			} else {
				log_debug("alt_num %d for partition %pUs\n",
					  alt_num, image_guid);
				*image_index = alt_num + 1;
			}

			goto out;
		}
	}

	log_debug("Partition with the image type %pUs not found\n",
		  &image_type_id);

out:
	return ret;
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
	struct udevice *dev;
	struct fwu_mdata mdata = { 0 };

	ret = fwu_get_dev_mdata(&dev, &mdata);
	if (ret)
		return ret;

	/*
	 * Swap the active index and previous_active_index fields
	 * in the FWU metadata
	 */
	cur_active_index = mdata.active_index;
	mdata.active_index = mdata.previous_active_index;
	mdata.previous_active_index = cur_active_index;

	/*
	 * Now write this updated FWU metadata to both the
	 * FWU metadata partitions
	 */
	ret = fwu_update_mdata(dev, &mdata);
	if (ret) {
		log_debug("Failed to update FWU metadata partitions\n");
		ret = -EIO;
	}

	return ret;
}

/**
 * fwu_clrset_image_accept() - Set or Clear the Acceptance bit for the image
 * @img_type_id: GUID of the image type for which the accepted bit is to be
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
static int fwu_clrset_image_accept(efi_guid_t *img_type_id, u32 bank, u8 action)
{
	int ret, i;
	struct udevice *dev;
	struct fwu_mdata mdata = { 0 };
	struct fwu_image_entry *img_entry;
	struct fwu_image_bank_info *img_bank_info;

	ret = fwu_get_dev_mdata(&dev, &mdata);
	if (ret)
		return ret;

	img_entry = &mdata.img_entry[0];
	for (i = 0; i < CONFIG_FWU_NUM_IMAGES_PER_BANK; i++) {
		if (!guidcmp(&img_entry[i].image_type_uuid, img_type_id)) {
			img_bank_info = &img_entry[i].img_bank_info[bank];
			if (action == IMAGE_ACCEPT_SET)
				img_bank_info->accepted |= FWU_IMAGE_ACCEPTED;
			else
				img_bank_info->accepted = 0;

			ret = fwu_update_mdata(dev, &mdata);
			goto out;
		}
	}

	/* Image not found */
	ret = -ENOENT;

out:
	return ret;
}

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
int fwu_accept_image(efi_guid_t *img_type_id, u32 bank)
{
	return fwu_clrset_image_accept(img_type_id, bank,
				       IMAGE_ACCEPT_SET);
}

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
int fwu_clear_accept_image(efi_guid_t *img_type_id, u32 bank)
{
	return fwu_clrset_image_accept(img_type_id, bank,
				       IMAGE_ACCEPT_CLEAR);
}
