// SPDX-License-Identifier: GPL-2.0+
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

static int fwu_trial_state_check(void)
{
	int ret, i;
	efi_status_t status;
	efi_uintn_t var_size;
	u16 trial_state_ctr;
	u32 nimages, active_bank, var_attributes, active_idx;
	struct fwu_mdata *mdata = NULL;
	struct fwu_image_entry *img_entry;
	struct fwu_image_bank_info *img_bank_info;

	ret = fwu_get_mdata(&mdata);
	if (ret)
		return ret;

	ret = 0;
	nimages = CONFIG_FWU_NUM_IMAGES_PER_BANK;
	active_bank = mdata->active_index;
	img_entry = &mdata->img_entry[0];
	for (i = 0; i < nimages; i++) {
		img_bank_info = &img_entry[i].img_bank_info[active_bank];
		if (!img_bank_info->accepted) {
			trial_state = 1;
			break;
		}
	}

	if (trial_state) {
		var_size = (efi_uintn_t)sizeof(trial_state_ctr);
		log_info("System booting in Trial State\n");
		var_attributes = EFI_VARIABLE_NON_VOLATILE |
			EFI_VARIABLE_BOOTSERVICE_ACCESS;
		status = efi_get_variable_int(L"TrialStateCtr",
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

			trial_state_ctr = 0;
			status = efi_set_variable_int(L"TrialStateCtr",
						      &efi_global_variable_guid,
						      var_attributes,
						      0,
						      &trial_state_ctr, false);
			if (status != EFI_SUCCESS) {
				log_err("Unable to clear TrialStateCtr variable\n");
				ret = -1;
				goto out;
			}
		} else {
			status = efi_set_variable_int(L"TrialStateCtr",
						      &efi_global_variable_guid,
						      var_attributes,
						      var_size,
						      &trial_state_ctr, false);
			if (status != EFI_SUCCESS) {
				log_err("Unable to increment TrialStateCtr variable\n");
				ret = -1;
				goto out;
			}
		}
	} else {
		trial_state_ctr = 0;
		status = efi_set_variable_int(L"TrialStateCtr",
					      &efi_global_variable_guid,
					      0,
					      0, &trial_state_ctr,
					      NULL);
	}

out:
	free(mdata);
	return ret;
}

u8 fwu_update_checks_pass(void)
{
	return !trial_state && boottime_check;
}

int fwu_trial_state_ctr_start(void)
{
	int ret;
	u32 var_attributes;
	efi_status_t status;
	efi_uintn_t var_size;
	u16 trial_state_ctr;

	var_size = (efi_uintn_t)sizeof(trial_state_ctr);
	var_attributes = EFI_VARIABLE_NON_VOLATILE |
		EFI_VARIABLE_BOOTSERVICE_ACCESS;

	trial_state_ctr = ret = 0;
	status = efi_set_variable_int(L"TrialStateCtr",
				      &efi_global_variable_guid,
				      var_attributes,
				      var_size,
				      &trial_state_ctr, false);
	if (status != EFI_SUCCESS) {
		log_err("Unable to increment TrialStateCtr variable\n");
		ret = -1;
	}

	return ret;
}

int fwu_boottime_checks(void)
{
	int ret;
	struct udevice *dev;
	u32 boot_idx, active_idx;

	if (uclass_get_device(UCLASS_FWU_MDATA, 0, &dev) || !dev) {
		log_err("FWU Metadata device not found\n");
		return 0;
	}

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
