// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2024, Linaro Limited
 */

#include <errno.h>
#include <fwu.h>
#include <fwu_mdata.h>
#include <log.h>

#include <linux/types.h>

#define FWU_MDATA_VERSION	0x2U
#define FWU_IMG_DESC_OFFSET	0x20U

static struct fwu_mdata g_mdata;

static inline struct fwu_fw_store_desc *fwu_get_fw_desc(struct fwu_mdata *mdata)
{
	return (struct fwu_fw_store_desc *)((u8 *)mdata + sizeof(*mdata));
}

static uint32_t fwu_check_trial_state(struct fwu_mdata *mdata, uint32_t bank)
{
	return mdata->bank_state[bank] == FWU_BANK_VALID ? 1 : 0;
}

static void fwu_data_init(void)
{
	int i;
	size_t image_info_size;
	void *dst_img_info, *src_img_info;
	struct fwu_data *data = fwu_get_data();
	struct fwu_mdata *mdata = data->fwu_mdata;

	data->crc32 = mdata->crc32;
	data->version = mdata->version;
	data->active_index = mdata->active_index;
	data->previous_active_index = mdata->previous_active_index;
	data->metadata_size = mdata->metadata_size;
	fwu_plat_get_bootidx(&data->boot_index);
	data->trial_state = fwu_check_trial_state(mdata, data->boot_index);

	data->num_banks = fwu_get_fw_desc(mdata)->num_banks;
	data->num_images = fwu_get_fw_desc(mdata)->num_images;

	for (i = 0; i < 4; i++) {
		data->bank_state[i] = mdata->bank_state[i];
	}

	image_info_size = sizeof(data->fwu_images);
	src_img_info = &fwu_get_fw_desc(mdata)->img_entry[0];
	dst_img_info = &data->fwu_images[0];

	memcpy(dst_img_info, src_img_info, image_info_size);
}

static int fwu_mdata_sanity_checks(void)
{
	uint8_t num_banks;
	uint16_t num_images;
	struct fwu_data *data = fwu_get_data();
	struct fwu_mdata *mdata = data->fwu_mdata;

	num_banks = fwu_get_fw_desc(mdata)->num_banks;
	num_images = fwu_get_fw_desc(mdata)->num_images;

	if (num_banks != CONFIG_FWU_NUM_BANKS) {
		log_err("Number of Banks(%u) in FWU Metadata different from the configured value(%d)",
			num_banks, CONFIG_FWU_NUM_BANKS);
		return -EINVAL;
	}

	if (num_images != CONFIG_FWU_NUM_IMAGES_PER_BANK) {
		log_err("Number of Images(%u) in FWU Metadata different from the configured value(%d)",
			num_images, CONFIG_FWU_NUM_IMAGES_PER_BANK);
		return -EINVAL;
	}

	return 0;
}

static int fwu_bank_state_update(bool trial_state, uint32_t bank)
{
	int ret;
	struct fwu_data *data = fwu_get_data();
	struct fwu_mdata *mdata = data->fwu_mdata;

	if (!trial_state && !fwu_bank_accepted(data, bank))
		return 0;

	mdata->bank_state[bank] = data->bank_state[bank] = trial_state ?
		FWU_BANK_VALID : FWU_BANK_ACCEPTED;

	ret = fwu_sync_mdata(mdata, BOTH_PARTS);
	if (ret)
		log_err("Unable to set bank_state for bank %u\n", bank);
	else
		data->trial_state = trial_state;

	return ret;
}

static int fwu_trial_state_start(uint update_index)
{
	int ret;

	ret = fwu_trial_state_ctr_start();
	if (ret)
		return ret;

	ret = fwu_bank_state_update(1, update_index);
	if (ret)
		return ret;

	return 0;
}

static bool fwu_get_mdata_mandatory(uint part)
{
	int ret = 0;
	struct udevice *fwu_dev = fwu_get_dev();

	memset(&g_mdata, 0, sizeof(struct fwu_mdata));

	ret = fwu_read_mdata(fwu_dev, &g_mdata,
			     part == PRIMARY_PART ? true : false,
			     sizeof(struct fwu_mdata));
	if (ret)
		return false;

	if (g_mdata.version != FWU_MDATA_VERSION) {
		log_err("FWU partition %u has metadata version %u. Expected value of %u\n",
			part, g_mdata.version, FWU_MDATA_VERSION);
		return false;
	}

	if (g_mdata.desc_offset != FWU_IMG_DESC_OFFSET) {
		log_err("Descriptor Offset(0x%x) in the FWU Metadata partition %u not equal to 0x20\n",
			g_mdata.desc_offset, part);
		log_err("Image information expected in the metadata\n");
		return false;
	}

	return true;
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
	struct fwu_mdata *mdata = data->fwu_mdata;
	void *dst_img_info, *src_img_info;

	image_info_size = sizeof(data->fwu_images);
	dst_img_info = &fwu_get_fw_desc(mdata)->img_entry[0];
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
int fwu_state_machine_updates(bool trial_state, uint32_t update_index)
{
	return trial_state ? fwu_trial_state_start(update_index) :
		fwu_bank_state_update(0, update_index);
}

/**
 * fwu_get_mdata_size() - Get the FWU metadata size
 * @mdata_size: Size of the metadata structure
 *
 * Get the size of the FWU metadata from the structure. This is later used
 * to allocate memory for the structure.
 *
 * Return: 0 if OK, -ve on error
 */
int fwu_get_mdata_size(uint32_t *mdata_size)
{
	struct fwu_data *data = fwu_get_data();

	if (data->metadata_size) {
		*mdata_size = data->metadata_size;
		return 0;
	}

	*mdata_size = g_mdata.metadata_size;
	if (!*mdata_size)
		return -EINVAL;

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

	/*
	 * First we read only the top level structure
	 * and get the size of the complete structure.
	 * Try reading the first partition first, if
	 * that does not work, try the secondary
	 * partition. The idea is, if one of the
	 * partitions is corrupted, it should be restored
	 * from the intact partition.
	 */
	if (!fwu_get_mdata_mandatory(PRIMARY_PART) &&
	    !fwu_get_mdata_mandatory(SECONDARY_PART)) {
		log_err("FWU metadata read failed\n");
		return -1;
	}

	ret = fwu_mdata_copies_allocate(g_mdata.metadata_size);
	if (ret)
		return ret;

	/*
	 * Now read the entire structure, both copies, and
	 * validate that the copies.
	 */
	ret = fwu_get_mdata(NULL);
	if (ret)
		return ret;

	ret = fwu_mdata_sanity_checks();
	if (ret)
		return ret;

	fwu_data_init();

	return 0;
}
