/*
 * Copyright (C) 2006 Atmel Corporation
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */
#ifndef __ASM_AVR32_BYTEORDER_H
#define __ASM_AVR32_BYTEORDER_H

#include <asm/types.h>

#define __arch__swab32(x) __builtin_bswap_32(x)
#define __arch__swab16(x) __builtin_bswap_16(x)

#if !defined(__STRICT_ANSI__) || defined(__KERNEL__)
# define __BYTEORDER_HAS_U64__
# define __SWAB_64_THRU_32__
#endif

#include <linux/byteorder/big_endian.h>

#endif /* __ASM_AVR32_BYTEORDER_H */
