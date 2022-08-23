// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2022, Linaro Limited
 */

#include <dm.h>
#include <efi.h>
#include <efi_loader.h>
#include <efi_variable.h>
#include <event.h>
#include <fwu.h>
#include <fwu_mdata.h>
#include <malloc.h>

#include <linux/errno.h>
#include <linux/types.h>

static u8 trial_state;
static u8 boottime_check;

#include <linux/errno.h>
#include <linux/types.h>
#include <u-boot/crc.h>

#define IMAGE_ACCEPT_SET	BIT(0)
#define IMAGE_ACCEPT_CLEAR	BIT(1)

static int trial_counter_update(u16 *trial_state_ctr)
{
	bool delete;
	u32 var_attr;
	efi_status_t status;
	efi_uintn_t var_size;

	delete = !trial_state_ctr ? true : false;
	var_size = !trial_state_ctr ? 0 : (efi_uintn_t)sizeof(*trial_state_ctr);
	var_attr = !trial_state_ctr ? 0 : EFI_VARIABLE_NON_VOLATILE |
		EFI_VARIABLE_BOOTSERVICE_ACCESS;
	status = efi_set_variable_int(u"TrialStateCtr",
				      &efi_global_variable_guid,
				      var_attr,
				      var_size, trial_state_ctr, false);

	if ((delete && (status != EFI_NOT_FOUND &&
			status != EFI_SUCCESS)) ||
	    (!delete && status != EFI_SUCCESS))
		return -1;

	return 0;
}

static int in_trial_state(struct fwu_mdata *mdata)
{
	u32 i, active_bank;
	struct fwu_image_entry *img_entry;
	struct fwu_image_bank_info *img_bank_info;

	active_bank = mdata->active_index;
	img_entry = &mdata->img_entry[0];
	for (i = 0; i < CONFIG_FWU_NUM_IMAGES_PER_BANK; i++) {
		img_bank_info = &img_entry[i].img_bank_info[active_bank];
		if (!img_bank_info->accepted) {
			return 1;
		}
	}

	return 0;
}

static int fwu_trial_state_check(struct udevice *dev)
{
	int ret;
	efi_status_t status;
	efi_uintn_t var_size;
	u16 trial_state_ctr;
	u32 var_attributes, active_idx;
	struct fwu_mdata mdata = { 0 };

	ret = fwu_get_mdata(dev, &mdata);
	if (ret)
		return ret;

	if ((trial_state = in_trial_state(&mdata))) {
		var_size = (efi_uintn_t)sizeof(trial_state_ctr);
		log_info("System booting in Trial State\n");
		var_attributes = EFI_VARIABLE_NON_VOLATILE |
			EFI_VARIABLE_BOOTSERVICE_ACCESS;
		status = efi_get_variable_int(u"TrialStateCtr",
					      &efi_global_variable_guid,
					      &var_attributes,
					      &var_size, &trial_state_ctr,
					      NULL);
		if (status != EFI_SUCCESS) {
			log_err("Unable to read TrialStateCtr variable\n");
			ret = -1;
			goto out;
		}

		++trial_state_ctr;
		if (trial_state_ctr > CONFIG_FWU_TRIAL_STATE_CNT) {
			log_info("Trial State count exceeded. Revert back to previous_active_index\n");
			active_idx = mdata.active_index;
			ret = fwu_revert_boot_index();
			if (ret) {
				log_err("Unable to revert active_index\n");
				goto out;
			}

			/* Delete the TrialStateCtr variable */
			ret = trial_counter_update(NULL);
			if (ret) {
				log_err("Unable to delete TrialStateCtr variable\n");
				goto out;
			}
		} else {
			ret = trial_counter_update(&trial_state_ctr);
			if (ret) {
				log_err("Unable to increment TrialStateCtr variable\n");
				goto out;
			}
		}
	} else {
		/* Delete the variable */
		ret = trial_counter_update(NULL);
		if (ret) {
			log_err("Unable to delete TrialStateCtr variable\n");
		}
	}

out:
	return ret;
}

static int fwu_get_dev(struct udevice **dev)

