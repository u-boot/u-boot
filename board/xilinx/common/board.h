/* SPDX-License-Identifier: GPL-2.0 */
/*
 * (C) Copyright 2020 Xilinx, Inc.
 * Michal Simek <michal.simek@xilinx.com>
 */

#ifndef _BOARD_XILINX_COMMON_BOARD_H
#define _BOARD_XILINX_COMMON_BOARD_H

#if CONFIG_IS_ENABLED(EFI_HAVE_CAPSULE_SUPPORT)
#define ZYNQ_BOOT_IMAGE_GUID \
	EFI_GUID(0x1ba29a15, 0x9969, 0x40aa, 0xb4, 0x24, \
		 0xe8, 0x61, 0x21, 0x61, 0x86, 0x64)

#define ZYNQ_UBOOT_IMAGE_GUID \
	EFI_GUID(0x1a5178f0, 0x87d3, 0x4f36, 0xac, 0x63, \
		 0x3b, 0x31, 0xa2, 0x3b, 0xe3, 0x05)

#define ZYNQMP_BOOT_IMAGE_GUID \
	EFI_GUID(0xde6066e8, 0x0256, 0x4fad, 0x82, 0x38, \
		 0xe4, 0x06, 0xe2, 0x74, 0xc4, 0xcf)

#define ZYNQMP_UBOOT_IMAGE_GUID \
	EFI_GUID(0xcf9ecfd4, 0x938b, 0x41c5, 0x85, 0x51, \
		 0x1f, 0x88, 0x3a, 0xb7, 0xdc, 0x18)
#endif /* EFI_HAVE_CAPSULE_SUPPORT */

int board_late_init_xilinx(void);

int xilinx_read_eeprom(void);

#endif /* BOARD_XILINX_COMMON_BOARD_H */
