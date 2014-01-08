/*
 * (C) Copyright 2012 Michal Simek <monstr@monstr.eu>
 * (C) Copyright 2013 Xilinx, Inc.
 *
 * Common configuration options for all Zynq boards.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __CONFIG_ZYNQ_COMMON_H
#define __CONFIG_ZYNQ_COMMON_H

/* High Level configuration Options */
#define CONFIG_ARMV7
#define CONFIG_ZYNQ

/* CPU clock */
#ifndef CONFIG_CPU_FREQ_HZ
# define CONFIG_CPU_FREQ_HZ	800000000
#endif

/* Cache options */
#define CONFIG_CMD_CACHE
#define CONFIG_SYS_CACHELINE_SIZE	32

#define CONFIG_SYS_L2CACHE_OFF
#ifndef CONFIG_SYS_L2CACHE_OFF
# define CONFIG_SYS_L2_PL310
# define CONFIG_SYS_PL310_BASE		0xf8f02000
#endif

/* Serial drivers */
#define CONFIG_BAUDRATE		115200
/* The following table includes the supported baudrates */
#define CONFIG_SYS_BAUDRATE_TABLE  \
	{300, 600, 1200, 2400, 4800, 9600, 19200, 38400, 57600, 115200, 230400}

/* Zynq Serial driver */
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
# define CONFIG_ZYNQ_SERIAL
#endif

/* DCC driver */
#if defined(CONFIG_ZYNQ_DCC)
# define CONFIG_ARM_DCC
# define CONFIG_CPU_V6 /* Required by CONFIG_ARM_DCC */
#endif

/* Ethernet driver */
#if defined(CONFIG_ZYNQ_GEM0) || defined(CONFIG_ZYNQ_GEM1)
# define CONFIG_NET_MULTI
# define CONFIG_ZYNQ_GEM
# define CONFIG_MII
# define CONFIG_SYS_FAULT_ECHO_LINK_DOWN
# define CONFIG_PHYLIB
# define CONFIG_PHY_MARVELL
#endif

/* SPI */
#ifdef CONFIG_ZYNQ_SPI
# define CONFIG_SPI_FLASH
# define CONFIG_SPI_FLASH_SST
# define CONFIG_CMD_SF
#endif

/* NOR */
#ifndef CONFIG_SYS_NO_FLASH
# define CONFIG_SYS_FLASH_BASE		0xE2000000
# define CONFIG_SYS_FLASH_SIZE		(16 * 1024 * 1024)
# define CONFIG_SYS_MAX_FLASH_BANKS	1
# define CONFIG_SYS_MAX_FLASH_SECT	512
# define CONFIG_SYS_FLASH_ERASE_TOUT	1000
# define CONFIG_SYS_FLASH_WRITE_TOUT	5000
# define CONFIG_FLASH_SHOW_PROGRESS	10
# define CONFIG_SYS_FLASH_CFI
# undef CONFIG_SYS_FLASH_EMPTY_INFO
# define CONFIG_FLASH_CFI_DRIVER
# undef CONFIG_SYS_FLASH_PROTECTION
# define CONFIG_SYS_FLASH_USE_BUFFER_WRITE
#endif

/* MMC */
#if defined(CONFIG_ZYNQ_SDHCI0) || defined(CONFIG_ZYNQ_SDHCI1)
# define CONFIG_MMC
# define CONFIG_GENERIC_MMC
# define CONFIG_SDHCI
# define CONFIG_ZYNQ_SDHCI
# define CONFIG_CMD_MMC
# define CONFIG_CMD_FAT
# define CONFIG_SUPPORT_VFAT
# define CONFIG_CMD_EXT2
# define CONFIG_DOS_PARTITION
#endif

/* I2C */
#if defined(CONFIG_ZYNQ_I2C0) || defined(CONFIG_ZYNQ_I2C1)
# define CONFIG_CMD_I2C
# define CONFIG_SYS_I2C
# define CONFIG_SYS_I2C_ZYNQ
# define CONFIG_SYS_I2C_ZYNQ_SPEED		100000
# define CONFIG_SYS_I2C_ZYNQ_SLAVE		1
#endif

/* EEPROM */
#ifdef CONFIG_ZYNQ_EEPROM
# define CONFIG_CMD_EEPROM
# define CONFIG_SYS_I2C_EEPROM_ADDR_LEN		1
# define CONFIG_SYS_I2C_EEPROM_ADDR		0x54
# define CONFIG_SYS_EEPROM_PAGE_WRITE_BITS	4
# define CONFIG_SYS_EEPROM_PAGE_WRITE_DELAY_MS	5
# define CONFIG_SYS_EEPROM_SIZE			1024 /* Bytes */
#endif

#define CONFIG_BOOTP_SERVERIP
#define CONFIG_BOOTP_BOOTPATH
#define CONFIG_BOOTP_GATEWAY
#define CONFIG_BOOTP_HOSTNAME
#define CONFIG_BOOTP_MAY_FAIL

