// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2024, Linaro Limited
 */

#include <errno.h>
#include <fwu.h>
#include <fwu_mdata.h>

#include <linux/types.h>

#define FWU_MDATA_VERSION	0x1U

static uint32_t fwu_check_trial_state(struct fwu_mdata *mdata, uint32_t bank)
{
	u32 i;
	struct fwu_image_entry *img_entry;
	struct fwu_image_bank_info *img_bank_info;

	img_entry = &mdata->img_entry[0];
	for (i = 0; i < CONFIG_FWU_NUM_IMAGES_PER_BANK; i++) {
		img_bank_info = &img_entry[i].img_bank_info[bank];
		if (!img_bank_info->accepted) {
			return 1;
		}
	}

	return 0;
}

static void fwu_data_init(void)
{
	size_t image_info_size;
	void *dst_img_info, *src_img_info;
	struct fwu_data *data = fwu_get_data();
	struct fwu_mdata *mdata = data->fwu_mdata;

	data->crc32 = mdata->crc32;
	data->version = mdata->version;
	data->active_index = mdata->active_index;
	data->previous_active_index = mdata->previous_active_index;

	data->metadata_size = sizeof(struct fwu_mdata);
	data->num_banks = CONFIG_FWU_NUM_BANKS;
	data->num_images = CONFIG_FWU_NUM_IMAGES_PER_BANK;
	fwu_plat_get_bootidx(&data->boot_index);
	data->trial_state = fwu_check_trial_state(mdata, data->boot_index);

	src_img_info = &mdata->img_entry[0];
	dst_img_info = &data->fwu_images[0];
	image_info_size = sizeof(data->fwu_images);

	memcpy(dst_img_info, src_img_info, image_info_size);
}

static int fwu_trial_state_update(bool trial_state, uint32_t bank)
{
	int ret;
	struct fwu_data *data = fwu_get_data();

	if (!trial_state && !fwu_bank_accepted(data, bank))
		return 0;

	if (trial_state) {
		ret = fwu_trial_state_ctr_start();
		if (ret)
			return ret;
	}

	data->trial_state = trial_state;

	return 0;
}

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
void fwu_populate_mdata_image_info(struct fwu_data *data)
{
	size_t image_info_size;
	void *dst_img_info, *src_img_info;
	struct fwu_mdata *mdata = data->fwu_mdata;

	image_info_size = sizeof(data->fwu_images);
	dst_img_info = &mdata->img_entry[0];
	src_img_info = &data->fwu_images[0];

	memcpy(dst_img_info, src_img_info, image_info_size);
}

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
int fwu_state_machine_updates(bool trial_state,
			      uint32_t update_index)
{
	return fwu_trial_state_update(trial_state, update_index);
}

/**
 * fwu_get_mdata_size() - Get the FWU metadata size
 * @mdata_size: Size of the metadata structure
 *
 * Get the size of the FWU metadata.
 *
 * Return: 0 if OK, -ve on error
 */
int fwu_get_mdata_size(uint32_t *mdata_size)
{
	*mdata_size = sizeof(struct fwu_mdata);

	return 0;
}

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
int fwu_init(void)
{
	int ret;
	uint32_t mdata_size;
	struct fwu_mdata mdata = {0};

	fwu_get_mdata_size(&mdata_size);

	ret = fwu_mdata_copies_allocate(mdata_size);
	if (ret)
		return ret;

	/*
	 * Now read the entire structure, both copies, and
	 * validate that the copies.
	 */
	ret = fwu_get_mdata(&mdata);
	if (ret)
		return ret;

	if (mdata.version != 0x1) {
		log_err("FWU metadata version %u. Expected value of %u\n",
			mdata.version, FWU_MDATA_VERSION);
		return -EINVAL;
	}

	fwu_data_init();

	return 0;
}
