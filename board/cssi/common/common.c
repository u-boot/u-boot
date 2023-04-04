// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2010-2020 CS Group
 * Charles Frey <charles.frey@c-s.fr>
 * Florent Trinh Thai <florent.trinh-thai@c-s.fr>
 * Christophe Leroy <christophe.leroy@c-s.fr>
 *
 * Common specific routines for the CS Group boards
 */

#include <fdt_support.h>

static int fdt_set_node_and_value(void *blob, char *node, const char *prop,
				  void *var, int size)
{
	int ret, off;

	off = fdt_path_offset(blob, node);

	if (off < 0) {
		printf("Cannot find %s node err:%s\n", node, fdt_strerror(off));

		return off;
	}

	ret = fdt_setprop(blob, off, prop, var, size);

	if (ret < 0)
		printf("Cannot set %s/%s prop err: %s\n", node, prop, fdt_strerror(ret));

	return ret;
}

/* Checks front/rear id and remove unneeded nodes from the blob */
void ft_cleanup(void *blob, unsigned long id, const char *prop, const char *compatible)
{
	int off;

	off = fdt_node_offset_by_compatible(blob, -1, compatible);

	while (off != -FDT_ERR_NOTFOUND) {
		const struct fdt_property *ids;
		int nb_ids, idx;
		int tmp = -1;

		ids = fdt_get_property(blob, off, prop, &nb_ids);

		for (idx = 0; idx < nb_ids; idx += 4) {
			if (*((uint32_t *)&ids->data[idx]) == id)
				break;
		}

		if (idx >= nb_ids)
			fdt_del_node(blob, off);
		else
			tmp = off;

		off = fdt_node_offset_by_compatible(blob, tmp, compatible);
	}

	fdt_set_node_and_value(blob, "/", prop, &id, sizeof(uint32_t));
}
