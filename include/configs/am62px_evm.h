/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Configuration header file for K3 AM62Px SoC family
 *
 * Copyright (C) 2023 Texas Instruments Incorporated - https://www.ti.com/
 */

#ifndef __CONFIG_AM62PX_EVM_H
#define __CONFIG_AM62PX_EVM_H

/**
 * define AM62PX_SK_TIBOOT3_IMAGE_GUID - firmware GUID for AM62PX sk tiboot3.bin
 * define AM62PX_SK_SPL_IMAGE_GUID     - firmware GUID for AM62PX sk SPL
 * define AM62PX_SK_UBOOT_IMAGE_GUID   - firmware GUID for AM62PX sk UBOOT
 *
 * These GUIDs are used in capsules updates to identify the corresponding
 * firmware object.
 *
 * Board developers using this as a starting reference should
 * define their own GUIDs to ensure that firmware repositories (like
 * LVFS) do not confuse them.
 */
#define AM62PX_SK_TIBOOT3_IMAGE_GUID \
	EFI_GUID(0xb08471b7, 0xbe2d, 0x4489, 0x87, 0xa1, \
		0xca, 0xb2, 0x8a, 0x0c, 0xf7, 0x43)

#define AM62PX_SK_SPL_IMAGE_GUID \
	EFI_GUID(0xd02ed781, 0x6d71, 0x4c1a, 0xa9, 0x99, \
		0x3c, 0x6a, 0x41, 0xc3, 0x63, 0x24)

#define AM62PX_SK_UBOOT_IMAGE_GUID \
	EFI_GUID(0x7e6aea51, 0x965c, 0x44ab, 0xb3, 0x88, \
		0xda, 0xeb, 0x03, 0xb5, 0x4f, 0x66)

/* Now for the remaining common defines */
#include <configs/ti_armv7_common.h>

#endif /* __CONFIG_AM62PX_EVM_H */
