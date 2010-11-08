/*
 * U-boot - Configuration file for BF561 Acvilon System On Module
 * For more information please go to http://www.niistt.ru/
 */

#ifndef __CONFIG_BF561_ACVILON_H__
#define __CONFIG_BF561_ACVILON_H__

#include <asm/config-pre.h>


/*
 * Processor Settings
 */
#define CONFIG_BFIN_CPU				bf561-0.5
#define CONFIG_BFIN_BOOT_MODE		BFIN_BOOT_BYPASS


/*
 * Clock Settings
 *	CCLK = (CLKIN * VCO_MULT) / CCLK_DIV
 *	SCLK = (CLKIN * VCO_MULT) / SCLK_DIV
 */
/* CONFIG_CLKIN_HZ is any value in Hz					*/
#define CONFIG_CLKIN_HZ				12000000
/* CLKIN_HALF controls the DF bit in PLL_CTL      0 = CLKIN		*/
/*                                                1 = CLKIN / 2		*/
#define CONFIG_CLKIN_HALF			0
/* PLL_BYPASS controls the BYPASS bit in PLL_CTL  0 = do not bypass	*/
/*                                                1 = bypass PLL	*/
#define CONFIG_PLL_BYPASS			0
/* VCO_MULT controls the MSEL (multiplier) bits in PLL_CTL		*/
/* Values can range from 0-63 (where 0 means 64)			*/
#define CONFIG_VCO_MULT				50
/* CCLK_DIV controls the core clock divider				*/
/* Values can be 1, 2, 4, or 8 ONLY					*/
#define CONFIG_CCLK_DIV				1
/* SCLK_DIV controls the system clock divider				*/
/* Values can range from 1-15						*/
#define CONFIG_SCLK_DIV				5


/*
 * Memory Settings
 */
#define CONFIG_MEM_ADD_WDTH			10
#define CONFIG_MEM_SIZE				128

#define CONFIG_EBIU_SDRRC_VAL		0x300
#define CONFIG_EBIU_SDGCTL_VAL		0x00B11189

#define CONFIG_EBIU_AMGCTL_VAL		0x4e
#define CONFIG_EBIU_AMBCTL0_VAL		0xffc2ffc2
#define CONFIG_EBIU_AMBCTL1_VAL		0x99b35554

#define CONFIG_SYS_MONITOR_LEN		(256 * 1024)
#define CONFIG_SYS_MALLOC_LEN		(128 * 1024)


/*
 * RTC Settings
 */
#define CONFIG_RTC_DS1337
#define CONFIG_SYS_I2C_RTC_ADDR 0x68

/* I2C SYSMON (LM75, AD7414 is almost compatible)                       */
#define CONFIG_DTT_LM75         1               /* ON Semi's LM75       */
#define CONFIG_DTT_SENSORS      {0}             /* Sensor addresses     */
#define CONFIG_SYS_I2C_DTT_ADDR 0x49
/*#define CONFIG_SYS_DTT_MAX_TEMP 70
#define CONFIG_SYS_DTT_LOW_TEMP -30
#define CONFIG_SYS_DTT_HYSTERESIS       3*/


/*
 * Network Settings
 */
#define ADI_CMDS_NETWORK			1
#define CONFIG_NET_MULTI
#define CONFIG_CMD_NET
#define CONFIG_CMD_MII
#define CONFIG_CMD_DATE
#define CONFIG_CMD_DTT

#if defined(CONFIG_CMD_NET)

#define CONFIG_SMC911X				1
#define CONFIG_SMC911X_32_BIT
/* #define CONFIG_SMC911X_16_BIT */
#define CONFIG_SMC911X_BASE			0x28000000

#endif /* (CONFIG_CMD_NET) */

#define CONFIG_HOSTNAME		bf561-acvilon

/* Uncomment next line to use fixed MAC address */
/* #define CONFIG_ETHADDR				02:80:ad:20:31:e8 */


/*
 * Flash Settings
 */
#define CONFIG_SYS_NO_FLASH


/*
 * I2C Settings
 */
#define CONFIG_HARD_I2C
/* Use 300kHz speed by default */
#define CONFIG_SYS_I2C_SPEED			0x00
#define CONFIG_PCA9564_I2C
#define CONFIG_PCA9564_BASE			0x2c000000


/*
 * SPI Settings
 */
#define CONFIG_BFIN_SPI
#define CONFIG_ENV_SPI_MAX_HZ		10000000
#define CONFIG_SF_DEFAULT_SPEED		10000000
#define CONFIG_SPI_FLASH
#define CONFIG_SPI_FLASH_ATMEL


/*
 * Env Storage Settings
 */
#define CONFIG_ENV_IS_IN_SPI_FLASH
/* #define CONFIG_CMD_SAVEENV */
#define CONFIG_ENV_SECT_SIZE		(1056 * 8)
#define CONFIG_ENV_OFFSET			((16 + 256) * 1056)
#define CONFIG_ENV_SIZE				(8 * 1056)


/*
 * NAND Settings
 * We're using NAND_PLAT driver to make things simplier
 */
#define CONFIG_NAND_PLAT
#define CONFIG_CMD_NAND
#define CONFIG_SYS_NAND_BASE		0x24000000
#define CONFIG_SYS_MAX_NAND_DEVICE	1

#define BFIN_NAND_CLE(chip) ((unsigned long)(chip)->IO_ADDR_W | (1 << 2))
#define BFIN_NAND_ALE(chip) ((unsigned long)(chip)->IO_ADDR_W | (1 << 3))
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
#define CONFIG_UART_CONSOLE			0
#define CONFIG_BAUDRATE				57600
#define CONFIG_SYS_PROMPT "Acvilon> "


/*
 * Pull in common ADI header for remaining command/environment setup
 */
#include <configs/bfin_adi_common.h>

#endif				/* __CONFIG_BF561_ACVILON_H__ */
