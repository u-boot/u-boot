/*
 * Config file for Compulab CM-T335 board
 *
 * Copyright (C) 2013, Compulab Ltd - http://compulab.co.il/
 *
 * Author: Ilya Ledvich <ilya@compulab.co.il>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __CONFIG_CM_T335_H
#define __CONFIG_CM_T335_H

#define CONFIG_CM_T335
#define CONFIG_NAND

#include <configs/ti_am335x_common.h>

#undef CONFIG_BOARD_LATE_INIT
#undef CONFIG_SPI
#undef CONFIG_OMAP3_SPI
#undef CONFIG_CMD_SPI
#undef CONFIG_SPL_OS_BOOT
#undef CONFIG_BOOTCOUNT_LIMIT
#undef CONFIG_SPL_AM33XX_ENABLE_RTC32K_OSC

#undef CONFIG_MAX_RAM_BANK_SIZE
#define CONFIG_MAX_RAM_BANK_SIZE	(512 << 20)	/* 512MB */

#undef CONFIG_SYS_PROMPT
#define CONFIG_SYS_PROMPT		"CM-T335 # "

#define CONFIG_OMAP_COMMON

#define MACH_TYPE_CM_T335		4586	/* Until the next sync */
#define CONFIG_MACH_TYPE		MACH_TYPE_CM_T335

/* Clock Defines */
#define V_OSCK				25000000  /* Clock output from T2 */
#define V_SCLK				(V_OSCK)

#define CONFIG_ENV_SIZE			(16 << 10)	/* 16 KiB */

#ifndef CONFIG_SPL_BUILD
#define MMCARGS \
	"mmcdev=0\0" \
	"mmcroot=/dev/mmcblk0p2 rw rootwait\0" \
	"mmcrootfstype=ext4\0" \
	"mmcargs=setenv bootargs console=${console} " \
		"root=${mmcroot} " \
		"rootfstype=${mmcrootfstype}\0" \
	"mmcboot=echo Booting from mmc ...; " \
		"run mmcargs; " \
		"bootm ${loadaddr}\0"

#define NANDARGS \
	"mtdids=" MTDIDS_DEFAULT "\0" \
	"mtdparts=" MTDPARTS_DEFAULT "\0" \
	"nandroot=ubi0:rootfs rw\0" \
	"nandrootfstype=ubifs\0" \
	"nandargs=setenv bootargs console=${console} " \
		"root=${nandroot} " \
		"rootfstype=${nandrootfstype} " \
		"ubi.mtd=${rootfs_name}\0" \
	"nandboot=echo Booting from nand ...; " \
		"run nandargs; " \
		"nboot ${loadaddr} nand0 900000; " \
		"bootm ${loadaddr}\0"


#define CONFIG_EXTRA_ENV_SETTINGS \
	"loadaddr=82000000\0" \
	"console=ttyO0,115200n8\0" \
	"rootfs_name=rootfs\0" \
	"loadbootscript=fatload mmc ${mmcdev} ${loadaddr} boot.scr\0" \
	"bootscript=echo Running bootscript from mmc ...; " \
		"source ${loadaddr}\0" \
	"loaduimage=fatload mmc ${mmcdev} ${loadaddr} uImage\0" \
	MMCARGS \
	NANDARGS

#define CONFIG_BOOTCOMMAND \
	"mmc dev ${mmcdev}; if mmc rescan; then " \
		"if run loadbootscript; then " \
			"run bootscript; " \
		"else " \
			"if run loaduimage; then " \
				"run mmcboot; " \
			"else run nandboot; " \
			"fi; " \
		"fi; " \
	"else run nandboot; fi"
#endif /* CONFIG_SPL_BUILD */

#define CONFIG_TIMESTAMP
#define CONFIG_SYS_AUTOLOAD		"no"

/* Serial console configuration */
#define CONFIG_CONS_INDEX		1
#define CONFIG_SERIAL1			1	/* UART0 */

/* NS16550 Configuration */
#define CONFIG_SYS_NS16550_COM1		0x44e09000	/* UART0 */
#define CONFIG_SYS_NS16550_COM2		0x48022000	/* UART1 */
#define CONFIG_BAUDRATE			115200

