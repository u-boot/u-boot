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
# define CONFIG_MMC
# define CONFIG_ZYNQ_SPI

#elif defined(CONFIG_ZC770_XM011)
# define CONFIG_ZYNQ_SERIAL_UART1
# define CONFIG_NAND_ZYNQ

#elif defined(CONFIG_ZC770_XM012)
# define CONFIG_ZYNQ_SERIAL_UART1
# undef CONFIG_SYS_NO_FLASH

#elif defined(CONFIG_ZC770_XM013)
# define CONFIG_ZYNQ_SERIAL_UART0
# define CONFIG_ZYNQ_GEM1
# define CONFIG_PHY_ADDR	7
# define CONFIG_ZYNQ_SPI

#else
# define CONFIG_ZYNQ_SERIAL_UART0
#endif

#include <configs/zynq_common.h>

/* Place a Xilinx Boot ROM header in u-boot image? */
#if defined(CONFIG_ZC770_XM010)
#define CONFIG_ZYNQ_XILINX_FLASH_HEADER
#define CONFIG_ZYNQ_XIP_START XPSS_QSPI_LIN_BASEADDR
#endif

#endif /* __CONFIG_ZYNQ_ZC770_H */
