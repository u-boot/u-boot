// SPDX-License-Identifier: GPL-2.0-only
/*
 *  EFI conformance profile table
 *
 *  Copyright (C) 2022 Arm Ltd.
 */

#define LOG_CATEGORY LOGC_EFI

#include <efi_loader.h>
#include <log.h>
#include <efi_api.h>
#include <malloc.h>

static const efi_guid_t efi_ecpt_guid = EFI_CONFORMANCE_PROFILES_TABLE_GUID;

/**
 * efi_ecpt_register() - Install the ECPT system table.
 *
 * Return: status code
 */
efi_status_t efi_ecpt_register(void)
{
	struct efi_conformance_profiles_table *ecpt;
	efi_status_t ret;
	size_t ecpt_size;

	static const efi_guid_t profiles[] = {
	#if CONFIG_IS_ENABLED(EFI_EBBR_2_1_CONFORMANCE)
		EFI_CONFORMANCE_PROFILE_EBBR_2_1_GUID,
	#endif
	};

	ecpt_size = sizeof(profiles)
		+ sizeof(struct efi_conformance_profiles_table);
	ret = efi_allocate_pool(EFI_BOOT_SERVICES_DATA, ecpt_size,
				(void **)&ecpt);

	if (ret != EFI_SUCCESS) {
		log_err("Out of memory\n");

		return ret;
	}

	memcpy(ecpt->conformance_profiles, profiles, sizeof(profiles));
	ecpt->version = EFI_CONFORMANCE_PROFILES_TABLE_VERSION;
	ecpt->number_of_profiles = ARRAY_SIZE(profiles);

	/* Install the ECPT in the system configuration table. */
	ret = efi_install_configuration_table(&efi_ecpt_guid, (void *)ecpt);
	if (ret != EFI_SUCCESS) {
		log_err("Failed to install ECPT\n");
		efi_free_pool(ecpt);

		return ret;
	}

	log_debug("ECPT created\n");

	return EFI_SUCCESS;
}
