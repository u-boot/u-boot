// SPDX-License-Identifier: GPL-2.0+
/*
 * efi_selftest_variables_runtime
 *
 * Copyright (c) 2024 Ilias Apalodimas <ilias.apalodimas@linaro.org>
 *
 * This unit test checks common service across boottime/runtime
 */

#include <efi_selftest.h>

#define EFI_INVALID_ATTR BIT(30)

int efi_st_query_variable_common(struct efi_runtime_services *runtime,
				 u32 attributes)
{
	efi_status_t ret;
	u64 max_storage, rem_storage, max_size;

	ret = runtime->query_variable_info(attributes,
					   &max_storage, &rem_storage,
					   &max_size);
	if (ret != EFI_SUCCESS) {
		efi_st_error("QueryVariableInfo failed\n");
		return EFI_ST_FAILURE;
	} else if (!max_storage || !rem_storage || !max_size) {
		efi_st_error("QueryVariableInfo: wrong info\n");
		return EFI_ST_FAILURE;
	}

	ret = runtime->query_variable_info(EFI_VARIABLE_RUNTIME_ACCESS,
					   &max_storage, &rem_storage,
					   &max_size);
	if (ret != EFI_INVALID_PARAMETER) {
		efi_st_error("QueryVariableInfo failed\n");
		return EFI_ST_FAILURE;
	}

	ret = runtime->query_variable_info(attributes,
					   NULL, &rem_storage,
					   &max_size);
	if (ret != EFI_INVALID_PARAMETER) {
		efi_st_error("QueryVariableInfo failed\n");
		return EFI_ST_FAILURE;
	}

	ret = runtime->query_variable_info(attributes,
					   &max_storage, NULL,
					   &max_size);
	if (ret != EFI_INVALID_PARAMETER) {
		efi_st_error("QueryVariableInfo failed\n");
		return EFI_ST_FAILURE;
	}

	ret = runtime->query_variable_info(attributes,
					   &max_storage, &rem_storage,
					   NULL);
	if (ret != EFI_INVALID_PARAMETER) {
		efi_st_error("QueryVariableInfo failed\n");
		return EFI_ST_FAILURE;
	}

	ret = runtime->query_variable_info(0, &max_storage, &rem_storage,
					   &max_size);
	if (ret != EFI_INVALID_PARAMETER) {
		efi_st_error("QueryVariableInfo failed\n");
		return EFI_ST_FAILURE;
	}

	ret = runtime->query_variable_info(attributes |
					   EFI_VARIABLE_AUTHENTICATED_WRITE_ACCESS |
					   EFI_VARIABLE_NON_VOLATILE,
					   &max_storage, &rem_storage,
					   &max_size);
	if (ret != EFI_UNSUPPORTED) {
		efi_st_error("QueryVariableInfo failed\n");
		return EFI_ST_FAILURE;
	}

	ret = runtime->query_variable_info(EFI_VARIABLE_NON_VOLATILE,
					   &max_storage, &rem_storage,
					   &max_size);
	if (ret != EFI_INVALID_PARAMETER) {
		efi_st_error("QueryVariableInfo failed\n");
		return EFI_ST_FAILURE;
	}

	/*
	 * Use a mix existing/non-existing attribute bits from the
	 * UEFI spec
	 */
	ret = runtime->query_variable_info(attributes | EFI_INVALID_ATTR |
					   EFI_VARIABLE_NON_VOLATILE,
					   &max_storage, &rem_storage,
					   &max_size);
	if (ret != EFI_INVALID_PARAMETER) {
		efi_st_error("QueryVariableInfo failed\n");
		return EFI_ST_FAILURE;
	}

	return EFI_ST_SUCCESS;
}
