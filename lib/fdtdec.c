/*
 * Copyright (c) 2011 The Chromium OS Authors.
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <common.h>
#include <serial.h>
#include <libfdt.h>
#include <fdtdec.h>

DECLARE_GLOBAL_DATA_PTR;

/*
 * Here are the type we know about. One day we might allow drivers to
 * register. For now we just put them here. The COMPAT macro allows us to
 * turn this into a sparse list later, and keeps the ID with the name.
 */
#define COMPAT(id, name) name
static const char * const compat_names[COMPAT_COUNT] = {
};

/**
 * Look in the FDT for an alias with the given name and return its node.
 *
 * @param blob	FDT blob
 * @param name	alias name to look up
 * @return node offset if found, or an error code < 0 otherwise
 */
static int find_alias_node(const void *blob, const char *name)
{
	const char *path;
	int alias_node;

	debug("find_alias_node: %s\n", name);
	alias_node = fdt_path_offset(blob, "/aliases");
	if (alias_node < 0)
		return alias_node;
	path = fdt_getprop(blob, alias_node, name, NULL);
	if (!path)
		return -FDT_ERR_NOTFOUND;
	return fdt_path_offset(blob, path);
}

fdt_addr_t fdtdec_get_addr(const void *blob, int node,
		const char *prop_name)
{
	const fdt_addr_t *cell;
	int len;

	debug("get_addr: %s\n", prop_name);
	cell = fdt_getprop(blob, node, prop_name, &len);
	if (cell && (len == sizeof(fdt_addr_t) ||
			len == sizeof(fdt_addr_t) * 2))
		return fdt_addr_to_cpu(*cell);
	return FDT_ADDR_T_NONE;
}

s32 fdtdec_get_int(const void *blob, int node, const char *prop_name,
		s32 default_val)
{
	const s32 *cell;
	int len;

	debug("get_size: %s\n", prop_name);
	cell = fdt_getprop(blob, node, prop_name, &len);
	if (cell && len >= sizeof(s32))
		return fdt32_to_cpu(cell[0]);
	return default_val;
}

int fdtdec_get_is_enabled(const void *blob, int node, int default_val)
{
	const char *cell;

	cell = fdt_getprop(blob, node, "status", NULL);
	if (cell)
		return 0 == strcmp(cell, "ok");
	return default_val;
}

enum fdt_compat_id fd_dec_lookup(const void *blob, int node)
{
	enum fdt_compat_id id;

	/* Search our drivers */
	for (id = COMPAT_UNKNOWN; id < COMPAT_COUNT; id++)
		if (0 == fdt_node_check_compatible(blob, node,
				compat_names[id]))
			return id;
	return COMPAT_UNKNOWN;
}

int fdtdec_next_compatible(const void *blob, int node,
		enum fdt_compat_id id)
{
	return fdt_node_offset_by_compatible(blob, node, compat_names[id]);
}

int fdtdec_next_alias(const void *blob, const char *name,
		enum fdt_compat_id id, int *upto)
{
#define MAX_STR_LEN 20
	char str[MAX_STR_LEN + 20];
	int node, err;

	/* snprintf() is not available */
	assert(strlen(name) < MAX_STR_LEN);
	sprintf(str, "%.*s%d", MAX_STR_LEN, name, *upto);
	(*upto)++;
	node = find_alias_node(blob, str);
	if (node < 0)
		return node;
	err = fdt_node_check_compatible(blob, node, compat_names[id]);
	if (err < 0)
		return err;
	return err ? -FDT_ERR_NOTFOUND : node;
}

/*
 * This function is a little odd in that it accesses global data. At some
 * point if the architecture board.c files merge this will make more sense.
 * Even now, it is common code.
 */
int fdtdec_check_fdt(void)
{
	/* We must have an fdt */
	if (fdt_check_header(gd->fdt_blob))
		panic("No valid fdt found - please append one to U-Boot\n"
			"binary or define CONFIG_OF_EMBED\n");
	return 0;
}
