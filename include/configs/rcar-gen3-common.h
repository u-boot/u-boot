/*
 * include/configs/rcar-gen3-common.h
 *	This file is R-Car Gen3 common configuration file.
 *
 * Copyright (C) 2015 Renesas Electronics Corporation
 *
 * SPDX-License-Identifier: GPL-2.0+
 */

#ifndef __RCAR_GEN3_COMMON_H
#define __RCAR_GEN3_COMMON_H

#include <asm/arch/rmobile.h>

#define CONFIG_CMD_DFL
#define CONFIG_CMD_SDRAM
#define CONFIG_CMD_FAT
#define CONFIG_CMD_EXT2
#define CONFIG_CMD_EXT4
#define CONFIG_CMD_EXT4_WRITE
#define CONFIG_CMD_FDT

#define CONFIG_REMAKE_ELF

/* boot option */
#define CONFIG_SUPPORT_RAW_INITRD

/* Support File sytems */
#define CONFIG_FAT_WRITE
#define CONFIG_DOS_PARTITION
#define CONFIG_SUPPORT_VFAT
#define CONFIG_FS_EXT4
#define CONFIG_EXT4_WRITE

#define CONFIG_CMDLINE_TAG
#define CONFIG_SETUP_MEMORY_TAGS
#define CONFIG_INITRD_TAG
#define CONFIG_CMDLINE_EDITING
#define CONFIG_OF_LIBFDT

#define CONFIG_BAUDRATE		115200

#undef	CONFIG_SHOW_BOOT_PROGRESS

#define CONFIG_ARCH_CPU_INIT
#define CONFIG_DISPLAY_CPUINFO
#define CONFIG_DISPLAY_BOARDINFO
#define CONFIG_BOARD_EARLY_INIT_F

#define CONFIG_SH_GPIO_PFC

/* console */
#undef  CONFIG_SYS_CONSOLE_INFO_QUIET
#undef  CONFIG_SYS_CONSOLE_OVERWRITE_ROUTINE
#undef  CONFIG_SYS_CONSOLE_ENV_OVERWRITE

#define CONFIG_SYS_LONGHELP
#define CONFIG_SYS_CBSIZE		256
#define CONFIG_SYS_PBSIZE		256
#define CONFIG_SYS_MAXARGS		16
#define CONFIG_SYS_BARGSIZE		512
#define CONFIG_SYS_BAUDRATE_TABLE	{ 115200, 38400 }

/* MEMORY */
#define CONFIG_SYS_TEXT_BASE		0x49000000
#define CONFIG_SYS_INIT_SP_ADDR		(CONFIG_SYS_TEXT_BASE + 0x7fff0)

#define CONFIG_SYS_SDRAM_BASE		(0x48000000)
#define CONFIG_SYS_SDRAM_SIZE		(1024u * 1024 * 1024 - 0x08000000)
#define CONFIG_SYS_LOAD_ADDR		(0x48080000)
#define CONFIG_NR_DRAM_BANKS		1

#define CONFIG_SYS_MONITOR_BASE		0x00000000
#define CONFIG_SYS_MONITOR_LEN		(256 * 1024)
#define CONFIG_SYS_MALLOC_LEN		(1 * 1024 * 1024)
#define CONFIG_SYS_BOOTMAPSZ		(8 * 1024 * 1024)

/* ENV setting */
#define CONFIG_ENV_OVERWRITE
#define CONFIG_ENV_SECT_SIZE	(128 * 1024)
#define CONFIG_ENV_SIZE		(CONFIG_ENV_SECT_SIZE)
#define CONFIG_ENV_SIZE_REDUND	(CONFIG_ENV_SIZE)

#define CONFIG_EXTRA_ENV_SETTINGS	\
	"fdt_high=0xffffffffffffffff\0"	\
	"initrd_high=0xffffffffffffffff\0"

#define CONFIG_BOOTARGS	\
	"console=ttySC0,115200 rw root=/dev/nfs "	\
	"nfsroot=192.168.0.1:/export/rfs ip=192.168.0.20"

#define CONFIG_BOOTCOMMAND	\
	"tftp 0x48080000 Image; " \
	"tftp 0x48000000 Image-r8a7795-salvator-x.dtb; " \
	"booti 0x48080000 - 0x48000000"

#endif	/* __RCAR_GEN3_COMMON_H */
