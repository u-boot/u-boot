/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2012 Michal Simek <monstr@monstr.eu>
 * (C) Copyright 2013 - 2018 Xilinx, Inc.
 *
 * Common configuration options for all Zynq boards.
 */

#ifndef __CONFIG_ZYNQ_COMMON_H
#define __CONFIG_ZYNQ_COMMON_H

/* CPU clock */
#ifndef CONFIG_CPU_FREQ_HZ
# define CONFIG_CPU_FREQ_HZ	800000000
#endif

#define CONFIG_REMAKE_ELF

/* Cache options */
#define CONFIG_SYS_L2CACHE_OFF
#ifndef CONFIG_SYS_L2CACHE_OFF
# define CONFIG_SYS_L2_PL310
# define CONFIG_SYS_PL310_BASE		0xf8f02000
#endif

#define ZYNQ_SCUTIMER_BASEADDR		0xF8F00600
#define CONFIG_SYS_TIMERBASE		ZYNQ_SCUTIMER_BASEADDR
#define CONFIG_SYS_TIMER_COUNTS_DOWN
#define CONFIG_SYS_TIMER_COUNTER	(CONFIG_SYS_TIMERBASE + 0x4)

/* Serial drivers */
/* The following table includes the supported baudrates */
#define CONFIG_SYS_BAUDRATE_TABLE  \
	{300, 600, 1200, 2400, 4800, 9600, 19200, 38400, 57600, 115200, 230400}

/* Ethernet driver */
#if defined(CONFIG_ZYNQ_GEM)
# define CONFIG_SYS_FAULT_ECHO_LINK_DOWN
# define CONFIG_BOOTP_MAY_FAIL
#endif

/* NOR */
#ifdef CONFIG_MTD_NOR_FLASH
# define CONFIG_SYS_MAX_FLASH_BANKS	1
# define CONFIG_SYS_MAX_FLASH_SECT	512
# define CONFIG_SYS_FLASH_ERASE_TOUT	1000
# define CONFIG_SYS_FLASH_WRITE_TOUT	5000
# define CONFIG_FLASH_SHOW_PROGRESS	10
# undef CONFIG_SYS_FLASH_EMPTY_INFO
# define CONFIG_SYS_FLASH_QUIET_TEST
#endif

#ifdef CONFIG_NAND_ZYNQ
#define CONFIG_SYS_MAX_NAND_DEVICE	1
#define CONFIG_SYS_NAND_ONFI_DETECTION
#endif

#ifdef CONFIG_USB_EHCI_ZYNQ
# define CONFIG_EHCI_IS_TDI

# define CONFIG_SYS_DFU_DATA_BUF_SIZE	0x600000
# define DFU_DEFAULT_POLL_TIMEOUT	300
# define CONFIG_THOR_RESET_OFF
#endif

/* enable preboot to be loaded before CONFIG_BOOTDELAY */

/* Boot configuration */
#define CONFIG_SYS_LOAD_ADDR		0 /* default? */

#ifdef CONFIG_SPL_BUILD
#define BOOTENV
#else

#ifdef CONFIG_CMD_MMC
#define BOOT_TARGET_DEVICES_MMC(func) func(MMC, mmc, 0) func(MMC, mmc, 1)
#else
#define BOOT_TARGET_DEVICES_MMC(func)
#endif

#ifdef CONFIG_CMD_USB
#define BOOT_TARGET_DEVICES_USB(func) func(USB, usb, 0) func(USB, usb, 1)
#else
#define BOOT_TARGET_DEVICES_USB(func)
#endif

#if defined(CONFIG_CMD_PXE) && defined(CONFIG_CMD_DHCP)
#define BOOT_TARGET_DEVICES_PXE(func) func(PXE, pxe, na)
#else
#define BOOT_TARGET_DEVICES_PXE(func)
#endif

#if defined(CONFIG_CMD_DHCP)
#define BOOT_TARGET_DEVICES_DHCP(func) func(DHCP, dhcp, na)
#else
#define BOOT_TARGET_DEVICES_DHCP(func)
#endif

#if defined(CONFIG_ZYNQ_QSPI)
# define BOOT_TARGET_DEVICES_QSPI(func)	func(QSPI, qspi, na)
#else
# define BOOT_TARGET_DEVICES_QSPI(func)
#endif

