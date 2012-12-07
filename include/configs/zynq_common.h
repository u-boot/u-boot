/*
 * (C) Copyright 2012 Xilinx
 *
 * Xilinx Zynq common configuration settings
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

#ifndef __CONFIG_ZYNQ_COMMON_H
#define __CONFIG_ZYNQ_COMMON_H

/*
 * High Level Configuration Options
 */
#define CONFIG_ARMV7 /* CPU */
#define CONFIG_ZYNQ /* SoC */

/* Default environment */
#define CONFIG_IPADDR   192.168.0.99
#define CONFIG_SERVERIP 192.168.0.101

#define CONFIG_SYS_SDRAM_BASE	0
#define CONFIG_SYS_SDRAM_SIZE	PHYS_SDRAM_1_SIZE

/* Total Size of Environment Sector */
#define CONFIG_ENV_SIZE		(128 << 10)
/* allow to overwrite serial and ethaddr */
#define CONFIG_ENV_OVERWRITE

/* Serial drivers */
#define CONFIG_BAUDRATE		115200
#define CONFIG_SYS_BAUDRATE_TABLE { 9600, 38400, 115200 }

/* Zynq serial driver */
#ifdef CONFIG_ZYNQ_SERIAL_UART0
# define CONFIG_ZYNQ_SERIAL_BASEADDR0	0xE0000000
# define CONFIG_ZYNQ_SERIAL_BAUDRATE0	CONFIG_BAUDRATE
# define CONFIG_ZYNQ_SERIAL_CLOCK0	50000000
#endif

#ifdef CONFIG_ZYNQ_SERIAL_UART1
# define CONFIG_ZYNQ_SERIAL_BASEADDR1	0xE0001000
# define CONFIG_ZYNQ_SERIAL_BAUDRATE1	CONFIG_BAUDRATE
# define CONFIG_ZYNQ_SERIAL_CLOCK1	50000000
#endif

#if defined(CONFIG_ZYNQ_SERIAL_UART0) || defined(CONFIG_ZYNQ_SERIAL_UART1)
#define CONFIG_SERIAL_MULTI
#define CONFIG_ZYNQ_SERIAL
#endif

/* Ethernet driver */
#ifdef CONFIG_ZYNQ_GEM0
# define CONFIG_ZYNQ_GEM_BASEADDR0	0xE000B000
#endif

#ifdef CONFIG_ZYNQ_GEM1
# define CONFIG_ZYNQ_GEM_BASEADDR1	0xE000C000
#endif

#if defined(CONFIG_ZYNQ_GEM0) || defined(CONFIG_ZYNQ_GEM1)
# define CONFIG_NET_MULTI
# define CONFIG_ZYNQ_GEM
# define CONFIG_MII
# define CONFIG_SYS_FAULT_ECHO_LINK_DOWN
# define CONFIG_PHYLIB
# define CONFIG_PHY_MARVELL
# define CONFIG_SYS_ENET
#endif

/* SCU timer address is hardcoded */
#define CONFIG_SCUTIMER_BASEADDR       0xF8F00600
#ifndef CONFIG_CPU_FREQ_HZ
#define CONFIG_CPU_FREQ_HZ     800000000
#endif
#define CONFIG_SYS_HZ          1000

#define CONFIG_AUTO_COMPLETE
#define CONFIG_SYS_HUSH_PARSER	/* use "hush" command parser */
#define CONFIG_SYS_PROMPT_HUSH_PS2	"> "
#define CONFIG_CMDLINE_EDITING
#define CONFIG_SYS_LONGHELP

#include <config_cmd_default.h>

#ifdef CONFIG_SYS_ENET
# define CONFIG_CMD_PING
# define CONFIG_CMD_MII
#else
# undef CONFIG_CMD_NET
# undef CONFIG_CMD_NFS
#endif

/* NOR flash */
#ifndef CONFIG_SYS_NO_FLASH
# define CONFIG_SYS_FLASH_BASE           0xE2000000
# define CONFIG_SYS_FLASH_SIZE           (16 * 1024 * 1024)
# define CONFIG_SYS_MAX_FLASH_BANKS      1
/* max number of sectors/blocks on one chip */
# define CONFIG_SYS_MAX_FLASH_SECT       512
# define CONFIG_SYS_FLASH_ERASE_TOUT     1000
# define CONFIG_SYS_FLASH_WRITE_TOUT     5000
# define CONFIG_FLASH_SHOW_PROGRESS	10

