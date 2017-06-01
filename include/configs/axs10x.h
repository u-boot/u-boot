/*
 * Copyright (C) 2013-2016 Synopsys, Inc. All rights reserved.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef _CONFIG_AXS10X_H_
#define _CONFIG_AXS10X_H_

#include <linux/sizes.h>
/*
 *  CPU configuration
 */
#define ARC_FPGA_PERIPHERAL_BASE	0xE0000000
#define ARC_APB_PERIPHERAL_BASE		0xF0000000
#define ARC_DWMMC_BASE			(ARC_FPGA_PERIPHERAL_BASE + 0x15000)
#define ARC_DWGMAC_BASE			(ARC_FPGA_PERIPHERAL_BASE + 0x18000)

/*
 * Memory configuration
 */
#define CONFIG_SYS_MONITOR_BASE		CONFIG_SYS_TEXT_BASE

#define CONFIG_SYS_DDR_SDRAM_BASE	0x80000000
#define CONFIG_SYS_SDRAM_BASE		CONFIG_SYS_DDR_SDRAM_BASE
#define CONFIG_SYS_SDRAM_SIZE		SZ_512M

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
 * NAND Flash configuration
 */
#define CONFIG_SYS_NAND_BASE		(ARC_FPGA_PERIPHERAL_BASE + 0x16000)
#define CONFIG_SYS_MAX_NAND_DEVICE	1

/*
 * UART configuration
 */
#define CONFIG_DW_SERIAL
#define CONFIG_SYS_NS16550_SERIAL
#define CONFIG_SYS_NS16550_CLK		33333333
#define CONFIG_SYS_NS16550_MEM32

/*
 * Ethernet PHY configuration
 */
#define CONFIG_MII
#define CONFIG_PHY_GIGE

/*
 * USB 1.1 configuration
 */
#define CONFIG_USB_OHCI_NEW
#define CONFIG_SYS_USB_OHCI_MAX_ROOT_PORTS 1

/*
 * Commands still not supported in Kconfig
 */
#define CONFIG_CMD_NAND

#define CONFIG_AUTO_COMPLETE
#define CONFIG_SYS_MAXARGS		16
#define CONFIG_CMDLINE_EDITING

/*
 * Environment settings
 */
#define CONFIG_ENV_IS_IN_FAT
#define CONFIG_ENV_SIZE			SZ_16K
#define FAT_ENV_INTERFACE		"mmc"
#define FAT_ENV_DEVICE_AND_PART		"0:1"
#define FAT_ENV_FILE			"uboot.env"
#define CONFIG_FAT_WRITE

/*
 * Environment configuration
 */
#define CONFIG_BOOTFILE			"uImage"
#define CONFIG_BOOTARGS			"console=ttyS3,115200n8"
#define CONFIG_LOADADDR			CONFIG_SYS_LOAD_ADDR

/*
 * Console configuration
 */
#define CONFIG_SYS_LONGHELP
#define CONFIG_SYS_CBSIZE		SZ_256
#define CONFIG_SYS_BARGSIZE		CONFIG_SYS_CBSIZE
#define CONFIG_SYS_PBSIZE		(CONFIG_SYS_CBSIZE + \
						sizeof(CONFIG_SYS_PROMPT) + 16)

/*
 * Misc utility configuration
 */
#define CONFIG_BOUNCE_BUFFER

#endif /* _CONFIG_AXS10X_H_ */
