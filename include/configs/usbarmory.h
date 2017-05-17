/*
 * USB armory MkI board configuration settings
 * http://inversepath.com/usbarmory
 *
 * Copyright (C) 2015, Inverse Path
 * Andrej Rosano <andrej@inversepath.com>
 *
 * SPDX-License-Identifier:|____GPL-2.0+
 */

#ifndef __CONFIG_H
#define __CONFIG_H

#define CONFIG_SYS_FSL_CLK
#define CONFIG_MXC_GPIO

#include <asm/arch/imx-regs.h>

#include <config_distro_defaults.h>

/* U-Boot environment */
#define CONFIG_ENV_OFFSET	(6 * 64 * 1024)
#define CONFIG_ENV_SIZE		(8 * 1024)
#define CONFIG_ENV_IS_IN_MMC
#define CONFIG_SYS_MMC_ENV_DEV	0

/* U-Boot general configurations */
#define CONFIG_SYS_CBSIZE	512
#define CONFIG_SYS_PBSIZE	(CONFIG_SYS_CBSIZE+sizeof(CONFIG_SYS_PROMPT)+16)
#define CONFIG_SYS_MAXARGS	16
#define CONFIG_SYS_BARGSIZE	CONFIG_SYS_CBSIZE

/* UART */
#define CONFIG_MXC_UART
#define CONFIG_MXC_UART_BASE	UART1_BASE
#define CONFIG_CONS_INDEX	1

/* SD/MMC */
#define CONFIG_FSL_ESDHC
#define CONFIG_SYS_FSL_ESDHC_ADDR	0
#define CONFIG_SYS_FSL_ESDHC_NUM	1

/* USB */
#define CONFIG_USB_EHCI_MX5
#define CONFIG_MXC_USB_PORT	1
#define CONFIG_MXC_USB_PORTSC	(PORT_PTS_UTMI | PORT_PTS_PTW)
#define CONFIG_MXC_USB_FLAGS	0

/* I2C */
#define CONFIG_SYS_I2C
#define CONFIG_SYS_I2C_MXC
#define CONFIG_SYS_I2C_MXC_I2C1		/* enable I2C bus 1 */
#define CONFIG_SYS_I2C_MXC_I2C2		/* enable I2C bus 2 */

/* Fuse */
#define CONFIG_FSL_IIM

/* U-Boot memory offsets */
#define CONFIG_LOADADDR		0x72000000
#define CONFIG_SYS_TEXT_BASE	0x77800000
#define CONFIG_SYS_LOAD_ADDR	CONFIG_LOADADDR

/* Linux boot */
#define CONFIG_HOSTNAME		usbarmory
#define CONFIG_BOOTCOMMAND						\
	"run distro_bootcmd; "						\
	"setenv bootargs console=${console} ${bootargs_default}; "	\
	"ext2load mmc 0:1 ${kernel_addr_r} /boot/zImage; "		\
	"ext2load mmc 0:1 ${fdt_addr_r} /boot/${fdtfile}; "		\
	"bootz ${kernel_addr_r} - ${fdt_addr_r}"

#define BOOT_TARGET_DEVICES(func) func(MMC, mmc, 0)

#include <config_distro_bootcmd.h>

#define MEM_LAYOUT_ENV_SETTINGS			\
	"kernel_addr_r=0x70800000\0"		\
	"fdt_addr_r=0x71000000\0"		\
	"scriptaddr=0x70800000\0"		\
	"pxefile_addr_r=0x70800000\0"		\
	"ramdisk_addr_r=0x73000000\0"

#define CONFIG_EXTRA_ENV_SETTINGS				\
	MEM_LAYOUT_ENV_SETTINGS					\
	"bootargs_default=root=/dev/mmcblk0p1 rootwait rw\0"	\
	"fdtfile=imx53-usbarmory.dtb\0"				\
	"console=ttymxc0,115200\0"				\
	BOOTENV

#ifndef CONFIG_CMDLINE
#define CONFIG_BOOTARGS "console=ttymxc0,115200 root=/dev/mmcblk0p1 rootwait rw"
#define USBARMORY_FIT_PATH	"/boot/usbarmory.itb"
#define USBARMORY_FIT_ADDR	"0x70800000"
#endif

/* Physical Memory Map */
#define CONFIG_NR_DRAM_BANKS		1
#define PHYS_SDRAM			CSD0_BASE_ADDR
#define PHYS_SDRAM_SIZE			(gd->ram_size)

#define CONFIG_SYS_SDRAM_BASE		PHYS_SDRAM
#define CONFIG_SYS_INIT_RAM_ADDR	IRAM_BASE_ADDR
#define CONFIG_SYS_INIT_RAM_SIZE	IRAM_SIZE

#define CONFIG_SYS_INIT_SP_OFFSET \
	(CONFIG_SYS_INIT_RAM_SIZE - GENERATED_GBL_DATA_SIZE)
#define CONFIG_SYS_INIT_SP_ADDR \
	(CONFIG_SYS_INIT_RAM_ADDR + CONFIG_SYS_INIT_SP_OFFSET)

#define CONFIG_SYS_MEMTEST_START	0x70000000
#define CONFIG_SYS_MEMTEST_END		0x90000000

#define CONFIG_SYS_MALLOC_LEN		(10 * 1024 * 1024)

#endif				/* __CONFIG_H */
