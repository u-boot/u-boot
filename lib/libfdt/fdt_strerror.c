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

struct fdt_errtabent {
	const char *str;
};

#define FDT_ERRTABENT(val) \
	[(val)] = { .str = #val, }

static struct fdt_errtabent fdt_errtable[] = {
	FDT_ERRTABENT(FDT_ERR_NOTFOUND),
	FDT_ERRTABENT(FDT_ERR_EXISTS),
	FDT_ERRTABENT(FDT_ERR_NOSPACE),

	FDT_ERRTABENT(FDT_ERR_BADOFFSET),
	FDT_ERRTABENT(FDT_ERR_BADPATH),
	FDT_ERRTABENT(FDT_ERR_BADSTATE),

	FDT_ERRTABENT(FDT_ERR_TRUNCATED),
	FDT_ERRTABENT(FDT_ERR_BADMAGIC),
	FDT_ERRTABENT(FDT_ERR_BADVERSION),
	FDT_ERRTABENT(FDT_ERR_BADSTRUCTURE),
	FDT_ERRTABENT(FDT_ERR_BADLAYOUT),
};
#define FDT_ERRTABSIZE	(sizeof(fdt_errtable) / sizeof(fdt_errtable[0]))

const char *fdt_strerror(int errval)
{
	if (errval > 0)
		return "<valid offset/length>";
	else if (errval == 0)
		return "<no error>";
	else if (errval > -FDT_ERRTABSIZE) {
		const char *s = fdt_errtable[-errval].str;

		if (s)
			return s;
	}

	return "<unknown error>";
}
