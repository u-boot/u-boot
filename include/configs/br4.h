/*
 * U-boot - Configuration file for BR4 Appliance
 *
 * based on bf537-stamp.h
 * Copyright (c) Switchfin Org. <dpn@switchfin.org>
 */

#ifndef __CONFIG_BR4_H__
#define __CONFIG_BR4_H__

#include <asm/config-pre.h>


/*
 * Processor Settings
 */
#define CONFIG_BFIN_CPU             bf537-0.3
#define CONFIG_BFIN_BOOT_MODE       BFIN_BOOT_SPI_MASTER


/*
 * Clock Settings
 *	CCLK = (CLKIN * VCO_MULT) / CCLK_DIV
 *	SCLK = (CLKIN * VCO_MULT) / SCLK_DIV
 */
/* CONFIG_CLKIN_HZ is any value in Hz					*/
#define CONFIG_CLKIN_HZ			25000000
/* CLKIN_HALF controls the DF bit in PLL_CTL      0 = CLKIN		*/
/*                                                1 = CLKIN / 2		*/
#define CONFIG_CLKIN_HALF		0
/* PLL_BYPASS controls the BYPASS bit in PLL_CTL  0 = do not bypass	*/
/*                                                1 = bypass PLL	*/
#define CONFIG_PLL_BYPASS		0
/* VCO_MULT controls the MSEL (multiplier) bits in PLL_CTL		*/
/* Values can range from 0-63 (where 0 means 64)			*/
#define CONFIG_VCO_MULT			24
/* CCLK_DIV controls the core clock divider				*/
/* Values can be 1, 2, 4, or 8 ONLY					*/
#define CONFIG_CCLK_DIV			1
/* SCLK_DIV controls the system clock divider				*/
/* Values can range from 1-15						*/
#define CONFIG_SCLK_DIV			5


/*
 * Memory Settings
 */
#define CONFIG_MEM_ADD_WDTH	10
#define CONFIG_MEM_SIZE		64

#define CONFIG_EBIU_SDRRC_VAL	0x306
#define CONFIG_EBIU_SDGCTL_VAL	0x8091998d

#define CONFIG_EBIU_AMGCTL_VAL	0xFF
#define CONFIG_EBIU_AMBCTL0_VAL	0x7BB07BB0
#define CONFIG_EBIU_AMBCTL1_VAL	0xFFC27BB0

#define CONFIG_SYS_MONITOR_LEN		(512 * 1024)
#define CONFIG_SYS_MALLOC_LEN		(384 * 1024)


/*
 * Network Settings
 */
#ifndef __ADSPBF534__
#define ADI_CMDS_NETWORK	1
#define CONFIG_BFIN_MAC
#define CONFIG_NETCONSOLE
#endif
#define CONFIG_HOSTNAME		br4
#define CONFIG_TFTP_BLOCKSIZE	4404
/* Uncomment next line to use fixed MAC address */
/* #define CONFIG_ETHADDR	5c:38:1a:80:a7:00 */


/*
 * Flash Settings
 */
#define CONFIG_SYS_NO_FLASH	/* We have no parallel FLASH */


/*
 * SPI Settings
 */
#define CONFIG_BFIN_SPI
#define CONFIG_ENV_SPI_MAX_HZ	30000000
#define CONFIG_SF_DEFAULT_SPEED	30000000
#define CONFIG_SPI_FLASH
#define CONFIG_SPI_FLASH_STMICRO


/*
 * Env Storage Settings
 */
#define CONFIG_ENV_IS_IN_SPI_FLASH
#define CONFIG_ENV_OFFSET	0x10000
#define CONFIG_ENV_SIZE		0x2000
#define CONFIG_ENV_SECT_SIZE	0x10000
#define CONFIG_ENV_IS_EMBEDDED_IN_LDR


/*
 * I2C Settings
 */
#define CONFIG_SYS_I2C
#define CONFIG_SYS_I2C_ADI


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
#define NAND_PLAT_GPIO_DEV_READY       GPIO_PF9

/*
 * Misc Settings
 */
#define CONFIG_BAUDRATE		115200
#define CONFIG_RTC_BFIN
#define CONFIG_UART_CONSOLE	0
#define CONFIG_SYS_PROMPT	"br4>"
#define CONFIG_BOOTCOMMAND	"run nandboot"
#define CONFIG_BOOTDELAY	2
#define CONFIG_LOADADDR		0x2000000

/*
 * Pull in common ADI header for remaining command/environment setup
 */
#include <configs/bfin_adi_common.h>

/*
 * Overwrite some settings defined in bfin_adi_common.h
 */
#undef NAND_ENV_SETTINGS
#define NAND_ENV_SETTINGS \
	"nandargs=set bootargs " CONFIG_BOOTARGS "\0" \
	"nandboot=" \
		"nand read $(loadaddr) 0x0 0x900000;" \
		"run nandargs;" \
		"bootm" \
		"\0"

#endif
