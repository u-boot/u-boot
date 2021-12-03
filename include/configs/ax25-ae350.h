/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2017 Andes Technology Corporation
 * Rick Chen, Andes Technology Corporation <rick@andestech.com>
 */

#ifndef __CONFIG_H
#define __CONFIG_H

#ifdef CONFIG_SPL
#define CONFIG_SPL_MAX_SIZE		0x00100000
#define CONFIG_SPL_BSS_START_ADDR	0x04000000
#define CONFIG_SPL_BSS_MAX_SIZE		0x00100000

#ifdef CONFIG_SPL_MMC
#define CONFIG_SPL_FS_LOAD_PAYLOAD_NAME		"u-boot.itb"
#endif
#endif

#define RISCV_MMODE_TIMERBASE           0xe6000000
#define RISCV_MMODE_TIMER_FREQ          60000000

#define RISCV_SMODE_TIMER_FREQ          60000000

/*
 * CPU and Board Configuration Options
 */

/*
 * Miscellaneous configurable options
 */
#define CONFIG_SYS_CBSIZE	1024	/* Console I/O Buffer Size */

/*
 * Print Buffer Size
 */
#define CONFIG_SYS_PBSIZE	\
	(CONFIG_SYS_CBSIZE + sizeof(CONFIG_SYS_PROMPT) + 16)

/*
 * max number of command args
 */
#define CONFIG_SYS_MAXARGS	16

/*
 * Boot Argument Buffer Size
 */
#define CONFIG_SYS_BARGSIZE	CONFIG_SYS_CBSIZE

/* DT blob (fdt) address */
#define CONFIG_SYS_FDT_BASE		0x800f0000

/*
 * Physical Memory Map
 */
#define PHYS_SDRAM_0	0x00000000		/* SDRAM Bank #1 */
#define PHYS_SDRAM_1	\
	(PHYS_SDRAM_0 + PHYS_SDRAM_0_SIZE)	/* SDRAM Bank #2 */
#define PHYS_SDRAM_0_SIZE	0x20000000	/* 512 MB */
#define PHYS_SDRAM_1_SIZE	0x20000000	/* 512 MB */
#define CONFIG_SYS_SDRAM_BASE	PHYS_SDRAM_0

/*
 * Serial console configuration
 */
#define CONFIG_SYS_NS16550_SERIAL
#ifndef CONFIG_DM_SERIAL
#define CONFIG_SYS_NS16550_REG_SIZE	-4
#endif
#define CONFIG_SYS_NS16550_CLK		19660800

/* Init Stack Pointer */
#define CONFIG_SYS_INIT_SP_ADDR		(CONFIG_SYS_SDRAM_BASE + 0x1000000 - \
					GENERATED_GBL_DATA_SIZE)

/* use CFI framework */

#define CONFIG_SYS_FLASH_CFI_WIDTH	FLASH_CFI_16BIT
#define CONFIG_SYS_CFI_FLASH_STATUS_POLL

/* support JEDEC */
#ifdef CONFIG_CFI_FLASH
#define CONFIG_SYS_MAX_FLASH_BANKS_DETECT	1
#endif/* Do not use CONFIG_FLASH_CFI_LEGACY to detect on board flash */
#define PHYS_FLASH_1			0x88000000	/* BANK 0 */
#define CONFIG_SYS_FLASH_BASE		PHYS_FLASH_1
#define CONFIG_SYS_FLASH_BANKS_LIST	{ PHYS_FLASH_1, }
#define CONFIG_SYS_MONITOR_BASE		PHYS_FLASH_1

#define CONFIG_SYS_FLASH_ERASE_TOUT	120000	/* TO for Flash Erase (ms) */
#define CONFIG_SYS_FLASH_WRITE_TOUT	500	/* TO for Flash Write (ms) */

/* max number of memory banks */
/*
 * There are 4 banks supported for this Controller,
 * but we have only 1 bank connected to flash on board
*/
#ifndef CONFIG_SYS_MAX_FLASH_BANKS_DETECT
#define CONFIG_SYS_MAX_FLASH_BANKS	1
#endif
#define CONFIG_SYS_FLASH_BANKS_SIZES {0x4000000}

/* max number of sectors on one chip */
#define CONFIG_FLASH_SECTOR_SIZE	(0x10000*2)
#define CONFIG_SYS_MAX_FLASH_SECT	512

/* environments */

/* SPI FLASH */

/*
 * For booting Linux, the board info and command line data
 * have to be in the first 16 MB of memory, since this is
 * the maximum mapped by the Linux kernel during initialization.
 */

/* Initial Memory map for Linux*/
#define CONFIG_SYS_BOOTMAPSZ	(64 << 20)
/* Increase max gunzip size */
#define CONFIG_SYS_BOOTM_LEN	(64 << 20)

/* Support autoboot from RAM (kernel image is loaded via debug port) */
#define KERNEL_IMAGE_ADDR	"0x2000000 "
#define BOOTENV_DEV_NAME_RAM(devtypeu, devtypel, instance) \
	"ram "
#define BOOTENV_DEV_RAM(devtypeu, devtypel, instance) \
	"bootcmd_ram=" \
	"booti " \
	KERNEL_IMAGE_ADDR \
	"- $fdtcontroladdr\0"

/* When we use RAM as ENV */

/* Enable distro boot */
#define BOOT_TARGET_DEVICES(func) \
	func(MMC, mmc, 0) \
	func(DHCP, dhcp, na) \
	func(RAM, ram, na)
#include <config_distro_bootcmd.h>

#define CONFIG_EXTRA_ENV_SETTINGS	\
				"kernel_addr_r=0x00080000\0" \
				"pxefile_addr_r=0x01f00000\0" \
				"scriptaddr=0x01f00000\0" \
				"fdt_addr_r=0x02000000\0" \
				"ramdisk_addr_r=0x02800000\0" \
				BOOTENV

#endif /* __CONFIG_H */
