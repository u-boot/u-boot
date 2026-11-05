// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * efi_selftest_fdtlib
 *
 * Copyright (c) 2026 Heinrich Schuchardt <heinrich.schuchardt@canonical.com>
 */

#include <efi_selftest.h>
#include <linux/libfdt.h>

/**
 * efi_st_f2h() - convert FDT value to host endianness.
 *
 * UEFI code is always low endian. The FDT is big endian.
 *
 * @val:	FDT value
 * Return:	converted value
 */
uint32_t efi_st_f2h(fdt32_t val)
{
	char *buf = (char *)&val;
	char i;

	/* Swap the bytes */
	i = buf[0]; buf[0] = buf[3]; buf[3] = i;
	i = buf[1]; buf[1] = buf[2]; buf[2] = i;

	return val;
}

/**
 * next_fdt_node() - skip the name of an FDT node
 *
 * @pos:	position of an FDT_BEGIN_NODE token
 * Return:	position of the next token after the FDT node name
 */
static const fdt32_t *next_fdt_node(const fdt32_t *pos)
{
	const char *c = (char *)&pos[1];
	size_t i;

	for (i = 0; c[i]; ++i)
		;

	return &pos[2 + (i >> 2)];
}

/**
 * efi_st_fdt_find_node() - find a device-tree node
 *
 * The root node or one of its direct children can be retrieved.
 *
 * @fdt:	flat device-tree
 * @node:	name of the node or NULL for the root node
 * Return:	pointer to the first byte of the node, or NULL
 */
const void *efi_st_fdt_find_node(const void *fdt, const u16 *node)
{
	const char *fdt_pos = fdt;
	const struct fdt_header *header = (struct fdt_header *)fdt;
	const fdt32_t *end;
	const fdt32_t *pos;
	size_t level = 0;

	if (!header) {
		efi_st_error("Missing device tree\n");
		return NULL;
	}

	if (efi_st_f2h(header->magic) != FDT_MAGIC) {
		efi_st_error("Wrong device tree magic\n");
		return NULL;
	}

	pos = (fdt32_t *)(fdt_pos + efi_st_f2h(header->off_dt_struct));
	end = (fdt32_t *)(fdt_pos + efi_st_f2h(header->totalsize));

	for (; pos < end;) {
		switch (efi_st_f2h(pos[0])) {
		case FDT_BEGIN_NODE: {
			const char *c = (char *)&pos[1];
			const fdt32_t *next = next_fdt_node(pos);

			if ((!node && !level) ||
			    (node && level == 1 &&
			     !efi_st_strcmp_16_8(node, c)))
				return pos;

			++level;
			pos = next;
			break;
		}
		case FDT_PROP: {
			struct fdt_property *prop = (struct fdt_property *)pos;

			pos = &pos[3 + ((efi_st_f2h(prop->len) + 3) >> 2)];
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
 * efi_st_fdt_get_property() - return location of an FDT property
 *
 * A property of the root node or one of its direct children can be retrieved.
 *
 * @fdt:	flat device-tree
 * @property:	name of the property
 * @node:	name of the node or NULL for root node
 * @len:	length of the property
 * Return:	location of the property value, or NULL
 */
const void *efi_st_fdt_get_property(const void *fdt, const u16 *property,
				    const u16 *node, u32 *len)
{
	const char *fdt_pos = fdt;
	struct fdt_header *header = (struct fdt_header *)fdt;
	const fdt32_t *end;
	const fdt32_t *pos;
	const char *strings;

	pos = efi_st_fdt_find_node(fdt, node);
	if (!pos)
		return NULL;

	pos = next_fdt_node(pos);
	end = (fdt32_t *)(fdt_pos + efi_st_f2h(header->totalsize));
	strings = fdt_pos + efi_st_f2h(header->off_dt_strings);

	for (; pos < end;) {
		switch (efi_st_f2h(pos[0])) {
		case FDT_PROP: {
			struct fdt_property *prop = (struct fdt_property *)pos;
			const char *label = &strings[efi_st_f2h(prop->nameoff)];

			if (!efi_st_strcmp_16_8(property, label)) {
				if (len)
					*len = efi_st_f2h(prop->len);
				return &pos[3];
			}

			pos = &pos[3 + ((efi_st_f2h(prop->len) + 3) >> 2)];
			break;
		}
		case FDT_NOP:
			++pos;
			break;
		case FDT_BEGIN_NODE:
		case FDT_END_NODE:
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
 * efi_st_fdt_get_string() - return string value of a property of an FDT node
 *
 * A string property of the root node or one of its direct children can be
 * retrieved. The caller is responsible for calling free() for the returned
 * buffer.
 *
 * @fdt:	flat device-tree
 * @property:	name of the property
 * @node:	name of the node or NULL for root node
 * Return:	allocated copy of the property string, or NULL
 */
char *efi_st_fdt_get_string(const void *fdt, const u16 *property,
			    const u16 *node)
{
	const char *prop;
	efi_status_t ret;
	char *str;
	u32 len;

	prop = efi_st_fdt_get_property(fdt, property, node, &len);
	if (!prop || !len)
		return NULL;

	/*
	 * The string might not be 0 terminated.
	 * It is safer to make a copy.
	 */
	ret = st_boottime->allocate_pool(EFI_LOADER_DATA, len + 1,
					       (void **)&str);
	if (ret != EFI_SUCCESS) {
		efi_st_error("AllocatePool failed\n");
		return NULL;
	}
	st_boottime->copy_mem(str, prop, len);
	str[len] = 0;

	return str;
}
