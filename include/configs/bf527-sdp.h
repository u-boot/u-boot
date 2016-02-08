/*
 * U-Boot - Configuration file for BF527 SDP board
 */

#ifndef __CONFIG_BF527_SDP_H__
#define __CONFIG_BF527_SDP_H__

#include <asm/config-pre.h>


/*
 * Processor Settings
 */
#define CONFIG_BFIN_CPU             bf527-0.2
#define CONFIG_BFIN_BOOT_MODE       BFIN_BOOT_PARA


/*
 * Clock Settings
 *	CCLK = (CLKIN * VCO_MULT) / CCLK_DIV
 *	SCLK = (CLKIN * VCO_MULT) / SCLK_DIV
 */
/* CONFIG_CLKIN_HZ is any value in Hz					*/
#define CONFIG_CLKIN_HZ			24000000
/* CLKIN_HALF controls the DF bit in PLL_CTL      0 = CLKIN		*/
/*                                                1 = CLKIN / 2		*/
#define CONFIG_CLKIN_HALF		0
/* PLL_BYPASS controls the BYPASS bit in PLL_CTL  0 = do not bypass	*/
/*                                                1 = bypass PLL	*/
#define CONFIG_PLL_BYPASS		0
/* VCO_MULT controls the MSEL (multiplier) bits in PLL_CTL		*/
/* Values can range from 0-63 (where 0 means 64)			*/
#define CONFIG_VCO_MULT			25
/* CCLK_DIV controls the core clock divider				*/
/* Values can be 1, 2, 4, or 8 ONLY					*/
#define CONFIG_CCLK_DIV			1
/* SCLK_DIV controls the system clock divider				*/
/* Values can range from 1-15						*/
#define CONFIG_SCLK_DIV			5

#define CONFIG_PLL_LOCKCNT_VAL	0x0200
#define CONFIG_PLL_CTL_VAL		0x2a00
#define CONFIG_VR_CTL_VAL		0x7090


/*
 * Memory Settings
 */
#define CONFIG_MEM_ADD_WDTH	9
#define CONFIG_MEM_SIZE		32

#define CONFIG_EBIU_SDRRC_VAL	0x00FE
#define CONFIG_EBIU_SDGCTL_VAL	0x8011998d

#define CONFIG_EBIU_AMGCTL_VAL	(AMCKEN | AMBEN_ALL)
#define CONFIG_EBIU_AMBCTL0_VAL	(B1WAT_15 | B1RAT_15 | B1HT_3 | B1RDYPOL | B0WAT_15 | B0RAT_15 | B0HT_3 | B0RDYPOL)
#define CONFIG_EBIU_AMBCTL1_VAL	(B3WAT_15 | B3RAT_15 | B3HT_3 | B3RDYPOL | B2WAT_15 | B2RAT_15 | B2HT_3 | B2RDYPOL)

#define CONFIG_SYS_MONITOR_LEN	(768 * 1024)
#define CONFIG_SYS_MALLOC_LEN	(640 * 1024)


/*
 * Flash Settings
 */
#define CONFIG_FLASH_CFI_DRIVER
#define CONFIG_SYS_FLASH_BASE		0x20000000
#define CONFIG_SYS_FLASH_CFI
#define CONFIG_SYS_FLASH_PROTECTION
#define CONFIG_SYS_MAX_FLASH_BANKS	1
#define CONFIG_SYS_MAX_FLASH_SECT	259


/*
 * SPI Settings
 */
#define CONFIG_BFIN_SPI
#define CONFIG_ENV_SPI_MAX_HZ	30000000
#define CONFIG_SF_DEFAULT_SPEED	30000000
#define CONFIG_SPI_FLASH_ALL


/*
 * Env Storage Settings
 */
#if (CONFIG_BFIN_BOOT_MODE == BFIN_BOOT_SPI_MASTER)
#define CONFIG_ENV_IS_IN_SPI_FLASH
#define CONFIG_ENV_OFFSET	0x10000
#define CONFIG_ENV_SIZE		0x2000
#define CONFIG_ENV_SECT_SIZE	0x10000
#define CONFIG_ENV_IS_EMBEDDED_IN_LDR
#else
#define CONFIG_ENV_IS_IN_FLASH
#define CONFIG_ENV_OFFSET	0x4000
#define CONFIG_ENV_ADDR		(CONFIG_SYS_FLASH_BASE + CONFIG_ENV_OFFSET)
#define CONFIG_ENV_SIZE		0x2000
#define CONFIG_ENV_SECT_SIZE	0x2000
#define CONFIG_ENV_IS_EMBEDDED_IN_LDR
#endif


/*
 * I2C Settings
 */
#define CONFIG_SYS_I2C
#define CONFIG_SYS_I2C_ADI


/*
 * Misc Settings
 */
#define CONFIG_MISC_INIT_R
#define CONFIG_UART_CONSOLE	0

/*
 * Pull in common ADI header for remaining command/environment setup
 */
#include <configs/bfin_adi_common.h>

#endif
