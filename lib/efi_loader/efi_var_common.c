// SPDX-License-Identifier: GPL-2.0+
/*
 * UEFI runtime variable services
 *
 * Copyright (c) 2020, Heinrich Schuchardt <xypron.glpk@gmx.de>
 */

#include <common.h>
#include <efi_loader.h>
#include <efi_variable.h>

/**
 * efi_efi_get_variable() - retrieve value of a UEFI variable
 *
 * This function implements the GetVariable runtime service.
 *
 * See the Unified Extensible Firmware Interface (UEFI) specification for
 * details.
 *
 * @variable_name:	name of the variable
 * @vendor:		vendor GUID
 * @attributes:		attributes of the variable
 * @data_size:		size of the buffer to which the variable value is copied
 * @data:		buffer to which the variable value is copied
 * Return:		status code
 */
efi_status_t EFIAPI efi_get_variable(u16 *variable_name,
				     const efi_guid_t *vendor, u32 *attributes,
				     efi_uintn_t *data_size, void *data)
{
	efi_status_t ret;

	EFI_ENTRY("\"%ls\" %pUl %p %p %p", variable_name, vendor, attributes,
		  data_size, data);

	ret = efi_get_variable_int(variable_name, vendor, attributes,
				   data_size, data, NULL);

	/* Remove EFI_VARIABLE_READ_ONLY flag */
	if (attributes)
		*attributes &= EFI_VARIABLE_MASK;

	return EFI_EXIT(ret);
}

/**
 * efi_set_variable() - set value of a UEFI variable
 *
 * This function implements the SetVariable runtime service.
 *
 * See the Unified Extensible Firmware Interface (UEFI) specification for
 * details.
 *
 * @variable_name:	name of the variable
 * @vendor:		vendor GUID
 * @attributes:		attributes of the variable
 * @data_size:		size of the buffer with the variable value
 * @data:		buffer with the variable value
 * Return:		status code
 */
efi_status_t EFIAPI efi_set_variable(u16 *variable_name,
				     const efi_guid_t *vendor, u32 attributes,
				     efi_uintn_t data_size, const void *data)
{
	efi_status_t ret;

	EFI_ENTRY("\"%ls\" %pUl %x %zu %p", variable_name, vendor, attributes,
		  data_size, data);

	/* Make sure that the EFI_VARIABLE_READ_ONLY flag is not set */
	if (attributes & ~(u32)EFI_VARIABLE_MASK)
		ret = EFI_INVALID_PARAMETER;
	else
		ret = efi_set_variable_int(variable_name, vendor, attributes,
					   data_size, data, true);

	return EFI_EXIT(ret);
}

/**
 * efi_get_next_variable_name() - enumerate the current variable names
 *
 * @variable_name_size:	size of variable_name buffer in byte
 * @variable_name:	name of uefi variable's name in u16
 * @vendor:		vendor's guid
 *
 * See the Unified Extensible Firmware Interface (UEFI) specification for
 * details.
 *
 * Return: status code
 */
efi_status_t EFIAPI efi_get_next_variable_name(efi_uintn_t *variable_name_size,
					       u16 *variable_name,
					       efi_guid_t *vendor)
{
	efi_status_t ret;

	EFI_ENTRY("%p \"%ls\" %pUl", variable_name_size, variable_name, vendor);

	ret = efi_get_next_variable_name_int(variable_name_size, variable_name,
					     vendor);

	return EFI_EXIT(ret);
}

/**
 * efi_query_variable_info() - get information about EFI variables
 *
 * This function implements the QueryVariableInfo() runtime service.
 *
 * See the Unified Extensible Firmware Interface (UEFI) specification for
 * details.
 *
 * @attributes:				bitmask to select variables to be
 *					queried
 * @maximum_variable_storage_size:	maximum size of storage area for the
 *					selected variable types
 * @remaining_variable_storage_size:	remaining size of storage are for the
 *					selected variable types
 * @maximum_variable_size:		maximum size of a variable of the
 *					selected type
 * Returns:				status code
 */
efi_status_t EFIAPI efi_query_variable_info(
			u32 attributes, u64 *maximum_variable_storage_size,
			u64 *remaining_variable_storage_size,
			u64 *maximum_variable_size)
{
	efi_status_t ret;

	EFI_ENTRY("%x %p %p %p", attributes, maximum_variable_storage_size,
		  remaining_variable_storage_size, maximum_variable_size);

	ret = efi_query_variable_info_int(attributes,
					  maximum_variable_storage_size,
					  remaining_variable_storage_size,
					  maximum_variable_size);

	return EFI_EXIT(ret);
}