{
	int ret;

	ret = uclass_first_device(UCLASS_FWU_MDATA, dev);
	if (ret) {
		log_debug("Cannot find fwu device\n");
		return ret;
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
		log_debug("crc32 check failed for %s FWU metadata partition\n",
			  pri_part ? "primary" : "secondary");
		return -EINVAL;
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
	struct udevice *dev;
	struct fwu_mdata mdata = { 0 };

	ret = fwu_get_dev(&dev);
	if (ret)
		return ret;

	ret = fwu_get_mdata(dev, &mdata);
	if (ret < 0) {
		log_debug("Unable to get valid FWU metadata\n");
		goto out;
	}

	/*
	 * Found the FWU metadata partition, now read the active_index
	 * value
	 */
	*active_idx = mdata.active_index;
	if (*active_idx >= CONFIG_FWU_NUM_BANKS) {
		log_debug("Active index value read is incorrect\n");
		ret = -EINVAL;
	}

out:
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
int fwu_update_active_index(uint active_idx)
{
	int ret;
	struct udevice *dev;
	struct fwu_mdata mdata = { 0 };

	if (active_idx >= CONFIG_FWU_NUM_BANKS) {
		log_debug("Invalid active index value\n");
		return -EINVAL;
	}

	ret = fwu_get_dev(&dev);
	if (ret)
		return ret;

	ret = fwu_get_mdata(dev, &mdata);
	if (ret < 0) {
		log_debug("Unable to get valid FWU metadata\n");
		goto out;
	}

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
	if (ret < 0) {
		log_debug("Failed to update FWU metadata partitions\n");
		ret = -EIO;
	}

out:
	return ret;
}

/**
 * fwu_get_image_alt_num() - Get the dfu alt number to be used for capsule update
 * @image_type_id: pointer to the image GUID as passed in the capsule
 * @update_bank: Bank to which the update is to be made
 * @alt_num: The alt_num for the image
 *
 * Based on the GUID value passed in the capsule, along with the bank to which the
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
	struct udevice *dev;
	struct fwu_mdata mdata = { 0 };
	struct fwu_image_entry *img_entry;
	struct fwu_image_bank_info *img_bank_info;

	ret = fwu_get_dev(&dev);
	if (ret)
		return ret;

	ret = fwu_get_mdata(dev, &mdata);
	if (ret) {
		log_debug("Unable to get valid FWU metadata\n");
		goto out;
	}

	ret = -EINVAL;
	/*
	 * The FWU metadata has been read. Now get the image_uuid for the
	 * image with the update_bank.
	 */
	for (i = 0; i < CONFIG_FWU_NUM_IMAGES_PER_BANK; i++) {
		if (!guidcmp(image_type_id,
			     &mdata.img_entry[i].image_type_uuid)) {
			img_entry = &mdata.img_entry[i];
			img_bank_info = &img_entry->img_bank_info[update_bank];
			image_guid = &img_bank_info->image_uuid;
			ret = fwu_plat_get_alt_num(dev, image_guid, alt_num);
			if (ret) {
				log_debug("alt_num not found for partition with GUID %pUs\n",
					  image_guid);
			} else {
				log_debug("alt_num %d for partition %pUs\n",
					  *alt_num, image_guid);
			}

			goto out;
		}
	}

	log_debug("Partition with the image type %pUs not found\n",
		  image_type_id);

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

	ret = fwu_get_dev(&dev);
	if (ret)
		return ret;

	ret = fwu_get_mdata(dev, &mdata);
	if (ret < 0) {
		log_debug("Unable to get valid FWU metadata\n");
		goto out;
	}

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
	if (ret < 0) {
		log_debug("Failed to update FWU metadata partitions\n");
		ret = -EIO;
	}

out:
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

	ret = fwu_get_dev(&dev);
	if (ret)
		return ret;

	ret = fwu_get_mdata(dev, &mdata);
	if (ret < 0) {
		log_debug("Unable to get valid FWU metadata\n");
		goto out;
	}

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
__weak int fwu_plat_get_update_index(uint *update_idx)
{
	int ret;
	u32 active_idx;

	ret = fwu_get_active_index(&active_idx);
	if (ret < 0)
		return -1;

	*update_idx = (active_idx + 1) % CONFIG_FWU_NUM_BANKS;

	return ret;
}

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
u8 fwu_update_checks_pass(void)
{
	return !trial_state && boottime_check;
}

/**
 * fwu_trial_state_ctr_start() - Start the Trial State counter
 *
 * Start the counter to identify the platform booting in the
 * Trial State. The counter is implemented as an EFI variable.
 *
 * Return: 0 if OK, -ve on error
 *
 */
int fwu_trial_state_ctr_start(void)
{
	int ret;
	u16 trial_state_ctr;

	trial_state_ctr = 0;
	ret = trial_counter_update(&trial_state_ctr);
	if (ret)
		log_err("Unable to initialise TrialStateCtr\n");

	return ret;
}

static int fwu_boottime_checks(void *ctx, struct event *event)

{
	int ret;
	struct udevice *dev;
	u32 boot_idx, active_idx;

	/* Don't have boot time checks on sandbox */
	if (IS_ENABLED(CONFIG_SANDBOX)) {
		boottime_check = 1;
		return 0;
	}

	ret = fwu_get_dev(&dev);
	if (ret)
		return ret;

	ret = fwu_mdata_check(dev);
	if (ret) {
		return 0;
	}

	/*
	 * Get the Boot Index, i.e. the bank from
	 * which the platform has booted. This value
	 * gets passed from the ealier stage bootloader
	 * which booted u-boot, e.g. tf-a. If the
	 * boot index is not the same as the
	 * active_index read from the FWU metadata,
	 * update the active_index.
	 */
	fwu_plat_get_bootidx(&boot_idx);
	if (boot_idx >= CONFIG_FWU_NUM_BANKS) {
		log_err("Received incorrect value of boot_index\n");
		return 0;
	}

	ret = fwu_get_active_index(&active_idx);
	if (ret) {
		log_err("Unable to read active_index\n");
		return 0;
	}

	if (boot_idx != active_idx) {
		log_info("Boot idx %u is not matching active idx %u, changing active_idx\n",
			 boot_idx, active_idx);
		ret = fwu_update_active_index(boot_idx);
		if (!ret)
			boottime_check = 1;

		return 0;
	}

	if (efi_init_obj_list() != EFI_SUCCESS)
		return 0;

	ret = fwu_trial_state_check(dev);
	if (!ret)
		boottime_check = 1;

	return 0;
}
EVENT_SPY(EVT_MAIN_LOOP, fwu_boottime_checks);
