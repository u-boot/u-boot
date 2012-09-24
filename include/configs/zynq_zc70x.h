/*
 * (C) Copyright 2012 Xilinx
 *
 * Configuration settings for the Xilinx Zynq ZC702 and ZC706 boards
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

#ifndef __CONFIG_ZYNQ_ZC70X_H
#define __CONFIG_ZYNQ_ZC70X_H

#define PHYS_SDRAM_1_SIZE (1024 * 1024 * 1024)

#define CONFIG_ZYNQ_SERIAL_UART1
#define CONFIG_ZYNQ_GEM0
#define CONFIG_PHY_ADDR	7

#define CONFIG_SYS_NO_FLASH


#include <configs/zynq_common.h>


#define CONFIG_CMD_SAVEENV	/* Command to save ENV to Flash */

#define CONFIG_AUTO_COMPLETE
#define CONFIG_CMDLINE_EDITING
#define CONFIG_SYS_LONGHELP

/*
 * SPI Settings
 */
#define CONFIG_ZYNQ_SPI
#define CONFIG_CMD_SPI
#define CONFIG_SF_DEFAULT_SPEED 30000000
#define CONFIG_SPI_FLASH
#define CONFIG_CMD_SF
#define CONFIG_SPI_FLASH_STMICRO
#define CONFIG_SPI_FLASH_SPANSION

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

#endif /* __CONFIG_ZYNQ_ZC70X_H */
