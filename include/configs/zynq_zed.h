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
#define CONFIG_PHY_ADDR	0

#define CONFIG_ZYNQ_GEM_OLD
#define CONFIG_XGMAC_PHY_ADDR CONFIG_PHY_ADDR
#define CONFIG_SYS_ENET

#define CONFIG_SYS_NO_FLASH

#define CONFIG_MMC
#define CONFIG_ZYNQ_SPI

#include <configs/zynq_common.h>

#undef CONFIG_EXTRA_ENV_SETTINGS
#define CONFIG_EXTRA_ENV_SETTINGS 	\
	"ethaddr=00:0a:35:00:01:22\0"	\
	"kernel_image=uImage\0"	\
	"ramdisk_image=uramdisk.image.gz\0"	\
	"devicetree_image=devicetree.dtb\0"	\
	"kernel_size=0x140000\0" 	\
	"ramdisk_size=0x200000\0" 	\
	"qspiboot=echo Copying Linux from QSPI flash to RAM...;" \
		"sf probe 0 0 0;" \
		"sf read 0x3000000 0x100000 0x400000;" \
		"sf read 0x2A00000 0x600000 0x20000;" \
		"echo Copying ramdisk...;" \
		"sf read 0x2000000 0x800000 0x400000;" \
		"bootm 0x3000000 0x2000000 0x2A00000\0" \
	"sdboot=echo Copying Linux from SD to RAM...;" \
		"mmcinfo;" \
		"fatload mmc 0 0x3000000 ${kernel_image};" \
		"fatload mmc 0 0x2A00000 ${devicetree_image};" \
		"fatload mmc 0 0x2000000 ${ramdisk_image};" \
		"bootm 0x3000000 0x2000000 0x2A00000\0" \
	"jtagboot=echo TFTPing Linux to RAM...;" \
		"tftp 0x3000000 ${kernel_image};" \
		"tftp 0x2A00000 ${devicetree_image};" \
		"tftp 0x2000000 ${ramdisk_image};" \
		"bootm 0x3000000 0x2000000 0x2A00000\0"

#endif /* __CONFIG_ZYNQ_ZED_H */