/* Total Size of Environment Sector */
#define CONFIG_ENV_SIZE			(128 << 10)

/* Allow to overwrite serial and ethaddr */
#define CONFIG_ENV_OVERWRITE

/* Environment */
#ifndef CONFIG_ENV_IS_NOWHERE
# ifndef CONFIG_SYS_NO_FLASH
#  define CONFIG_ENV_IS_IN_FLASH
# elif defined(CONFIG_SYS_NO_FLASH)
#  define CONFIG_ENV_IS_NOWHERE
# endif

# define CONFIG_ENV_SECT_SIZE		CONFIG_ENV_SIZE
# define CONFIG_ENV_OFFSET		0xE0000
# define CONFIG_CMD_SAVEENV
#endif

/* Default environment */
#define CONFIG_EXTRA_ENV_SETTINGS	\
	"fit_image=fit.itb\0"		\
	"load_addr=0x2000000\0"		\
	"fit_size=0x800000\0"		\
	"flash_off=0x100000\0"		\
	"nor_flash_off=0xE2100000\0"	\
	"fdt_high=0x20000000\0"		\
	"initrd_high=0x20000000\0"	\
	"norboot=echo Copying FIT from NOR flash to RAM... && " \
		"cp.b ${nor_flash_off} ${load_addr} ${fit_size} && " \
		"bootm ${load_addr}\0" \
	"sdboot=echo Copying FIT from SD to RAM... && " \
		"fatload mmc 0 ${load_addr} ${fit_image} && " \
		"bootm ${load_addr}\0" \
	"jtagboot=echo TFTPing FIT to RAM... && " \
		"tftp ${load_addr} ${fit_image} && " \
		"bootm ${load_addr}\0"
#define CONFIG_BOOTCOMMAND		"run $modeboot"
#define CONFIG_BOOTDELAY		3 /* -1 to Disable autoboot */
#define CONFIG_SYS_LOAD_ADDR		0 /* default? */

/* Miscellaneous configurable options */
#define CONFIG_SYS_PROMPT		"zynq-uboot> "
#define CONFIG_SYS_HUSH_PARSER

#define CONFIG_CMDLINE_EDITING
#define CONFIG_AUTO_COMPLETE
#define CONFIG_BOARD_LATE_INIT
#define CONFIG_SYS_LONGHELP
#define CONFIG_SYS_MAXARGS		15 /* max number of command args */
#define CONFIG_SYS_CBSIZE		256 /* Console I/O Buffer Size */
#define CONFIG_SYS_PBSIZE		(CONFIG_SYS_CBSIZE + \
					sizeof(CONFIG_SYS_PROMPT) + 16)

/* Physical Memory map */
#define CONFIG_SYS_TEXT_BASE		0x4000000

#define CONFIG_NR_DRAM_BANKS		1
#define CONFIG_SYS_SDRAM_BASE		0

#define CONFIG_SYS_MEMTEST_START	CONFIG_SYS_SDRAM_BASE
#define CONFIG_SYS_MEMTEST_END		(CONFIG_SYS_SDRAM_BASE + 0x1000)

#define CONFIG_SYS_MALLOC_LEN		0x400000
#define CONFIG_SYS_INIT_RAM_ADDR	CONFIG_SYS_SDRAM_BASE
#define CONFIG_SYS_INIT_RAM_SIZE	CONFIG_SYS_MALLOC_LEN
#define CONFIG_SYS_INIT_SP_ADDR		(CONFIG_SYS_INIT_RAM_ADDR + \
					CONFIG_SYS_INIT_RAM_SIZE - \
					GENERATED_GBL_DATA_SIZE)

/* Enable the PL to be downloaded */
#define CONFIG_FPGA
#define CONFIG_FPGA_XILINX
#define CONFIG_FPGA_ZYNQPL
#define CONFIG_CMD_FPGA

/* Open Firmware flat tree */
#define CONFIG_OF_LIBFDT

/* FIT support */
#define CONFIG_FIT
#define CONFIG_FIT_VERBOSE	1 /* enable fit_format_{error,warning}() */

/* FDT support */
#define CONFIG_OF_CONTROL
#define CONFIG_OF_SEPARATE
#define CONFIG_DISPLAY_BOARDINFO_LATE

/* RSA support */
#define CONFIG_FIT_SIGNATURE
#define CONFIG_RSA

/* Boot FreeBSD/vxWorks from an ELF image */
#if defined(CONFIG_ZYNQ_BOOT_FREEBSD)
# define CONFIG_API
# define CONFIG_CMD_ELF
# define CONFIG_SYS_MMC_MAX_DEVICE	1
#endif

/* Commands */
#include <config_cmd_default.h>

#define CONFIG_CMD_PING
#define CONFIG_CMD_DHCP
#define CONFIG_CMD_MII

#endif /* __CONFIG_ZYNQ_COMMON_H */
