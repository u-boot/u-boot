/*
 * U-boot - Configuration file for BF526 EZBrd board
 */

#ifndef __CONFIG_BF526_EZBRD_H__
#define __CONFIG_BF526_EZBRD_H__

#include <asm/config-pre.h>


/*
 * Processor Settings
 */
#define CONFIG_BFIN_CPU             bf526-0.0
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
/* This board has a 64meg MT48H32M16 */
#define CONFIG_MEM_ADD_WDTH	10
#define CONFIG_MEM_SIZE		64

#define CONFIG_EBIU_SDRRC_VAL	0x0267
#define CONFIG_EBIU_SDGCTL_VAL	(SCTLE | CL_2 | PASR_ALL | TRAS_6 | TRP_4 | TRCD_2 | TWR_2 | PSS)

#define CONFIG_EBIU_AMGCTL_VAL	(AMCKEN | AMBEN_ALL)
#define CONFIG_EBIU_AMBCTL0_VAL	(B1WAT_15 | B1RAT_15 | B1HT_3 | B1RDYPOL | B0WAT_15 | B0RAT_15 | B0HT_3 | B0RDYPOL)
#define CONFIG_EBIU_AMBCTL1_VAL	(B3WAT_15 | B3RAT_15 | B3HT_3 | B3RDYPOL | B2WAT_15 | B2RAT_15 | B2HT_3 | B2RDYPOL)

#define CONFIG_SYS_MONITOR_LEN	(768 * 1024)
#define CONFIG_SYS_MALLOC_LEN	(512 * 1024)


/*
 * NAND Settings
 * (can't be used same time as ethernet)
 */
#if (CONFIG_BFIN_BOOT_MODE == BFIN_BOOT_NAND)
# define CONFIG_BFIN_NFC
# define CONFIG_BFIN_NFC_BOOTROM_ECC
#endif
#ifdef CONFIG_BFIN_NFC
#define CONFIG_BFIN_NFC_CTL_VAL	0x0033
#define CONFIG_DRIVER_NAND_BFIN
#define CONFIG_SYS_NAND_BASE		0 /* not actually used */
#define CONFIG_SYS_MAX_NAND_DEVICE	1
#define CONFIG_CMD_NAND
#endif


/*
 * Network Settings
 */
#if !defined(__ADSPBF522__) && !defined(__ADSPBF523__) && \
    !defined(__ADSPBF524__) && !defined(__ADSPBF525__) && !defined(CONFIG_BFIN_NFC)
#define ADI_CMDS_NETWORK	1
#define CONFIG_BFIN_MAC
#define CONFIG_RMII
#define CONFIG_NETCONSOLE	1
#endif
#define CONFIG_HOSTNAME		bf526-ezbrd
/* Uncomment next line to use fixed MAC address */
/* #define CONFIG_ETHADDR	02:80:ad:20:31:e8 */
#define CONFIG_LIB_RAND

/*
 * Flash Settings
 */
#define CONFIG_FLASH_CFI_DRIVER
#define CONFIG_SYS_FLASH_BASE		0x20000000
#define CONFIG_SYS_FLASH_CFI
#define CONFIG_SYS_FLASH_PROTECTION
#define CONFIG_SYS_MAX_FLASH_BANKS	1
#define CONFIG_SYS_MAX_FLASH_SECT	71


/*
 * SPI Settings
 */
#define CONFIG_BFIN_SPI
#define CONFIG_ENV_SPI_MAX_HZ	30000000
#define CONFIG_SF_DEFAULT_SPEED	30000000
#define CONFIG_SPI_FLASH
#define CONFIG_SPI_FLASH_SST


/*
 * Env Storage Settings
 */
#if (CONFIG_BFIN_BOOT_MODE == BFIN_BOOT_SPI_MASTER)
#define CONFIG_ENV_IS_IN_SPI_FLASH
#define CONFIG_ENV_OFFSET	0x4000
#define CONFIG_ENV_SIZE		0x2000
#define CONFIG_ENV_SECT_SIZE	0x2000
#else
#define CONFIG_ENV_IS_IN_FLASH
#define CONFIG_ENV_OFFSET	0x4000
#define CONFIG_ENV_ADDR		(CONFIG_SYS_FLASH_BASE + CONFIG_ENV_OFFSET)
#define CONFIG_ENV_SIZE		0x2000
#define CONFIG_ENV_SECT_SIZE	0x2000
#endif
#define CONFIG_ENV_IS_EMBEDDED_IN_LDR


/*
 * I2C Settings
 */
#define CONFIG_SYS_I2C
#define CONFIG_SYS_I2C_ADI


/*
 * USB Settings
 */
#if !defined(__ADSPBF522__) && !defined(__ADSPBF523__)
#define CONFIG_USB
#define CONFIG_MUSB_HCD
#define CONFIG_USB_BLACKFIN
#define CONFIG_USB_STORAGE
#define CONFIG_MUSB_TIMEOUT 100000
#endif


/*
 * Misc Settings
 */
#define CONFIG_MISC_INIT_R
#define CONFIG_RTC_BFIN
#define CONFIG_UART_CONSOLE	1

/* define to enable run status via led */
/* #define CONFIG_STATUS_LED */
#ifdef CONFIG_STATUS_LED
#define CONFIG_GPIO_LED
#define CONFIG_BOARD_SPECIFIC_LED
/* use LED0 to indicate booting/alive */
#define STATUS_LED_BOOT 0
#define STATUS_LED_BIT GPIO_PF8
#define STATUS_LED_STATE STATUS_LED_ON
#define STATUS_LED_PERIOD (CONFIG_SYS_HZ / 4)
/* use LED1 to indicate crash */
#define STATUS_LED_CRASH 1
#define STATUS_LED_BIT1 GPIO_PG11
#define STATUS_LED_STATE1 STATUS_LED_ON
#define STATUS_LED_PERIOD1 (CONFIG_SYS_HZ / 2)
/* #define STATUS_LED_BIT2 GPIO_PG12 */
#endif


/*
 * Pull in common ADI header for remaining command/environment setup
 */
#include <configs/bfin_adi_common.h>

#endif
