// SPDX-License-Identifier: GPL-2.0+
/*
 * UPL handoff command functions
 *
 * Copyright 2024 Google LLC
 * Written by Simon Glass <sjg@chromium.org>
 */

#define LOG_CATEGORY UCLASS_BOOTSTD

#include <string.h>
#include <upl.h>

/* Names of bootmodes */
const char *const bootmode_names[UPLBM_COUNT] = {
	[UPLBM_FULL]	= "full",
	[UPLBM_MINIMAL]	= "minimal",
	[UPLBM_FAST]	= "fast",
	[UPLBM_DIAG]	= "diag",
	[UPLBM_DEFAULT]	= "default",
	[UPLBM_S2]	= "s2",
	[UPLBM_S3]	= "s3",
	[UPLBM_S4]	= "s4",
	[UPLBM_S5]	= "s5",
	[UPLBM_FACTORY]	= "factory",
	[UPLBM_FLASH]	= "flash",
	[UPLBM_RECOVERY] = "recovery",
};

/* Names of memory usages */
const char *const usage_names[UPLUS_COUNT] = {
	[UPLUS_ACPI_RECLAIM]	= "acpi-reclaim",
	[UPLUS_ACPI_NVS]	= "acpi-nvs",
	[UPLUS_BOOT_CODE]	= "boot-code",
	[UPLUS_BOOT_DATA]	= "boot-data",
	[UPLUS_RUNTIME_CODE]	= "runtime-code",
	[UPLUS_RUNTIME_DATA]	= "runtime-data",
};

/* Names of access types */
const char *const access_types[UPLUS_COUNT] = {
	[UPLAT_MMIO]	= "mmio",
	[UPLAT_IO]	= "io",
};

/* Names of graphics formats */
const char *const graphics_formats[UPLUS_COUNT] = {
	[UPLGF_ARGB32]	= "a8r8g8b8",
	[UPLGF_ABGR32]	= "a8b8g8r8",
	[UPLGF_ABGR64]	= "a16b16g16r16",
};

void upl_init(struct upl *upl)
{
	memset(upl, '\0', sizeof(struct upl));
	alist_init_struct(&upl->image, struct upl_image);
	alist_init_struct(&upl->mem, struct upl_mem);
	alist_init_struct(&upl->memmap, struct upl_memmap);
	alist_init_struct(&upl->memres, struct upl_memres);
}
