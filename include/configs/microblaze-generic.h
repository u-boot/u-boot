/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2007-2010 Michal Simek
 *
 * Michal SIMEK <monstr@monstr.eu>
 */

#ifndef __CONFIG_H
#define __CONFIG_H

/* Microblaze is microblaze_0 */
#define XILINX_FSL_NUMBER	3

#define CONFIG_SYS_BOOTM_LEN	(64 * 1024 * 1024)

/* uart */
/* The following table includes the supported baudrates */
# define CONFIG_SYS_BAUDRATE_TABLE \
	{300, 600, 1200, 2400, 4800, 9600, 19200, 38400, 57600, 115200, 230400}

/* Stack location before relocation */
#define CONFIG_SYS_INIT_SP_OFFSET	(CONFIG_SYS_TEXT_BASE - \
					 CONFIG_SYS_MALLOC_F_LEN)

#ifdef CONFIG_CFI_FLASH
/* ?empty sector */
# define CONFIG_SYS_FLASH_EMPTY_INFO	1
/* max number of memory banks */
/* max number of sectors on one chip */
# define CONFIG_SYS_MAX_FLASH_SECT	2048
#endif

#ifndef XILINX_DCACHE_BYTE_SIZE
#define XILINX_DCACHE_BYTE_SIZE	32768
#endif

/* size of console buffer */
#define	CONFIG_SYS_CBSIZE	512
/* max number of command args */
#define	CONFIG_SYS_MAXARGS	15

#define	CONFIG_HOSTNAME		"microblaze-generic"

/* architecture dependent code */
#if defined(CONFIG_CMD_PXE) && defined(CONFIG_CMD_DHCP)
#define BOOT_TARGET_DEVICES_PXE(func)	func(PXE, pxe, na)
#else
#define BOOT_TARGET_DEVICES_PXE(func)
#endif

#if defined(CONFIG_CMD_DHCP)
#define BOOT_TARGET_DEVICES_DHCP(func)	func(DHCP, dhcp, na)
#else
#define BOOT_TARGET_DEVICES_DHCP(func)
#endif

#if defined(CONFIG_SPI_FLASH)
# define BOOT_TARGET_DEVICES_QSPI(func) func(QSPI, qspi, na)
#else
# define BOOT_TARGET_DEVICES_QSPI(func)
#endif

#if defined(CONFIG_MTD_NOR_FLASH)
# define BOOT_TARGET_DEVICES_NOR(func)  func(NOR, nor, na)
#else
# define BOOT_TARGET_DEVICES_NOR(func)
#endif

#define BOOTENV_DEV_NOR(devtypeu, devtypel, instance) \
	"bootcmd_nor=cp.b ${script_offset_nor} ${scriptaddr} ${script_size_f} && " \
		"echo NOR: Trying to boot script at ${scriptaddr} && " \
		"source ${scriptaddr}; echo NOR: SCRIPT FAILED: continuing...;\0"

#define BOOTENV_DEV_NAME_NOR(devtypeu, devtypel, instance) \
	"nor "

#define BOOTENV_DEV_QSPI(devtypeu, devtypel, instance) \
	"bootcmd_qspi=sf probe 0 0 0 && " \
	"sf read ${scriptaddr} ${script_offset_f} ${script_size_f} && " \
	"echo QSPI: Trying to boot script at ${scriptaddr} && " \
	"source ${scriptaddr}; echo QSPI: SCRIPT FAILED: continuing...;\0"

#define BOOTENV_DEV_NAME_QSPI(devtypeu, devtypel, instance) \
	"qspi "

#define BOOT_TARGET_DEVICES_JTAG(func)	func(JTAG, jtag, na)

#define BOOTENV_DEV_JTAG(devtypeu, devtypel, instance) \
	"bootcmd_jtag=echo JTAG: Trying to boot script at ${scriptaddr} && " \
		"source ${scriptaddr}; echo JTAG: SCRIPT FAILED: continuing...;\0"

#define BOOTENV_DEV_NAME_JTAG(devtypeu, devtypel, instance) \
	"jtag "

#define BOOT_TARGET_DEVICES(func) \
	BOOT_TARGET_DEVICES_JTAG(func) \
	BOOT_TARGET_DEVICES_QSPI(func) \
	BOOT_TARGET_DEVICES_NOR(func) \
	BOOT_TARGET_DEVICES_DHCP(func) \
	BOOT_TARGET_DEVICES_PXE(func)

#include <config_distro_bootcmd.h>

#ifndef CONFIG_EXTRA_ENV_SETTINGS
#define CONFIG_EXTRA_ENV_SETTINGS \
	"unlock=yes\0"\
	"nor0=flash-0\0"\
	"mtdparts=mtdparts=flash-0:"\
	"256k(u-boot),256k(env),3m(kernel),"\
	"1m(romfs),1m(cramfs),-(jffs2)\0"\
	"nc=setenv stdout nc;"\
	"setenv stdin nc\0" \
	"serial=setenv stdout serial;"\
	"setenv stdin serial\0"\
	"script_size_f=0x40000\0"\
	BOOTENV
#endif

/* SPL part */

#define CONFIG_SYS_UBOOT_BASE		CONFIG_SYS_TEXT_BASE

/* for booting directly linux */
#define CONFIG_SYS_FDT_BASE		(CONFIG_SYS_TEXT_BASE + \
					0x40000)

#define CONFIG_SYS_SPL_ARGS_ADDR	(CONFIG_SYS_TEXT_BASE + \
					 0x1000000)

/* SP location before relocation, must use scratch RAM */
/* BRAM start */
#define CONFIG_SYS_INIT_RAM_ADDR	0x0
/* BRAM size - will be generated */
#define CONFIG_SYS_INIT_RAM_SIZE	0x100000

# define CONFIG_SPL_STACK_ADDR		(CONFIG_SYS_INIT_RAM_ADDR + \
					 CONFIG_SYS_INIT_RAM_SIZE)

/* Just for sure that there is a space for stack */
#define CONFIG_SPL_STACK_SIZE		0x100

#define CONFIG_SPL_MAX_FOOTPRINT	(CONFIG_SYS_INIT_RAM_SIZE - \
					 CONFIG_SYS_INIT_RAM_ADDR - \
					 CONFIG_SYS_MALLOC_F_LEN - \
					 CONFIG_SPL_STACK_SIZE)

#endif	/* __CONFIG_H */
