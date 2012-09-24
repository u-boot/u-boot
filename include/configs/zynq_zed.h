/*
 * (C) Copyright 2012 Xilinx
 *
 * Configuration for Zynq Evaluation and Development Board - ZedBoard
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

#ifndef __CONFIG_ZYNQ_ZED_H
#define __CONFIG_ZYNQ_ZED_H

#define PHYS_SDRAM_1_SIZE (512 * 1024 * 1024)

#define CONFIG_ZYNQ_SERIAL_UART1
#define CONFIG_ZYNQ_GEM0
#define CONFIG_PHY_ADDR	0

#define CONFIG_SYS_NO_FLASH

#include <configs/zynq_common.h>



#undef CONFIG_EXTRA_ENV_SETTINGS
#define CONFIG_EXTRA_ENV_SETTINGS 	\
	"ethaddr=00:0a:35:00:01:22\0"	\
	"kernel_size=0x140000\0" 	\
	"ramdisk_size=0x200000\0" 	\
	"qspiboot=sf probe 0 0 0;" \
		"sf read 0x8000 0x100000 0x2c0000;" \
		"sf read 0x1000000 0x3c0000 0x40000;" \
		"sf read 0x800000 0x400000 0x800000;" \
		"go 0x8000\0" \
	"sdboot=echo Copying Linux from SD to RAM...;" \
		"mmcinfo;" \
		"fatload mmc 0 0x8000 zImage;" \
		"fatload mmc 0 0x1000000 devicetree.dtb;" \
		"fatload mmc 0 0x800000 ramdisk8M.image.gz;" \
		"go 0x8000\0" \
	"jtagboot=echo TFTPing Linux to RAM...;" \
		"tftp 0x8000 zImage;" \
		"tftp 0x1000000 devicetree.dtb;" \
		"tftp 0x800000 ramdisk8M.image.gz;" \
		"go 0x8000\0"

#define CONFIG_AUTO_COMPLETE
#define CONFIG_CMDLINE_EDITING

/*
 * SPI Settings
 */
#define CONFIG_ZYNQ_SPI
#define CONFIG_CMD_SPI
#define CONFIG_SF_DEFAULT_SPEED 30000000
#define CONFIG_SPI_FLASH
#define CONFIG_CMD_SF
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

#endif /* __CONFIG_ZYNQ_ZED_H */
