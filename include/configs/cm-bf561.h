/*
 * U-boot - Configuration file for CM-BF561 board
 */

#ifndef __CONFIG_CM_BF561_H__
#define __CONFIG_CM_BF561_H__

#include <asm/config-pre.h>


/*
 * Processor Settings
 */
#define CONFIG_BFIN_CPU             bf561-0.3
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
#define CONFIG_VCO_MULT			22
/* CCLK_DIV controls the core clock divider				*/
/* Values can be 1, 2, 4, or 8 ONLY					*/
#define CONFIG_CCLK_DIV			1
/* SCLK_DIV controls the system clock divider				*/
/* Values can range from 1-15						*/
#define CONFIG_SCLK_DIV			5


/*
 * Memory Settings
 */
#define CONFIG_MEM_ADD_WDTH	9
#define CONFIG_MEM_SIZE		64

#define CONFIG_EBIU_SDRRC_VAL	((((CONFIG_SCLK_HZ / 1000) * 64) / 4096) - (7 + 2))
#define CONFIG_EBIU_SDGCTL_VAL	(SCTLE | PSS | TWR_2 | TRCD_2 | TRP_2 | TRAS_7 | PASR_ALL | CL_3)

#define CONFIG_EBIU_AMGCTL_VAL	(CDPRIO | B3_PEN | B2_PEN | B1_PEN | B0_PEN | AMBEN_ALL | AMCKEN)
#define CONFIG_EBIU_AMBCTL0_VAL	(B1WAT_7 | B1RAT_11 | B1HT_2 | B1ST_3 | B0WAT_7 | B0RAT_11 | B0HT_2 | B0ST_3)
#define CONFIG_EBIU_AMBCTL1_VAL	(B3WAT_7 | B3RAT_11 | B3HT_2 | B3ST_3 | B2WAT_7 | B2RAT_11 | B2HT_2 | B2ST_3)

#define CONFIG_SYS_MONITOR_LEN	(256 * 1024)
#define CONFIG_SYS_MALLOC_LEN	(128 * 1024)


/*
 * Network Settings
 */
#define ADI_CMDS_NETWORK	1
/* The next 2 lines are for use with DEV-BF5xx */
#define CONFIG_DRIVER_SMC91111	1
#define CONFIG_SMC91111_BASE	0x28000300
/* The next 3 lines are for use with EXT-BF5xx-USB-ETH2 */
/* #define CONFIG_DRIVER_SMC911X 1 */
/* #define CONFIG_DRIVER_SMC911X_BASE 0x24080000 /! AMS1 */
/* #define CONFIG_DRIVER_SMC911X_32_BIT 1 */
#define CONFIG_HOSTNAME		cm-bf561
/* Uncomment next line to use fixed MAC address */
/* #define CONFIG_ETHADDR	02:80:ad:20:31:cf */


/*
 * Flash Settings
 */
#define CONFIG_FLASH_CFI_DRIVER
#define CONFIG_SYS_FLASH_BASE		0x20000000
#define CONFIG_SYS_FLASH_CFI
#define CONFIG_SYS_FLASH_PROTECTION
#define CONFIG_SYS_MAX_FLASH_BANKS	1
#define CONFIG_SYS_MAX_FLASH_SECT	67


/*
 * Env Storage Settings
 */
#define CONFIG_ENV_IS_IN_FLASH	1
#define CONFIG_ENV_OFFSET	0x20000
#define CONFIG_ENV_SECT_SIZE	0x20000
#define CONFIG_ENV_SIZE		0x10000


/*
 * Misc Settings
 */
#define CONFIG_BAUDRATE		115200
#define CONFIG_UART_CONSOLE	0


/*
 * Pull in common ADI header for remaining command/environment setup
 */
#include <configs/bfin_adi_common.h>

#endif
