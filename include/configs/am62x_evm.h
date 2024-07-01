/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Configuration header file for K3 AM625 SoC family
 *
 * Copyright (C) 2020-2022 Texas Instruments Incorporated - https://www.ti.com/
 *	Suman Anna <s-anna@ti.com>
 */

#ifndef __CONFIG_AM625_EVM_H
#define __CONFIG_AM625_EVM_H

/**
 * define AM62X_SK_TIBOOT3_IMAGE_GUID - firmware GUID for AM62X sk tiboot3.bin
 * define AM62X_SK_SPL_IMAGE_GUID     - firmware GUID for AM62X sk SPL
 * define AM62X_SK_UBOOT_IMAGE_GUID   - firmware GUID for AM62X sk UBOOT
 *
 * These GUIDs are used in capsules updates to identify the corresponding
 * firmware object.
 *
 * Board developers using this as a starting reference should
 * define their own GUIDs to ensure that firmware repositories (like
 * LVFS) do not confuse them.
 */
#define AM62X_SK_TIBOOT3_IMAGE_GUID \
	EFI_GUID(0xabcb83d2, 0x9cb6, 0x4351, 0xb8, 0xf1, \
		0x64, 0x94, 0xbb, 0xe3, 0x70, 0x0a)

#define AM62X_SK_SPL_IMAGE_GUID \
	EFI_GUID(0xaee355fc, 0xbf97, 0x4264, 0x8c, 0x82, \
		0x43, 0x72, 0x55, 0xef, 0xdc, 0x1d)

#define AM62X_SK_UBOOT_IMAGE_GUID \
	EFI_GUID(0x28ab8c6c, 0xfca8, 0x41d3, 0x8e, 0xa1, \
		0x5f, 0x17, 0x1b, 0x7d, 0x29, 0x29)

/* Now for the remaining common defines */
#include <configs/ti_armv7_common.h>

#endif /* __CONFIG_AM625_EVM_H */
