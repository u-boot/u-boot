/*
 * U-boot - Configuration file for BF533 STAMP board
 */

#ifndef __CONFIG_BF533_STAMP_H__
#define __CONFIG_BF533_STAMP_H__

#include <asm/config-pre.h>


/*
 * Processor Settings
 */
#define CONFIG_BFIN_CPU             bf533-0.3
#define CONFIG_BFIN_BOOT_MODE       BFIN_BOOT_BYPASS


/*
 * Clock Settings
 *	CCLK = (CLKIN * VCO_MULT) / CCLK_DIV
 *	SCLK = (CLKIN * VCO_MULT) / SCLK_DIV
 */
/* CONFIG_CLKIN_HZ is any value in Hz					*/
#define CONFIG_CLKIN_HZ			11059200
/* CLKIN_HALF controls the DF bit in PLL_CTL      0 = CLKIN		*/
/*                                                1 = CLKIN / 2		*/
#define CONFIG_CLKIN_HALF		0
/* PLL_BYPASS controls the BYPASS bit in PLL_CTL  0 = do not bypass	*/
/*                                                1 = bypass PLL	*/
#define CONFIG_PLL_BYPASS		0
/* VCO_MULT controls the MSEL (multiplier) bits in PLL_CTL		*/
/* Values can range from 0-63 (where 0 means 64)			*/
#define CONFIG_VCO_MULT			45
/* CCLK_DIV controls the core clock divider				*/
/* Values can be 1, 2, 4, or 8 ONLY					*/
#define CONFIG_CCLK_DIV			1
/* SCLK_DIV controls the system clock divider				*/
/* Values can range from 1-15						*/
#define CONFIG_SCLK_DIV			6 /* note: 1.2 boards can go faster */


/*
 * Memory Settings
 */
#define CONFIG_MEM_ADD_WDTH	11
#define CONFIG_MEM_SIZE		128

#define CONFIG_EBIU_SDRRC_VAL	0x268
#define CONFIG_EBIU_SDGCTL_VAL	0x911109

#define CONFIG_EBIU_AMGCTL_VAL	0xFF
#define CONFIG_EBIU_AMBCTL0_VAL	0xBBC3BBC3
#define CONFIG_EBIU_AMBCTL1_VAL	0x99B39983

#define CONFIG_SYS_MONITOR_LEN	(256 * 1024)
#define CONFIG_SYS_MALLOC_LEN	(384 * 1024)


/*
 * Network Settings
 */
#define ADI_CMDS_NETWORK	1
#define CONFIG_DRIVER_SMC91111	1
#define CONFIG_SMC91111_BASE	0x20300300
#define SMC91111_EEPROM_INIT() \
	do { \
		*pFIO_DIR |= PF1; \
		*pFIO_FLAG_S = PF1; \
		SSYNC(); \
	} while (0)
#define CONFIG_HOSTNAME		bf533-stamp
/* Uncomment next line to use fixed MAC address */
/* #define CONFIG_ETHADDR	02:80:ad:20:31:b8 */


/*
 * Flash Settings
 */
#define CONFIG_FLASH_CFI_DRIVER
#define CONFIG_SYS_FLASH_BASE		0x20000000
#define CONFIG_SYS_FLASH_CFI
#define CONFIG_SYS_FLASH_CFI_AMD_RESET
#define CONFIG_SYS_MAX_FLASH_BANKS	1
#define CONFIG_SYS_MAX_FLASH_SECT	67


/*
 * SPI Settings
 */
#define CONFIG_BFIN_SPI
#define CONFIG_ENV_SPI_MAX_HZ	30000000
#define CONFIG_SF_DEFAULT_SPEED	30000000
#define CONFIG_SPI_FLASH
#define CONFIG_SPI_FLASH_ATMEL
#define CONFIG_SPI_FLASH_SPANSION
#define CONFIG_SPI_FLASH_STMICRO
#define CONFIG_SPI_FLASH_WINBOND


/*
 * Env Storage Settings
 */
#if (CONFIG_BFIN_BOOT_MODE == BFIN_BOOT_SPI_MASTER)
#define CONFIG_ENV_IS_IN_SPI_FLASH
#define CONFIG_ENV_OFFSET	0x10000
#define CONFIG_ENV_SIZE		0x2000
#define CONFIG_ENV_SECT_SIZE	0x10000
#else
#define CONFIG_ENV_IS_IN_FLASH
#define CONFIG_ENV_OFFSET	0x4000
#define CONFIG_ENV_ADDR		(CONFIG_SYS_FLASH_BASE + CONFIG_ENV_OFFSET)
#define CONFIG_ENV_SIZE		0x2000
#define CONFIG_ENV_SECT_SIZE	0x2000
#endif
#if (CONFIG_BFIN_BOOT_MODE == BFIN_BOOT_BYPASS)
#define ENV_IS_EMBEDDED
#else
#define ENV_IS_EMBEDDED_CUSTOM
#endif
#ifdef ENV_IS_EMBEDDED
/* WARNING - the following is hand-optimized to fit within
 * the sector before the environment sector. If it throws
 * an error during compilation remove an object here to get
 * it linked after the configuration sector.
 */
