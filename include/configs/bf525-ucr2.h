/*
 * U-boot - Configuration file for bf525-ucr2 board
 * The board includes ADSP-BF525 rev. 0.2,
 * 32-bit SDRAM (SAMSUNG K4S561632H-UC75),
 * USB 2.0 High Speed OTG USB WIFI,
 * SPI flash (cFeon EN25Q128 16 MB),
 * Support PPI and ITU-R656,
 * See http://www.ucrobotics.com/?q=cn/ucr2
 */

#ifndef __CONFIG_BF525_UCR2_H__
#define __CONFIG_BF525_UCR2_H__

#include <asm/config-pre.h>

/*
 * Processor Settings
 */
#define CONFIG_BFIN_CPU             bf525-0.2
#define CONFIG_BFIN_BOOT_MODE       BFIN_BOOT_SPI_MASTER

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
#define CONFIG_VCO_MULT			20
/* CCLK_DIV controls the core clock divider				*/
/* Values can be 1, 2, 4, or 8 ONLY					*/
#define CONFIG_CCLK_DIV			1
/* SCLK_DIV controls the system clock divider				*/
/* Values can range from 1-15						*/
#define CONFIG_SCLK_DIV			4

/*
 * Memory Settings
 */
#define CONFIG_MEM_ADD_WDTH	9
#define CONFIG_MEM_SIZE		32

/*
 * SDRAM reference page
 * http://docs.blackfin.uclinux.org/doku.php?id=bfin:sdram
 */
#define CONFIG_EBIU_SDRRC_VAL   0x3f8
#define CONFIG_EBIU_SDGCTL_VAL  0x9111cd

#define CONFIG_EBIU_AMGCTL_VAL  (AMBEN_ALL)
#define CONFIG_EBIU_AMBCTL0_VAL (B1WAT_7 | B1RAT_11 | B1HT_2 | B1ST_3 | B0WAT_7 | B0RAT_11 | B0HT_2 | B0ST_3)
#define CONFIG_EBIU_AMBCTL1_VAL (B3WAT_7 | B3RAT_11 | B3HT_2 | B3ST_3 | B2WAT_7 | B2RAT_11 | B2HT_2 | B2ST_3)

#define CONFIG_SYS_MONITOR_LEN	(320 * 1024)
#define CONFIG_SYS_MALLOC_LEN	(320 * 1024)

/* We don't have a parallel flash chip */
#define CONFIG_SYS_NO_FLASH

/* support for serial flash */
#define CONFIG_BFIN_SPI
#define CONFIG_CMD_SF
#define CONFIG_SF_DEFAULT_HZ	30000000
#define CONFIG_SPI_FLASH_EON

#define CONFIG_ENV_IS_IN_SPI_FLASH
#define CONFIG_ENV_SPI_MAX_HZ	30000000
#define CONFIG_ENV_OFFSET	0x10000
#define CONFIG_ENV_SIZE		0x10000
#define CONFIG_ENV_SECT_SIZE	0x10000
#define CONFIG_ENV_OVERWRITE	1

/*
 * Misc Settings
 */
#define CONFIG_UART_CONSOLE	0

#define CONFIG_BAUDRATE		115200
#define CONFIG_BFIN_SERIAL
#define CONFIG_BOOTARGS		"root=/dev/mtdblock0 rw"
#define CONFIG_BOOTCOMMAND	"run sfboot"
#define CONFIG_BOOTDELAY	5
#define CONFIG_EXTRA_ENV_SETTINGS \
	"sfboot=sf probe 1;" \
		"sf read 0x1000000 0x20000 0x300000;" \
		"bootm 0x1000000\0"

#endif
