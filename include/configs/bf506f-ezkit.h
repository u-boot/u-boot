/*
 * U-Boot - Configuration file for BF506F EZ-Kit board
 */

#ifndef __CONFIG_BF506F_EZKIT_H__
#define __CONFIG_BF506F_EZKIT_H__

#include <asm/config-pre.h>

/*
 * Processor Settings
 */
#define CONFIG_BFIN_CPU             bf506-0.0
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
#define CONFIG_SCLK_DIV			5

/*
 * Memory Settings
 */
#define CONFIG_MEM_SIZE		0

#define CONFIG_EBIU_AMGCTL_VAL	(AMCKEN | AMBEN_ALL)
#define CONFIG_EBIU_AMBCTL0_VAL	0xffc2ffc2
#define CONFIG_EBIU_AMBCTL1_VAL	0xffc2ffc2

#define CONFIG_SYS_MONITOR_BASE (L1_DATA_A_SRAM_END)
#define CONFIG_SYS_MONITOR_LEN	(4 * 1024)
#define CONFIG_SYS_MALLOC_LEN	(4 * 1024)

/*
 * Flash Settings
 */
/*
#define CONFIG_FLASH_CFI_DRIVER
#define CONFIG_SYS_FLASH_BASE		0x20000000
#define CONFIG_SYS_FLASH_CFI
#define CONFIG_SYS_MAX_FLASH_BANKS	1
#define CONFIG_SYS_MAX_FLASH_SECT	71
#define CONFIG_MONITOR_IS_IN_RAM
*/
#define CONFIG_SYS_NO_FLASH

/*
 * SPI Settings
 */
#define CONFIG_BFIN_SPI
#define CONFIG_ENV_SPI_MAX_HZ	30000000
#define CONFIG_SF_DEFAULT_SPEED	30000000

/*
 * Env Storage Settings
 */
#define CONFIG_ENV_IS_NOWHERE
#define CONFIG_ENV_SIZE 0x400

/*
 * Misc Settings
 */
#define CONFIG_BOARD_EARLY_INIT_F
#define CONFIG_ICACHE_OFF
#define CONFIG_DCACHE_OFF
#define CONFIG_UART_CONSOLE	0
#define CONFIG_BAUDRATE 115200
#define CONFIG_BFIN_SERIAL

#undef CONFIG_GZIP
#undef CONFIG_ZLIB
#undef CONFIG_BOOTM_RTEMS
#undef CONFIG_BOOTM_LINUX

#endif
