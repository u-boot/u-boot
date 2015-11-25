/*
 * U-boot - Configuration file for IP04 board (having BF532 processor)
 *
 * Copyright (c) 2006 Intratrade Ltd., Ivan Danov, idanov@gmail.com
 *
 * Copyright (c) 2005-2010 Analog Devices Inc.
 *
 * (C) Copyright 2000-2004
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * Licensed under the GPL-2 or later.
 */

#ifndef __CONFIG_IP04_H__
#define __CONFIG_IP04_H__

#include <asm/config-pre.h>


/*
 * Processor Settings
 */
#define CONFIG_BFIN_CPU             bf532-0.5
#define CONFIG_BFIN_BOOT_MODE       BFIN_BOOT_NAND


/*
 * Clock Settings
 *	CCLK = (CLKIN * VCO_MULT) / CCLK_DIV
 *	SCLK = (CLKIN * VCO_MULT) / SCLK_DIV
 */
/* CONFIG_CLKIN_HZ is any value in Hz					*/
#define CONFIG_CLKIN_HZ			10000000
/* CLKIN_HALF controls the DF bit in PLL_CTL      0 = CLKIN		*/
/*                                                1 = CLKIN / 2		*/
#define CONFIG_CLKIN_HALF		0
/* PLL_BYPASS controls the BYPASS bit in PLL_CTL  0 = do not bypass	*/
/*                                                1 = bypass PLL	*/
#define CONFIG_PLL_BYPASS		0
/* VCO_MULT controls the MSEL (multiplier) bits in PLL_CTL		*/
/* Values can range from 0-63 (where 0 means 64)			*/
#define CONFIG_VCO_MULT			40
/* CCLK_DIV controls the core clock divider				*/
/* Values can be 1, 2, 4, or 8 ONLY					*/
#define CONFIG_CCLK_DIV			1
/* SCLK_DIV controls the system clock divider				*/
/* Values can range from 1-15						*/
#define CONFIG_SCLK_DIV			3


/*
 * Memory Settings
 */
#define CONFIG_MEM_ADD_WDTH	10
#define CONFIG_MEM_SIZE		64

#define CONFIG_EBIU_SDRRC_VAL	0x408
#define CONFIG_EBIU_SDGCTL_VAL	0x9111cd

#define CONFIG_EBIU_AMGCTL_VAL	0xFF
#define CONFIG_EBIU_AMBCTL0_VAL	0xffc2ffc2
#define CONFIG_EBIU_AMBCTL1_VAL	0xffc2ffc2

#define CONFIG_SYS_MONITOR_LEN		(384 * 1024)
#define CONFIG_SYS_MALLOC_LEN		(128 * 1024)


/*
 * Network Settings
 */
#define ADI_CMDS_NETWORK	1
#define CONFIG_HOSTNAME		IP04

#define CONFIG_DRIVER_DM9000	1
#define CONFIG_DM9000_NO_SROM
#define CONFIG_DM9000_BASE	0x20100000
#define DM9000_IO		CONFIG_DM9000_BASE
#define DM9000_DATA		(CONFIG_DM9000_BASE + 2)


/*
 * Flash Settings
 */
#define CONFIG_ENV_OVERWRITE	1
#define CONFIG_SYS_NO_FLASH		/* we have only NAND */


/*
 * SPI Settings
 */
#define CONFIG_BFIN_SPI
#define CONFIG_ENV_SPI_MAX_HZ	30000000
#define CONFIG_SF_DEFAULT_SPEED	30000000


/*
 * Env Storage Settings
 */
#define CONFIG_ENV_IS_IN_SPI_FLASH
#define CONFIG_PREBOOT		"echo starting from spi flash"
#define CONFIG_ENV_OFFSET	0x30000
#define CONFIG_ENV_SIZE		0x10000
#define CONFIG_ENV_SECT_SIZE	0x10000


/*
 * NAND Settings
 */
#define CONFIG_NAND_PLAT
#define CONFIG_SYS_NAND_BASE		0x20000000
#define CONFIG_SYS_MAX_NAND_DEVICE	1

#define BFIN_NAND_CLE(chip) ((unsigned long)(chip)->IO_ADDR_W | (1 << 2))
#define BFIN_NAND_ALE(chip) ((unsigned long)(chip)->IO_ADDR_W | (1 << 1))
#define BFIN_NAND_WRITE(addr, cmd) \
	do { \
		bfin_write8(addr, cmd); \
		SSYNC(); \
	} while (0)

#define NAND_PLAT_WRITE_CMD(chip, cmd) BFIN_NAND_WRITE(BFIN_NAND_CLE(chip), cmd)
#define NAND_PLAT_WRITE_ADR(chip, cmd) BFIN_NAND_WRITE(BFIN_NAND_ALE(chip), cmd)
#define NAND_PLAT_GPIO_DEV_READY       GPIO_PF10


/*
 * Misc Settings
 */
#define CONFIG_BAUDRATE		115200
#define CONFIG_UART_CONSOLE	0

#undef CONFIG_SHOW_BOOT_PROGRESS
/* Enable this if bootretry required; currently it's disabled */
#define CONFIG_BOOT_RETRY_TIME	-1
#define CONFIG_BOOTCOMMAND	"run nandboot"


/*
 * Pull in common ADI header for remaining command/environment setup
 */
#include <configs/bfin_adi_common.h>

#endif
