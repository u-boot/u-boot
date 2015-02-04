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
#define CONFIG_BOARD_EARLY_INIT_F

#define CONFIG_NR_DRAM_BANKS		1

#define CONFIG_X86_SERIAL

/* ns16550 UART is memory-mapped in Quark SoC */
#undef  CONFIG_SYS_NS16550_PORT_MAPPED

#define CONFIG_PCI_MEM_BUS		0x90000000
#define CONFIG_PCI_MEM_PHYS		CONFIG_PCI_MEM_BUS
#define CONFIG_PCI_MEM_SIZE		0x20000000

#define CONFIG_PCI_PREF_BUS		0xb0000000
#define CONFIG_PCI_PREF_PHYS		CONFIG_PCI_PREF_BUS
#define CONFIG_PCI_PREF_SIZE		0x20000000

#define CONFIG_PCI_IO_BUS		0x2000
#define CONFIG_PCI_IO_PHYS		CONFIG_PCI_IO_BUS
#define CONFIG_PCI_IO_SIZE		0xe000

#define CONFIG_SYS_EARLY_PCI_INIT
#define CONFIG_PCI_PNP

#define CONFIG_STD_DEVICES_SETTINGS	"stdin=serial\0" \
					"stdout=serial\0" \
					"stderr=serial\0"

/* SATA is not supported in Quark SoC */
#undef CONFIG_SCSI_AHCI
#undef CONFIG_CMD_SCSI

/* Video is not supported in Quark SoC */
#undef CONFIG_VIDEO
#undef CONFIG_CFB_CONSOLE

/* SD/MMC support */
#define CONFIG_MMC
#define CONFIG_SDHCI
#define CONFIG_GENERIC_MMC
#define CONFIG_MMC_SDMA
#define CONFIG_CMD_MMC

#endif	/* __CONFIG_H */