#if defined(CONFIG_NAND_ZYNQ)
# define BOOT_TARGET_DEVICES_NAND(func)	func(NAND, nand, na)
#else
# define BOOT_TARGET_DEVICES_NAND(func)
#endif

#if defined(CONFIG_MTD_NOR_FLASH)
# define BOOT_TARGET_DEVICES_NOR(func)	func(NOR, nor, na)
#else
# define BOOT_TARGET_DEVICES_NOR(func)
#endif

#define BOOTENV_DEV_QSPI(devtypeu, devtypel, instance) \
	"bootcmd_qspi=sf probe 0 0 0 && " \
		      "sf read ${scriptaddr} ${script_offset_f} ${script_size_f} && " \
		      "echo QSPI: Trying to boot script at ${scriptaddr} && " \
		      "source ${scriptaddr}; echo QSPI: SCRIPT FAILED: continuing...;\0"

#define BOOTENV_DEV_NAME_QSPI(devtypeu, devtypel, instance) \
	"qspi "

#define BOOTENV_DEV_NAND(devtypeu, devtypel, instance) \
	"bootcmd_nand=nand info && " \
		      "nand read ${scriptaddr} ${script_offset_f} ${script_size_f} && " \
		      "echo NAND: Trying to boot script at ${scriptaddr} && " \
		      "source ${scriptaddr}; echo NAND: SCRIPT FAILED: continuing...;\0"

#define BOOTENV_DEV_NAME_NAND(devtypeu, devtypel, instance) \
	"nand "

#define BOOTENV_DEV_NOR(devtypeu, devtypel, instance) \
	"script_offset_nor=0xE2FC0000\0"        \
	"bootcmd_nor=cp.b ${script_offset_nor} ${scriptaddr} ${script_size_f} && " \
		     "echo NOR: Trying to boot script at ${scriptaddr} && " \
		     "source ${scriptaddr}; echo NOR: SCRIPT FAILED: continuing...;\0"

#define BOOTENV_DEV_NAME_NOR(devtypeu, devtypel, instance) \
	"nor "

#define BOOT_TARGET_DEVICES_JTAG(func)  func(JTAG, jtag, na)

#define BOOTENV_DEV_JTAG(devtypeu, devtypel, instance) \
	"bootcmd_jtag=echo JTAG: Trying to boot script at ${scriptaddr} && " \
		"source ${scriptaddr}; echo JTAG: SCRIPT FAILED: continuing...;\0"

#define BOOTENV_DEV_NAME_JTAG(devtypeu, devtypel, instance) \
	"jtag "

#define BOOT_TARGET_DEVICES_USB_DFU(func) \
	func(USB_DFU, usb_dfu, 0) func(USB_DFU, usb_dfu, 1)

#define BOOTENV_DEV_USB_DFU(devtypeu, devtypel, instance) \
	"bootcmd_" #devtypel #instance "=setenv dfu_alt_info boot.scr ram " \
	"$scriptaddr $script_size_f && " \
	"dfu " #instance " ram " #instance " 60 && " \
	"echo DFU" #instance ": Trying to boot script at ${scriptaddr} && " \
	"source ${scriptaddr}; " \
	"echo DFU" #instance ": SCRIPT FAILED: continuing...;\0"

#define BOOTENV_DEV_NAME_USB_DFU(devtypeu, devtypel, instance) \
	""

#define BOOT_TARGET_DEVICES_USB_THOR(func) \
	func(USB_THOR, usb_thor, 0) func(USB_THOR, usb_thor, 1)

#define BOOTENV_DEV_USB_THOR(devtypeu, devtypel, instance) \
	"bootcmd_" #devtypel #instance "=setenv dfu_alt_info boot.scr ram " \
	"$scriptaddr $script_size_f && " \
	"thordown " #instance " ram " #instance " && " \
	"echo THOR" #instance ": Trying to boot script at ${scriptaddr} && " \
	"source ${scriptaddr}; " \
	"echo THOR" #instance ": SCRIPT FAILED: continuing...;\0"

#define BOOTENV_DEV_NAME_USB_THOR(devtypeu, devtypel, instance) \
	""

