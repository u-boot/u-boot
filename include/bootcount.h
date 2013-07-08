/*
 * (C) Copyright 2012
 * Stefan Roese, DENX Software Engineering, sr@denx.de.
 *
 * SPDX-License-Identifier:	GPL-2.0+
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
