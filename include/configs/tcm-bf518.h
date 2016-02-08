/*
 * U-Boot - Configuration file for Bluetechnix TCM-BF518 board
 */

#ifndef __CONFIG_TCM_BF518_H__
#define __CONFIG_TCM_BF518_H__

#include <asm/config-pre.h>


/*
 * Processor Settings
 */
#define CONFIG_BFIN_CPU             bf518-0.0
#define CONFIG_BFIN_BOOT_MODE       BFIN_BOOT_PARA


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
#define CONFIG_VCO_MULT			16
/* CCLK_DIV controls the core clock divider				*/
/* Values can be 1, 2, 4, or 8 ONLY					*/
#define CONFIG_CCLK_DIV			1
/* SCLK_DIV controls the system clock divider				*/
/* Values can range from 1-15						*/
#define CONFIG_SCLK_DIV			4


/*
 * Memory Settings
 */
/* This board has a 32meg MT48H16M16 */
#define CONFIG_MEM_ADD_WDTH	9
#define CONFIG_MEM_SIZE		32

#define CONFIG_EBIU_SDRRC_VAL	0x3f8
#define CONFIG_EBIU_SDGCTL_VAL	0x9111cd

#define CONFIG_EBIU_AMGCTL_VAL	(AMBEN_ALL)
#define CONFIG_EBIU_AMBCTL0_VAL	(B1WAT_7 | B1RAT_11 | B1HT_2 | B1ST_3 | B0WAT_7 | B0RAT_11 | B0HT_2 | B0ST_3)
#define CONFIG_EBIU_AMBCTL1_VAL	(B3WAT_7 | B3RAT_11 | B3HT_2 | B3ST_3 | B2WAT_7 | B2RAT_11 | B2HT_2 | B2ST_3)

#define CONFIG_SYS_MONITOR_LEN	(512 * 1024)
#define CONFIG_SYS_MALLOC_LEN	(384 * 1024)


/*
 * Network Settings
 */
#if !defined(__ADSPBF512__) && !defined(__ADSPBF514__)
#define ADI_CMDS_NETWORK	1
#define CONFIG_BFIN_MAC
#define CONFIG_NETCONSOLE	1
#endif
#define CONFIG_HOSTNAME		tcm-bf518

/*
 * Flash Settings
 */
#define CONFIG_FLASH_CFI_DRIVER
#define CONFIG_SYS_FLASH_BASE		0x20000000
#define CONFIG_SYS_FLASH_CFI
#define CONFIG_SYS_FLASH_PROTECTION
#define CONFIG_SYS_MAX_FLASH_BANKS	1
#define CONFIG_SYS_MAX_FLASH_SECT	19


/*
 * SPI Settings
 */
#define CONFIG_BFIN_SPI
#define CONFIG_ENV_SPI_MAX_HZ	30000000
#define CONFIG_SF_DEFAULT_SPEED	30000000


/*
 * Env Storage Settings
 */
#define CONFIG_ENV_IS_IN_FLASH
#define CONFIG_ENV_OFFSET	0x8000
#define CONFIG_ENV_ADDR		(CONFIG_SYS_FLASH_BASE + CONFIG_ENV_OFFSET)
#define CONFIG_ENV_SIZE		0x2000
#define CONFIG_ENV_SECT_SIZE	0x8000
#define CONFIG_ENV_IS_EMBEDDED_IN_LDR


/*
 * I2C Settings
 */
#define CONFIG_SYS_I2C
#define CONFIG_SYS_I2C_ADI


/*
 * Misc Settings
 */
#define CONFIG_BAUDRATE		115200
#define CONFIG_RTC_BFIN
#define CONFIG_UART_CONSOLE	0
#define CONFIG_BOOTCOMMAND	"run flashboot"
#define FLASHBOOT_ENV_SETTINGS	"flashboot=bootm 0x20040000\0"

/*
 * Pull in common ADI header for remaining command/environment setup
 */
#include <configs/bfin_adi_common.h>

#endif
