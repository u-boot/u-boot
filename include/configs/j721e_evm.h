/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Configuration header file for K3 J721E EVM
 *
 * Copyright (C) 2018-2020 Texas Instruments Incorporated - https://www.ti.com/
 *	Lokesh Vutla <lokeshvutla@ti.com>
 */

#ifndef __CONFIG_J721E_EVM_H
#define __CONFIG_J721E_EVM_H

#include <linux/sizes.h>

/* FLASH Configuration */
#define CFG_SYS_FLASH_BASE		0x000000000

/* SPL Loader Configuration */
#if defined(CONFIG_TARGET_J721E_A72_EVM) || defined(CONFIG_TARGET_J7200_A72_EVM)
#define CFG_SYS_UBOOT_BASE		0x50280000
/* Image load address in RAM for DFU boot*/
#else
#define CFG_SYS_UBOOT_BASE		0x50080000
#endif

/**
 * define J721E_SK_TIBOOT3_IMAGE_GUID - firmware GUID for J721e sk tiboot3.bin
 * define J721E_SK_SPL_IMAGE_GUID     - firmware GUID for J721e sk SPL
 * define J721E_SK_UBOOT_IMAGE_GUID   - firmware GUID for J721e sk UBOOT
 * define J721E_SK_SYSFW_IMAGE_GUID   - firmware GUID for J721e sk SYSFW
 *
 * These GUIDs are used in capsules updates to identify the corresponding
 * firmware object.
 *
 * Board developers using this as a starting reference should
 * define their own GUIDs to ensure that firmware repositories (like
 * LVFS) do not confuse them.
 */
#define J721E_SK_TIBOOT3_IMAGE_GUID \
	EFI_GUID(0xe672b518, 0x7cd7, 0x4014, 0xbd, 0x8d, \
		 0x40, 0x72, 0x4d, 0x0a, 0xd4, 0xdc)

#define J721E_SK_SPL_IMAGE_GUID \
	EFI_GUID(0x86f710ad, 0x10cf, 0x46ea, 0xac, 0x67, \
		 0x85, 0x6a, 0xe0, 0x6e, 0xfa, 0xd2)

#define J721E_SK_UBOOT_IMAGE_GUID \
	EFI_GUID(0x81b58fb0, 0x3b00, 0x4add, 0xa2, 0x0a, \
		 0xc1, 0x85, 0xbb, 0xac, 0xa1, 0xed)

#define J721E_SK_SYSFW_IMAGE_GUID \
	EFI_GUID(0x6fd10680, 0x361b, 0x431f, 0x80, 0xaa, \
		 0x89, 0x94, 0x55, 0x81, 0x9e, 0x11)

/* Now for the remaining common defines */
#include <configs/ti_armv7_common.h>

#endif /* __CONFIG_J721E_EVM_H */
