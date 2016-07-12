/*
 * Copyright (C) 2014 Marek Vasut <marex@denx.de>
 * Copyright (C) 2016 Pavel Machek <pavel@denx.de>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */
#ifndef __CONFIG_SOCFPGA_IS1_H__
#define __CONFIG_SOCFPGA_IS1_H__

#include <asm/arch/base_addr_ac5.h>

/* U-Boot Commands */
#define CONFIG_SYS_NO_FLASH
#define CONFIG_DOS_PARTITION
#define CONFIG_FAT_WRITE
#define CONFIG_HW_WATCHDOG

/* Memory configurations */
#define PHYS_SDRAM_1_SIZE		0x10000000

/* Booting Linux */
#define CONFIG_BOOTFILE		"zImage"
#define CONFIG_BOOTARGS		"console=ttyS0," __stringify(CONFIG_BAUDRATE)
#define CONFIG_LOADADDR		0x01000000
#define CONFIG_SYS_LOAD_ADDR	CONFIG_LOADADDR
#define CONFIG_ENV_IS_IN_SPI_FLASH

/* Ethernet on SoC (EMAC) */
#if defined(CONFIG_CMD_NET)
#define CONFIG_ARP_TIMEOUT		500UL

/* PHY */
#define CONFIG_PHY_MICREL
#define CONFIG_PHY_MICREL_KSZ9021
#endif

/* The rest of the configuration is shared */
#include <configs/socfpga_common.h>

/*
 * Bootcounter
 */
#define CONFIG_BOOTCOUNT_LIMIT
/* last 2 lwords in OCRAM */
#define CONFIG_SYS_BOOTCOUNT_ADDR       0xfffffff8
#define CONFIG_SYS_BOOTCOUNT_BE

#endif	/* __CONFIG_SOCFPGA_IS1_H__ */
