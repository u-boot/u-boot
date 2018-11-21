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

static struct efi_boot_services *boottime;
static const char *fdt;

/* This should be sufficent for */
#define BUFFERSIZE 0x100000

static efi_guid_t fdt_guid = EFI_FDT_GUID;

/*
 * Convert FDT value to host endianness.
 *
 * @val		FDT value
 * @return	converted value
 */
static uint32_t f2h(fdt32_t val)
{
	char *buf = (char *)&val;
	char i;

#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
	/* Swap the bytes */
	i = buf[0]; buf[0] = buf[3]; buf[3] = i;
	i = buf[1]; buf[1] = buf[2]; buf[2] = i;
#endif
	return *(uint32_t *)buf;
}

/*
 * Return the value of a property of the FDT root node.
 *
 * @name	name of the property
 * @return	value of the property
 */
static char *get_property(const u16 *property)
{
	struct fdt_header *header = (struct fdt_header *)fdt;
	const fdt32_t *pos;
	const char *strings;

	if (!header)
		return NULL;

	if (f2h(header->magic) != FDT_MAGIC) {
		printf("Wrong magic\n");
		return NULL;
	}

	pos = (fdt32_t *)(fdt + f2h(header->off_dt_struct));
	strings = fdt + f2h(header->off_dt_strings);

	for (;;) {
		switch (f2h(pos[0])) {
		case FDT_BEGIN_NODE: {
			char *c = (char *)&pos[1];
			size_t i;

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
			if (!efi_st_strcmp_16_8(property, label)) {
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
					efi_st_printf("AllocatePool failed\n");
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
			pos = &pos[1];
			break;
		default:
			return NULL;
		}
	}
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
	efi_uintn_t i;

	boottime = systable->boottime;

	/* Find configuration tables */
	for (i = 0; i < systable->nr_tables; ++i) {
		if (!efi_st_memcmp(&systable->tables[i].guid, &fdt_guid,
				   sizeof(efi_guid_t)))
			fdt = systable->tables[i].table;
	}
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
	char *str;
	efi_status_t ret;

	str = get_property(L"compatible");
	if (str) {
		efi_st_printf("compatible: %s\n", str);
		ret = boottime->free_pool(str);
		if (ret != EFI_SUCCESS) {
			efi_st_error("FreePool failed\n");
			return EFI_ST_FAILURE;
		}
	} else {
		efi_st_printf("Missing property 'compatible'\n");
		return EFI_ST_FAILURE;
	}
	str = get_property(L"serial-number");
	if (str) {
		efi_st_printf("serial-number: %s\n", str);
		ret = boottime->free_pool(str);
		if (ret != EFI_SUCCESS) {
			efi_st_error("FreePool failed\n");
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
	.on_request = true,
};
