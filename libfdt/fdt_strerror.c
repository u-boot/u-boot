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

struct errtabent {
	const char *str;
};

#define ERRTABENT(val) \
	[(val)] = { .str = #val, }

static struct errtabent errtable[] = {
	ERRTABENT(FDT_ERR_NOTFOUND),
	ERRTABENT(FDT_ERR_EXISTS),
	ERRTABENT(FDT_ERR_NOSPACE),

	ERRTABENT(FDT_ERR_BADOFFSET),
	ERRTABENT(FDT_ERR_BADPATH),
	ERRTABENT(FDT_ERR_BADSTATE),

	ERRTABENT(FDT_ERR_TRUNCATED),
	ERRTABENT(FDT_ERR_BADMAGIC),
	ERRTABENT(FDT_ERR_BADVERSION),
	ERRTABENT(FDT_ERR_BADSTRUCTURE),
	ERRTABENT(FDT_ERR_BADLAYOUT),
};
#define ERRTABSIZE	(sizeof(errtable) / sizeof(errtable[0]))

const char *fdt_strerror(int errval)
{
	if (errval > 0)
		return "<valid offset/length>";
	else if (errval == 0)
		return "<no error>";
	else if (errval > -ERRTABSIZE) {
		const char *s = errtable[-errval].str;

		if (s)
			return s;
	}

	return "<unknown error>";
}
