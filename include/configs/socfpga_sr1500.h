/*
 * Copyright (C) 2015 Stefan Roese <sr@denx.de>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */
#ifndef __CONFIG_SOCFPGA_SR1500_H__
#define __CONFIG_SOCFPGA_SR1500_H__

#include <asm/arch/base_addr_ac5.h>

#define CONFIG_HW_WATCHDOG

/* Memory configurations */
#define PHYS_SDRAM_1_SIZE		0x40000000	/* 1GiB on SR1500 */

/* Booting Linux */
#define CONFIG_LOADADDR		0x01000000
#define CONFIG_SYS_LOAD_ADDR	CONFIG_LOADADDR

/* Ethernet on SoC (EMAC) */
#define CONFIG_PHY_INTERFACE_MODE	PHY_INTERFACE_MODE_RGMII
/* The PHY is autodetected, so no MII PHY address is needed here */
#define CONFIG_PHY_MARVELL
#define PHY_ANEG_TIMEOUT	8000

/* Environment */
#define CONFIG_ENV_IS_IN_SPI_FLASH

/* Enable SPI NOR flash reset, needed for SPI booting */
#define CONFIG_SPI_N25Q256A_RESET

/*
 * Bootcounter
 */
#define CONFIG_BOOTCOUNT_LIMIT
/* last 2 lwords in OCRAM */
#define CONFIG_SYS_BOOTCOUNT_ADDR	0xfffffff8
#define CONFIG_SYS_BOOTCOUNT_BE

/* Environment setting for SPI flash */
#define CONFIG_SYS_REDUNDAND_ENVIRONMENT
#define CONFIG_ENV_SECT_SIZE	(64 * 1024)
#define CONFIG_ENV_SIZE		(16 * 1024)
#define CONFIG_ENV_OFFSET	0x000e0000
#define CONFIG_ENV_OFFSET_REDUND (CONFIG_ENV_OFFSET + CONFIG_ENV_SECT_SIZE)
#define CONFIG_ENV_SPI_BUS	0
#define CONFIG_ENV_SPI_CS	0
#define CONFIG_ENV_SPI_MODE	SPI_MODE_3
#define CONFIG_ENV_SPI_MAX_HZ	100000000	/* Use max of 100MHz */
#define CONFIG_SF_DEFAULT_SPEED	100000000

/*
 * The QSPI NOR flash layout on SR1500:
 *
 * 0000.0000 - 0003.ffff: SPL (4 times)
 * 0004.0000 - 000d.ffff: U-Boot
 * 000e.0000 - 000e.ffff: env1
 * 000f.0000 - 000f.ffff: env2
 */

/* The rest of the configuration is shared */
#include <configs/socfpga_common.h>

#endif	/* __CONFIG_SOCFPGA_SR1500_H__ */