/* I2C Configuration */
#define CONFIG_SYS_I2C_EEPROM_ADDR	0x50	/* Main EEPROM */
#define CONFIG_SYS_I2C_EEPROM_ADDR_LEN	1
#define CONFIG_SYS_I2C_EEPROM_BUS	0

/* SPL */
#define CONFIG_SPL_LDSCRIPT		"$(CPUDIR)/am33xx/u-boot-spl.lds"

/* Network. */
#define CONFIG_PHY_GIGE
#define CONFIG_PHYLIB
#define CONFIG_PHY_ATHEROS

/* NAND support */
#define CONFIG_SYS_NAND_5_ADDR_CYCLE
#define CONFIG_SYS_NAND_PAGE_COUNT	(CONFIG_SYS_NAND_BLOCK_SIZE / \
					 CONFIG_SYS_NAND_PAGE_SIZE)
#define CONFIG_SYS_NAND_PAGE_SIZE	2048
#define CONFIG_SYS_NAND_OOBSIZE		64
#define CONFIG_SYS_NAND_BLOCK_SIZE	(128 * 1024)
#define CONFIG_SYS_NAND_BAD_BLOCK_POS	NAND_LARGE_BADBLOCK_POS
#define CONFIG_SYS_NAND_ECCPOS		{ 2, 3, 4, 5, 6, 7, 8, 9, \
					 10, 11, 12, 13, 14, 15, 16, 17, \
					 18, 19, 20, 21, 22, 23, 24, 25, \
					 26, 27, 28, 29, 30, 31, 32, 33, \
					 34, 35, 36, 37, 38, 39, 40, 41, \
					 42, 43, 44, 45, 46, 47, 48, 49, \
					 50, 51, 52, 53, 54, 55, 56, 57, }

#define CONFIG_SYS_NAND_ECCSIZE		512
#define CONFIG_SYS_NAND_ECCBYTES	14

#define CONFIG_SYS_NAND_U_BOOT_START	CONFIG_SYS_TEXT_BASE

#undef CONFIG_SYS_NAND_U_BOOT_OFFS
#define CONFIG_SYS_NAND_U_BOOT_OFFS	0x200000

#define CONFIG_CMD_NAND
#define MTDIDS_DEFAULT			"nand0=nand"
#define MTDPARTS_DEFAULT		"mtdparts=nand:2m(spl)," \
					"1m(u-boot),1m(u-boot-env)," \
					"1m(dtb),4m(splash)," \
					"6m(kernel),-(rootfs)"
#define CONFIG_ENV_IS_IN_NAND
#define CONFIG_ENV_OFFSET		0x300000 /* environment starts here */
#define CONFIG_SYS_ENV_SECT_SIZE	(128 << 10)	/* 128 KiB */
#define CONFIG_SYS_NAND_ONFI_DETECTION
#ifdef CONFIG_SPL_OS_BOOT
#define CONFIG_CMD_SPL_NAND_OFS		0x400000 /* un-assigned: (using dtb) */
#define CONFIG_SYS_NAND_SPL_KERNEL_OFFS	0x500000
#define CONFIG_CMD_SPL_WRITE_SIZE	0x2000
#endif

/* GPIO pin + bank to pin ID mapping */
#define GPIO_PIN(_bank, _pin)		((_bank << 5) + _pin)

/* Status LED */
#define CONFIG_STATUS_LED
#define CONFIG_GPIO_LED
#define CONFIG_BOARD_SPECIFIC_LED
#define STATUS_LED_BIT			GPIO_PIN(2, 0)
/* Status LED polarity is inversed, so init it in the "off" state */
#define STATUS_LED_STATE		STATUS_LED_OFF
#define STATUS_LED_PERIOD		(CONFIG_SYS_HZ / 2)
#define STATUS_LED_BOOT			0

#ifndef CONFIG_SPL_BUILD
/*
 * Enable PCA9555 at I2C0-0x26.
 * First select the I2C0 bus with "i2c dev 0", then use "pca953x" command.
 */
#define CONFIG_PCA953X
#define CONFIG_CMD_PCA953X
#define CONFIG_CMD_PCA953X_INFO
#define CONFIG_SYS_I2C_PCA953X_ADDR	0x26
#define CONFIG_SYS_I2C_PCA953X_WIDTH	{ {0x26, 16} }
#endif /* CONFIG_SPL_BUILD */

#endif	/* __CONFIG_CM_T335_H */