# define LDS_BOARD_TEXT \
	cpu/blackfin/traps.o		(.text .text.*); \
	cpu/blackfin/interrupt.o	(.text .text.*); \
	cpu/blackfin/serial.o		(.text .text.*); \
	common/dlmalloc.o		(.text .text.*); \
	lib_generic/crc32.o		(.text .text.*); \
	. = DEFINED(env_offset) ? env_offset : .; \
	common/env_embedded.o		(.text .text.*);
#endif


/*
 * I2C Settings
 * By default PF2 is used as SDA and PF3 as SCL on the Stamp board
 */
#define CONFIG_SOFT_I2C
#ifdef CONFIG_SOFT_I2C
#define PF_SCL PF3
#define PF_SDA PF2
#define I2C_INIT \
	do { \
		*pFIO_DIR |= PF_SCL; \
		SSYNC(); \
	} while (0)
#define I2C_ACTIVE \
	do { \
		*pFIO_DIR |= PF_SDA; \
		*pFIO_INEN &= ~PF_SDA; \
		SSYNC(); \
	} while (0)
#define I2C_TRISTATE \
	do { \
		*pFIO_DIR &= ~PF_SDA; \
		*pFIO_INEN |= PF_SDA; \
		SSYNC(); \
	} while (0)
#define I2C_READ ((*pFIO_FLAG_D & PF_SDA) != 0)
#define I2C_SDA(bit) \
	do { \
		if (bit) \
			*pFIO_FLAG_S = PF_SDA; \
		else \
			*pFIO_FLAG_C = PF_SDA; \
		SSYNC(); \
	} while (0)
#define I2C_SCL(bit) \
	do { \
		if (bit) \
			*pFIO_FLAG_S = PF_SCL; \
		else \
			*pFIO_FLAG_C = PF_SCL; \
		SSYNC(); \
	} while (0)
#define I2C_DELAY		udelay(5)	/* 1/4 I2C clock duration */

#define CONFIG_SYS_I2C_SPEED		50000
#define CONFIG_SYS_I2C_SLAVE		0
#endif


/*
 * Compact Flash / IDE / ATA Settings
 */

/* Enabled below option for CF support */
/* #define CONFIG_STAMP_CF */
#if defined(CONFIG_STAMP_CF)
#define CONFIG_MISC_INIT_R
#define CONFIG_DOS_PARTITION	1
#undef  CONFIG_IDE_8xx_DIRECT		/* no pcmcia interface required */
#undef  CONFIG_IDE_LED			/* no led for ide supported */
#undef  CONFIG_IDE_RESET		/* no reset for ide supported */

#define CONFIG_SYS_IDE_MAXBUS		1
#define CONFIG_SYS_IDE_MAXDEVICE	(CONFIG_SYS_IDE_MAXBUS * 1)

#define CONFIG_SYS_ATA_BASE_ADDR	0x20200000
#define CONFIG_SYS_ATA_IDE0_OFFSET	0x0000

#define CONFIG_SYS_ATA_DATA_OFFSET	0x0020	/* data I/O */
#define CONFIG_SYS_ATA_REG_OFFSET	0x0020	/* normal register accesses */
#define CONFIG_SYS_ATA_ALT_OFFSET	0x0007	/* alternate registers */

#define CONFIG_SYS_ATA_STRIDE		2

#undef CONFIG_EBIU_AMBCTL1_VAL
#define CONFIG_EBIU_AMBCTL1_VAL	0x99B3ffc2
#endif


/*
 * Misc Settings
 */
#define CONFIG_RTC_BFIN
#define CONFIG_UART_CONSOLE	0

/* FLASH/ETHERNET uses the same async bank */
#define SHARED_RESOURCES 	1

/* define to enable boot progress via leds */
/* #define CONFIG_SHOW_BOOT_PROGRESS */

/* define to enable run status via led */
/* #define CONFIG_STATUS_LED */
#ifdef CONFIG_STATUS_LED
#define CONFIG_BOARD_SPECIFIC_LED
#ifndef __ASSEMBLY__
typedef unsigned int led_id_t;
void __led_init(led_id_t mask, int state);
void __led_set(led_id_t mask, int state);
void __led_toggle(led_id_t mask);
#endif
/* use LED1 to indicate booting/alive */
#define STATUS_LED_BOOT 0
#define STATUS_LED_BIT 1
#define STATUS_LED_STATE STATUS_LED_ON
#define STATUS_LED_PERIOD (CONFIG_SYS_HZ / 4)
/* use LED2 to indicate crash */
#define STATUS_LED_CRASH 1
#define STATUS_LED_BIT1 2
#define STATUS_LED_STATE1 STATUS_LED_ON
#define STATUS_LED_PERIOD1 (CONFIG_SYS_HZ / 2)
#endif

/* define to enable splash screen support */
/* #define CONFIG_VIDEO */


/*
 * Pull in common ADI header for remaining command/environment setup
 */
#include <configs/bfin_adi_common.h>

#endif
