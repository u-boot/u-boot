/*
 * (C) Copyright 2012 Xilinx
 *
 * Configuration settings for the Xilinx Zynq ZC770 board.
 * See zynq_common.h for Zynq common configs
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#ifndef __CONFIG_ZYNQ_ZC770_H
#define __CONFIG_ZYNQ_ZC770_H

#define PHYS_SDRAM_1_SIZE (1024 * 1024 * 1024)

#define CONFIG_SYS_NO_FLASH

#if defined(CONFIG_ZC770_XM010)
# define CONFIG_ZYNQ_SERIAL_UART1
# define CONFIG_ZYNQ_GEM0
# define CONFIG_PHY_ADDR	7

#elif defined(CONFIG_ZC770_XM011)
# define CONFIG_ZYNQ_SERIAL_UART1

#elif defined(CONFIG_ZC770_XM012)
# define CONFIG_ZYNQ_SERIAL_UART1
# undef CONFIG_SYS_NO_FLASH

#elif defined(CONFIG_ZC770_XM013)
# define CONFIG_ZYNQ_SERIAL_UART0
# define CONFIG_ZYNQ_GEM1
# define CONFIG_PHY_ADDR	7

#else
# define CONFIG_ZYNQ_SERIAL_UART0
#endif




#include <configs/zynq_common.h>


#undef CONFIG_CMD_SETGETDCR	/* README says 4xx only */

#define CONFIG_TIMESTAMP	/* print image timestamp on bootm, etc */


#define CONFIG_AUTO_COMPLETE
#define CONFIG_CMDLINE_EDITING

/* this is to initialize GEM at uboot start */
/* #define CONFIG_ZYNQ_INIT_GEM	*/
/* this is to set ipaddr, ethaddr and serverip env variables. */
#define CONFIG_ZYNQ_IP_ENV

#if defined(CONFIG_ZC770_XM010) || defined(CONFIG_ZC770_XM012)
/* Place a Xilinx Boot ROM header in u-boot image? */
#define CONFIG_ZYNQ_XILINX_FLASH_HEADER
#endif




/*
 * SPI Settings
 */
#if defined(CONFIG_ZC770_XM010) || defined(CONFIG_ZC770_XM013)
#define CONFIG_ZYNQ_SPI
#define CONFIG_CMD_SPI
#define CONFIG_SF_DEFAULT_SPEED 30000000
#define CONFIG_SPI_FLASH
#define CONFIG_CMD_SF

#ifdef CONFIG_ZYNQ_XILINX_FLASH_HEADER
/* Address Xilinx boot rom should use to launch u-boot */
#define CONFIG_ZYNQ_XIP_START XPSS_QSPI_LIN_BASEADDR
#endif
#endif

#if defined(CONFIG_ZC770_XM013)
#define CONFIG_SPI_FLASH_SPANSION
#endif

#if defined(CONFIG_ZC770_XM010)
#define CONFIG_SPI_FLASH_STMICRO
#endif

/*
 * NAND Flash settings
 */
#if defined(CONFIG_ZC770_XM011)
#define CONFIG_NAND_ZYNQ
#define CONFIG_CMD_NAND
#define CONFIG_CMD_NAND_LOCK_UNLOCK
#define CONFIG_SYS_MAX_NAND_DEVICE 1
#define CONFIG_SYS_NAND_BASE XPSS_NAND_BASEADDR
#define CONFIG_SYS_NAND_ONFI_DETECTION
#define CONFIG_MTD_DEVICE
#endif

#if defined(CONFIG_ZC770_XM010)
/* Secure Digital */
#define CONFIG_MMC

#ifdef CONFIG_MMC
#define CONFIG_GENERIC_MMC
#define CONFIG_ZYNQ_MMC
#define CONFIG_CMD_MMC
#define CONFIG_CMD_FAT
#define CONFIG_CMD_EXT2
#define CONFIG_DOS_PARTITION
#endif
#endif

#endif /* __CONFIG_ZYNQ_ZC770_H */
