/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Configuration header file for K3 AM642 SoC family
 *
 * Copyright (C) 2020-2021 Texas Instruments Incorporated - https://www.ti.com/
 *	Keerthy <j-keerthy@ti.com>
 */

#ifndef __CONFIG_AM642_EVM_H
#define __CONFIG_AM642_EVM_H

/**
 * define AM64X_SK_TIBOOT3_IMAGE_GUID - firmware GUID for AM64X sk tiboot3.bin
 * define AM64X_SK_SPL_IMAGE_GUID     - firmware GUID for AM64X sk SPL
 * define AM64X_SK_UBOOT_IMAGE_GUID   - firmware GUID for AM64X sk UBOOT
 *
 * These GUIDs are used in capsules updates to identify the corresponding
 * firmware object.
 *
 * Board developers using this as a starting reference should
 * define their own GUIDs to ensure that firmware repositories (like
 * LVFS) do not confuse them.
 */
#define AM64X_SK_TIBOOT3_IMAGE_GUID \
	EFI_GUID(0xede0a0d5, 0x9116, 0x4bfb, 0xaa, 0x54, \
		0x09, 0xe9, 0x7b, 0x5a, 0xfe, 0x1a)

#define AM64X_SK_SPL_IMAGE_GUID \
	EFI_GUID(0x77678f5c, 0x64d4, 0x4910, 0xad, 0x75, \
		0x52, 0xc9, 0xd9, 0x5c, 0xdb, 0x1d)

#define AM64X_SK_UBOOT_IMAGE_GUID \
	EFI_GUID(0xc6ad43a9, 0x7d31, 0x4f5d, 0x83, 0xe9, \
		0xb8, 0xef, 0xec, 0xae, 0x05, 0xbf)

/* Now for the remaining common defines */
#include <configs/ti_armv7_common.h>

#endif /* __CONFIG_AM642_EVM_H */
