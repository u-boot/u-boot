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

#include <configs/zynq_common.h>

#undef CONFIG_ZYNQ_XIL_LQSPI

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

/* Uncomment it if you don't want Flash */
//#define CONFIG_SYS_NO_FLASH

#include <config_cmd_default.h>
#define CONFIG_CMD_DATE		/* RTC? */
#define CONFIG_CMD_PING		/* Might be useful for debugging */
#define CONFIG_CMD_SAVEENV	/* Command to save ENV to Flash */
#define CONFIG_REGINFO		/* Again, debugging */
#undef CONFIG_CMD_SETGETDCR	/* README says 4xx only */

#define CONFIG_TIMESTAMP	/* print image timestamp on bootm, etc */

#define CONFIG_PANIC_HANG /* For development/debugging */

#define CONFIG_AUTO_COMPLETE
#define CONFIG_CMDLINE_EDITING

#undef CONFIG_FIT
#undef CONFIG_FIT_VERBOSE

/* this is to initialize GEM at uboot start */
/* #define CONFIG_ZYNQ_INIT_GEM	*/
/* this is to set ipaddr, ethaddr and serverip env variables. */
#define CONFIG_ZYNQ_IP_ENV

/*
 * NOR Flash Settings
 */
#ifndef CONFIG_SYS_NO_FLASH

#define CONFIG_SYS_FLASH_BASE           0xE2000000
#define CONFIG_SYS_FLASH_SIZE           (16 * 1024 * 1024)
#define CONFIG_SYS_MAX_FLASH_BANKS      1
/* max number of sectors/blocks on one chip */
#define CONFIG_SYS_MAX_FLASH_SECT       512
#define CONFIG_SYS_FLASH_ERASE_TOUT     1000
#define CONFIG_SYS_FLASH_WRITE_TOUT     5000
#define CONFIG_FLASH_SHOW_PROGRESS	10

#define CONFIG_SYS_FLASH_CFI
#undef CONFIG_SYS_FLASH_EMPTY_INFO
#define CONFIG_FLASH_CFI_DRIVER
#undef CONFIG_SYS_FLASH_PROTECTION /* don't use hardware protection */
#define CONFIG_SYS_FLASH_USE_BUFFER_WRITE /* use buffered writes (20x faster) */
#define CONFIG_ENV_OFFSET		0xC0000		/*768 KB*/
#define CONFIG_ENV_SECT_SIZE    	0x20000		/*128 KB*/
#define CONFIG_ENV_IS_IN_FLASH
#else
#define CONFIG_ENV_IS_NOWHERE
#endif

/* HW to use */
# define CONFIG_UART0
#define CONFIG_TTC0
#define CONFIG_GEM0
#define CONFIG_ZYNQ_GEM
#define CONFIG_XGMAC_PHY_ADDR 0x17

/*
 * Physical Memory map
 */
#define PHYS_SDRAM_1_SIZE (256 * 1024 * 1024)

/*
 * SPI Settings
 */
#define CONFIG_ZYNQ_SPI
#define CONFIG_CMD_SPI
#define CONFIG_SF_DEFAULT_SPEED 30000000
#define CONFIG_SPI_FLASH
#define CONFIG_CMD_SF
#define CONFIG_SPI_FLASH_SPANSION
#define CONFIG_SPI_FLASH_WINBOND
#define CONFIG_SPI_FLASH_STMICRO

/*
 * NAND Flash settings
 */
#define CONFIG_NAND_ZYNQ
#define CONFIG_CMD_NAND
#define CONFIG_CMD_NAND_LOCK_UNLOCK
#define CONFIG_SYS_MAX_NAND_DEVICE 1
#define CONFIG_SYS_NAND_BASE XPSS_NAND_BASEADDR
#define CONFIG_SYS_NAND_ONFI_DETECTION
#define CONFIG_MTD_DEVICE

/* Place a Xilinx Boot ROM header in u-boot image? */
#define CONFIG_ZYNQ_XILINX_FLASH_HEADER

#ifdef CONFIG_ZYNQ_XILINX_FLASH_HEADER
/* Address Xilinx boot rom should use to launch u-boot */
#ifdef CONFIG_ZYNQ_XIL_LQSPI
#define CONFIG_ZYNQ_XIP_START XPSS_QSPI_LIN_BASEADDR
#else
/* NOR */
#define CONFIG_ZYNQ_XIP_START CONFIG_SYS_FLASH_BASE
#endif
#endif

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

#endif /* __CONFIG_ZYNQ_EP107_H */
