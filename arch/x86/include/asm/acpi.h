/*
 * From coreboot
 *
 * Copyright (C) 2004 SUSE LINUX AG
 * Copyright (C) 2004 Nick Barker
 * Copyright (C) 2008-2009 coresystems GmbH
 * (Written by Stefan Reinauer <stepan@coresystems.de>)
 *
 * SPDX-License-Identifier:	GPL-2.0
 */

#ifndef __ASM_ACPI_H
#define __ASM_ACPI_H

#define RSDP_SIG		"RSD PTR "  /* RSDT pointer signature */
#define ACPI_TABLE_CREATOR	"U-BootAC"  /* Must be exactly 8 bytes long! */
#define OEM_ID			"U-Boot"    /* Must be exactly 6 bytes long! */
#define ASLC			"U-Bo"      /* Must be exactly 4 bytes long! */

/* 0 = S0, 1 = S1 ...*/
int acpi_get_slp_type(void);
void apci_set_slp_type(int type);

#endif
