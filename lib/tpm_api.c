// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2019 Google LLC
 */

#include <common.h>
#include <dm.h>
#include <log.h>
#include <tpm_api.h>
#include <tpm-v1.h>
#include <tpm-v2.h>
#include <tpm_api.h>

static bool is_tpm1(struct udevice *dev)
{
	return IS_ENABLED(CONFIG_TPM_V1) && tpm_get_version(dev) == TPM_V1;
}

static bool is_tpm2(struct udevice *dev)
{
	return IS_ENABLED(CONFIG_TPM_V2) && tpm_get_version(dev) == TPM_V2;
}

u32 tpm_startup(struct udevice *dev, enum tpm_startup_type mode)
{
	if (is_tpm1(dev)) {
		return tpm1_startup(dev, mode);
	} else if (is_tpm2(dev)) {
		enum tpm2_startup_types type;

		switch (mode) {
		case TPM_ST_CLEAR:
			type = TPM2_SU_CLEAR;
			break;
		case TPM_ST_STATE:
			type = TPM2_SU_STATE;
			break;
		default:
		case TPM_ST_DEACTIVATED:
			return -EINVAL;
		}
		return tpm2_startup(dev, type);
	} else {
		return -ENOSYS;
	}
}

u32 tpm_resume(struct udevice *dev)
{
	if (is_tpm1(dev))
		return tpm1_startup(dev, TPM_ST_STATE);
	else if (is_tpm2(dev))
		return tpm2_startup(dev, TPM2_SU_STATE);
	else
		return -ENOSYS;
}

u32 tpm_self_test_full(struct udevice *dev)
{
	if (is_tpm1(dev))
		return tpm1_self_test_full(dev);
	else if (is_tpm2(dev))
		return tpm2_self_test(dev, TPMI_YES);
	else
		return -ENOSYS;
}

u32 tpm_continue_self_test(struct udevice *dev)
{
	if (is_tpm1(dev))
		return tpm1_continue_self_test(dev);
	else if (is_tpm2(dev))
		return tpm2_self_test(dev, TPMI_NO);
	else
		return -ENOSYS;
}

u32 tpm_clear_and_reenable(struct udevice *dev)
{
	u32 ret;

	log_info("TPM: Clear and re-enable\n");
	ret = tpm_force_clear(dev);
	if (ret != TPM_SUCCESS) {
		log_err("Can't initiate a force clear\n");
		return ret;
	}

	if (is_tpm1(dev)) {
		ret = tpm1_physical_enable(dev);
		if (ret != TPM_SUCCESS) {
			log_err("TPM: Can't set enabled state\n");
			return ret;
		}

		ret = tpm1_physical_set_deactivated(dev, 0);
		if (ret != TPM_SUCCESS) {
			log_err("TPM: Can't set deactivated state\n");
			return ret;
		}
	}

	return TPM_SUCCESS;
}

u32 tpm_nv_enable_locking(struct udevice *dev)
{
	if (is_tpm1(dev))
		return tpm1_nv_define_space(dev, TPM_NV_INDEX_LOCK, 0, 0);
	else if (is_tpm2(dev))
		return -ENOSYS;
	else
		return -ENOSYS;
}

u32 tpm_nv_read_value(struct udevice *dev, u32 index, void *data, u32 count)
{
	if (is_tpm1(dev))
		return tpm1_nv_read_value(dev, index, data, count);
	else if (is_tpm2(dev))
		return tpm2_nv_read_value(dev, index, data, count);
	else
		return -ENOSYS;
}

u32 tpm_nv_write_value(struct udevice *dev, u32 index, const void *data,
		       u32 count)
{
	if (is_tpm1(dev))
		return tpm1_nv_write_value(dev, index, data, count);
	else if (is_tpm2(dev))
		return tpm2_nv_write_value(dev, index, data, count);
	else
		return -ENOSYS;
}

u32 tpm_set_global_lock(struct udevice *dev)
{
	return tpm_nv_write_value(dev, TPM_NV_INDEX_0, NULL, 0);
}

