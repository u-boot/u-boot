/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Configuration header file for BeagleBoneAI64
 *
 * https://beagleboard.org/ai-64
 *
 * Copyright (C) 2024 Texas Instruments Incorporated - https://www.ti.com/
 */

#ifndef __CONFIG_BEAGLEBONEAI64_H
#define __CONFIG_BEAGLEBONEAI64_H

/* FLASH Configuration */
#define CFG_SYS_FLASH_BASE		0x000000000

/* SPL Loader Configuration */
#define CFG_SYS_UBOOT_BASE		0x50080000

/**
 * define BEAGLEBONEAI64_TIBOOT3_IMAGE_GUID - firmware GUID for BeagleBoneAI64
 *                                            tiboot3.bin
 * define BEAGLEBONEAI64_SPL_IMAGE_GUID     - firmware GUID for BeagleBoneAI64
 *                                            SPL
 * define BEAGLEBONEAI64_UBOOT_IMAGE_GUID   - firmware GUID for BeagleBoneAI64
 *                                            UBOOT
 * define BEAGLEBONEAI64_SYSFW_IMAGE_GUID   - firmware GUID for BeagleBoneAI64
 *                                            SYSFW
 *
 * These GUIDs are used in capsules updates to identify the corresponding
 * firmware object.
 *
 * Board developers using this as a starting reference should
 * define their own GUIDs to ensure that firmware repositories (like
 * LVFS) do not confuse them.
 */
#define BEAGLEBONEAI64_TIBOOT3_IMAGE_GUID \
	EFI_GUID(0x772a4810, 0x2194, 0x4923, 0x87, 0x54, \
		0x01, 0x15, 0x87, 0x0e, 0xf3, 0x67)

#define BEAGLEBONEAI64_SPL_IMAGE_GUID \
	EFI_GUID(0x83447222, 0x1e26, 0x40cd, 0xa3, 0x95, \
		0xb7, 0xde, 0x09, 0x57, 0xe8, 0x75)

#define BEAGLEBONEAI64_UBOOT_IMAGE_GUID \
	EFI_GUID(0x4249ff77, 0xc17d, 0x4eb7, 0xa1, 0xdb, \
		0x45, 0xaa, 0x98, 0x87, 0xd4, 0x9e)

#define BEAGLEBONEAI64_SYSFW_IMAGE_GUID \
	EFI_GUID(0xdfc9c683, 0x49b7, 0x46bd, 0xb3, 0xc1, \
		0x3a, 0x3b, 0x2f, 0xdb, 0x13, 0x5b)

/* Now for the remaining common defines */
#include <configs/ti_armv7_common.h>

#endif /* __CONFIG_BEAGLEBONEAI64_H */
