/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2016-2017 Socionext Inc.
 */
#ifndef __CONFIG_H
#define __CONFIG_H

/* Timers for fasp(TIMCLK) */
#define CONFIG_SYS_TIMERBASE		0x31080000	/* AP Timer 1 (ARM-SP804) */

/*
 * SDRAM (for initialize)
 */
#define CONFIG_SYS_SDRAM_BASE		(0x80000000)	/* Start address of DDR3 */
#define PHYS_SDRAM_SIZE			(0x7c000000)	/* Default size (2GB - Secure memory) */

#define CONFIG_VERY_BIG_RAM				/* SynQuacer supports up to 64GB */
#define CONFIG_MAX_MEM_MAPPED		PHYS_SDRAM_SIZE

#define SQ_DRAMINFO_BASE		(0x2e00ffc0)	/* DRAM info from TF-A */

/*
 * Boot info
 */
#define CONFIG_SYS_INIT_SP_ADDR		(0xe0000000)	/* stack of init proccess */

/*
 * Hardware drivers support
 */

/* RTC */
#define CONFIG_SYS_I2C_RTC_ADDR		0x51

/* Serial (pl011)       */
#define UART_CLK			(62500000)
#define CONFIG_PL011_CLOCK		UART_CLK
#define CONFIG_PL01x_PORTS		{(void *)(0x2a400000)}

/* Support MTD */
#define CONFIG_SYS_MAX_FLASH_BANKS	1
#define CONFIG_SYS_FLASH_BASE		(0x08000000)
#define CONFIG_SYS_FLASH_BANKS_LIST	{CONFIG_SYS_FLASH_BASE}

#define CONFIG_SYS_MEMTEST_START	(CONFIG_SYS_SDRAM_BASE + (512 * 1024))
#define CONFIG_SYS_MEMTEST_END		(CONFIG_SYS_SDRAM_BASE + PHYS_SDRAM_SIZE)

#define CONFIG_SYS_CBSIZE		1024
#define CONFIG_SYS_MAXARGS		128
#define CONFIG_SYS_PBSIZE		(CONFIG_SYS_CBSIZE + sizeof(CONFIG_SYS_PROMPT) + 16)

/* Since U-Boot 64bit PCIe support is limited, disable 64bit MMIO support */
/* #define CONFIG_SYS_PCI_64BIT		1 */

#define DEFAULT_DFU_ALT_INFO "dfu_alt_info="				\
			"mtd nor1=u-boot.bin raw 200000 100000;"	\
			"fip.bin raw 180000 78000;"			\
			"optee.bin raw 500000 100000\0"

/* Distro boot settings */
#ifndef CONFIG_SPL_BUILD
#ifdef CONFIG_CMD_USB
#define BOOT_TARGET_DEVICE_USB(func)	func(USB, usb, 0)
#else
#define BOOT_TARGET_DEVICE_USB(func)
#endif

#ifdef CONFIG_CMD_MMC
#define BOOT_TARGET_DEVICE_MMC(func)	func(MMC, mmc, 0)
#else
#define BOOT_TARGET_DEVICE_MMC(func)
#endif

#ifdef CONFIG_CMD_NVME
#define BOOT_TARGET_DEVICE_NVME(func)	func(NVME, nvme, 0)
#else
#define BOOT_TARGET_DEVICE_NVME(func)
#endif

#ifdef CONFIG_CMD_SCSI
#define BOOT_TARGET_DEVICE_SCSI(func)	func(SCSI, scsi, 0) func(SCSI, scsi, 1)
#else
#define BOOT_TARGET_DEVICE_SCSI(func)
#endif

#define BOOT_TARGET_DEVICES(func)	\
	BOOT_TARGET_DEVICE_USB(func)	\
	BOOT_TARGET_DEVICE_MMC(func)	\
	BOOT_TARGET_DEVICE_SCSI(func)	\
	BOOT_TARGET_DEVICE_NVME(func)	\

#include <config_distro_bootcmd.h>
#else /* CONFIG_SPL_BUILD */
#define BOOTENV
#endif

#define	CONFIG_EXTRA_ENV_SETTINGS		\
	"fdt_addr_r=0x9fe00000\0"		\
	"kernel_addr_r=0x90000000\0"		\
	"ramdisk_addr_r=0xa0000000\0"		\
	"scriptaddr=0x88000000\0"		\
	"pxefile_addr_r=0x88100000\0"		\
	DEFAULT_DFU_ALT_INFO			\
	BOOTENV

#endif /* __CONFIG_H */
