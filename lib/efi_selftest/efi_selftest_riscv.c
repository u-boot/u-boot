// SPDX-License-Identifier: GPL-2.0+
/*
 * efi_selftest_riscv
 *
 * Copyright (c) 2022 Ventana Micro Systems Inc
 *
 * Test the RISCV_EFI_BOOT_PROTOCOL.
 *
 * The following services are tested:
 * get_boot_hartid
 */

#include <efi_selftest.h>
#include <efi_riscv.h>
#include <linux/libfdt.h>

static const struct efi_system_table *systemtab;
static const struct efi_boot_services *boottime;
static const char *fdt;
static const efi_guid_t riscv_efi_boot_protocol_guid = RISCV_EFI_BOOT_PROTOCOL_GUID;
static const efi_guid_t fdt_guid = EFI_FDT_GUID;

/**
 * efi_st_get_config_table() - get configuration table
 *
 * @guid:	GUID of the configuration table
 * Return:	pointer to configuration table or NULL
 */
static void *efi_st_get_config_table(const efi_guid_t *guid)
{
	size_t i;

	for (i = 0; i < systab.nr_tables; i++) {
		if (!guidcmp(guid, &systemtab->tables[i].guid))
			return systemtab->tables[i].table;
	}
	return NULL;
}

/*
 * Setup unit test.
 *
 * @handle:	handle of the loaded image
 * @systable:	system table
 * @return:	EFI_ST_SUCCESS for success
 */
static int setup(const efi_handle_t img_handle,
		 const struct efi_system_table *systable)
{
	systemtab = systable;
	boottime = systable->boottime;

	fdt = efi_st_get_config_table(&fdt_guid);

	if (!fdt) {
		efi_st_error("Missing device tree\n");
		return EFI_ST_FAILURE;
	}
	return EFI_ST_SUCCESS;
}

/*
 * Execute unit test.
 *
 * @return:	EFI_ST_SUCCESS for success
 */
static int execute(void)
{
	efi_status_t ret;
	struct riscv_efi_boot_protocol *prot;
	efi_uintn_t efi_hartid, fdt_hartid;
	int chosen_node, len;
	const fdt32_t *prop;

	/* Get riscv boot protocol */
	ret = boottime->locate_protocol(&riscv_efi_boot_protocol_guid, NULL,
					(void **)&prot);
	if (ret != EFI_SUCCESS) {
		efi_st_error("RISC-V Boot Protocol not available\n");
		return EFI_ST_FAILURE;
	}

	/* Get Boot Hart ID from EFI protocol */
	ret = prot->get_boot_hartid(prot, &efi_hartid);
	if (ret != EFI_SUCCESS) {
		efi_st_error("Could not retrieve boot hart ID\n");
		return EFI_ST_FAILURE;
	}

	/* Get Boot Hart ID from FDT */
	chosen_node = fdt_path_offset(fdt, "/chosen");
	if (chosen_node < 0) {
		efi_st_error("/chosen node not found\n");
		return EFI_ST_FAILURE;
	}

	prop = fdt_getprop((void *)fdt, chosen_node, "boot-hartid", &len);
	if (!prop || len != sizeof(u32)) {
		efi_st_error("boot-hartid not found\n");
		return EFI_ST_FAILURE;
	}

	fdt_hartid = fdt32_to_cpu(*prop);

	/* Boot Hart ID should be same */
	if (efi_hartid != fdt_hartid) {
		efi_st_error("boot-hartid is not same in EFI and FDT\n");
		return EFI_ST_FAILURE;
	}

	return EFI_ST_SUCCESS;
}

EFI_UNIT_TEST(riscv) = {
	.name = "riscv",
	.phase = EFI_EXECUTE_BEFORE_BOOTTIME_EXIT,
	.setup = setup,
	.execute = execute,
};
