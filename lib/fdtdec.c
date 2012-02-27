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
	COMPAT(UNKNOWN, "<none>"),
};

const char *fdtdec_get_compatible(enum fdt_compat_id id)
{
	/* We allow reading of the 'unknown' ID for testing purposes */
	assert(id >= 0 && id < COMPAT_COUNT);
	return compat_names[id];
}

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

int fdtdec_get_is_enabled(const void *blob, int node)
{
	const char *cell;

	/*
	 * It should say "okay", so only allow that. Some fdts use "ok" but
	 * this is a bug. Please fix your device tree source file. See here
	 * for discussion:
	 *
	 * http://www.mail-archive.com/u-boot@lists.denx.de/msg71598.html
	 */
	cell = fdt_getprop(blob, node, "status", NULL);
	if (cell)
		return 0 == strcmp(cell, "okay");
	return 1;
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
	node = find_alias_node(blob, str);
	if (node < 0)
		return node;
	err = fdt_node_check_compatible(blob, node, compat_names[id]);
	if (err < 0)
		return err;
	if (err)
		return -FDT_ERR_NOTFOUND;
	(*upto)++;
	return node;
}

/* TODO: Can we tighten this code up a little? */
int fdtdec_find_aliases_for_id(const void *blob, const char *name,
			enum fdt_compat_id id, int *node_list, int maxcount)
{
	int name_len = strlen(name);
	int nodes[maxcount];
	int num_found = 0;
	int offset, node;
	int alias_node;
	int count;
	int i, j;

	/* find the alias node if present */
	alias_node = fdt_path_offset(blob, "/aliases");

	/*
	 * start with nothing, and we can assume that the root node can't
	 * match
	 */
	memset(nodes, '\0', sizeof(nodes));

	/* First find all the compatible nodes */
	for (node = count = 0; node >= 0 && count < maxcount;) {
		node = fdtdec_next_compatible(blob, node, id);
		if (node >= 0)
			nodes[count++] = node;
	}
	if (node >= 0)
		debug("%s: warning: maxcount exceeded with alias '%s'\n",
		       __func__, name);

	/* Now find all the aliases */
	memset(node_list, '\0', sizeof(*node_list) * maxcount);

	for (offset = fdt_first_property_offset(blob, alias_node);
			offset > 0;
			offset = fdt_next_property_offset(blob, offset)) {
		const struct fdt_property *prop;
		const char *path;
		int number;
		int found;

		node = 0;
		prop = fdt_get_property_by_offset(blob, offset, NULL);
		path = fdt_string(blob, fdt32_to_cpu(prop->nameoff));
		if (prop->len && 0 == strncmp(path, name, name_len))
			node = fdt_path_offset(blob, prop->data);
		if (node <= 0)
			continue;

		/* Get the alias number */
		number = simple_strtoul(path + name_len, NULL, 10);
		if (number < 0 || number >= maxcount) {
			debug("%s: warning: alias '%s' is out of range\n",
			       __func__, path);
			continue;
		}

		/* Make sure the node we found is actually in our list! */
		found = -1;
		for (j = 0; j < count; j++)
			if (nodes[j] == node) {
				found = j;
				break;
			}

		if (found == -1) {
			debug("%s: warning: alias '%s' points to a node "
				"'%s' that is missing or is not compatible "
				" with '%s'\n", __func__, path,
				fdt_get_name(blob, node, NULL),
			       compat_names[id]);
			continue;
		}

		/*
		 * Add this node to our list in the right place, and mark
		 * it as done.
		 */
		if (fdtdec_get_is_enabled(blob, node)) {
			node_list[number] = node;
			if (number >= num_found)
				num_found = number + 1;
		}
		nodes[j] = 0;
	}

	/* Add any nodes not mentioned by an alias */
	for (i = j = 0; i < maxcount; i++) {
		if (!node_list[i]) {
			for (; j < maxcount; j++)
				if (nodes[j] &&
					fdtdec_get_is_enabled(blob, nodes[j]))
					break;

			/* Have we run out of nodes to add? */
			if (j == maxcount)
				break;

			assert(!node_list[i]);
			node_list[i] = nodes[j++];
			if (i >= num_found)
				num_found = i + 1;
		}
	}

	return num_found;
}

/*
 * This function is a little odd in that it accesses global data. At some
 * point if the architecture board.c files merge this will make more sense.
 * Even now, it is common code.
 */
int fdtdec_check_fdt(void)
{
	/* We must have an fdt */
	if (((uintptr_t)gd->fdt_blob & 3) || fdt_check_header(gd->fdt_blob))
		panic("No valid fdt found - please append one to U-Boot\n"
			"binary or define CONFIG_OF_EMBED\n");
	return 0;
}
