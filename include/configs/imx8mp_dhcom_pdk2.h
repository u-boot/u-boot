/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2022 Marek Vasut <marex@denx.de>
 */

#ifndef __IMX8MP_DHCOM_PDK2_H
#define __IMX8MP_DHCOM_PDK2_H

#include <linux/sizes.h>
#include <linux/stringify.h>
#include <asm/arch/imx-regs.h>

#define CONFIG_SYS_BOOTM_LEN		SZ_128M

#define CONFIG_SPL_MAX_SIZE		(148 * 1024)
#define CONFIG_SYS_MONITOR_LEN		SZ_1M

#ifdef CONFIG_SPL_BUILD
#define CONFIG_SPL_STACK		0x96FC00
#define CONFIG_SPL_BSS_START_ADDR	0x0096FC00
#define CONFIG_SPL_BSS_MAX_SIZE		0x400	/* 1 KiB */
#define CONFIG_SYS_SPL_MALLOC_START	0x4c000000
#define CONFIG_SYS_SPL_MALLOC_SIZE	SZ_512K	/* 512 kiB */

/* For RAW image gives a error info not panic */
#define CONFIG_SPL_ABORT_ON_RAW_IMAGE

#endif

/* Link Definitions */
#define CONFIG_SYS_INIT_RAM_ADDR	0x40000000
#define CONFIG_SYS_INIT_RAM_SIZE	0x200000
#define CONFIG_SYS_INIT_SP_OFFSET \
	(CONFIG_SYS_INIT_RAM_SIZE - GENERATED_GBL_DATA_SIZE)
#define CONFIG_SYS_INIT_SP_ADDR \
	(CONFIG_SYS_INIT_RAM_ADDR + CONFIG_SYS_INIT_SP_OFFSET)

#define CONFIG_SYS_SDRAM_BASE		0x40000000
#define PHYS_SDRAM			0x40000000
#define PHYS_SDRAM_SIZE			0x20000000 /* Minimum 512 MiB DDR */

#define CONFIG_MXC_UART_BASE		UART1_BASE_ADDR

/* Monitor Command Prompt */
#define CONFIG_SYS_CBSIZE		2048
#define CONFIG_SYS_MAXARGS		64
#define CONFIG_SYS_BARGSIZE		CONFIG_SYS_CBSIZE
#define CONFIG_SYS_PBSIZE		\
	(CONFIG_SYS_CBSIZE + sizeof(CONFIG_SYS_PROMPT) + 16)

/* PHY needs a longer autonegotiation timeout after reset */
#define PHY_ANEG_TIMEOUT		20000
#define FEC_QUIRK_ENET_MAC

/* USDHC */
#define CONFIG_SYS_FSL_USDHC_NUM	2
#define CONFIG_SYS_FSL_ESDHC_ADDR	0

#if !defined(CONFIG_SPL_BUILD)

#define CONFIG_EXTRA_ENV_SETTINGS					\
	"altbootcmd=run bootcmd ; reset\0"				\
	"bootlimit=3\0"							\
	"kernel_addr_r=" __stringify(CONFIG_SYS_LOAD_ADDR) "\0"		\
	"pxefile_addr_r=" __stringify(CONFIG_SYS_LOAD_ADDR) "\0"	\
	"ramdisk_addr_r=0x58000000\0"					\
	"scriptaddr=" __stringify(CONFIG_SYS_LOAD_ADDR) "\0"		\
	/* Give slow devices beyond USB HUB chance to come up. */	\
	"usb_pgood_delay=2000\0"					\
	"dfu_alt_info="							\
		/* RAM block at DRAM offset 256..768 MiB */		\
		"ram ram0=ram ram 0x50000000 0x20000000&"		\
		/* 16 MiB SPI NOR */					\
		"mtd nor0=sf raw 0x0 0x1000000\0"			\
	"dh_update_env="						\
		"setenv dh_update_env true ; saveenv ; saveenv\0"	\
	"dh_update_sf_gen_fcfb="					\
		"setexpr sfaddr ${loadaddr} - 0x1000 ; "		\
		"base ${sfaddr} ; "					\
		"mw 0 0 0x400 ; "					\
		"mw 0x400 0x42464346 ; "				\
		"mw 0x404 0x56010000 ; "				\
		"mw 0x40c 00030300 ; "					\
		"mw 0x444 0x00020101 ; "				\
		"mw 0x450 0x10000000 ; "				\
		"mw 0x480 0x0818040b ; "				\
		"mw 0x484 0x24043008 ; "				\
		"mw 0x5c0 0x100 ; "					\
		"mw 0x5c4 0x10000 ; "					\
		"base 0\0"						\
	"dh_update_sf_write_data="					\
		"setexpr sfaddr ${loadaddr} - 0x1000 ; "		\
		"setexpr filesize ${filesize} + 0x1000 ; "		\
		"sf probe && sf update ${sfaddr} 0 ${filesize}\0"	\
	"dh_update_sd_to_sf="						\
		"load mmc 0:1 ${loadaddr} boot/flash.bin && "		\
		"run dh_update_sf_gen_fcfb dh_update_sf_write_data\0"	\
	"dh_update_emmc_to_sf="						\
		"load mmc 1:1 ${loadaddr} boot/flash.bin && "		\
		"run dh_update_sf_gen_fcfb dh_update_sf_write_data\0"	\
	BOOTENV

#define BOOT_TARGET_DEVICES(func)	\
	func(MMC, mmc, 0)		\
	func(MMC, mmc, 1)		\
	func(USB, usb, 0)		\
	func(DHCP, dhcp, na)

#include <config_distro_bootcmd.h>

#endif

#endif
