/*
 * Copyright (C) 2015, Bin Meng <bmeng.cn@gmail.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

/*
 * board/config.h - configuration options, board specific
 */

#ifndef __CONFIG_H
#define __CONFIG_H

#include <configs/x86-common.h>

#define CONFIG_SYS_MONITOR_LEN		(1 << 20)
#define CONFIG_ARCH_EARLY_INIT_R

#define CONFIG_STD_DEVICES_SETTINGS	"stdin=serial,i8042-kbd\0" \
					"stdout=serial,vidconsole\0" \
					"stderr=serial,vidconsole\0"

/*
 * ATA/SATA support for QEMU x86 targets
 *   - Only legacy IDE controller is supported for QEMU '-M pc' target
 *   - AHCI controller is supported for QEMU '-M q35' target
 *
 * Default configuraion is to support the QEMU default x86 target
 * Undefine CONFIG_CMD_IDE to support q35 target
 */
#define CONFIG_CMD_IDE
#ifdef CONFIG_CMD_IDE
#define CONFIG_SYS_IDE_MAXBUS		2
#define CONFIG_SYS_IDE_MAXDEVICE	4
#define CONFIG_SYS_ATA_BASE_ADDR	0
#define CONFIG_SYS_ATA_DATA_OFFSET	0
#define CONFIG_SYS_ATA_REG_OFFSET	0
#define CONFIG_SYS_ATA_ALT_OFFSET	0
#define CONFIG_SYS_ATA_IDE0_OFFSET	0x1f0
#define CONFIG_SYS_ATA_IDE1_OFFSET	0x170
#define CONFIG_ATAPI

#undef CONFIG_SCSI_AHCI
#undef CONFIG_SCSI
#else
#define CONFIG_SCSI_DEV_LIST		\
	{PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_ICH9_AHCI}
#endif

/* GPIO is not supported */
#undef CONFIG_INTEL_ICH6_GPIO

/* SPI is not supported */
#undef CONFIG_ENV_IS_IN_SPI_FLASH
#define CONFIG_ENV_IS_NOWHERE

#endif	/* __CONFIG_H */
