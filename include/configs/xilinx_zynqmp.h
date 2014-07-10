/*
 * Configuration for Xilinx ZynqMP
 * (C) Copyright 2014 Xilinx, Inc.
 * Michal Simek <michal.simek@xilinx.com>
 *
 * Based on Configuration for Versatile Express
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __XILINX_ZYNQMP_H
#define __XILINX_ZYNQMP_H

#define CONFIG_REMAKE_ELF

/* #define CONFIG_ARMV8_SWITCH_TO_EL1 */

#define CONFIG_SYS_NO_FLASH

#define CONFIG_SYS_GENERIC_BOARD

#define XILINX_ZYNQMP

/* Generic Interrupt Controller Definitions */
#define CONFIG_GICV2
#define GICD_BASE	0xF9001000
#define GICC_BASE	0xF9002000

/* Physical Memory Map */
#define CONFIG_NR_DRAM_BANKS		1
#define CONFIG_SYS_SDRAM_BASE		0
#define CONFIG_SYS_SDRAM_SIZE		0x40000000

#define CONFIG_SYS_MEMTEST_START	0x10000000
#define CONFIG_SYS_MEMTEST_END		0x40000000

/* Have release address at the end of 256MB for now */
#define CPU_RELEASE_ADDR	0xFFFFFF0

/* Cache Definitions */
#define CONFIG_SYS_DCACHE_OFF
#define CONFIG_SYS_ICACHE_OFF

#define CONFIG_IDENT_STRING		" Xilinx ZynqMP"
#define CONFIG_BOOTP_VCI_STRING		"U-boot.armv8.Xilinx_ZynqMP"

/* Text base on 16MB for now - 0 doesn't work */
#define CONFIG_SYS_TEXT_BASE		0x100000
#define CONFIG_SYS_INIT_SP_ADDR		(CONFIG_SYS_SDRAM_BASE + 0x7fff0)

/* Flat Device Tree Definitions */
#define CONFIG_OF_LIBFDT

#define CONFIG_DEFAULT_DEVICE_TREE	zynqmp

/* Generic Timer Definitions */
#define COUNTER_FREQUENCY		0x1800000 /* 24MHz */

/* Size of malloc() pool */
#define CONFIG_SYS_MALLOC_LEN		(CONFIG_ENV_SIZE + 0x400000)

/* Serial setup */
#define CONFIG_ZYNQ_SERIAL_UART0
#define CONFIG_ZYNQ_SERIAL

#define CONFIG_ZYNQ_QSPI
#define CONFIG_ZYNQ_SDHCI0

#define CONFIG_CONS_INDEX		0
#define CONFIG_BAUDRATE			115200
#define CONFIG_SYS_BAUDRATE_TABLE	{ 9600, 19200, 38400, 57600, 115200 }

/* Command line configuration */
#define CONFIG_CMD_BDI
#define CONFIG_CMD_BOOTD
#define CONFIG_CMD_CACHE
#define CONFIG_CMD_ECHO
#define CONFIG_CMD_ENV
#define CONFIG_CMD_FAT
#define CONFIG_CMD_FLASH
#define CONFIG_CMD_IMI
#define CONFIG_CMD_MEMORY
#define CONFIG_CMD_MISC
#define CONFIG_CMD_RUN
#define CONFIG_CMD_SAVEENV
#define CONFIG_CMD_SOURCE
#define CONFIG_DOS_PARTITION

#define CONFIG_CMD_NET
#define CONFIG_CMD_PING
#define CONFIG_CMD_DHCP
#define CONFIG_CMD_MII
#define CONFIG_CMD_TFTPPUT

/* BOOTP options */
#define CONFIG_BOOTP_BOOTFILESIZE
#define CONFIG_BOOTP_BOOTPATH
#define CONFIG_BOOTP_GATEWAY
#define CONFIG_BOOTP_HOSTNAME
#define CONFIG_BOOTP_MAY_FAIL
#define CONFIG_BOOTP_SERVERIP


