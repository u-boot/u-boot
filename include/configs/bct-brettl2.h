/*
 * U-boot - Configuration file for BF536 brettl2 board
 */

#ifndef __CONFIG_BCT_BRETTL2_H__
#define __CONFIG_BCT_BRETTL2_H__

#include <asm/config-pre.h>


/*
 * Processor Settings
 */
#define CONFIG_BFIN_CPU             bf536-0.3
#define CONFIG_BFIN_BOOT_MODE       BFIN_BOOT_BYPASS


/*
 * Clock Settings
 *	CCLK = (CLKIN * VCO_MULT) / CCLK_DIV
 *	SCLK = (CLKIN * VCO_MULT) / SCLK_DIV
 */
/* CONFIG_CLKIN_HZ is any value in Hz					*/
#define CONFIG_CLKIN_HZ			16384000
/* CLKIN_HALF controls the DF bit in PLL_CTL	  0 = CLKIN		*/
/*						  1 = CLKIN / 2		*/
#define CONFIG_CLKIN_HALF		0
/* PLL_BYPASS controls the BYPASS bit in PLL_CTL  0 = do not bypass	*/
/*						  1 = bypass PLL	*/
#define CONFIG_PLL_BYPASS		0
/* VCO_MULT controls the MSEL (multiplier) bits in PLL_CTL		*/
/* Values can range from 0-63 (where 0 means 64)			*/
#define CONFIG_VCO_MULT			24
/* CCLK_DIV controls the core clock divider				*/
/* Values can be 1, 2, 4, or 8 ONLY					*/
#define CONFIG_CCLK_DIV			1
/* SCLK_DIV controls the system clock divider				*/
/* Values can range from 1-15						*/
#define CONFIG_SCLK_DIV			3
#define CONFIG_VR_CTL_VAL	(VLEV_110 | GAIN_20 | FREQ_1000)


/*
 * Memory Settings
 */
#define CONFIG_MEM_ADD_WDTH	9
#define CONFIG_MEM_SIZE		32


/*
 * SDRAM Settings
 */
#define CONFIG_EBIU_SDRRC_VAL	0x07f6
#define CONFIG_EBIU_SDGCTL_VAL	0x9111cd

#define CONFIG_EBIU_AMGCTL_VAL	(AMBEN_ALL)
#define CONFIG_EBIU_AMBCTL0_VAL	(B1WAT_7 | B1RAT_11 | B1HT_2 | B1ST_3 | B0WAT_7 | B0RAT_11 | B0HT_2 | B0ST_3)
#define CONFIG_EBIU_AMBCTL1_VAL	(B3WAT_7 | B3RAT_11 | B3HT_2 | B3ST_3 | B2WAT_7 | B2RAT_11 | B2HT_2 | B2ST_3)

#define CONFIG_SYS_MONITOR_LEN		(256 * 1024)
#define CONFIG_SYS_MALLOC_LEN		(128 * 1024)


/*
 * Network Settings
 */
#ifndef __ADSPBF534__
#define ADI_CMDS_NETWORK	1
#define CONFIG_BFIN_MAC		1
#define CONFIG_NETCONSOLE	1
#define CONFIG_HOSTNAME		brettl2
#define CONFIG_IPADDR		192.168.233.224
#define CONFIG_GATEWAYIP	192.168.233.1
#define CONFIG_SERVERIP		192.168.233.53
#define CONFIG_ROOTPATH		"/romfs/brettl2"
/* Uncomment next line to use fixed MAC address */
/* #define CONFIG_ETHADDR	02:80:ad:20:31:e8 */
#endif


/*
 * Flash Settings
 */
#define CONFIG_FLASH_CFI_DRIVER
#define CONFIG_SYS_FLASH_CFI
#define CONFIG_CFI_FLASH_USE_WEAK_ACCESSORS
#define CONFIG_SYS_FLASH_PROTECTION
#define CONFIG_SYS_FLASH_BASE			0x20000000
#define CONFIG_SYS_MAX_FLASH_BANKS		1
#define CONFIG_SYS_MAX_FLASH_SECT		135


/*
 * Env Storage Settings
 */
#define CONFIG_ENV_IS_IN_FLASH 1
#define CONFIG_ENV_OFFSET	0x4000
#define CONFIG_ENV_SIZE		0x2000
#define CONFIG_ENV_SECT_SIZE	0x10000

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
	arch/blackfin/lib/libblackfin.o (.text*); \
	arch/blackfin/cpu/libblackfin.o (.text*); \
	. = DEFINED(env_offset) ? env_offset : .; \
	common/env_embedded.o (.text*);
#endif


/*
 * I2C Settings
 */
#define CONFIG_BFIN_TWI_I2C	1
#define CONFIG_HARD_I2C		1


/*
 * Misc Settings
 */
#define CONFIG_BOOTDELAY	1
#define CONFIG_LOADADDR		0x800000
#define CONFIG_MISC_INIT_R
#define CONFIG_UART_CONSOLE	0
#define CONFIG_BAUDRATE		115200
#define CONFIG_MTD_DEVICE
#define CONFIG_MTD_PARTITIONS
#define CONFIG_SYS_HUSH_PARSER


/*
 * Pull in common ADI header for remaining command/environment setup
 */
#include <configs/bfin_adi_common.h>

/* disable unnecessary features */
#undef CONFIG_BOOTM_RTEMS
#undef CONFIG_BZIP2
#undef CONFIG_KALLSYMS

#endif
