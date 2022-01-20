// SPDX-License-Identifier: GPL-2.0+
/*
 *  EFI application tables support
 *
 *  Copyright (c) 2016 Alexander Graf
 */

#define LOG_CATEGORY LOGC_EFI

#include <common.h>
#include <efi_loader.h>
#include <log.h>
#include <mapmem.h>
#include <smbios.h>

/*
 * Install the SMBIOS table as a configuration table.
 *
 * Return:	status code
 */
efi_status_t efi_smbios_register(void)
{
	/* Map within the low 32 bits, to allow for 32bit SMBIOS tables */
	u64 dmi_addr = U32_MAX;
	efi_status_t ret;
	void *dmi;

	/* Reserve 4kiB page for SMBIOS */
	ret = efi_allocate_pages(EFI_ALLOCATE_MAX_ADDRESS,
				 EFI_RUNTIME_SERVICES_DATA, 1, &dmi_addr);

	if (ret != EFI_SUCCESS) {
		/* Could not find space in lowmem, use highmem instead */
		ret = efi_allocate_pages(EFI_ALLOCATE_ANY_PAGES,
					 EFI_RUNTIME_SERVICES_DATA, 1,
					 &dmi_addr);

		if (ret != EFI_SUCCESS)
			return ret;
	}

	/*
	 * Generate SMBIOS tables - we know that efi_allocate_pages() returns
	 * a 4k-aligned address, so it is safe to assume that
	 * write_smbios_table() will write the table at that address.
	 */
	assert(!(dmi_addr & 0xf));
	dmi = (void *)(uintptr_t)dmi_addr;
	if (write_smbios_table(map_to_sysmem(dmi)))
		/* Install SMBIOS information as configuration table */
		return efi_install_configuration_table(&smbios_guid, dmi);
	efi_free_pages(dmi_addr, 1);
	log_err("Cannot create SMBIOS table\n");
	return EFI_SUCCESS;
}
