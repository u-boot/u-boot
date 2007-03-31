/*
 * libfdt - Flat Device Tree manipulation
 * Copyright (C) 2006 David Gibson, IBM Corporation.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * as published by the Free Software Foundation; either version 2.1 of
 * the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA
 */
#include "libfdt_env.h"

#include <fdt.h>
#include <libfdt.h>

#include "libfdt_internal.h"

int fdt_setprop_inplace(void *fdt, int nodeoffset, const char *name,
			const void *val, int len)
{
	void *propval;
	int proplen;

	propval = fdt_getprop(fdt, nodeoffset, name, &proplen);
	if (! propval)
		return proplen;

	if (proplen != len)
		return -FDT_ERR_NOSPACE;

	memcpy(propval, val, len);
	return 0;
}

static void nop_region(void *start, int len)
{
	uint32_t *p;

	for (p = start; (void *)p < (start + len); p++)
		*p = cpu_to_fdt32(FDT_NOP);
}

int fdt_nop_property(void *fdt, int nodeoffset, const char *name)
{
	struct fdt_property *prop;
	int len;

	prop = fdt_get_property(fdt, nodeoffset, name, &len);
	if (! prop)
		return len;

	nop_region(prop, len + sizeof(*prop));

	return 0;
}

int _fdt_node_end_offset(void *fdt, int nodeoffset)
{
	int level = 0;
	uint32_t tag;
	int offset, nextoffset;

	tag = _fdt_next_tag(fdt, nodeoffset, &nextoffset);
	if (tag != FDT_BEGIN_NODE)
		return -FDT_ERR_BADOFFSET;
	do {
		offset = nextoffset;
		tag = _fdt_next_tag(fdt, offset, &nextoffset);

		switch (tag) {
		case FDT_END:
			return offset;

		case FDT_BEGIN_NODE:
			level++;
			break;

		case FDT_END_NODE:
			level--;
			break;

		case FDT_PROP:
		case FDT_NOP:
			break;

		default:
			return -FDT_ERR_BADSTRUCTURE;
		}
	} while (level >= 0);

	return nextoffset;
}

int fdt_nop_node(void *fdt, int nodeoffset)
{
	int endoffset;

	endoffset = _fdt_node_end_offset(fdt, nodeoffset);
	if (endoffset < 0)
		return endoffset;

	nop_region(fdt_offset_ptr(fdt, nodeoffset, 0), endoffset - nodeoffset);
	return 0;
}
