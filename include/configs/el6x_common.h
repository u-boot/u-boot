/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) Stefano Babic <sbabic@denx.de>
 *
 * Configuration settings for the E+L i.MX6Q DO82 board.
 */

#ifndef __EL6Q_COMMON_CONFIG_H
#define __EL6Q_COMMON_CONFIG_H

#include <linux/stringify.h>

#define CONFIG_BOARD_NAME		EL6Q

#include "mx6_common.h"

#ifdef CONFIG_SPL
#include "imx6_spl.h"
#endif

/* MMC Configs */
#define CONFIG_SYS_FSL_ESDHC_ADDR	0
#define CONFIG_SYS_FSL_USDHC_NUM	2

/* PMIC */
#define CONFIG_POWER_PFUZE100
#define CONFIG_POWER_PFUZE100_I2C_ADDR	0x08

/* Commands */

#define CONFIG_MXC_UART_BASE	UART2_BASE

#define CONFIG_BOARD_NAME	EL6Q

#define CONFIG_EXTRA_ENV_SETTINGS                                               \
	"board="__stringify(CONFIG_BOARD_NAME)"\0"                              \
	"cma_size="__stringify(EL6Q_CMA_SIZE)"\0"                               \
	"chp_size="__stringify(EL6Q_COHERENT_POOL_SIZE)"\0"                     \
	"console=" CONSOLE_DEV "\0"					\
	"fdtfile=undefined\0" \
	"fdt_high=0xffffffff\0" \
	"fdt_addr_r=0x18000000\0" \
	"fdt_addr=0x18000000\0" \
	"findfdt=setenv fdtfile " CONFIG_DEFAULT_FDT_FILE "\0"                  \
	"kernel_addr_r=" __stringify(CONFIG_SYS_LOAD_ADDR) "\0" \
	"scriptaddr=" __stringify(CONFIG_SYS_LOAD_ADDR) "\0" \
	"pxefile_addr_r=" __stringify(CONFIG_SYS_LOAD_ADDR) "\0" \
	BOOTENV

#define BOOT_TARGET_DEVICES(func) \
	func(MMC, mmc, 0) \
	func(MMC, mmc, 1) \
	func(PXE, PXE, na) \
	func(DHCP, dhcp, na)

#include <config_distro_bootcmd.h>

#define CONFIG_ARP_TIMEOUT     200UL

/* Physical Memory Map */
#define PHYS_SDRAM                     MMDC0_ARB_BASE_ADDR

#define CONFIG_SYS_SDRAM_BASE          PHYS_SDRAM
#define CONFIG_SYS_INIT_RAM_ADDR       IRAM_BASE_ADDR
#define CONFIG_SYS_INIT_RAM_SIZE       IRAM_SIZE

#define CONFIG_SYS_INIT_SP_OFFSET \
	(CONFIG_SYS_INIT_RAM_SIZE - GENERATED_GBL_DATA_SIZE)
#define CONFIG_SYS_INIT_SP_ADDR \
	(CONFIG_SYS_INIT_RAM_ADDR + CONFIG_SYS_INIT_SP_OFFSET)

/* environment organization */

#endif                         /* __EL6Q_COMMON_CONFIG_H */
