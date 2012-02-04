/*
 * U-boot - Configuration file for BF548 STAMP board
 */

#ifndef __CONFIG_BF548_EZKIT_H__
#define __CONFIG_BF548_EZKIT_H__

#include <asm/config-pre.h>


/*
 * Processor Settings
 */
#define CONFIG_BFIN_CPU             bf548-0.0
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
#define CONFIG_VCO_MULT			21
/* CCLK_DIV controls the core clock divider				*/
/* Values can be 1, 2, 4, or 8 ONLY					*/
#define CONFIG_CCLK_DIV			1
/* SCLK_DIV controls the system clock divider				*/
/* Values can range from 1-15						*/
#define CONFIG_SCLK_DIV			4


/*
 * Memory Settings
 */
#define CONFIG_MEM_ADD_WDTH	10
#define CONFIG_MEM_SIZE		64

#define CONFIG_EBIU_DDRCTL0_VAL	0x218A83FE
#define CONFIG_EBIU_DDRCTL1_VAL	0x20022222
#define CONFIG_EBIU_DDRCTL2_VAL	0x00000021

/* Default EZ-Kit bank mapping:
 *	Async Bank 0 - 32MB Burst Flash
 *	Async Bank 1 - Ethernet
 *	Async Bank 2 - Nothing
 *	Async Bank 3 - Nothing
 */
#define CONFIG_EBIU_AMGCTL_VAL	0xFF
#define CONFIG_EBIU_AMBCTL0_VAL	0x7BB07BB0
#define CONFIG_EBIU_AMBCTL1_VAL	0xFFC27BB0
#define CONFIG_EBIU_FCTL_VAL	(BCLK_4)
#define CONFIG_EBIU_MODE_VAL	(B0MODE_FLASH)

#define CONFIG_SYS_MONITOR_LEN	(1024 * 1024)
#define CONFIG_SYS_MALLOC_LEN	(768 * 1024)


/*
 * Network Settings
 */
#define ADI_CMDS_NETWORK	1
#define CONFIG_SMC911X	1
#define CONFIG_SMC911X_BASE	0x24000000
#define CONFIG_SMC911X_16_BIT
#define CONFIG_HOSTNAME		bf548-ezkit
/* Uncomment next line to use fixed MAC address */
/* #define CONFIG_ETHADDR	02:80:ad:20:31:e8 */


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
#define CONFIG_SPI_FLASH
#define CONFIG_SPI_FLASH_STMICRO


/*
 * Env Storage Settings
 */
#if (CONFIG_BFIN_BOOT_MODE == BFIN_BOOT_SPI_MASTER)
#define CONFIG_ENV_IS_IN_SPI_FLASH
#define CONFIG_ENV_OFFSET	0x10000
#define CONFIG_ENV_SIZE		0x2000
#define CONFIG_ENV_SECT_SIZE	0x10000
#define CONFIG_ENV_IS_EMBEDDED_IN_LDR
#elif (CONFIG_BFIN_BOOT_MODE == BFIN_BOOT_NAND)
#define CONFIG_ENV_IS_IN_NAND
#define CONFIG_ENV_OFFSET	0x60000
#define CONFIG_ENV_SIZE		0x20000
#else
/* The BF548-EZKIT uses a top boot flash */
#define CONFIG_ENV_IS_IN_FLASH	1
#define CONFIG_ENV_ADDR		(CONFIG_SYS_FLASH_BASE + CONFIG_ENV_OFFSET)
#define CONFIG_ENV_OFFSET	(0x1000000 - CONFIG_ENV_SECT_SIZE)
#define CONFIG_ENV_SIZE		CONFIG_ENV_SECT_SIZE
#define CONFIG_ENV_SECT_SIZE	0x8000
#endif


/*
 * NAND Settings
 */
#define CONFIG_BFIN_NFC_CTL_VAL	0x0033
#if (CONFIG_BFIN_BOOT_MODE == BFIN_BOOT_NAND)
# define CONFIG_BFIN_NFC_BOOTROM_ECC
#endif
#define CONFIG_DRIVER_NAND_BFIN
#define CONFIG_SYS_NAND_BASE		0 /* not actually used */
#define CONFIG_SYS_MAX_NAND_DEVICE	1


/*
 * I2C Settings
 */
#define CONFIG_BFIN_TWI_I2C	1
#define CONFIG_HARD_I2C		1


/*
 * SATA
 */
#if !defined(__ADSPBF544__)
#define CONFIG_LIBATA
#define CONFIG_SYS_SATA_MAX_DEVICE 1
#define CONFIG_LBA48
#define CONFIG_PATA_BFIN
#define CONFIG_BFIN_ATAPI_BASE_ADDR	0xFFC03800
#define CONFIG_BFIN_ATA_MODE	XFER_PIO_4
#endif


/*
 * SDH Settings
 */
#if !defined(__ADSPBF544__)
#define CONFIG_GENERIC_MMC
#define CONFIG_MMC
#define CONFIG_BFIN_SDH
#endif


/*
 * USB Settings
 */
#if !defined(__ADSPBF544__)
#define CONFIG_USB
#define CONFIG_MUSB_HCD
#define CONFIG_USB_BLACKFIN
#define CONFIG_USB_STORAGE
#define CONFIG_MUSB_TIMEOUT 100000
#endif


/*
 * Misc Settings
 */
#define CONFIG_BOARD_EARLY_INIT_F
#define CONFIG_BOARD_SIZE_LIMIT $$(( 512 * 1024 ))
#define CONFIG_RTC_BFIN
#define CONFIG_UART_CONSOLE	1
#define CONFIG_BFIN_SPI_IMG_SIZE 0x50000

#ifndef __ADSPBF542__
/* Don't waste time transferring a logo over the UART */
# if (CONFIG_BFIN_BOOT_MODE != BFIN_BOOT_UART)
#  define CONFIG_VIDEO
#  define EASYLOGO_HEADER <asm/bfin_logo_230x230_gzip.h>
# endif
# define CONFIG_DEB_DMA_URGENT
#endif

/* Define if want to do post memory test */
#undef CONFIG_POST
#ifdef CONFIG_POST
#define CONFIG_POST_BSPEC1_GPIO_LEDS \
	GPIO_PG6, GPIO_PG7, GPIO_PG8, GPIO_PG9, GPIO_PG10, GPIO_PG11,
#define CONFIG_POST_BSPEC2_GPIO_BUTTONS \
	GPIO_PB8, GPIO_PB9, GPIO_PB10, GPIO_PB11
#define CONFIG_POST_BSPEC2_GPIO_NAMES \
	13, 12, 11, 10,
#define CONFIG_SYS_POST_FLASH_START	10
#define CONFIG_SYS_POST_FLASH_END	127
#endif


/*
 * Pull in common ADI header for remaining command/environment setup
 */
#include <configs/bfin_adi_common.h>

#endif
