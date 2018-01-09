/*
 * Copyright (C) 2017 Synopsys, Inc. All rights reserved.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef _CONFIG_HSDK_H_
#define _CONFIG_HSDK_H_

#include <linux/sizes.h>

/*
 *  CPU configuration
 */
#define ARC_PERIPHERAL_BASE		0xF0000000
#define ARC_DWMMC_BASE			(ARC_PERIPHERAL_BASE + 0xA000)
#define ARC_DWGMAC_BASE			(ARC_PERIPHERAL_BASE + 0x18000)

/*
 * Memory configuration
 */
#define CONFIG_SYS_MONITOR_BASE		CONFIG_SYS_TEXT_BASE

#define CONFIG_SYS_DDR_SDRAM_BASE	0x80000000
#define CONFIG_SYS_SDRAM_BASE		CONFIG_SYS_DDR_SDRAM_BASE
#define CONFIG_SYS_SDRAM_SIZE		SZ_1G

#define CONFIG_SYS_INIT_SP_ADDR		\
	(CONFIG_SYS_SDRAM_BASE + 0x1000 - GENERATED_GBL_DATA_SIZE)

#define CONFIG_SYS_MALLOC_LEN		SZ_2M
#define CONFIG_SYS_BOOTM_LEN		SZ_32M
#define CONFIG_SYS_LOAD_ADDR		0x82000000

/*
 * This board might be of different versions so handle it
 */
#define CONFIG_BOARD_TYPES

/*
 * UART configuration
 */
#define CONFIG_DW_SERIAL
#define CONFIG_SYS_NS16550_SERIAL
#define CONFIG_SYS_NS16550_CLK		33330000
#define CONFIG_SYS_NS16550_MEM32

/*
 * Ethernet PHY configuration
 */
#define CONFIG_MII

/*
 * USB 1.1 configuration
 */
#define CONFIG_USB_OHCI_NEW
#define CONFIG_SYS_USB_OHCI_MAX_ROOT_PORTS 1

/*
 * Environment settings
 */
#define CONFIG_ENV_SIZE			SZ_16K

/*
 * Environment configuration
 */
#define CONFIG_BOOTFILE			"uImage"
#define CONFIG_LOADADDR			CONFIG_SYS_LOAD_ADDR

/*
 * Console configuration
 */
#define CONFIG_AUTO_COMPLETE
#define CONFIG_CMDLINE_EDITING
#define CONFIG_SYS_LONGHELP

/*
 * Misc utility configuration
 */
#define CONFIG_BOUNCE_BUFFER

#endif /* _CONFIG_HSDK_H_ */
