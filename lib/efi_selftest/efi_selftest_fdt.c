// SPDX-License-Identifier: GPL-2.0+
/*
 * efi_selftest_pos
 *
 * Copyright (c) 2018 Heinrich Schuchardt <xypron.glpk@gmx.de>
 *
 * Test the EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL.
 *
 * The following services are tested:
 * OutputString, TestString, SetAttribute.
 */

#include <efi_selftest.h>
#include <linux/libfdt.h>

static const struct efi_system_table *systemtab;
static const struct efi_boot_services *boottime;
static const char *fdt;

/* This should be sufficient for */
#define BUFFERSIZE 0x100000

static const efi_guid_t fdt_guid = EFI_FDT_GUID;
static const efi_guid_t acpi_guid = EFI_ACPI_TABLE_GUID;

/**
 * f2h() - convert FDT value to host endianness.
 *
 * UEFI code is always low endian. The FDT is big endian.
 *
 * @val:	FDT value
 * Return:	converted value
 */
static uint32_t f2h(fdt32_t val)
{
	char *buf = (char *)&val;
	char i;

	/* Swap the bytes */
	i = buf[0]; buf[0] = buf[3]; buf[3] = i;
	i = buf[1]; buf[1] = buf[2]; buf[2] = i;

	return val;
}

/**
 * get_property() - return value of a property of an FDT node
 *
 * A property of the root node or one of its direct children can be
 * retrieved.
 *
 * @property	name of the property
 * @node	name of the node or NULL for root node
 * @return	value of the property
 */
static char *get_property(const u16 *property, const u16 *node)
{
	struct fdt_header *header = (struct fdt_header *)fdt;
	const fdt32_t *end;
	const fdt32_t *pos;
	const char *strings;
	size_t level = 0;
	const char *nodelabel = NULL;

	if (!header) {
		efi_st_error("Missing device tree\n");
		return NULL;
	}

	if (f2h(header->magic) != FDT_MAGIC) {
		efi_st_error("Wrong device tree magic\n");
		return NULL;
	}

	pos = (fdt32_t *)(fdt + f2h(header->off_dt_struct));
	end = &pos[f2h(header->totalsize) >> 2];
	strings = fdt + f2h(header->off_dt_strings);

	for (; pos < end;) {
		switch (f2h(pos[0])) {
		case FDT_BEGIN_NODE: {
			const char *c = (char *)&pos[1];
			size_t i;

			if (level == 1)
				nodelabel = c;
			++level;
			for (i = 0; c[i]; ++i)
				;
			pos = &pos[2 + (i >> 2)];
			break;
		}
		case FDT_PROP: {
			struct fdt_property *prop = (struct fdt_property *)pos;
			const char *label = &strings[f2h(prop->nameoff)];
			efi_status_t ret;

			/* Check if this is the property to be returned */
			if (!efi_st_strcmp_16_8(property, label) &&
			    ((level == 1 && !node) ||
			     (level == 2 && node &&
			      !efi_st_strcmp_16_8(node, nodelabel)))) {
				char *str;
				efi_uintn_t len = f2h(prop->len);

				if (!len)
					return NULL;
				/*
				 * The string might not be 0 terminated.
				 * It is safer to make a copy.
				 */
				ret = boottime->allocate_pool(
					EFI_LOADER_DATA, len + 1,
					(void **)&str);
				if (ret != EFI_SUCCESS) {
					efi_st_error("AllocatePool failed\n");
					return NULL;
				}
				boottime->copy_mem(str, &pos[3], len);
				str[len] = 0;

				return str;
			}

			pos = &pos[3 + ((f2h(prop->len) + 3) >> 2)];
			break;
		}
		case FDT_NOP:
			++pos;
			break;
		case FDT_END_NODE:
			--level;
			++pos;
			break;
		case FDT_END:
			return NULL;
		default:
			efi_st_error("Invalid device tree token\n");
			return NULL;
		}
	}
	efi_st_error("Missing FDT_END token\n");
	return NULL;
}

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

/*
 * Execute unit test.
 *
 * @return:	EFI_ST_SUCCESS for success
 */
static int execute(void)
{
	char *str;
	efi_status_t ret;

	str = get_property(L"compatible", NULL);
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
	str = get_property(L"serial-number", NULL);
	if (str) {
		efi_st_printf("serial-number: %s\n", str);
		ret = boottime->free_pool(str);
		if (ret != EFI_SUCCESS) {
			efi_st_error("FreePool failed\n");
			return EFI_ST_FAILURE;
		}
	}
	str = get_property(L"boot-hartid", L"chosen");
	if (IS_ENABLED(CONFIG_RISCV)) {
		if (str) {
			efi_st_printf("boot-hartid: %u\n",
				      f2h(*(fdt32_t *)str));
			ret = boottime->free_pool(str);
			if (ret != EFI_SUCCESS) {
				efi_st_error("FreePool failed\n");
				return EFI_ST_FAILURE;
			}
		} else {
			efi_st_error("boot-hartid not found\n");
			return EFI_ST_FAILURE;
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
