// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * efi_selftest_dt_fixup
 *
 * Copyright (c) 2026 Heinrich Schuchardt <heinrich.schuchardt@canonical.com>
 *
 * Test the EFI_DT_FIXUP_PROTOCOL.
 * In the test the /chosen/u-boot,version property is in manipulated.
 * The fixup protocol should reset it.
 */

#include <efi_dt_fixup.h>
#include <efi_selftest.h>
#include <linux/libfdt.h>

static const struct efi_boot_services *boottime;
static const void *fdt;
static char *u_boot_version;

static const efi_guid_t fdt_guid = EFI_FDT_GUID;
static const efi_guid_t dt_fixup_guid = EFI_DT_FIXUP_PROTOCOL_GUID;
static const efi_guid_t dt_fixup2_guid = EFI_DT_FIXUP_PROTOCOL2_GUID;

/**
 * get_dtb_size() - get size of the device-tree
 *
 * @dtb:	device-tree
 * Return:	size of the device-tree
 */
static efi_uintn_t get_dtb_size(const void *dtb)
{
	const struct fdt_header *header = dtb;

	return efi_st_f2h(header->totalsize);
}

/**
 * test_dt_fixup() - test a device-tree fix-up protocol
 *
 * @guid:	GUID of the protocol
 * @current_proto:	true for current EFI_DT_FIXUP_PROTOCOL
 * Return:	EFI_ST_SUCCESS for success
 */
static int test_dt_fixup(const efi_guid_t *guid, const int current_proto)
{
	void *interface;
	const char *version;
	efi_physical_addr_t addr = 0;
	efi_status_t ret;
	efi_uintn_t buffer_size = 0;
	efi_uintn_t dtb_size = get_dtb_size(fdt);
	efi_uintn_t pages;
	void *dtb;
	u32 len;
	struct efi_dt_fixup_protocol *dt_fixup;
	struct efi_dt_fixup_protocol2 *dt_fixup2;

	ret = boottime->locate_protocol(guid, NULL, &interface);
	if (ret != EFI_SUCCESS) {
		efi_st_error("Device-tree fix-up protocol not available\n");
		return EFI_ST_FAILURE;
	}

	if (current_proto) {
		dt_fixup2 = interface;
		ret = dt_fixup2->fixup(dt_fixup2, (void *)fdt, &buffer_size);
	} else {
		dt_fixup = interface;
		ret = dt_fixup->fixup(dt_fixup, (void *)fdt, &buffer_size,
				      EFI_DT_APPLY_FIXUPS);
	}
	if (ret != EFI_BUFFER_TOO_SMALL) {
		efi_st_error("Fixup did not return EFI_BUFFER_TOO_SMALL\n");
		return EFI_ST_FAILURE;
	}
	if (buffer_size < dtb_size) {
		efi_st_error("Fixup returned too small buffer size\n");
		return EFI_ST_FAILURE;
	}

	pages = efi_size_in_pages(buffer_size);
	ret = boottime->allocate_pages(EFI_ALLOCATE_ANY_PAGES, EFI_LOADER_DATA,
				       pages, &addr);
	if (ret != EFI_SUCCESS) {
		efi_st_error("AllocatePages failed\n");
		return EFI_ST_FAILURE;
	}
	dtb = (void *)(uintptr_t)addr;
	boottime->copy_mem(dtb, fdt, dtb_size);

	version = efi_st_fdt_get_property(dtb, u"u-boot,version", u"chosen", &len);
	if (!version || !len) {
		efi_st_error("Missing property 'u-boot,version'\n");
		return EFI_ST_FAILURE;
	}
	*(char *)version = 0;

	if (current_proto)
		ret = dt_fixup2->fixup(dt_fixup2, dtb, &buffer_size);
	else
		ret = dt_fixup->fixup(dt_fixup, dtb, &buffer_size,
				       EFI_DT_APPLY_FIXUPS);
	if (ret != EFI_SUCCESS) {
		efi_st_error("Fixup failed\n");
		return EFI_ST_FAILURE;
	}

	version = efi_st_fdt_get_property(dtb, u"u-boot,version", u"chosen", &len);
	if (!version || !len) {
		efi_st_error("Missing property 'u-boot,version' after fix-up\n");
		return EFI_ST_FAILURE;
	}
	if (memcmp(version, u_boot_version, strlen(u_boot_version) + 1)) {
		efi_st_error("Incorrect u-boot,version after fix-up\n");
		return EFI_ST_FAILURE;
	}
	ret = boottime->free_pages(addr, pages);
	if (ret != EFI_SUCCESS) {
		efi_st_error("FreePages failed\n");
		return EFI_ST_FAILURE;
	}

	return EFI_ST_SUCCESS;
}

/**
 * setup() - set up unit test
 *
 * @handle:	handle of the loaded image
 * @systable:	system table
 * Return:	EFI_ST_SUCCESS for success
 */
static int setup(const efi_handle_t handle,
		 const struct efi_system_table *systable)
{
	boottime = systable->boottime;
	fdt = efi_st_get_config_table(&fdt_guid);
	if (!fdt) {
		efi_st_error("Missing device tree\n");
		return EFI_ST_FAILURE;
	}
	if (efi_st_f2h(((struct fdt_header *)fdt)->magic) != FDT_MAGIC) {
		efi_st_error("Wrong device tree magic\n");
		return EFI_ST_FAILURE;
	}

	u_boot_version = efi_st_fdt_get_string(fdt, u"u-boot,version", u"chosen");
	if (!u_boot_version) {
		efi_st_error("Missing property 'u-boot,version'\n");
		return EFI_ST_FAILURE;
	}
	if (!*u_boot_version) {
		efi_st_error("Empty property 'u-boot,version'\n");
		return EFI_ST_FAILURE;
	}

	return EFI_ST_SUCCESS;
}

/**
 * execute() - execute unit test
 *
 * Return:	EFI_ST_SUCCESS for success
 */
static int execute(void)
{
	if (test_dt_fixup(&dt_fixup_guid, 0) != EFI_ST_SUCCESS)
		return EFI_ST_FAILURE;
	if (test_dt_fixup(&dt_fixup2_guid, 1) != EFI_ST_SUCCESS)
		return EFI_ST_FAILURE;

	return EFI_ST_SUCCESS;
}

/**
 * teardown() - tear down unit test
 *
 * Return:	EFI_ST_SUCCESS for success
 */
static int teardown(void)
{
	if (u_boot_version) {
		efi_status_t ret;

		ret = boottime->free_pool(u_boot_version);
		if (ret != EFI_SUCCESS) {
			efi_st_error("FreePool failed\n");
			return EFI_ST_FAILURE;
		}
	}

	return EFI_ST_SUCCESS;
}

EFI_UNIT_TEST(dt_fixup) = {
	.name = "device-tree fix-up protocol",
	.phase = EFI_EXECUTE_BEFORE_BOOTTIME_EXIT,
	.setup = setup,
	.execute = execute,
	.teardown = teardown,
};
