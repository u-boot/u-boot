/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * This file is Geist board configuration.
 *
 * Copyright (C) 2025-2026 Renesas Electronics Corporation
 */

#ifndef __GEIST_H
#define __GEIST_H

#include "rcar-gen3-common.h"

/* Environment in eMMC, at the end of 2nd "boot sector" */

#define CFG_SYS_FLASH_BANKS_LIST	{ 0x08000000 }
#define CFG_SYS_WRITE_SWAPPED_DATA

#endif /* __GEIST_H */
