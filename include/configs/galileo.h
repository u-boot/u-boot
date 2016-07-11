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
#define CONFIG_ARCH_EARLY_INIT_R
#define CONFIG_ARCH_MISC_INIT

/* ns16550 UART is memory-mapped in Quark SoC */
#undef  CONFIG_SYS_NS16550_PORT_MAPPED

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

/* 10/100M Ethernet support */
#define CONFIG_DESIGNWARE_ETH
#define CONFIG_DW_ALTDESCRIPTOR

/* Environment configuration */
#define CONFIG_ENV_SECT_SIZE		0x1000
#define CONFIG_ENV_OFFSET		0

#endif	/* __CONFIG_H */
