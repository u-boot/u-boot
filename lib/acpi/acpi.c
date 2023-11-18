// SPDX-License-Identifier: GPL-2.0
/*
 * Utility functions for ACPI
 *
 * Copyright 2023 Google LLC
 */

#include <common.h>
#include <mapmem.h>
#include <acpi/acpi_table.h>
#include <asm/global_data.h>

DECLARE_GLOBAL_DATA_PTR;

struct acpi_table_header *acpi_find_table(const char *sig)
{
	struct acpi_rsdp *rsdp;
	struct acpi_rsdt *rsdt;
	struct acpi_xsdt *xsdt;
	int len, i, count;

	rsdp = map_sysmem(gd_acpi_start(), 0);
	if (!rsdp)
		return NULL;
	if (rsdp->xsdt_address) {
		xsdt = map_sysmem(rsdp->xsdt_address, 0);
		len = xsdt->header.length - sizeof(xsdt->header);
		count = len / sizeof(u64);
	} else {
		if (!rsdp->rsdt_address)
			return NULL;
		rsdt = map_sysmem(rsdp->rsdt_address, 0);
		len = rsdt->header.length - sizeof(rsdt->header);
		count = len / sizeof(u32);
	}
	for (i = 0; i < count; i++) {
		struct acpi_table_header *hdr;

		if (rsdp->xsdt_address)
			hdr = map_sysmem(xsdt->entry[i], 0);
		else
			hdr = map_sysmem(rsdt->entry[i], 0);
		if (!memcmp(hdr->signature, sig, ACPI_NAME_LEN))
			return hdr;
		if (!memcmp(hdr->signature, "FACP", ACPI_NAME_LEN)) {
			struct acpi_fadt *fadt = (struct acpi_fadt *)hdr;

			if (!memcmp(sig, "DSDT", ACPI_NAME_LEN) && fadt->dsdt)
				return map_sysmem(fadt->dsdt, 0);
			if (!memcmp(sig, "FACS", ACPI_NAME_LEN) &&
			    fadt->firmware_ctrl)
				return map_sysmem(fadt->firmware_ctrl, 0);
		}
	}

	return NULL;
}
