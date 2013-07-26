#ifndef _LIBFDT_INTERNAL_H
#define _LIBFDT_INTERNAL_H
/*
 * libfdt - Flat Device Tree manipulation
 * Copyright (C) 2006 David Gibson, IBM Corporation.
 * SPDX-License-Identifier:	GPL-2.0+ BSD-2-Clause
 */
#include <fdt.h>

#define FDT_ALIGN(x, a)		(((x) + (a) - 1) & ~((a) - 1))
#define FDT_TAGALIGN(x)		(FDT_ALIGN((x), FDT_TAGSIZE))

#define FDT_CHECK_HEADER(fdt) \
	{ \
		int err; \
		if ((err = fdt_check_header(fdt)) != 0) \
			return err; \
	}

int _fdt_check_node_offset(const void *fdt, int offset);
int _fdt_check_prop_offset(const void *fdt, int offset);
const char *_fdt_find_string(const char *strtab, int tabsize, const char *s);
int _fdt_node_end_offset(void *fdt, int nodeoffset);

static inline const void *_fdt_offset_ptr(const void *fdt, int offset)
{
	return (const char *)fdt + fdt_off_dt_struct(fdt) + offset;
}

static inline void *_fdt_offset_ptr_w(void *fdt, int offset)
{
	return (void *)(uintptr_t)_fdt_offset_ptr(fdt, offset);
}

static inline const struct fdt_reserve_entry *_fdt_mem_rsv(const void *fdt, int n)
{
	const struct fdt_reserve_entry *rsv_table =
		(const struct fdt_reserve_entry *)
		((const char *)fdt + fdt_off_mem_rsvmap(fdt));

	return rsv_table + n;
}
static inline struct fdt_reserve_entry *_fdt_mem_rsv_w(void *fdt, int n)
{
	return (void *)(uintptr_t)_fdt_mem_rsv(fdt, n);
}

#define FDT_SW_MAGIC		(~FDT_MAGIC)

#endif /* _LIBFDT_INTERNAL_H */
