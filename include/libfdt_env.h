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

#include <stddef.h>
#include <linux/types.h>
#include <asm/byteorder.h>
#ifdef USE_HOSTCC
#include <string.h>
#else
#include <linux/string.h>
#endif /* USE_HOSTCC */

extern struct fdt_header *fdt;  /* Pointer to the working fdt */

#define fdt32_to_cpu(x)		__be32_to_cpu(x)
#define cpu_to_fdt32(x)		__cpu_to_be32(x)
#define fdt64_to_cpu(x)		__be64_to_cpu(x)
#define cpu_to_fdt64(x)		__cpu_to_be64(x)

#endif /* _LIBFDT_ENV_H */
