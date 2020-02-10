/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2016 Freescale Semiconductor, Inc.
 *
 * Configuration settings for the Embedded Artists i.MX7ULP COM board.
 */

#ifndef __MX7ULP_COM_CONFIG_H
#define __MX7ULP_COM_CONFIG_H

#include <linux/sizes.h>
#include <asm/arch/imx-regs.h>

#define CONFIG_BOARD_POSTCLK_INIT
#define CONFIG_SYS_BOOTM_LEN		0x1000000

/*
 * Detect overlap between U-Boot image and environment area in build-time
 *
 * CONFIG_BOARD_SIZE_LIMIT = CONFIG_ENV_OFFSET - u-boot-dtb.imx offset
 * CONFIG_BOARD_SIZE_LIMIT = 768k - 1k = 767k = 785408
 *
 * Currently CONFIG_BOARD_SIZE_LIMIT does not handle expressions, so
 * write the direct value here
 */
#define CONFIG_BOARD_SIZE_LIMIT		785408
#define CONFIG_SYS_MMC_ENV_DEV		0
#define CONFIG_MMCROOT			"/dev/mmcblk0p2"
#define CONFIG_SYS_MMC_IMG_LOAD_PART	1

/* Using ULP WDOG for reset */
#define WDOG_BASE_ADDR			WDG1_RBASE

#define CONFIG_SYS_HZ_CLOCK		1000000 /* Fixed at 1MHz from TSTMR */

#define CONFIG_INITRD_TAG
#define CONFIG_CMDLINE_TAG
#define CONFIG_SETUP_MEMORY_TAGS

/* Size of malloc() pool */
#define CONFIG_SYS_MALLOC_LEN		(8 * SZ_1M)

/* UART */
#define LPUART_BASE			LPUART4_RBASE

/* allow to overwrite serial and ethaddr */
#define CONFIG_ENV_OVERWRITE

/* Physical Memory Map */

#define PHYS_SDRAM			0x60000000
#define CONFIG_SYS_MEMTEST_START	PHYS_SDRAM
#define CONFIG_SYS_SDRAM_BASE		PHYS_SDRAM

#define CONFIG_LOADADDR			0x60800000

#define CONFIG_SYS_MEMTEST_END		(CONFIG_SYS_MEMTEST_START + SZ_512M)

#define CONFIG_EXTRA_ENV_SETTINGS \
	"image=zImage\0" \
	"console=ttyLP0\0" \
	"fdt_high=0xffffffff\0" \
	"initrd_high=0xffffffff\0" \
	"fdt_file=imx7ulp-com.dtb\0" \
	"fdt_addr=0x63000000\0" \
	"mmcdev="__stringify(CONFIG_SYS_MMC_ENV_DEV)"\0" \
	"mmcpart=" __stringify(CONFIG_SYS_MMC_IMG_LOAD_PART) "\0" \
	"mmcroot=" CONFIG_MMCROOT " rootwait rw\0" \
	"mmcargs=setenv bootargs console=${console},${baudrate} " \
		"root=${mmcroot}\0" \
	"loadimage=fatload mmc ${mmcdev}:${mmcpart} ${loadaddr} ${image}\0" \
	"loadfdt=fatload mmc ${mmcdev}:${mmcpart} ${fdt_addr} ${fdt_file}\0" \
	"mmcboot=echo Booting from mmc ...; " \
		"run mmcargs; " \
		"if run loadfdt; then " \
			"bootz ${loadaddr} - ${fdt_addr}; " \
		"fi;\0" \

#define CONFIG_BOOTCOMMAND \
	"if run loadimage; then " \
		"run mmcboot; " \
	"fi; " \

#define CONFIG_SYS_LOAD_ADDR		CONFIG_LOADADDR

#define CONFIG_SYS_INIT_RAM_ADDR	IRAM_BASE_ADDR
#define CONFIG_SYS_INIT_RAM_SIZE	SZ_256K

#define CONFIG_SYS_INIT_SP_OFFSET \
	(CONFIG_SYS_INIT_RAM_SIZE - GENERATED_GBL_DATA_SIZE)
#define CONFIG_SYS_INIT_SP_ADDR \
	(CONFIG_SYS_INIT_RAM_ADDR + CONFIG_SYS_INIT_SP_OFFSET)

#if !CONFIG_IS_ENABLED(SYS_DCACHE_OFF)
#define CONFIG_CMD_CACHE
#endif

#define CONFIG_MXC_USB_PORTSC		(PORT_PTS_UTMI | PORT_PTS_PTW)
#endif	/* __CONFIG_H */
