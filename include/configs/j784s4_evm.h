/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Configuration header file for K3 J784S4 EVM
 *
 * Copyright (C) 2023-2024 Texas Instruments Incorporated - https://www.ti.com/
 *	Hari Nagalla <hnagalla@ti.com>
 */

#ifndef __CONFIG_J784S4_EVM_H
#define __CONFIG_J784S4_EVM_H

/**
 * define AM69_SK_TIBOOT3_IMAGE_GUID - firmware GUID for AM69 SK tiboot3.bin
 * define AM69_SK_SPL_IMAGE_GUID     - firmware GUID for AM69 SK SPL
 * define AM69_SK_UBOOT_IMAGE_GUID   - firmware GUID for AM69 SK UBOOT
 *
 * These GUIDs are used in capsules updates to identify the corresponding
 * firmware object.
 *
 * Board developers using this as a starting reference should
 * define their own GUIDs to ensure that firmware repositories (like
 * LVFS) do not confuse them.
 */
#define AM69_SK_TIBOOT3_IMAGE_GUID \
	EFI_GUID(0xadf49ec5, 0x61bb, 0x4dbe, 0x8b, 0x8d,	\
		 0x39, 0xdf, 0x4d, 0x7e, 0xbf, 0x46)

#define AM69_SK_SPL_IMAGE_GUID \
	EFI_GUID(0x787f0059, 0x63a1, 0x461c, 0xa1, 0x8e, \
		 0x9d, 0x83, 0x83, 0x45, 0xfe, 0x8e)

#define AM69_SK_UBOOT_IMAGE_GUID \
	EFI_GUID(0x9300505d, 0x6ec5, 0x4ff8, 0x99, 0xe4, \
		 0x54, 0x59, 0xa0, 0x4b, 0xe6, 0x17)

/* Now for the remaining common defines */
#include <configs/ti_armv7_common.h>

#endif /* __CONFIG_J784S4_EVM_H */
