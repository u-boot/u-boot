/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (c) 2020, Heinrich Schuchardt <xypron.glpk@gmx.de>
 */

#ifndef _EFI_VARIABLE_H
#define _EFI_VARIABLE_H

#include <linux/bitops.h>

#define EFI_VARIABLE_READ_ONLY BIT(31)

/**
 * efi_get_variable() - retrieve value of a UEFI variable
 *
 * @variable_name:	name of the variable
 * @vendor:		vendor GUID
 * @attributes:		attributes of the variable
 * @data_size:		size of the buffer to which the variable value is copied
 * @data:		buffer to which the variable value is copied
 * @timep:		authentication time (seconds since start of epoch)
 * Return:		status code
 */
efi_status_t efi_get_variable_int(u16 *variable_name, const efi_guid_t *vendor,
				  u32 *attributes, efi_uintn_t *data_size,
				  void *data, u64 *timep);

/**
 * efi_set_variable() - set value of a UEFI variable
 *
 * @variable_name:	name of the variable
 * @vendor:		vendor GUID
 * @attributes:		attributes of the variable
 * @data_size:		size of the buffer with the variable value
 * @data:		buffer with the variable value
 * @ro_check:		check the read only read only bit in attributes
 * Return:		status code
 */
efi_status_t efi_set_variable_int(u16 *variable_name, const efi_guid_t *vendor,
				  u32 attributes, efi_uintn_t data_size,
				  const void *data, bool ro_check);

#endif
