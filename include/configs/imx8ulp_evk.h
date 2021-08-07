/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2020 NXP
 */

#ifndef __IMX8ULP_EVK_H
#define __IMX8ULP_EVK_H

#include <linux/sizes.h>
#include <asm/arch/imx-regs.h>

#define CONFIG_SYS_BOOTM_LEN		(SZ_64M)
#define CONFIG_SPL_MAX_SIZE		(148 * 1024)
#define CONFIG_SYS_MONITOR_LEN		(512 * 1024)
#define CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_USE_SECTOR
#define CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_SECTOR	0x300
#define CONFIG_SYS_MMCSD_FS_BOOT_PARTITION	1
#define CONFIG_SYS_UBOOT_BASE	(QSPI0_AMBA_BASE + CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_SECTOR * 512)

#ifdef CONFIG_SPL_BUILD
#define CONFIG_SPL_LDSCRIPT		"arch/arm/cpu/armv8/u-boot-spl.lds"
#define CONFIG_SPL_STACK		0x22050000
#define CONFIG_SPL_BSS_START_ADDR	0x22048000
#define CONFIG_SPL_BSS_MAX_SIZE		0x2000	/* 8 KB */
#define CONFIG_SYS_SPL_MALLOC_START	0x22040000
#define CONFIG_SYS_SPL_MALLOC_SIZE	0x8000	/* 32 KB */

#define CONFIG_MALLOC_F_ADDR		0x22040000

#define CONFIG_SPL_LOAD_FIT_ADDRESS	0x95000000 /* SPL_RAM needed */

#define CONFIG_SPL_ABORT_ON_RAW_IMAGE /* For RAW image gives a error info not panic */

#endif

#define CONFIG_SERIAL_TAG

#define CONFIG_REMAKE_ELF

#define CONFIG_BOARD_EARLY_INIT_F
#define CONFIG_BOARD_LATE_INIT

/* ENET Config */
#if defined(CONFIG_FEC_MXC)
#define CONFIG_ETHPRIME                 "FEC"
#define PHY_ANEG_TIMEOUT		20000

#define CONFIG_FEC_XCV_TYPE		RMII
#define CONFIG_FEC_MXC_PHYADDR		1

#define IMX_FEC_BASE			0x29950000
#endif

#ifdef CONFIG_DISTRO_DEFAULTS
#define BOOT_TARGET_DEVICES(func) \
	func(MMC, mmc, 0)

#include <config_distro_bootcmd.h>
#else
#define BOOTENV
#endif

/* Initial environment variables */
#define CONFIG_EXTRA_ENV_SETTINGS		\
	BOOTENV \
	"scriptaddr=" __stringify(CONFIG_LOADADDR) "\0" \
	"kernel_addr_r=" __stringify(CONFIG_LOADADDR) "\0" \
	"image=Image\0" \
	"console=ttyLP1,115200 earlycon\0" \
	"fdt_addr_r=0x83000000\0"			\
	"boot_fit=no\0" \
	"fdtfile=imx8ulp-evk.dtb\0" \
	"initrd_addr=0x83800000\0"		\
	"bootm_size=0x10000000\0" \
	"mmcpart=" __stringify(CONFIG_SYS_MMC_IMG_LOAD_PART) "\0" \
	"mmcroot=" CONFIG_MMCROOT " rootwait rw\0" \

/* Link Definitions */
#define CONFIG_LOADADDR			0x80480000

#define CONFIG_SYS_LOAD_ADDR		CONFIG_LOADADDR

#define CONFIG_SYS_INIT_RAM_ADDR	0x80000000
#define CONFIG_SYS_INIT_RAM_SIZE	0x80000
#define CONFIG_SYS_INIT_SP_OFFSET	(CONFIG_SYS_INIT_RAM_SIZE - GENERATED_GBL_DATA_SIZE)
#define CONFIG_SYS_INIT_SP_ADDR		(CONFIG_SYS_INIT_RAM_ADDR + CONFIG_SYS_INIT_SP_OFFSET)

#define CONFIG_ENV_OVERWRITE
#define CONFIG_MMCROOT			"/dev/mmcblk2p2"

/* Size of malloc() pool */
#define CONFIG_SYS_MALLOC_LEN		(CONFIG_ENV_SIZE + SZ_16M)

#define CONFIG_SYS_SDRAM_BASE		0x80000000
#define PHYS_SDRAM			0x80000000
#define PHYS_SDRAM_SIZE			0x80000000 /* 2GB DDR */

/* Monitor Command Prompt */
#define CONFIG_SYS_PROMPT_HUSH_PS2	"> "
#define CONFIG_SYS_CBSIZE		2048
#define CONFIG_SYS_MAXARGS		64
#define CONFIG_SYS_BARGSIZE		CONFIG_SYS_CBSIZE
#define CONFIG_SYS_PBSIZE		(CONFIG_SYS_CBSIZE + sizeof(CONFIG_SYS_PROMPT) + 16)

/* Using ULP WDOG for reset */
#define WDOG_BASE_ADDR			WDG3_RBASE
#endif
