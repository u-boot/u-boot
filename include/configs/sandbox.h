/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (c) 2011 The Chromium OS Authors.
 */

#ifndef __CONFIG_H
#define __CONFIG_H

#define CONFIG_IO_TRACE

#define CONFIG_MALLOC_F_ADDR		0x0010000

/* GUIDs for capsule updatable firmware images */
#define SANDBOX_UBOOT_IMAGE_GUID \
	EFI_GUID(0x09d7cf52, 0x0720, 0x4710, 0x91, 0xd1, \
		 0x08, 0x46, 0x9b, 0x7f, 0xe9, 0xc8)

#define SANDBOX_UBOOT_ENV_IMAGE_GUID \
	EFI_GUID(0x5a7021f5, 0xfef2, 0x48b4, 0xaa, 0xba, \
		 0x83, 0x2e, 0x77, 0x74, 0x18, 0xc0)

#define SANDBOX_FIT_IMAGE_GUID \
	EFI_GUID(0x3673b45d, 0x6a7c, 0x46f3, 0x9e, 0x60, \
		 0xad, 0xab, 0xb0, 0x3f, 0x79, 0x37)

/* Size of our emulated memory */
#define SB_CONCAT(x, y) x ## y
#define SB_TO_UL(s) SB_CONCAT(s, UL)
#define CONFIG_SYS_SDRAM_BASE		0
#define CONFIG_SYS_SDRAM_SIZE \
		(SB_TO_UL(CONFIG_SANDBOX_RAM_SIZE_MB) << 20)

#define CONFIG_SYS_BAUDRATE_TABLE	{4800, 9600, 19200, 38400, 57600,\
					115200}

#ifndef SANDBOX_NO_SDL
#define CONFIG_SANDBOX_SDL
#endif

#endif