u32 tpm_write_lock(struct udevice *dev, u32 index)
{
	if (is_tpm1(dev))
		return -ENOSYS;
	else if (is_tpm2(dev))
		return tpm2_write_lock(dev, index);
	else
		return -ENOSYS;
}

u32 tpm_pcr_extend(struct udevice *dev, u32 index, const void *in_digest,
		   void *out_digest)
{
	if (is_tpm1(dev))
		return tpm1_extend(dev, index, in_digest, out_digest);
	else if (is_tpm2(dev))
		return tpm2_pcr_extend(dev, index, TPM2_ALG_SHA256, in_digest,
				       TPM2_DIGEST_LEN);
	else
		return -ENOSYS;
}

u32 tpm_pcr_read(struct udevice *dev, u32 index, void *data, size_t count)
{
	if (is_tpm1(dev))
		return tpm1_pcr_read(dev, index, data, count);
	else if (is_tpm2(dev))
		return -ENOSYS;
	else
		return -ENOSYS;
}

u32 tpm_tsc_physical_presence(struct udevice *dev, u16 presence)
{
	if (is_tpm1(dev))
		return tpm1_tsc_physical_presence(dev, presence);

	/*
	 * Nothing to do on TPM2 for this; use platform hierarchy availability
	 * instead.
	 */
	else if (is_tpm2(dev))
		return 0;
	else
		return -ENOSYS;
}

u32 tpm_finalise_physical_presence(struct udevice *dev)
{
	if (is_tpm1(dev))
		return tpm1_finalise_physical_presence(dev);

	/* Nothing needs to be done with tpm2 */
	else if (is_tpm2(dev))
		return 0;
	else
		return -ENOSYS;
}

u32 tpm_read_pubek(struct udevice *dev, void *data, size_t count)
{
	if (is_tpm1(dev))
		return tpm1_read_pubek(dev, data, count);
	else if (is_tpm2(dev))
		return -ENOSYS; /* not implemented yet */
	else
		return -ENOSYS;
}

u32 tpm_force_clear(struct udevice *dev)
{
	if (is_tpm1(dev))
		return tpm1_force_clear(dev);
	else if (is_tpm2(dev))
		return tpm2_clear(dev, TPM2_RH_PLATFORM, NULL, 0);
	else
		return -ENOSYS;
}

u32 tpm_physical_enable(struct udevice *dev)
{
	if (is_tpm1(dev))
		return tpm1_physical_enable(dev);

	/* Nothing needs to be done with tpm2 */
	else if (is_tpm2(dev))
		return 0;
	else
		return -ENOSYS;
}

u32 tpm_physical_disable(struct udevice *dev)
{
	if (is_tpm1(dev))
		return tpm1_physical_disable(dev);

	/* Nothing needs to be done with tpm2 */
	else if (is_tpm2(dev))
		return 0;
	else
		return -ENOSYS;
}

u32 tpm_physical_set_deactivated(struct udevice *dev, u8 state)
{
	if (is_tpm1(dev))
		return tpm1_physical_set_deactivated(dev, state);
	/* Nothing needs to be done with tpm2 */
	else if (is_tpm2(dev))
		return 0;
	else
		return -ENOSYS;
}

u32 tpm_get_capability(struct udevice *dev, u32 cap_area, u32 sub_cap,
		       void *cap, size_t count)
{
	if (is_tpm1(dev))
		return tpm1_get_capability(dev, cap_area, sub_cap, cap, count);
	else if (is_tpm2(dev))
		return tpm2_get_capability(dev, cap_area, sub_cap, cap, count);
	else
		return -ENOSYS;
}

u32 tpm_get_permissions(struct udevice *dev, u32 index, u32 *perm)
{
	if (is_tpm1(dev))
		return tpm1_get_permissions(dev, index, perm);
	else if (is_tpm2(dev))
		return -ENOSYS; /* not implemented yet */
	else
		return -ENOSYS;
}

u32 tpm_get_random(struct udevice *dev, void *data, u32 count)
{
	if (is_tpm1(dev))
		return tpm1_get_random(dev, data, count);
	else if (is_tpm2(dev))
		return -ENOSYS; /* not implemented yet */
	else
		return -ENOSYS;
}
