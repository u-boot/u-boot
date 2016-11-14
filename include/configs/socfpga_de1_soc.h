/*
 * Copyright (C) 2016 Marek Vasut <marex@denx.de>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */
#ifndef __CONFIG_TERASIC_DE1_SOC_H__
#define __CONFIG_TERASIC_DE1_SOC_H__

#include <asm/arch/base_addr_ac5.h>

/* U-Boot Commands */
#define CONFIG_SYS_NO_FLASH
#define CONFIG_DOS_PARTITION
#define CONFIG_FAT_WRITE
#define CONFIG_HW_WATCHDOG

/* Memory configurations */
#define PHYS_SDRAM_1_SIZE		0x40000000	/* 1GiB */

/* Booting Linux */
#define CONFIG_BOOTFILE		"fitImage"
#define CONFIG_BOOTARGS		"console=ttyS0," __stringify(CONFIG_BAUDRATE)
#define CONFIG_BOOTCOMMAND	"run mmcload; run mmcboot"
#define CONFIG_LOADADDR		0x01000000
#define CONFIG_SYS_LOAD_ADDR	CONFIG_LOADADDR

/* Ethernet on SoC (EMAC) */
#if defined(CONFIG_CMD_NET)
#define CONFIG_PHY_MICREL
#define CONFIG_PHY_MICREL_KSZ9021
#endif

#define CONFIG_ENV_IS_IN_MMC

/* Extra Environment */
#define CONFIG_EXTRA_ENV_SETTINGS \
	"loadaddr=" __stringify(CONFIG_SYS_LOAD_ADDR) "\0" \
	"ramboot=setenv bootargs " CONFIG_BOOTARGS ";" \
		"bootm ${loadaddr} - ${fdtaddr}\0" \
	"bootimage=zImage\0" \
	"fdtaddr=100\0" \
	"fdtimage=socfpga.dtb\0" \
	"bootm ${loadaddr} - ${fdtaddr}\0" \
	"mmcroot=/dev/mmcblk0p2\0" \
	"mmcboot=setenv bootargs " CONFIG_BOOTARGS \
		" root=${mmcroot} rw rootwait;" \
		"bootz ${loadaddr} - ${fdtaddr}\0" \
	"mmcload=mmc rescan;" \
		"load mmc 0:1 ${loadaddr} ${bootimage};" \
		"load mmc 0:1 ${fdtaddr} ${fdtimage}\0" \

/* The rest of the configuration is shared */
#include <configs/socfpga_common.h>

#define CONFIG_SPL_ABORT_ON_RAW_IMAGE

#endif	/* __CONFIG_TERASIC_DE1_SOC_H__ */
