// SPDX-License-Identifier: GPL-2.0+
/*
 * NEORV32 CLINT definitions
 */

#ifndef __NEORV32_CLINT_H
#define __NEORV32_CLINT_H

#include <linux/types.h>

#include <neorv32/neorv32.h>

/* CLINT register offsets */
#define NEORV32_CLINT_MSWI_OFFSET     0x0000
#define NEORV32_CLINT_MTIMECMP_OFFSET 0x4000
#define NEORV32_CLINT_MTIME_OFFSET    0xBFF8

#endif /* __NEORV32_CLINT_H */
