// SPDX-License-Identifier: GPL-2.0+
/*
 *  EFI Capsule
 *
 *  Copyright (c) 2018 Linaro Limited
 *			Author: AKASHI Takahiro
 */

#include <common.h>
#include <efi_loader.h>
#include <efi_variable.h>
#include <fs.h>
#include <malloc.h>
#include <sort.h>

const efi_guid_t efi_guid_capsule_report = EFI_CAPSULE_REPORT_GUID;

/**
 * get_last_capsule - get the last capsule index
 *
 * Retrieve the index of the capsule invoked last time from "CapsuleLast"
 * variable.
 *
 * Return:
 * * > 0	- the last capsule index invoked
 * * 0xffff	- on error, or no capsule invoked yet
 */
static __maybe_unused unsigned int get_last_capsule(void)
{
	u16 value16[11]; /* "CapsuleXXXX": non-null-terminated */
	char value[11], *p;
	efi_uintn_t size;
	unsigned long index = 0xffff;
	efi_status_t ret;

	size = sizeof(value16);
	ret = efi_get_variable_int(L"CapsuleLast", &efi_guid_capsule_report,
				   NULL, &size, value16, NULL);
	if (ret != EFI_SUCCESS || u16_strncmp(value16, L"Capsule", 7))
		goto err;

	p = value;
	utf16_utf8_strcpy(&p, value16);
	strict_strtoul(&value[7], 16, &index);
err:
	return index;
}

/**
 * set_capsule_result - set a result variable
 * @capsule:		Capsule
 * @return_status:	Return status
 *
 * Create and set a result variable, "CapsuleXXXX", for the capsule,
 * @capsule.
 */
static __maybe_unused
void set_capsule_result(int index, struct efi_capsule_header *capsule,
			efi_status_t return_status)
{
	u16 variable_name16[12];
	struct efi_capsule_result_variable_header result;
	struct efi_time time;
	efi_status_t ret;

	efi_create_indexed_name(variable_name16, "Capsule", index);

	result.variable_total_size = sizeof(result);
	result.capsule_guid = capsule->capsule_guid;
	ret = EFI_CALL((*efi_runtime_services.get_time)(&time, NULL));
	if (ret == EFI_SUCCESS)
		memcpy(&result.capsule_processed, &time, sizeof(time));
	else
		memset(&result.capsule_processed, 0, sizeof(time));
	result.capsule_status = return_status;
	ret = efi_set_variable(variable_name16, &efi_guid_capsule_report,
			       EFI_VARIABLE_NON_VOLATILE |
			       EFI_VARIABLE_BOOTSERVICE_ACCESS |
			       EFI_VARIABLE_RUNTIME_ACCESS,
			       sizeof(result), &result);
	if (ret)
		printf("EFI: creating %ls failed\n", variable_name16);
}

/**
 * efi_update_capsule() - process information from operating system
 * @capsule_header_array:	Array of virtual address pointers
 * @capsule_count:		Number of pointers in capsule_header_array
 * @scatter_gather_list:	Array of physical address pointers
 *
 * This function implements the UpdateCapsule() runtime service.
 *
 * See the Unified Extensible Firmware Interface (UEFI) specification for
 * details.
 *
 * Return:			status code
 */
efi_status_t EFIAPI efi_update_capsule(
		struct efi_capsule_header **capsule_header_array,
		efi_uintn_t capsule_count,
		u64 scatter_gather_list)
{
	struct efi_capsule_header *capsule;
	unsigned int i;
	efi_status_t ret;

	EFI_ENTRY("%p, %lu, %llu\n", capsule_header_array, capsule_count,
		  scatter_gather_list);

	if (!capsule_count) {
		ret = EFI_INVALID_PARAMETER;
		goto out;
	}

	ret = EFI_UNSUPPORTED;
	for (i = 0, capsule = *capsule_header_array; i < capsule_count;
	     i++, capsule = *(++capsule_header_array)) {
	}
out:
	return EFI_EXIT(ret);
}

/**
 * efi_query_capsule_caps() - check if capsule is supported
 * @capsule_header_array:	Array of virtual pointers
 * @capsule_count:		Number of pointers in capsule_header_array
 * @maximum_capsule_size:	Maximum capsule size
 * @reset_type:			Type of reset needed for capsule update
 *
 * This function implements the QueryCapsuleCapabilities() runtime service.
 *
 * See the Unified Extensible Firmware Interface (UEFI) specification for
 * details.
 *
 * Return:			status code
 */
efi_status_t EFIAPI efi_query_capsule_caps(
		struct efi_capsule_header **capsule_header_array,
		efi_uintn_t capsule_count,
		u64 *maximum_capsule_size,
		u32 *reset_type)
{
	struct efi_capsule_header *capsule __attribute__((unused));
	unsigned int i;
	efi_status_t ret;

	EFI_ENTRY("%p, %lu, %p, %p\n", capsule_header_array, capsule_count,
		  maximum_capsule_size, reset_type);

	if (!maximum_capsule_size) {
		ret = EFI_INVALID_PARAMETER;
		goto out;
	}

	*maximum_capsule_size = U64_MAX;
	*reset_type = EFI_RESET_COLD;

	ret = EFI_SUCCESS;
	for (i = 0, capsule = *capsule_header_array; i < capsule_count;
	     i++, capsule = *(++capsule_header_array)) {
		/* TODO */
	}
out:
	return EFI_EXIT(ret);
}
