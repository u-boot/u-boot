/* SPDX-License-Identifier: GPL-2.0-or-later OR MIT */
/*
 * Configuration header file for PHYTEC phyCORE-AM68x
 *
 */

#ifndef __PHYCORE_AM68X_H
#define __PHYCORE_AM68X_H

/* DDR Configuration */
#define CFG_SYS_SDRAM_BASE		0x80000000

/* SPL Loader Configuration */
#if defined(CONFIG_TARGET_PHYCORE_AM68X_A72)
#define CFG_SYS_UBOOT_BASE              0x50280000
/* Image load address in RAM for DFU boot*/
#else
#define CFG_SYS_UBOOT_BASE              0x50080000
#endif

#define PHYCORE_AM6XX_FW_NAME_TIBOOT3	u"PHYCORE_AM68X_TIBOOT3"
#define PHYCORE_AM6XX_FW_NAME_SPL	u"PHYCORE_AM68X_SPL"
#define PHYCORE_AM6XX_FW_NAME_UBOOT	u"PHYCORE_AM68X_UBOOT"

#endif /* __PHYCORE_AM62AX_H */
