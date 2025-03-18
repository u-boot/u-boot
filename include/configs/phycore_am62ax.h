/* SPDX-License-Identifier: GPL-2.0-or-later OR MIT */
/*
 * Configuration header file for PHYTEC phyCORE-AM62Ax
 *
 * Copyright (C) 2024 PHYTEC America LLC
 * Author: Garrett Giordano <ggiordano@phytec.com>
 */

#ifndef __PHYCORE_AM62AX_H
#define __PHYCORE_AM62AX_H

/* DDR Configuration */
#define CFG_SYS_SDRAM_BASE		0x80000000

#define PHYCORE_AM6XX_FW_NAME_TIBOOT3	u"PHYCORE_AM62AX_TIBOOT3"
#define PHYCORE_AM6XX_FW_NAME_SPL	u"PHYCORE_AM62AX_SPL"
#define PHYCORE_AM6XX_FW_NAME_UBOOT	u"PHYCORE_AM62AX_UBOOT"

#endif /* __PHYCORE_AM62AX_H */
