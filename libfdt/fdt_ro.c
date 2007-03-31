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

#define CHECK_HEADER(fdt) \
	{ \
		int err; \
		if ((err = _fdt_check_header(fdt)) != 0) \
			return err; \
	}

static int offset_streq(const void *fdt, int offset,
			const char *s, int len)
{
	const char *p = fdt_offset_ptr(fdt, offset, len+1);

	if (! p)
		/* short match */
		return 0;

	if (memcmp(p, s, len) != 0)
		return 0;

	if (p[len] != '\0')
		return 0;

	return 1;
}

char *fdt_string(const void *fdt, int stroffset)
{
	return (char *)fdt + fdt_off_dt_strings(fdt) + stroffset;
}

int fdt_subnode_offset_namelen(const void *fdt, int parentoffset,
			       const char *name, int namelen)
{
	int level = 0;
	uint32_t tag;
	int offset, nextoffset;

	CHECK_HEADER(fdt);

	tag = _fdt_next_tag(fdt, parentoffset, &nextoffset);
	if (tag != FDT_BEGIN_NODE)
		return -FDT_ERR_BADOFFSET;

	do {
		offset = nextoffset;
		tag = _fdt_next_tag(fdt, offset, &nextoffset);

		switch (tag) {
		case FDT_END:
			return -FDT_ERR_TRUNCATED;

		case FDT_BEGIN_NODE:
			level++;
			if (level != 1)
				continue;
			if (offset_streq(fdt, offset+FDT_TAGSIZE, name, namelen))
				/* Found it! */
				return offset;
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

	return -FDT_ERR_NOTFOUND;
}

int fdt_subnode_offset(const void *fdt, int parentoffset,
		       const char *name)
{
	return fdt_subnode_offset_namelen(fdt, parentoffset, name, strlen(name));
}

int fdt_path_offset(const void *fdt, const char *path)
{
	const char *end = path + strlen(path);
	const char *p = path;
	int offset = 0;

	CHECK_HEADER(fdt);

	if (*path != '/')
		return -FDT_ERR_BADPATH;

	while (*p) {
		const char *q;

		while (*p == '/')
			p++;
		if (! *p)
			return -FDT_ERR_BADPATH;
		q = strchr(p, '/');
		if (! q)
			q = end;

		offset = fdt_subnode_offset_namelen(fdt, offset, p, q-p);
		if (offset < 0)
			return offset;

		p = q;
	}

	return offset;	
}

struct fdt_property *fdt_get_property(const void *fdt,
				      int nodeoffset,
				      const char *name, int *lenp)
{
	int level = 0;
	uint32_t tag;
	struct fdt_property *prop;
	int namestroff;
	int offset, nextoffset;
	int err;

	if ((err = _fdt_check_header(fdt)) != 0)
		goto fail;

	err = -FDT_ERR_BADOFFSET;
	if (nodeoffset % FDT_TAGSIZE)
		goto fail;

	tag = _fdt_next_tag(fdt, nodeoffset, &nextoffset);
	if (tag != FDT_BEGIN_NODE)
		goto fail;

	do {
		offset = nextoffset;

		tag = _fdt_next_tag(fdt, offset, &nextoffset);
		switch (tag) {
		case FDT_END:
			err = -FDT_ERR_TRUNCATED;
			goto fail;

		case FDT_BEGIN_NODE:
			level++;
			break;

		case FDT_END_NODE:
			level--;
			break;

		case FDT_PROP:
			if (level != 0)
				continue;

			err = -FDT_ERR_BADSTRUCTURE;
			prop = fdt_offset_ptr_typed(fdt, offset, prop);
			if (! prop)
				goto fail;
			namestroff = fdt32_to_cpu(prop->nameoff);
			if (streq(fdt_string(fdt, namestroff), name)) {
				/* Found it! */
				int len = fdt32_to_cpu(prop->len);
				prop = fdt_offset_ptr(fdt, offset,
						      sizeof(*prop)+len);
				if (! prop)
					goto fail;

				if (lenp)
					*lenp = len;
				
				return prop;
			}
			break;

		case FDT_NOP:
			break;

		default:
			err = -FDT_ERR_BADSTRUCTURE;
			goto fail;
		}
	} while (level >= 0);

	err = -FDT_ERR_NOTFOUND;
 fail:
	if (lenp)
		*lenp = err;
	return NULL;
}

void *fdt_getprop(const void *fdt, int nodeoffset,
		  const char *name, int *lenp)
{
	const struct fdt_property *prop;

	prop = fdt_get_property(fdt, nodeoffset, name, lenp);
	if (! prop)
		return NULL;

	return prop->data;
}
