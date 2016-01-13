/*
 * Copyright (C) 2013-2014 Synopsys, Inc. All rights reserved.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef _CONFIG_AXS101_H_
#define _CONFIG_AXS101_H_

/*
 *  CPU configuration
 */
#define CONFIG_SYS_TIMER_RATE		CONFIG_SYS_CLK_FREQ

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
#define CONFIG_SYS_SDRAM_SIZE		0x20000000	/* 512 Mb */

#define CONFIG_SYS_INIT_SP_ADDR		\
	(CONFIG_SYS_SDRAM_BASE + 0x1000 - GENERATED_GBL_DATA_SIZE)

#define CONFIG_SYS_MALLOC_LEN		0x200000	/* 2 MB */
#define CONFIG_SYS_BOOTM_LEN		0x2000000	/* 32 MB */
#define CONFIG_SYS_LOAD_ADDR		0x82000000

/*
 * This board might be of different versions so handle it
 */
#define CONFIG_BOARD_TYPES
#define CONFIG_BOARD_EARLY_INIT_F

#define CONFIG_OF_LIBFDT

/*
 * NAND Flash configuration
 */
#define CONFIG_SYS_NO_FLASH
#define CONFIG_SYS_NAND_BASE		(ARC_FPGA_PERIPHERAL_BASE + 0x16000)
#define CONFIG_SYS_MAX_NAND_DEVICE	1

/*
 * UART configuration
 */
#define CONFIG_DW_SERIAL
#define CONFIG_SYS_NS16550_SERIAL
#define CONFIG_SYS_NS16550_CLK		33333333
#define CONFIG_SYS_NS16550_MEM32
#define CONFIG_BAUDRATE			115200

/*
 * I2C configuration
 */
#define CONFIG_SYS_I2C
#define CONFIG_SYS_I2C_DW
#define CONFIG_I2C_ENV_EEPROM_BUS	2
#define CONFIG_SYS_I2C_SPEED		100000
#define CONFIG_SYS_I2C_SPEED1		100000
#define CONFIG_SYS_I2C_SPEED2		100000
#define CONFIG_SYS_I2C_SLAVE		0
#define CONFIG_SYS_I2C_SLAVE1		0
#define CONFIG_SYS_I2C_SLAVE2		0
#define CONFIG_SYS_I2C_BASE		0xE001D000
#define CONFIG_SYS_I2C_BASE1		0xE001E000
#define CONFIG_SYS_I2C_BASE2		0xE001F000
#define CONFIG_SYS_I2C_BUS_MAX		3
#define IC_CLK				50

/*
 * EEPROM configuration
 */
#define CONFIG_SYS_I2C_EEPROM_ADDR		(0xA8 >> 1)
#define CONFIG_SYS_I2C_EEPROM_ADDR_LEN		1
#define CONFIG_SYS_I2C_EEPROM_ADDR_OVERFLOW	1
#define CONFIG_SYS_EEPROM_PAGE_WRITE_BITS	3
#define CONFIG_SYS_EEPROM_PAGE_WRITE_DELAY_MS	64

/*
 * SD/MMC configuration
 */
#define CONFIG_MMC
#define CONFIG_GENERIC_MMC
#define CONFIG_DWMMC
#define CONFIG_DOS_PARTITION

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
#define CONFIG_CMD_FAT
#define CONFIG_CMD_MMC
#define CONFIG_CMD_NAND

#define CONFIG_AUTO_COMPLETE
#define CONFIG_SYS_MAXARGS		16

/*
 * Environment settings
 */
#define CONFIG_ENV_IS_IN_EEPROM
#define CONFIG_ENV_SIZE			0x00200		/* 512 bytes */
#define CONFIG_ENV_OFFSET		0

/*
 * Environment configuration
 */
#define CONFIG_BOOTDELAY		3
#define CONFIG_BOOTFILE			"uImage"
#define CONFIG_BOOTARGS			"console=ttyS3,115200n8"
#define CONFIG_LOADADDR			CONFIG_SYS_LOAD_ADDR

/*
 * Console configuration
 */
#define CONFIG_SYS_LONGHELP
#define CONFIG_SYS_CBSIZE		256
#define CONFIG_SYS_BARGSIZE		CONFIG_SYS_CBSIZE
#define CONFIG_SYS_PBSIZE		(CONFIG_SYS_CBSIZE + \
						sizeof(CONFIG_SYS_PROMPT) + 16)

/*
 * Misc utility configuration
 */
#define CONFIG_BOUNCE_BUFFER

#endif /* _CONFIG_AXS101_H_ */