# define CONFIG_SYS_FLASH_CFI
# undef CONFIG_SYS_FLASH_EMPTY_INFO
# define CONFIG_FLASH_CFI_DRIVER

# undef CONFIG_SYS_FLASH_PROTECTION /* don't use hardware protection */
# define CONFIG_SYS_FLASH_USE_BUFFER_WRITE /* use buffered writes (20x faster) */
#endif

#ifdef CONFIG_ZYNQ_SPI
# define CONFIG_SF_DEFAULT_SPEED	30000000
# define CONFIG_SPI_FLASH
/* # define CONFIG_SPI_FLASH_ATMEL */
# define CONFIG_SPI_FLASH_SPANSION
# define CONFIG_SPI_FLASH_STMICRO
# define CONFIG_SPI_FLASH_WINBOND
# define CONFIG_CMD_SPI
# define CONFIG_CMD_SF
#endif

#ifdef CONFIG_MMC
# define CONFIG_GENERIC_MMC
# define CONFIG_ZYNQ_MMC
# define CONFIG_CMD_MMC
# define CONFIG_CMD_FAT
# define CONFIG_CMD_EXT2
# define CONFIG_DOS_PARTITION
/* For now, use only single block reads for the MMC */
# define CONFIG_SYS_MMC_MAX_BLK_COUNT	1
#endif

#ifdef CONFIG_NAND_ZYNQ
# define CONFIG_CMD_NAND
# define CONFIG_CMD_NAND_LOCK_UNLOCK
# define CONFIG_SYS_MAX_NAND_DEVICE 1
# define CONFIG_SYS_NAND_BASE XPSS_NAND_BASEADDR
# define CONFIG_SYS_NAND_ONFI_DETECTION
# define CONFIG_MTD_DEVICE
#endif

#ifndef CONFIG_ENV_IS_NOWHERE
# ifndef CONFIG_SYS_NO_FLASH
/* Environment in NOR flash */
#  define CONFIG_ENV_IS_IN_FLASH
# elif defined(CONFIG_ZYNQ_SPI)
/* Environment in Serial Flash */
#  define CONFIG_ENV_IS_IN_SPI_FLASH
# elif defined(CONFIG_NAND_ZYNQ)
/* Environment in NAND flash */
#  define CONFIG_ENV_IS_IN_NAND
# endif

# define CONFIG_ENV_SECT_SIZE		CONFIG_ENV_SIZE
# define CONFIG_ENV_OFFSET		0xE0000
# define CONFIG_CMD_SAVEENV	/* Command to save ENV to Flash */
#endif

/* For development/debugging */
#ifdef DEBUG
# define CONFIG_CMD_REGINFO
# define CONFIG_PANIC_HANG
#endif

#define CONFIG_SYS_TEXT_BASE 0x04000000

/*
 * Open Firmware flat tree
 */
#define CONFIG_OF_LIBFDT

/* Default environment */
#define CONFIG_EXTRA_ENV_SETTINGS	\
	"ethaddr=00:0a:35:00:01:22\0"	\
	"kernel_image=uImage\0"	\
	"ramdisk_image=uramdisk.image.gz\0"	\
	"devicetree_image=devicetree.dtb\0"	\
	"kernel_size=0x500000\0"	\
	"devicetree_size=0x20000\0"	\
	"ramdisk_size=0x5E0000\0"	\
	"fdt_high=0x20000000\0"	\
	"initrd_high=0x20000000\0"	\
	"norboot=echo Copying Linux from NOR flash to RAM...;" \
		"cp 0xE2100000 0x3000000 ${kernel_size};" \
		"cp 0xE2600000 0x2A00000 ${devicetree_size};" \
		"echo Copying ramdisk...;" \
		"cp 0xE2620000 0x2000000 ${ramdisk_size};" \
		"bootm 0x3000000 0x2000000 0x2A00000\0" \
	"qspiboot=echo Copying Linux from QSPI flash to RAM...;" \
		"sf probe 0 0 0;" \
		"sf read 0x3000000 0x100000 ${kernel_size};" \
		"sf read 0x2A00000 0x600000 ${devicetree_size};" \
		"echo Copying ramdisk...;" \
		"sf read 0x2000000 0x620000 ${ramdisk_size};" \
		"bootm 0x3000000 0x2000000 0x2A00000\0" \
	"sdboot=echo Copying Linux from SD to RAM...;" \
		"mmcinfo;" \
		"fatload mmc 0 0x3000000 ${kernel_image};" \
		"fatload mmc 0 0x2A00000 ${devicetree_image};" \
		"fatload mmc 0 0x2000000 ${ramdisk_image};" \
		"bootm 0x3000000 0x2000000 0x2A00000\0" \
	"nandboot=echo Copying Linux from NAND flash to RAM...;" \
		"nand read 0x3000000 0x100000 ${kernel_size};" \
		"nand read 0x2A00000 0x600000 ${devicetree_size};" \
		"echo Copying ramdisk...;" \
		"nand read 0x2000000 0x620000 ${ramdisk_size};" \
		"bootm 0x3000000 0x2000000 0x2A00000\0" \
	"jtagboot=echo TFTPing Linux to RAM...;" \
		"tftp 0x3000000 ${kernel_image};" \
		"tftp 0x2A00000 ${devicetree_image};" \
		"tftp 0x2000000 ${ramdisk_image};" \
		"bootm 0x3000000 0x2000000 0x2A00000\0"

