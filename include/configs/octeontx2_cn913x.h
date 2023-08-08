/*
 * Copyright (C) 2016 Stefan Roese <sr@denx.de>
 * Copyright (C) 2019 Marvell International Ltd.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef _CONFIG_OCTEONTX2_CN913X_H
#define _CONFIG_OCTEONTX2_CN913X_H

#define CONFIG_DEFAULT_CONSOLE		"console=ttyS0,115200 "\
					"earlycon=uart8250,mmio32,0xf0512000"

#include <configs/mvebu_armada-common.h>

/*
 * High Level Configuration Options (easy to change)
 */
#define CONFIG_SYS_TCLK		250000000	/* 250MHz */

#define CONFIG_BOARD_EARLY_INIT_R

#define CONFIG_SYS_MAX_NAND_DEVICE	1
#define CONFIG_SYS_NAND_MAX_CHIPS	1
#define CONFIG_SYS_NAND_ONFI_DETECTION
#define CONFIG_SYS_NAND_USE_FLASH_BBT

#define CONFIG_USB_MAX_CONTROLLER_COUNT (3 + 3)

#define BOOT_TARGET_DEVICES(func) \
	func(MMC, mmc, 1) \
	func(MMC, mmc, 0) \
	func(USB, usb, 0) \
	func(SCSI, scsi, 0) \
	func(PXE, pxe, na) \
	func(DHCP, dhcp, na)

/* RTC configuration */
#ifdef CONFIG_MARVELL_RTC
#define ERRATA_FE_3124064
#endif

#endif /* _CONFIG_OCTEONTX2_CN913X_H */
