/*
 * (C) Copyright 2011, Stefan Kristiansson <stefan.kristiansson@saunalahti.fi>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __ASM_OPENRISC_BITOPS_H
#define __ASM_OPENRISC_BITOPS_H

#define PLATFORM_FLS
#include <asm/bitops/fls.h>
#define PLATFORM_FFS
#include <asm/bitops/ffs.h>

#include <asm-generic/bitops/__fls.h>
#include <asm-generic/bitops/fls64.h>
#include <asm-generic/bitops/__ffs.h>

#define hweight32(x) generic_hweight32(x)
#define hweight16(x) generic_hweight16(x)
#define hweight8(x) generic_hweight8(x)

#endif /* __ASM_GENERIC_BITOPS_H */