/* default boot is according to the bootmode switch settings */
#define CONFIG_BOOTCOMMAND "run modeboot"

#define CONFIG_BOOTDELAY	3 /* -1 to Disable autoboot */

#define CONFIG_SYS_PROMPT	"zynq-uboot> "

/* CONFIG_SYS_MONITOR_BASE? */
/* CONFIG_SYS_MONITOR_LEN? */

/* Keep L2 Cache Disabled */
#define CONFIG_SYS_L2CACHE_OFF
#define CONFIG_SYS_CACHELINE_SIZE	32

/*
 * Physical Memory map
 */
#define CONFIG_NR_DRAM_BANKS    	1
#define PHYS_SDRAM_1            	0

#define CONFIG_SYS_MEMTEST_START PHYS_SDRAM_1
#define CONFIG_SYS_MEMTEST_END (CONFIG_SYS_MEMTEST_START + \
				    PHYS_SDRAM_1_SIZE - (16 * 1024 * 1024))

#define CONFIG_SYS_SDRAM_BASE		0
#define CONFIG_SYS_INIT_RAM_ADDR	0xFFFF0000
#define CONFIG_SYS_INIT_RAM_SIZE	0x1000
#define CONFIG_SYS_INIT_SP_ADDR		(CONFIG_SYS_INIT_RAM_ADDR + \
					CONFIG_SYS_INIT_RAM_SIZE - \
					GENERATED_GBL_DATA_SIZE)

#define CONFIG_SYS_MALLOC_LEN 0x400000
#define CONFIG_SYS_MAXARGS 16
#define CONFIG_SYS_CBSIZE 2048
#define CONFIG_SYS_PBSIZE (CONFIG_SYS_CBSIZE + sizeof(CONFIG_SYS_PROMPT) + 16)


#define CONFIG_SYS_LOAD_ADDR	0 /* default? */

/* Enable the PL to be downloaded */

#define CONFIG_FPGA
#define CONFIG_FPGA_XILINX
#define CONFIG_FPGA_ZYNQPL
#define CONFIG_CMD_FPGA

/* FIT support */
#define CONFIG_FIT		1
#define CONFIG_FIT_VERBOSE	1 /* enable fit_format_{error,warning}() */

#define CONFIG_BOARD_LATE_INIT

#undef CONFIG_BOOTM_NETBSD


/* FIXME this should be removed pretty soon */
#define XPSS_QSPI_BASEADDR		0xE000D000
#define SD_BASEADDR			0xE0100000
#define XPSS_NAND_BASEADDR		0xE1000000
#define XPSS_CRTL_PARPORT_BASEADDR	0xE000E000

#define RTL_30
#ifdef RTL_30
# define XPSS_QSPI_LIN_BASEADDR		0xFC000000
# define XPSS_SYS_CTRL_BASEADDR		0xF8000000
# define XPSS_DDR_CTRL_BASEADDR		0xF8006000
# define XPSS_SCU_BASEADDR		0xF8F00000
# define XPSS_DEV_CFG_APB_BASEADDR	0xF8007000
#else
# define XPSS_QSPI_LIN_BASEADDR		0xE6000000
# define XPSS_SYS_CTRL_BASEADDR		0xFE000000
# define XPSS_DDR_CTRL_BASEADDR		0xFE006000
# define XPSS_SCU_BASEADDR		0xFEF00000
# define XPSS_DEV_CFG_APB_BASEADDR	0xFE007000
#endif

#endif /* __CONFIG_ZYNQ_COMMON_H */
