/*
 * (C) Copyright 2012 Xilinx
 *
 * Configuration settings for the Xilinx Zynq EP107 board.
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

#ifndef __CONFIG_ZYNQ_EP107_H
#define __CONFIG_ZYNQ_EP107_H

#define CONFIG_EP107 /* Board */

#define PHYS_SDRAM_1_SIZE (256 * 1024 * 1024)

#define CONFIG_ZYNQ_SERIAL_UART0
#define CONFIG_ZYNQ_GEM0
#define CONFIG_PHY_ADDR	23

#define CONFIG_CPU_FREQ_HZ	12500000

#define CONFIG_MMC
#define CONFIG_ZYNQ_SPI
#define CONFIG_NAND_ZYNQ

#include <configs/zynq_common.h>

#undef CONFIG_EXTRA_ENV_SETTINGS
#define CONFIG_EXTRA_ENV_SETTINGS	\
	"ethaddr=00:0a:35:00:01:22\0"	\
	"kernel_size=0x140000\0"	\
	"ramdisk_size=0x200000\0"	\
	"nand_kernel_size=0x400000\0"	\
	"nand_ramdisk_size=0x400000\0"	\
	"norboot=echo Copying Linux from NOR flash to RAM...;" \
		"cp 0xE2100000 0x8000 ${kernel_size};" \
		"cp 0xE2600000 0x1000000 0x8000;" \
		"echo Copying ramdisk...;" \
		"cp 0xE3000000 0x800000 ${ramdisk_size};" \
		"go 0x8000\0" \
	"qspiboot=echo Copying Linux from QSPI flash to RAM...;" \
		"cp 0xFC100000 0x8000 ${kernel_size};" \
		"cp 0xFC600000 0x1000000 0x8000;" \
		"echo Copying ramdisk...;" \
		"cp 0xFC800000 0x800000 ${ramdisk_size};" \
		"go 0x8000\0" \
	"sdboot=echo Copying Linux from SD to RAM...;" \
		"mmcinfo;" \
		"fatload mmc 0 0x8000 zImage;" \
		"fatload mmc 0 0x1000000 devicetree.dtb;" \
		"fatload mmc 0 0x800000 ramdisk8M.image.gz;" \
		"go 0x8000\0" \
	"nandboot=echo Copying Linux from NAND flash to RAM...;" \
		"nand read 0x8000 0x200000 ${nand_kernel_size};" \
		"nand read 0x1000000 0x700000 0x20000;" \
		"echo Copying ramdisk...;" \
		"nand read 0x800000 0x900000 ${nand_ramdisk_size};" \
		"go 0x8000\0" \
	"jtagboot=echo TFTPing Linux to RAM...;" \
		"tftp 0x8000 zImage;" \
		"tftp 0x1000000 devicetree.dtb;" \
		"tftp 0x800000 ramdisk8M.image.gz;" \
		"go 0x8000\0"

#undef CONFIG_FIT
#undef CONFIG_FIT_VERBOSE


/* Place a Xilinx Boot ROM header in u-boot image? */
#define CONFIG_ZYNQ_XILINX_FLASH_HEADER

#ifdef CONFIG_ZYNQ_XILINX_FLASH_HEADER
/* Address Xilinx boot rom should use to launch u-boot */
/* NOR */
#define CONFIG_ZYNQ_XIP_START CONFIG_SYS_FLASH_BASE
#endif

#endif /* __CONFIG_ZYNQ_EP107_H */
