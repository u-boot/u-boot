/*
 * U-boot - Configuration file for IBF-DSP561 board
 */

#ifndef __CONFIG_IBF_DSP561__H__
#define __CONFIG_IBF_DSP561__H__

#include <asm/config-pre.h>


/*
 * Processor Settings
 */
#define CONFIG_BFIN_CPU             bf561-0.5
#define CONFIG_BFIN_BOOT_MODE       BFIN_BOOT_BYPASS


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
#define CONFIG_MEM_ADD_WDTH	9
#define CONFIG_MEM_SIZE		64

#define CONFIG_EBIU_SDRRC_VAL	0x377
#define CONFIG_EBIU_SDGCTL_VAL	0x91998d
#define CONFIG_EBIU_SDBCTL_VAL	0x15

#define CONFIG_EBIU_AMGCTL_VAL	0x3F
#define CONFIG_EBIU_AMBCTL0_VAL	0x7BB07BB0
#define CONFIG_EBIU_AMBCTL1_VAL	0xFFC27BB0

#define CONFIG_SYS_MONITOR_LEN	(256 * 1024)
#define CONFIG_SYS_MALLOC_LEN	(128 * 1024)


/*
 * Flash Settings
 */
#define CONFIG_SYS_FLASH_CFI		/* The flash is CFI compatible */
#define CONFIG_FLASH_CFI_DRIVER	/* Use common CFI driver */
#define CONFIG_SYS_FLASH_CFI_AMD_RESET
#define CONFIG_SYS_FLASH_BASE		0x20000000
#define CONFIG_SYS_MAX_FLASH_BANKS	1	/* max number of memory banks */
#define CONFIG_SYS_MAX_FLASH_SECT	135	/* max number of sectors on one chip */
/* The BF561-EZKIT uses a top boot flash */
#define CONFIG_ENV_IS_IN_FLASH	1
#define CONFIG_ENV_ADDR		0x20004000
#define CONFIG_ENV_OFFSET		(CONFIG_ENV_ADDR - CONFIG_SYS_FLASH_BASE)
#define CONFIG_ENV_SIZE		0x2000
#define CONFIG_ENV_SECT_SIZE	0x10000	/* Total Size of Environment Sector */
#if (CONFIG_BFIN_BOOT_MODE == BFIN_BOOT_BYPASS)
#define ENV_IS_EMBEDDED
#else
#define CONFIG_ENV_IS_EMBEDDED_IN_LDR
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
	lib_generic/zlib.o		(.text .text.*); \
	board/ibf-dsp561/ibf-dsp561.o	(.text .text.*); \
	. = DEFINED(env_offset) ? env_offset : .; \
	common/env_embedded.o		(.text .text.*);
#endif


/*
 * I2C Settings
 */
#define CONFIG_SOFT_I2C		1
#define PF_SCL			0x1/*PF0*/
#define PF_SDA			0x2/*PF1*/

#ifdef CONFIG_SOFT_I2C
#define I2C_INIT       do { *pFIO0_DIR |= PF_SCL; SSYNC(); } while (0)
#define I2C_ACTIVE     do { *pFIO0_DIR |= PF_SDA; *pFIO0_INEN &= ~PF_SDA; SSYNC(); } while (0)
#define I2C_TRISTATE   do { *pFIO0_DIR &= ~PF_SDA; *pFIO0_INEN |= PF_SDA; SSYNC(); } while (0)
#define I2C_READ       ((*pFIO0_FLAG_D & PF_SDA) != 0)
#define I2C_SDA(bit) \
	do { \
		if (bit) \
			*pFIO0_FLAG_S = PF_SDA; \
		else \
			*pFIO0_FLAG_C = PF_SDA; \
		SSYNC(); \
	} while (0)
#define I2C_SCL(bit) \
	do { \
		if (bit) \
			*pFIO0_FLAG_S = PF_SCL; \
		else \
			*pFIO0_FLAG_C = PF_SCL; \
		SSYNC(); \
	} while (0)
#define I2C_DELAY		udelay(5)	/* 1/4 I2C clock duration */

#define CONFIG_SYS_I2C_SPEED	50000
#define CONFIG_SYS_I2C_SLAVE	0
#endif


/*
 * Misc Settings
 */
#define CONFIG_UART_CONSOLE	0


/*
 * Pull in common ADI header for remaining command/environment setup
 */
#include <configs/bfin_adi_common.h>

#endif
