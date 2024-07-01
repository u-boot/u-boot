/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Configuration header file for BeaglePlay
 *
 * https://beagleplay.org/
 *
 * Copyright (C) 2024 Texas Instruments Incorporated - https://www.ti.com/
 */

#ifndef __CONFIG_BEAGLEPLAY_H
#define __CONFIG_BEAGLEPLAY_H

/**
 * define BEAGLEPLAY_TIBOOT3_IMAGE_GUID - firmware GUID for BeaglePlay
 *                                        tiboot3.bin
 * define BEAGLEPLAY_SPL_IMAGE_GUID     - firmware GUID for BeaglePlay SPL
 * define BEAGLEPLAY_UBOOT_IMAGE_GUID   - firmware GUID for BeaglePlay UBOOT
 *
 * These GUIDs are used in capsules updates to identify the corresponding
 * firmware object.
 *
 * Board developers using this as a starting reference should
 * define their own GUIDs to ensure that firmware repositories (like
 * LVFS) do not confuse them.
 */
#define BEAGLEPLAY_TIBOOT3_IMAGE_GUID \
	EFI_GUID(0x0e225a09, 0xf720, 0x4d57, 0x91, 0x20, \
		0xe2, 0x8f, 0x73, 0x7f, 0x5a, 0x5e)

#define BEAGLEPLAY_SPL_IMAGE_GUID \
	EFI_GUID(0xb2e7cc49, 0x1a5a, 0x4036, 0xae, 0x01, \
		0x33, 0x87, 0xc3, 0xbe, 0xf6, 0x57)

#define BEAGLEPLAY_UBOOT_IMAGE_GUID \
	EFI_GUID(0x92c92b11, 0xa7ee, 0x486f, 0xaa, 0xa2, \
		0x71, 0x3d, 0x84, 0x42, 0x5b, 0x0e)

/* Now for the remaining common defines */
#include <configs/ti_armv7_common.h>

#endif /* __CONFIG_BEAGLEPLAY_H */