#ifdef CONFIG_ZYNQ_QSPI
# define CONFIG_SF_DEFAULT_SPEED        30000000
# define CONFIG_SPI_FLASH
# define CONFIG_SPI_FLASH_BAR
# define CONFIG_SPI_FLASH_SPANSION
# define CONFIG_SPI_FLASH_STMICRO
# define CONFIG_SPI_FLASH_WINBOND
# define CONFIG_CMD_SPI
# define CONFIG_CMD_SF
#endif

#if defined(CONFIG_ZYNQ_SDHCI0) || defined(CONFIG_ZYNQ_SDHCI1)
# define CONFIG_MMC
# define CONFIG_GENERIC_MMC
# define CONFIG_SDHCI
# define CONFIG_ZYNQ_SDHCI
# define CONFIG_CMD_MMC
#endif

/* Miscellaneous configurable options */
#define CONFIG_SYS_LOAD_ADDR		0x8000000

/* Initial environment variables */
#define CONFIG_EXTRA_ENV_SETTINGS       \
	"ethaddr=00:0a:35:00:01:22\0"	\
	"kernel_addr=0x200000\0"	\
	"serverip=10.0.2.2\0"	\
	"ipaddr=10.0.2.15\0"		\
	"initrd_addr=0xa00000\0"	\
	"initrd_size=0x2000000\0"	\
	"fdt_addr=0x100000\0"		\
	"fdt_high=0x10000000\0"		\
	"netboot=tftpboot 10000000 image.ub && bootm\0"	\
	"qspiboot=sf probe 0; sf read 10000000 0 1000000; bootm 10080000\0"	\
	"sdboot=mmcinfo && fatload mmc 0:0 10000000 image.ub && bootm 10000000\0"	\
	"jtagboot=tftpboot 1000000 uImage && "		\
		"tftpboot 20000000 system.dtb && "	\
		"bootm 1000000 - 20000000\0"

#define CONFIG_BOOTARGS		"console=ttyPS0"
#define CONFIG_BOOTCOMMAND	"echo Hello Xilinx ZynqMP; run $modeboot"
#define CONFIG_BOOTDELAY	5

#define CONFIG_BOARD_LATE_INIT

/* Do not preserve environment */
#define CONFIG_ENV_IS_NOWHERE		1
#define CONFIG_ENV_SIZE			0x1000

/* Monitor Command Prompt */
/* Console I/O Buffer Size */
#define CONFIG_SYS_CBSIZE		512
#define CONFIG_SYS_PROMPT		"ZynqMP> "
#define CONFIG_SYS_PBSIZE		(CONFIG_SYS_CBSIZE + \
					sizeof(CONFIG_SYS_PROMPT) + 16)
#define CONFIG_SYS_HUSH_PARSER
#define CONFIG_SYS_BARGSIZE		CONFIG_SYS_CBSIZE
#define CONFIG_SYS_LONGHELP
#define CONFIG_CMDLINE_EDITING		1
/* max command args */
#define CONFIG_SYS_MAXARGS		64

#define CONFIG_ZYNQ_GEM0
#define CONFIG_ZYNQ_GEM_PHY_ADDR0	7

/* Ethernet driver */
#if defined(CONFIG_ZYNQ_GEM0) || defined(CONFIG_ZYNQ_GEM1) || \
	defined(CONFIG_ZYNQ_GEM2) || defined(CONFIG_ZYNQ_GEM3)
# define CONFIG_NET_MULTI
# define CONFIG_ZYNQ_GEM
# define CONFIG_MII
# define CONFIG_SYS_FAULT_ECHO_LINK_DOWN
# define CONFIG_PHYLIB
# define CONFIG_PHY_MARVELL
#endif

#define CONFIG_FIT
#define CONFIG_FIT_VERBOSE       /* enable fit_format_{error,warning}() */

#define CONFIG_SYS_BOOTM_LEN	(60 * 1024 * 1024)

#endif /* __XILINX_ZYNQMP_H */
