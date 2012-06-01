/*
 * Hacked together,
 * hopefully functional.
 *
 * See zynq_common.h for Zynq common configs
 */

#ifndef __CONFIG_ZYNQ_ZC702_H
#define __CONFIG_ZYNQ_ZC702_H

/*
 * High Level Configuration Options
 */
#define CONFIG_ARM1176		1 /* CPU */
#define CONFIG_ZYNQ		1 /* SoC? */
#define CONFIG_ZC702		1 /* Board */

#include <configs/zynq_common.h>

#undef CONFIG_ZYNQ_XIL_LQSPI

/* no NOR on zc702 */
#define CONFIG_SYS_NO_FLASH
#define CONFIG_ENV_IS_NOWHERE	1

#include <config_cmd_default.h>	
#define CONFIG_CMD_DATE		/* RTC? */
#define CONFIG_CMD_PING		/* Might be useful for debugging */
#define CONFIG_CMD_SAVEENV	/* Command to save ENV to Flash */
#define CONFIG_REGINFO		/* Again, debugging */
#undef CONFIG_CMD_SETGETDCR	/* README says 4xx only */

#define CONFIG_TIMESTAMP	/* print image timestamp on bootm, etc */

/* IPADDR, SERVERIP */
/* Need I2C for RTC? */

#define CONFIG_PANIC_HANG	1 /* For development/debugging */

#define CONFIG_AUTO_COMPLETE	1
#define CONFIG_CMDLINE_EDITING	1

/* this is to initialize GEM at uboot start */
/* #define CONFIG_ZYNQ_INIT_GEM	*/
/* this is to set ipaddr, ethaddr and serverip env variables. */
#define CONFIG_ZYNQ_IP_ENV

/* HW to use */
#define CONFIG_UART1		1
#define CONFIG_TTC0		1
#define CONFIG_GEM0		1
#define CONFIG_NET_MULTI
#define CONFIG_XGMAC_PHY_ADDR	0x7

/*
 * These were lifted straight from imx31_phycore, and may well be very wrong.
 */
//#define CONFIG_ENV_SIZE			4096
#define CONFIG_ENV_SIZE			0x10000
#define CONFIG_SYS_MALLOC_LEN		0x400000
#define CONFIG_SYS_MAXARGS		16
#define CONFIG_SYS_CBSIZE		256
#define CONFIG_SYS_PBSIZE		(CONFIG_SYS_CBSIZE+sizeof(CONFIG_SYS_PROMPT)+16) /* phycore */

/*
 * SPI Settings
 */
#define CONFIG_CMD_SPI
#define CONFIG_ENV_SPI_MAX_HZ   30000000
#define CONFIG_SF_DEFAULT_SPEED 30000000
#define CONFIG_SPI_FLASH
#define CONFIG_CMD_SF
/* #define CONFIG_XILINX_PSS_QSPI_USE_DUAL_FLASH */
#define CONFIG_SPI_FLASH_STMICRO

/* Place a Xilinx Boot ROM header in u-boot image? */
#undef CONFIG_ZYNQ_XILINX_FLASH_HEADER */

#ifdef CONFIG_ZYNQ_XILINX_FLASH_HEADER
/* Address Xilinx boot rom should use to launch u-boot */
#ifdef CONFIG_ZYNQ_XIL_LQSPI
#define CONFIG_ZYNQ_XIP_START XPSS_QSPI_LIN_BASEADDR
#endif
#endif

/* Secure Digital */
#define CONFIG_MMC     1

#ifdef CONFIG_MMC
#define CONFIG_GENERIC_MMC
#define CONFIG_CMD_MMC
#define CONFIG_CMD_FAT
#define CONFIG_CMD_EXT2
#define CONFIG_DOS_PARTITION
#endif

#define BOARD_LATE_INIT	1

#endif /* __CONFIG_ZYNQ_ZC702_H */
