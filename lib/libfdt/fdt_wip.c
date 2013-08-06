/*
 * libfdt - Flat Device Tree manipulation
 * Copyright (C) 2006 David Gibson, IBM Corporation.
 *
 * libfdt is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 *
 *  a) This library is free software; you can redistribute it and/or
 *     modify it under the terms of the GNU General Public License as
 *     published by the Free Software Foundation; either version 2 of the
 *     License, or (at your option) any later version.
 *
 *     This library is distributed in the hope that it will be useful,
 *     but WITHOUT ANY WARRANTY; without even the implied warranty of
 *     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *     GNU General Public License for more details.
 *
 *     You should have received a copy of the GNU General Public
 *     License along with this library; if not, write to the Free
 *     Software Foundation, Inc., 51 Franklin St, Fifth Floor, Boston,
 *     MA 02110-1301 USA
 *
 * Alternatively,
 *
 *  b) Redistribution and use in source and binary forms, with or
 *     without modification, are permitted provided that the following
 *     conditions are met:
 *
 *     1. Redistributions of source code must retain the above
 *        copyright notice, this list of conditions and the following
 *        disclaimer.
 *     2. Redistributions in binary form must reproduce the above
 *        copyright notice, this list of conditions and the following
 *        disclaimer in the documentation and/or other materials
 *        provided with the distribution.
 *
 *     THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND
 *     CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
 *     INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 *     MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 *     DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 *     CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 *     SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 *     NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 *     LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 *     HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 *     CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 *     OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 *     EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
#include "libfdt_env.h"

#ifndef USE_HOSTCC
#include <fdt.h>
#include <libfdt.h>
#else
#include "fdt_host.h"
#endif

#include "libfdt_internal.h"

int fdt_setprop_inplace(void *fdt, int nodeoffset, const char *name,
			const void *val, int len)
{
	void *propval;
	int proplen;

	propval = fdt_getprop_w(fdt, nodeoffset, name, &proplen);
	if (! propval)
		return proplen;

	if (proplen != len)
		return -FDT_ERR_NOSPACE;

	memcpy(propval, val, len);
	return 0;
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
	if (! prop)
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

#define FDT_MAX_DEPTH	32

static int str_in_list(const char *str, char * const list[], int count)
{
	int i;

	for (i = 0; i < count; i++)
		if (!strcmp(list[i], str))
			return 1;

	return 0;
}

int fdt_find_regions(const void *fdt, char * const inc[], int inc_count,
		     char * const exc_prop[], int exc_prop_count,
		     struct fdt_region region[], int max_regions,
		     char *path, int path_len, int add_string_tab)
{
	int stack[FDT_MAX_DEPTH];
	char *end;
	int nextoffset = 0;
	uint32_t tag;
	int count = 0;
	int start = -1;
	int depth = -1;
	int want = 0;
	int base = fdt_off_dt_struct(fdt);

	end = path;
	*end = '\0';
	do {
		const struct fdt_property *prop;
		const char *name;
		const char *str;
		int include = 0;
		int stop_at = 0;
		int offset;
		int len;

		offset = nextoffset;
		tag = fdt_next_tag(fdt, offset, &nextoffset);
		stop_at = nextoffset;

		switch (tag) {
		case FDT_PROP:
			include = want >= 2;
			stop_at = offset;
			prop = fdt_get_property_by_offset(fdt, offset, NULL);
			str = fdt_string(fdt, fdt32_to_cpu(prop->nameoff));
			if (str_in_list(str, exc_prop, exc_prop_count))
				include = 0;
			break;

		case FDT_NOP:
			include = want >= 2;
			stop_at = offset;
			break;

		case FDT_BEGIN_NODE:
			depth++;
			if (depth == FDT_MAX_DEPTH)
				return -FDT_ERR_BADSTRUCTURE;
			name = fdt_get_name(fdt, offset, &len);
			if (end - path + 2 + len >= path_len)
				return -FDT_ERR_NOSPACE;
			if (end != path + 1)
				*end++ = '/';
			strcpy(end, name);
			end += len;
			stack[depth] = want;
			if (want == 1)
				stop_at = offset;
			if (str_in_list(path, inc, inc_count))
				want = 2;
			else if (want)
				want--;
			else
				stop_at = offset;
			include = want;
			break;

		case FDT_END_NODE:
			include = want;
			want = stack[depth--];
			while (end > path && *--end != '/')
				;
			*end = '\0';
			break;

		case FDT_END:
			include = 1;
			break;
		}

		if (include && start == -1) {
			/* Should we merge with previous? */
			if (count && count <= max_regions &&
			    offset == region[count - 1].offset +
					region[count - 1].size - base)
				start = region[--count].offset - base;
			else
				start = offset;
		}

		if (!include && start != -1) {
			if (count < max_regions) {
				region[count].offset = base + start;
				region[count].size = stop_at - start;
			}
			count++;
			start = -1;
		}
	} while (tag != FDT_END);

	if (nextoffset != fdt_size_dt_struct(fdt))
		return -FDT_ERR_BADLAYOUT;

	/* Add a region for the END tag and the string table */
	if (count < max_regions) {
		region[count].offset = base + start;
		region[count].size = nextoffset - start;
		if (add_string_tab)
			region[count].size += fdt_size_dt_strings(fdt);
	}
	count++;

	return count;
}
