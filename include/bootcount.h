/*
 * (C) Copyright 2012
 * Stefan Roese, DENX Software Engineering, sr@denx.de.
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include <common.h>
#include <asm/io.h>
#include <asm/byteorder.h>

#if !defined(CONFIG_SYS_BOOTCOUNT_LE) && !defined(CONFIG_SYS_BOOTCOUNT_BE)
# if __BYTE_ORDER == __LITTLE_ENDIAN
#  define CONFIG_SYS_BOOTCOUNT_LE
# else
#  define CONFIG_SYS_BOOTCOUNT_BE
# endif
#endif

#ifdef CONFIG_SYS_BOOTCOUNT_LE
static inline void raw_bootcount_store(volatile u32 *addr, u32 data)
{
	out_le32(addr, data);
}

static inline u32 raw_bootcount_load(volatile u32 *addr)
{
	return in_le32(addr);
}
#else
static inline void raw_bootcount_store(volatile u32 *addr, u32 data)
{
	out_be32(addr, data);
}

static inline u32 raw_bootcount_load(volatile u32 *addr)
{
	return in_be32(addr);
}
#endif
