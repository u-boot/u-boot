// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2022, Linaro Limited
 */


#include <dm.h>
#include <efi.h>
#include <efi_loader.h>
#include <efi_variable.h>
#include <fwu.h>
#include <fwu_mdata.h>
#include <malloc.h>

#include <linux/errno.h>
#include <linux/types.h>

static u8 trial_state;
static u8 boottime_check;

__weak int fwu_plat_get_update_index(u32 *update_idx)
{
	int ret;
	u32 active_idx;

	ret = fwu_get_active_index(&active_idx);

	if (ret < 0)
		return -1;

	*update_idx = active_idx ^= 0x1;

	return ret;
}

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

static int fwu_trial_state_check(void)
{
	int ret;
	efi_status_t status;
	efi_uintn_t var_size;
	u16 trial_state_ctr;
	u32 var_attributes, active_idx;
	struct fwu_mdata *mdata = NULL;

	ret = fwu_get_mdata(&mdata);
	if (ret)
		return ret;

	if ((trial_state = in_trial_state(mdata))) {
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
			active_idx = mdata->active_index;
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
	free(mdata);
	return ret;
}

u8 fwu_update_checks_pass(void)
{
	return !trial_state && boottime_check;
}

int fwu_boottime_checks(void)
{
	int ret;
	u32 boot_idx, active_idx;

	ret = fwu_mdata_check();
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

	ret = fwu_trial_state_check();
	if (!ret)
		boottime_check = 1;

	return 0;
}
