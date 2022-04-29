/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2019 NXP
 */

#ifndef __IMX8MP_EVK_H
#define __IMX8MP_EVK_H

#include <linux/sizes.h>
#include <linux/stringify.h>
#include <asm/arch/imx-regs.h>

#define CONFIG_SYS_BOOTM_LEN		(32 * SZ_1M)

#define CONFIG_SPL_MAX_SIZE		(152 * 1024)
#define CONFIG_SYS_MONITOR_LEN		(512 * 1024)
#define CONFIG_SYS_UBOOT_BASE	(QSPI0_AMBA_BASE + CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_SECTOR * 512)

#ifdef CONFIG_SPL_BUILD
/*#define CONFIG_ENABLE_DDR_TRAINING_DEBUG*/
#define CONFIG_SPL_STACK		0x960000
#define CONFIG_SPL_BSS_START_ADDR	0x0098FC00
#define CONFIG_SPL_BSS_MAX_SIZE		0x400	/* 1 KB */
#define CONFIG_SYS_SPL_MALLOC_START	0x42200000
#define CONFIG_SYS_SPL_MALLOC_SIZE	SZ_512K	/* 512 KB */

#define CONFIG_SPL_ABORT_ON_RAW_IMAGE

#define CONFIG_POWER_PCA9450

#endif

#if defined(CONFIG_CMD_NET)
#define CONFIG_FEC_MXC_PHYADDR          1

#define DWC_NET_PHYADDR			1

#define PHY_ANEG_TIMEOUT 20000

#endif

#ifndef CONFIG_SPL_BUILD
#define BOOT_TARGET_DEVICES(func) \
       func(MMC, mmc, 1) \
       func(MMC, mmc, 2)

#include <config_distro_bootcmd.h>
#endif

/* Initial environment variables */
#define CONFIG_EXTRA_ENV_SETTINGS		\
	BOOTENV \
	"scriptaddr=" __stringify(CONFIG_SYS_LOAD_ADDR) "\0" \
	"kernel_addr_r=" __stringify(CONFIG_SYS_LOAD_ADDR) "\0" \
	"image=Image\0" \
	"console=ttymxc1,115200 earlycon=ec_imx6q,0x30890000,115200\0" \
	"fdt_addr_r=0x43000000\0"			\
	"boot_fdt=try\0" \
	"fdtfile=" CONFIG_DEFAULT_FDT_FILE "\0" \
	"initrd_addr=0x43800000\0"		\
	"bootm_size=0x10000000\0" \
	"mmcpart=1\0" \
	"mmcroot=/dev/mmcblk1p2 rootwait rw\0" \

/* Link Definitions */

#define CONFIG_SYS_INIT_RAM_ADDR	0x40000000
#define CONFIG_SYS_INIT_RAM_SIZE	0x80000
#define CONFIG_SYS_INIT_SP_OFFSET \
	(CONFIG_SYS_INIT_RAM_SIZE - GENERATED_GBL_DATA_SIZE)
#define CONFIG_SYS_INIT_SP_ADDR \
	(CONFIG_SYS_INIT_RAM_ADDR + CONFIG_SYS_INIT_SP_OFFSET)


/* Totally 6GB DDR */
#define CONFIG_SYS_SDRAM_BASE		0x40000000
#define PHYS_SDRAM			0x40000000
#define PHYS_SDRAM_SIZE			0xC0000000	/* 3 GB */
#define PHYS_SDRAM_2			0x100000000
#define PHYS_SDRAM_2_SIZE		0xC0000000	/* 3 GB */

#define CONFIG_MXC_UART_BASE		UART_BASE_ADDR(2)

/* Monitor Command Prompt */
#define CONFIG_SYS_CBSIZE		2048
#define CONFIG_SYS_MAXARGS		64
#define CONFIG_SYS_BARGSIZE CONFIG_SYS_CBSIZE
#define CONFIG_SYS_PBSIZE		(CONFIG_SYS_CBSIZE + \
					sizeof(CONFIG_SYS_PROMPT) + 16)

#endif
