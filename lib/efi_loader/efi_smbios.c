/*
 *  EFI application tables support
 *
 *  Copyright (c) 2016 Alexander Graf
 *
 *  SPDX-License-Identifier:     GPL-2.0+
 */

#include <common.h>
#include <efi_loader.h>
#include <inttypes.h>
#include <smbios.h>

static const efi_guid_t smbios_guid = SMBIOS_TABLE_GUID;

void efi_smbios_register(void)
{
	/* Map within the low 32 bits, to allow for 32bit SMBIOS tables */
	uint64_t dmi = 0xffffffff;
	/* Reserve 4kb for SMBIOS */
	uint64_t pages = 1;
	int memtype = EFI_RUNTIME_SERVICES_DATA;

	if (efi_allocate_pages(1, memtype, pages, &dmi) != EFI_SUCCESS)
		return;

	/* Generate SMBIOS tables */
	write_smbios_table(dmi);

	/* And expose them to our EFI payload */
	efi_install_configuration_table(&smbios_guid, (void*)(uintptr_t)dmi);
}
