/*
 * U-Boot - Configuration file for BF609 EZ-Kit board
 */

#ifndef __CONFIG_BF609_EZKIT_H__
#define __CONFIG_BF609_EZKIT_H__

#include <asm/config-pre.h>

/*
 * Processor Settings
 */
#define CONFIG_BFIN_CPU             bf609-0.0
#define CONFIG_BFIN_BOOT_MODE       BFIN_BOOT_PARA

/* For ez-board version 1.0, else undef this */
#define CONFIG_BFIN_BOARD_VERSION_1_0

/*
 * Clock Settings
 *	CCLK = (CLKIN * VCO_MULT) / CCLK_DIV
 *	SCLK = (CLKIN * VCO_MULT) / SYSCLK_DIV
 *	SCLK0 = SCLK / SCLK0_DIV
 *	SCLK1 = SCLK / SCLK1_DIV
 */
/* CONFIG_CLKIN_HZ is any value in Hz					*/
#define CONFIG_CLKIN_HZ			(25000000)
/* CLKIN_HALF controls the DF bit in PLL_CTL      0 = CLKIN		*/
/*                                                1 = CLKIN / 2		*/
#define CONFIG_CLKIN_HALF		(0)

/* VCO_MULT controls the MSEL (multiplier) bits in PLL_CTL		*/
/* Values can range from 0-127 (where 0 means 128)			*/
#define CONFIG_VCO_MULT			(20)

/* CCLK_DIV controls the core clock divider				*/
/* Values can range from 0-31 (where 0 means 32)			*/
#define CONFIG_CCLK_DIV			(1)
/* SCLK_DIV controls the system clock divider				*/
/* Values can range from 0-31 (where 0 means 32)			*/
#define CONFIG_SCLK_DIV		(4)
/* Values can range from 0-7 (where 0 means 8)				*/
#define CONFIG_SCLK0_DIV		(1)
#define CONFIG_SCLK1_DIV		(1)
/* DCLK_DIV controls the DDR clock divider				*/
/* Values can range from 0-31 (where 0 means 32)			*/
#define CONFIG_DCLK_DIV			(2)
/* OCLK_DIV controls the output clock divider				*/
/* Values can range from 0-127 (where 0 means 128)			*/
#define CONFIG_OCLK_DIV			(16)

/*
 * Memory Settings
 */
#define CONFIG_MEM_SIZE		128

#define CONFIG_SMC_GCTL_VAL	0x00000010
#define CONFIG_SMC_B0CTL_VAL	0x01007011
#define CONFIG_SMC_B0TIM_VAL	0x08170977
#define CONFIG_SMC_B0ETIM_VAL	0x00092231

#define CONFIG_SYS_MONITOR_LEN	(768 * 1024)
#define CONFIG_SYS_MALLOC_LEN	(512 * 1024)

#define CONFIG_HW_WATCHDOG
/*
 * Network Settings
 */
#define ADI_CMDS_NETWORK
#define CONFIG_NETCONSOLE
#define CONFIG_HOSTNAME		"bf609-ezkit"
#define CONFIG_PHY_ADDR		1
#define CONFIG_DW_PORTS		1
#define CONFIG_DW_ALTDESCRIPTOR
#define CONFIG_MII

/* i2c Settings */
#define CONFIG_SYS_I2C
#define CONFIG_SYS_I2C_ADI

/*
 * Flash Settings
 */
#undef CONFIG_CMD_JFFS2
#define CONFIG_SYS_FLASH_CFI_WIDTH     2
#define CONFIG_FLASH_CFI_DRIVER
#define CONFIG_SYS_FLASH_BASE          0xb0000000
#define CONFIG_SYS_FLASH_CFI
#define CONFIG_SYS_FLASH_PROTECTION
#define CONFIG_SYS_MAX_FLASH_BANKS     1
#define CONFIG_SYS_MAX_FLASH_SECT      131
#define CONFIG_CFI_FLASH_USE_WEAK_ACCESSORS

/*
 * SPI Settings
 */
#define CONFIG_BFIN_SPI6XX
#define CONFIG_ENV_SPI_MAX_HZ	25000000
#define CONFIG_SF_DEFAULT_SPEED	25000000
#define CONFIG_SPI_FLASH_ALL

/*
 * Env Storage Settings
 */
#if (CONFIG_BFIN_BOOT_MODE == BFIN_BOOT_SPI_MASTER)
#define CONFIG_ENV_IS_IN_SPI_FLASH
#define CONFIG_ENV_OFFSET       0x10000
#define CONFIG_ENV_SIZE         0x2000
#define CONFIG_ENV_SECT_SIZE    0x10000
#define CONFIG_ENV_IS_EMBEDDED_IN_LDR
#elif (CONFIG_BFIN_BOOT_MODE == BFIN_BOOT_NAND)
#define CONFIG_ENV_IS_IN_NAND
#define CONFIG_ENV_OFFSET       0x60000
#define CONFIG_ENV_SIZE         0x20000
#else
#define CONFIG_ENV_IS_IN_FLASH
#define CONFIG_ENV_ADDR         (CONFIG_SYS_FLASH_BASE + CONFIG_ENV_OFFSET)
#define CONFIG_ENV_OFFSET       0x8000
#define CONFIG_ENV_SIZE         0x8000
#define CONFIG_ENV_SECT_SIZE    0x8000
#define CONFIG_ENV_IS_EMBEDDED_IN_LDR
#endif

#define FLASHBOOT_ENV_SETTINGS "flashboot=bootm 0xB0100000\0"

/*
 * SDH Settings
 */
#define CONFIG_GENERIC_MMC
#define CONFIG_MMC
#define CONFIG_BFIN_SDH

/*
 * Misc Settings
 */
#define CONFIG_BOARD_EARLY_INIT_F
#define CONFIG_UART_CONSOLE	0

#define CONFIG_CMD_SOFTSWITCH

#define CONFIG_SYS_MEMTEST_END (CONFIG_STACKBASE - 20*1024*1024 + 4)
#define CONFIG_BFIN_SOFT_SWITCH

#define CONFIG_ADI_GPIO2

#if 0
#define CONFIG_UART_MEM 1024
#undef CONFIG_UART_CONSOLE
#undef CONFIG_JTAG_CONSOLE
#undef CONFIG_UART_CONSOLE_IS_JTAG
#endif

#define CONFIG_BOARD_SIZE_LIMIT $$((512 * 1024))

/*
 * Run core 1 from L1 SRAM start address when init uboot on core 0
 */
/* #define CONFIG_CORE1_RUN	1 */

/*
 * Pull in common ADI header for remaining command/environment setup
 */
#include <configs/bfin_adi_common.h>
#endif
