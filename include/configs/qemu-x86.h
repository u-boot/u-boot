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

#define CONFIG_X86_SERIAL

#define CONFIG_PCI_MEM_BUS		0xc0000000
#define CONFIG_PCI_MEM_PHYS		CONFIG_PCI_MEM_BUS
#define CONFIG_PCI_MEM_SIZE		0x10000000

#define CONFIG_PCI_PREF_BUS		0xd0000000
#define CONFIG_PCI_PREF_PHYS		CONFIG_PCI_PREF_BUS
#define CONFIG_PCI_PREF_SIZE		0x10000000

#define CONFIG_PCI_IO_BUS		0x2000
#define CONFIG_PCI_IO_PHYS		CONFIG_PCI_IO_BUS
#define CONFIG_PCI_IO_SIZE		0xe000

#define CONFIG_PCI_PNP
#define CONFIG_E1000

#define CONFIG_STD_DEVICES_SETTINGS	"stdin=serial\0" \
					"stdout=serial\0" \
					"stderr=serial\0"

#define CONFIG_SCSI_DEV_LIST		\
	{PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_82371SB_1}

/* GPIO is not supported */
#undef CONFIG_INTEL_ICH6_GPIO
#undef CONFIG_CMD_GPIO

/* SPI is not supported */
#undef CONFIG_ICH_SPI
#undef CONFIG_ENV_IS_IN_SPI_FLASH
#define CONFIG_ENV_IS_NOWHERE

/* Video is not supported */
#undef CONFIG_VIDEO
#undef CONFIG_CFB_CONSOLE

#endif	/* __CONFIG_H */
