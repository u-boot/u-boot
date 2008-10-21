/*
 * libfdt - Flat Device Tree manipulation (build/run environment adaptation)
 * Copyright (C) 2007 Gerald Van Baren, Custom IDEAS, vanbaren@cideas.com
 * Original version written by David Gibson, IBM Corporation.
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

#ifndef _LIBFDT_ENV_H
#define _LIBFDT_ENV_H

#ifdef USE_HOSTCC
#include <stdint.h>
#include <string.h>
#include <endian.h>
#include <byteswap.h>
#else
#include <linux/string.h>
#include <linux/types.h>
#include <asm/byteorder.h>
#endif /* USE_HOSTCC */

#include <stddef.h>
extern struct fdt_header *working_fdt;  /* Pointer to the working fdt */

#if __BYTE_ORDER == __LITTLE_ENDIAN
#define fdt32_to_cpu(x)		bswap_32(x)
#define cpu_to_fdt32(x)		bswap_32(x)
#define fdt64_to_cpu(x)		bswap_64(x)
#define cpu_to_fdt64(x)		bswap_64(x)
#else
#define fdt32_to_cpu(x)		(x)
#define cpu_to_fdt32(x)		(x)
#define fdt64_to_cpu(x)		(x)
#define cpu_to_fdt64(x)		(x)
#endif

/*
 * Types for `void *' pointers.
 *
 * Note: libfdt uses this definition from /usr/include/stdint.h.
 * Define it here rather than pulling in all of stdint.h.
 */
#if __WORDSIZE == 64
typedef unsigned long int       uintptr_t;
#else
typedef unsigned int            uintptr_t;
#endif

#endif /* _LIBFDT_ENV_H */
