/*
 * Copyright (C) 2006 Atmel Corporation
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
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
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
