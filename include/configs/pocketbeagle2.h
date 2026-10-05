/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Configuration header file for PocketBeagle 2
 *
 * Copyright (C) 2025 Texas Instruments Incorporated - https://www.ti.com/
 */

#ifndef __CONFIG_POCKETBEAGLE2_H
#define __CONFIG_POCKETBEAGLE2_H

/**
 * define POCKETBEAGLE2_TIBOOT3_IMAGE_GUID - firmware GUID for PocketBeagle 2
 *                                           tiboot3.bin
 * define POCKETBEAGLE2_SPL_IMAGE_GUID     - firmware GUID for PocketBeagle 2 SPL
 * define POCKETBEAGLE2_UBOOT_IMAGE_GUID   - firmware GUID for PocketBeagle 2 UBOOT
 *
 * These GUIDs are used in capsules updates to identify the corresponding
 * firmware object.
 *
 * Board developers using this as a starting reference should
 * define their own GUIDs to ensure that firmware repositories (like
 * LVFS) do not confuse them.
 */
#define POCKETBEAGLE2_TIBOOT3_IMAGE_GUID                                   \
	EFI_GUID(0xF0D53723, 0x106E, 0x5346, 0x8A, 0x41, 0x85, 0xEB, 0x2D, \
		 0x07, 0xC7, 0x20)

#define POCKETBEAGLE2_SPL_IMAGE_GUID                                       \
	EFI_GUID(0xC73FDEA2, 0x3964, 0x58C8, 0x8A, 0x25, 0xE5, 0x51, 0x02, \
		 0x17, 0x2E, 0x1D)

#define POCKETBEAGLE2_UBOOT_IMAGE_GUID                                     \
	EFI_GUID(0x520E1012, 0xDE6C, 0x5992, 0xB4, 0x3A, 0x95, 0xD5, 0x3D, \
		 0x83, 0x32, 0xF2)

/* Now for the remaining common defines */
#include <configs/ti_armv7_common.h>

#endif /* __CONFIG_POCKETBEAGLE2_H */