#define BOOT_TARGET_DEVICES(func) \
	BOOT_TARGET_DEVICES_JTAG(func) \
	BOOT_TARGET_DEVICES_MMC(func) \
	BOOT_TARGET_DEVICES_QSPI(func) \
	BOOT_TARGET_DEVICES_NAND(func) \
	BOOT_TARGET_DEVICES_NOR(func) \
	BOOT_TARGET_DEVICES_USB_DFU(func) \
	BOOT_TARGET_DEVICES_USB_THOR(func) \
	BOOT_TARGET_DEVICES_USB(func) \
	BOOT_TARGET_DEVICES_PXE(func) \
	BOOT_TARGET_DEVICES_DHCP(func)

#include <config_distro_bootcmd.h>
#endif /* CONFIG_SPL_BUILD */

/* Default environment */
#ifndef CONFIG_EXTRA_ENV_SETTINGS
#define CONFIG_EXTRA_ENV_SETTINGS	\
	"scriptaddr=0x20000\0"	\
	"script_size_f=0x40000\0"	\
	"fdt_addr_r=0x1f00000\0"        \
	"pxefile_addr_r=0x2000000\0"    \
	"kernel_addr_r=0x2000000\0"     \
	"scriptaddr=0x3000000\0"        \
	"ramdisk_addr_r=0x3100000\0"    \
	BOOTENV
#endif

/* Miscellaneous configurable options */

#define CONFIG_CLOCKS
#define CONFIG_SYS_MAXARGS		32 /* max number of command args */
#define CONFIG_SYS_CBSIZE		2048 /* Console I/O Buffer Size */

#define CONFIG_SYS_INIT_RAM_ADDR	0xFFFF0000
#define CONFIG_SYS_INIT_RAM_SIZE	0x2000
#define CONFIG_SYS_INIT_SP_ADDR		(CONFIG_SYS_INIT_RAM_ADDR + \
					CONFIG_SYS_INIT_RAM_SIZE - \
					GENERATED_GBL_DATA_SIZE)


/* Extend size of kernel image for uncompression */
#define CONFIG_SYS_BOOTM_LEN	(60 * 1024 * 1024)

/* Boot FreeBSD/vxWorks from an ELF image */
#define CONFIG_SYS_MMC_MAX_DEVICE	1

/* MMC support */
#ifdef CONFIG_MMC_SDHCI_ZYNQ
#define CONFIG_SPL_FS_LOAD_PAYLOAD_NAME     "u-boot.img"
#endif

/* Address in RAM where the parameters must be copied by SPL. */
#define CONFIG_SYS_SPL_ARGS_ADDR	0x10000000

#define CONFIG_SPL_FS_LOAD_ARGS_NAME		"system.dtb"
#define CONFIG_SPL_FS_LOAD_KERNEL_NAME		"uImage"

/* Not using MMC raw mode - just for compilation purpose */
#define CONFIG_SYS_MMCSD_RAW_MODE_ARGS_SECTOR	0
#define CONFIG_SYS_MMCSD_RAW_MODE_ARGS_SECTORS	0
#define CONFIG_SYS_MMCSD_RAW_MODE_KERNEL_SECTOR	0

/* qspi mode is working fine */
#ifdef CONFIG_ZYNQ_QSPI
#define CONFIG_SYS_SPI_ARGS_OFFS	0x200000
#define CONFIG_SYS_SPI_ARGS_SIZE	0x80000
#define CONFIG_SYS_SPI_KERNEL_OFFS	(CONFIG_SYS_SPI_ARGS_OFFS + \
					CONFIG_SYS_SPI_ARGS_SIZE)
#endif

/* SP location before relocation, must use scratch RAM */

/* 3 * 64kB blocks of OCM - one is on the top because of bootrom */
#define CONFIG_SPL_MAX_SIZE	0x30000

/* On the top of OCM space */
#define CONFIG_SYS_SPL_MALLOC_START	CONFIG_SPL_STACK_R_ADDR
#define CONFIG_SYS_SPL_MALLOC_SIZE	0x2000000

/*
 * SPL stack position - and stack goes down
 * 0xfffffe00 is used for putting wfi loop.
 * Set it up as limit for now.
 */
#define CONFIG_SPL_STACK	0xfffffe00

/* BSS setup */
#define CONFIG_SPL_BSS_START_ADDR	0x100000
#define CONFIG_SPL_BSS_MAX_SIZE		0x100000

#endif /* __CONFIG_ZYNQ_COMMON_H */
