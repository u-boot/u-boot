/*
 * libfdt - Flat Device Tree manipulation
 * Copyright (C) 2006 David Gibson, IBM Corporation.
 * SPDX-License-Identifier:	GPL-2.0+ BSD-2-Clause
 */
#include <libfdt_env.h>

#ifndef USE_HOSTCC
#include <fdt.h>
#include <libfdt.h>
#else
#include "fdt_host.h"
#endif

#include "libfdt_internal.h"

int fdt_setprop_inplace_namelen_partial(void *fdt, int nodeoffset,
					const char *name, int namelen,
					uint32_t idx, const void *val,
					int len)
{
	void *propval;
	int proplen;

	propval = fdt_getprop_namelen_w(fdt, nodeoffset, name, namelen,
					&proplen);
	if (!propval)
		return proplen;

	if (proplen < (len + idx))
		return -FDT_ERR_NOSPACE;

	memcpy((char *)propval + idx, val, len);
	return 0;
}

int fdt_setprop_inplace(void *fdt, int nodeoffset, const char *name,
			const void *val, int len)
{
	const void *propval;
	int proplen;

	propval = fdt_getprop(fdt, nodeoffset, name, &proplen);
	if (!propval)
		return proplen;

	if (proplen != len)
		return -FDT_ERR_NOSPACE;

	return fdt_setprop_inplace_namelen_partial(fdt, nodeoffset, name,
						   strlen(name), 0,
						   val, len);
}

static void _fdt_nop_region(void *start, int len)
{
	fdt32_t *p;

	for (p = start; (char *)p < ((char *)start + len); p++)
		*p = cpu_to_fdt32(FDT_NOP);
}

int fdt_nop_property(void *fdt, int nodeoffset, const char *name)
{
	struct fdt_property *prop;
	int len;

	prop = fdt_get_property_w(fdt, nodeoffset, name, &len);
	if (!prop)
		return len;

	_fdt_nop_region(prop, len + sizeof(*prop));

	return 0;
}

int _fdt_node_end_offset(void *fdt, int offset)
{
	int depth = 0;

	while ((offset >= 0) && (depth >= 0))
		offset = fdt_next_node(fdt, offset, &depth);

	return offset;
}

int fdt_nop_node(void *fdt, int nodeoffset)
{
	int endoffset;

	endoffset = _fdt_node_end_offset(fdt, nodeoffset);
	if (endoffset < 0)
		return endoffset;

	_fdt_nop_region(fdt_offset_ptr_w(fdt, nodeoffset, 0),
			endoffset - nodeoffset);
	return 0;
}
