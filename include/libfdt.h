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

#ifndef _LIBFDT_H
#define _LIBFDT_H

#include <fdt.h>
#include <libfdt_env.h>

#define FDT_FIRST_SUPPORTED_VERSION	0x10
#define FDT_LAST_SUPPORTED_VERSION	0x11

/* Error codes: informative error codes */
#define FDT_ERR_NOTFOUND	1
#define FDT_ERR_EXISTS		2
#define FDT_ERR_NOSPACE		3

/* Error codes: codes for bad parameters */
#define FDT_ERR_BADOFFSET	4
#define FDT_ERR_BADPATH		5
#define FDT_ERR_BADSTATE	6

/* Error codes: codes for bad device tree blobs */
#define FDT_ERR_TRUNCATED	7
#define FDT_ERR_BADMAGIC	8
#define FDT_ERR_BADVERSION	9
#define FDT_ERR_BADSTRUCTURE	10
#define FDT_ERR_BADLAYOUT	11

#define FDT_ERR_MAX		11

#define fdt_get_header(fdt, field) \
	(fdt32_to_cpu(((struct fdt_header *)(fdt))->field))
#define fdt_magic(fdt)			(fdt_get_header(fdt, magic))
#define fdt_totalsize(fdt)		(fdt_get_header(fdt, totalsize))
#define fdt_off_dt_struct(fdt)		(fdt_get_header(fdt, off_dt_struct))
#define fdt_off_dt_strings(fdt)		(fdt_get_header(fdt, off_dt_strings))
#define fdt_off_mem_rsvmap(fdt)		(fdt_get_header(fdt, off_mem_rsvmap))
#define fdt_version(fdt)		(fdt_get_header(fdt, version))
#define fdt_last_comp_version(fdt)	(fdt_get_header(fdt, last_comp_version))
#define fdt_boot_cpuid_phys(fdt)	(fdt_get_header(fdt, boot_cpuid_phys))
#define fdt_size_dt_strings(fdt)	(fdt_get_header(fdt, size_dt_strings))
#define fdt_size_dt_struct(fdt)		(fdt_get_header(fdt, size_dt_struct))

#define fdt_set_header(fdt, field, val) \
	((struct fdt_header *)(fdt))->field = cpu_to_fdt32(val)

int fdt_check_header(const void *fdt);

void *fdt_offset_ptr(const void *fdt, int offset, int checklen);

#define fdt_offset_ptr_typed(fdt, offset, var) \
	((typeof(var))(fdt_offset_ptr((fdt), (offset), sizeof(*(var)))))

int fdt_move(const void *fdt, void *buf, int bufsize);

/* Read-only functions */
char *fdt_string(const void *fdt, int stroffset);

int fdt_subnode_offset_namelen(const void *fdt, int parentoffset,
			       const char *name, int namelen);
int fdt_subnode_offset(const void *fdt, int parentoffset, const char *name);

int fdt_path_offset(const void *fdt, const char *path);

struct fdt_property *fdt_get_property(const void *fdt, int nodeoffset,
				      const char *name, int *lenp);
void *fdt_getprop(const void *fdt, int nodeoffset,
		  const char *name, int *lenp);

uint32_t fdt_next_tag(const void *fdt, int offset,
		      int *nextoffset, char **namep);
int fdt_num_reservemap(void *fdt, int *used, int *total);
int fdt_get_reservemap(void *fdt, int n, struct fdt_reserve_entry *re);

/* Write-in-place functions */
int fdt_setprop_inplace(void *fdt, int nodeoffset, const char *name,
			const void *val, int len);

#define fdt_setprop_inplace_typed(fdt, nodeoffset, name, val) \
	({ \
		typeof(val) x = val; \
		fdt_setprop_inplace(fdt, nodeoffset, name, &x, sizeof(x)); \
	})

int fdt_nop_property(void *fdt, int nodeoffset, const char *name);
int fdt_nop_node(void *fdt, int nodeoffset);
int fdt_insert_reservemap_entry(void *fdt, int n, uint64_t addr, uint64_t size);


/* Sequential-write functions */
int fdt_create(void *buf, int bufsize);
int fdt_add_reservemap_entry(void *fdt, uint64_t addr, uint64_t size);
int fdt_finish_reservemap(void *fdt);
int fdt_begin_node(void *fdt, const char *name);
int fdt_property(void *fdt, const char *name, const void *val, int len);
#define fdt_property_typed(fdt, name, val) \
	({ \
		typeof(val) x = (val); \
		fdt_property((fdt), (name), &x, sizeof(x)); \
	})
#define fdt_property_string(fdt, name, str) \
	fdt_property(fdt, name, str, strlen(str)+1)
int fdt_end_node(void *fdt);
int fdt_finish(void *fdt);
int fdt_replace_reservemap_entry(void *fdt, int n, uint64_t addr, uint64_t size);

/* Read-write functions */
int fdt_open_into(void *fdt, void *buf, int bufsize);
int fdt_pack(void *fdt);

int fdt_setprop(void *fdt, int nodeoffset, const char *name,
		const void *val, int len);
#define fdt_setprop_typed(fdt, nodeoffset, name, val) \
	({ \
		typeof(val) x = (val); \
		fdt_setprop((fdt), (nodeoffset), (name), &x, sizeof(x)); \
	})
#define fdt_setprop_string(fdt, nodeoffset, name, str) \
	fdt_setprop((fdt), (nodeoffset), (name), (str), strlen(str)+1)
int fdt_delprop(void *fdt, int nodeoffset, const char *name);
int fdt_add_subnode_namelen(void *fdt, int parentoffset,
			    const char *name, int namelen);
int fdt_add_subnode(void *fdt, int parentoffset, const char *name);
int fdt_del_node(void *fdt, int nodeoffset);

/* Extra functions */
const char *fdt_strerror(int errval);

#endif /* _LIBFDT_H */
