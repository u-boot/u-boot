// SPDX-License-Identifier: GPL-2.0+
/*
 * efi_selftest_fdt
 *
 * Copyright (c) 2018 Heinrich Schuchardt <xypron.glpk@gmx.de>
 * Copyright (c) 2022 Ventana Micro Systems Inc
 *
 * Check the device tree, test the RISCV_EFI_BOOT_PROTOCOL.
 */

#include <efi_riscv.h>
#include <efi_selftest.h>
#include <linux/libfdt.h>

static const struct efi_system_table *systemtab;
static const struct efi_boot_services *boottime;
static const char *fdt;

/* This should be sufficient for */
#define BUFFERSIZE 0x100000

static const efi_guid_t fdt_guid = EFI_FDT_GUID;
static const efi_guid_t acpi_guid = EFI_ACPI_TABLE_GUID;
static const efi_guid_t riscv_efi_boot_protocol_guid =
				RISCV_EFI_BOOT_PROTOCOL_GUID;

/**
 * get_boot_hartid_fdt() - return boot-hartid property of the chosen node
 *
 * Return:	location of the boot-hartid property, or NULL
 */
static const fdt32_t *get_boot_hartid_fdt(void)
{
	const fdt32_t *prop;
	u32 len;

	prop = efi_st_fdt_get_property(fdt, u"boot-hartid", u"chosen", &len);
	if (!prop || len != sizeof(*prop))
		return NULL;

	return prop;
}

/*
 * Setup unit test.
 *
 * @handle:	handle of the loaded image
 * @systable:	system table
 * Return:	EFI_ST_SUCCESS for success
 */
static int setup(const efi_handle_t img_handle,
		 const struct efi_system_table *systable)
{
	void *acpi;

	systemtab = systable;
	boottime = systable->boottime;

	acpi = efi_st_get_config_table(&acpi_guid);
	fdt = efi_st_get_config_table(&fdt_guid);

	if (!fdt) {
		efi_st_error("Missing device tree\n");
		return EFI_ST_FAILURE;
	}
	if (acpi) {
		efi_st_error("Found ACPI table and device tree\n");
		return EFI_ST_FAILURE;
	}
	return EFI_ST_SUCCESS;
}

__maybe_unused static efi_status_t get_boot_hartid(efi_uintn_t *efi_hartid)
{
	efi_status_t ret;
	struct riscv_efi_boot_protocol *prot;

	/* Get RISC-V boot protocol */
	ret = boottime->locate_protocol(&riscv_efi_boot_protocol_guid, NULL,
					(void **)&prot);
	if (ret != EFI_SUCCESS) {
		efi_st_error("RISC-V Boot Protocol not available\n");
		return EFI_ST_FAILURE;
	}

	/* Get boot hart ID from EFI protocol */
	ret = prot->get_boot_hartid(prot, efi_hartid);
	if (ret != EFI_SUCCESS) {
		efi_st_error("Could not retrieve boot hart ID\n");
		return EFI_ST_FAILURE;
	}

	return EFI_ST_SUCCESS;
}

/*
 * Execute unit test.
 *
 * Return:	EFI_ST_SUCCESS for success
 */
static int execute(void)
{
	char *str;
	efi_status_t ret;

	str = efi_st_fdt_get_string(fdt, u"compatible", NULL);
	if (str) {
		efi_st_printf("compatible: %s\n", str);
		ret = boottime->free_pool(str);
		if (ret != EFI_SUCCESS) {
			efi_st_error("FreePool failed\n");
			return EFI_ST_FAILURE;
		}
	} else {
		efi_st_error("Missing property 'compatible'\n");
		return EFI_ST_FAILURE;
	}
	str = efi_st_fdt_get_string(fdt, u"serial-number", NULL);
	if (str) {
		efi_st_printf("serial-number: %s\n", str);
		ret = boottime->free_pool(str);
		if (ret != EFI_SUCCESS) {
			efi_st_error("FreePool failed\n");
			return EFI_ST_FAILURE;
		}
	}
	if (IS_ENABLED(CONFIG_EFI_TCG2_PROTOCOL_MEASURE_DTB)) {
		str = efi_st_fdt_get_string(fdt, u"kaslr-seed", u"chosen");
		if (str) {
			efi_st_error("kaslr-seed with measured fdt\n");
			return EFI_ST_FAILURE;
		}
	}
	if (IS_ENABLED(CONFIG_RISCV)) {
		const fdt32_t *prop;
		u32 fdt_hartid;

		prop = get_boot_hartid_fdt();
		if (!prop) {
			efi_st_error("boot-hartid missing in devicetree\n");
			return EFI_ST_FAILURE;
		}
		fdt_hartid = efi_st_f2h(*prop);
		efi_st_printf("boot-hartid: %u\n", fdt_hartid);

		if (IS_ENABLED(CONFIG_EFI_RISCV_BOOT_PROTOCOL)) {
			efi_uintn_t efi_hartid;
			int r;

			r = get_boot_hartid(&efi_hartid);
			if (r != EFI_ST_SUCCESS)
				return r;
			/* Boot hart ID should be same */
			if (efi_hartid != fdt_hartid) {
				efi_st_error("boot-hartid differs: prot 0x%p, DT 0x%.8x\n",
					     (void *)(uintptr_t)efi_hartid,
					     fdt_hartid);
				return EFI_ST_FAILURE;
			}
		}
	}

	return EFI_ST_SUCCESS;
}

EFI_UNIT_TEST(fdt) = {
	.name = "device tree",
	.phase = EFI_EXECUTE_BEFORE_BOOTTIME_EXIT,
	.setup = setup,
	.execute = execute,
};
