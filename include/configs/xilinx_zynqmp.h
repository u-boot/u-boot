/*
 * Configuration for Xilinx ZynqMP
 * (C) Copyright 2014 - 2015 Xilinx, Inc.
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

/* Generic Interrupt Controller Definitions */
#define CONFIG_GICV2
#define GICD_BASE	0xF9010000
#define GICC_BASE	0xF9020000

/* Physical Memory Map */
#define CONFIG_NR_DRAM_BANKS		1
#define CONFIG_SYS_SDRAM_BASE		0
#define CONFIG_SYS_SDRAM_SIZE		0x40000000

#define CONFIG_SYS_MEMTEST_START	CONFIG_SYS_SDRAM_BASE
#define CONFIG_SYS_MEMTEST_END		CONFIG_SYS_SDRAM_SIZE

/* Have release address at the end of 256MB for now */
#define CPU_RELEASE_ADDR	0xFFFFFF0

/* Cache Definitions */
#define CONFIG_SYS_CACHELINE_SIZE	64

#define CONFIG_IDENT_STRING		" Xilinx ZynqMP"

#define CONFIG_SYS_INIT_SP_ADDR		(CONFIG_SYS_SDRAM_BASE + 0x7fff0)

/* Flat Device Tree Definitions */
#define CONFIG_OF_LIBFDT

/* Generic Timer Definitions - setup in EL3. Setup by ATF for other cases */
#define COUNTER_FREQUENCY		4000000

/* Size of malloc() pool */
#define CONFIG_SYS_MALLOC_LEN		(CONFIG_ENV_SIZE + 0x2000000)

/* Serial setup */
#if defined(CONFIG_ZYNQMP_DCC)
# define CONFIG_ARM_DCC
# define CONFIG_CPU_ARMV8
#else
# if defined(CONFIG_ZYNQ_SERIAL_UART0) || defined(CONFIG_ZYNQ_SERIAL_UART1)
#  define CONFIG_ZYNQ_SERIAL
# endif
#endif

#define CONFIG_CONS_INDEX		0
#define CONFIG_BAUDRATE			115200
#define CONFIG_SYS_BAUDRATE_TABLE \
	{ 4800, 9600, 19200, 38400, 57600, 115200 }

/* Command line configuration */
#define CONFIG_CMD_ENV
#define CONFIG_CMD_EXT2
#define CONFIG_CMD_EXT4
#define CONFIG_CMD_FAT
#define CONFIG_CMD_FS_GENERIC
#define CONFIG_DOS_PARTITION
#define CONFIG_CMD_ELF
#define CONFIG_MP

#define CONFIG_CMD_MII

/* BOOTP options */
#define CONFIG_BOOTP_BOOTFILESIZE
#define CONFIG_BOOTP_BOOTPATH
#define CONFIG_BOOTP_GATEWAY
#define CONFIG_BOOTP_HOSTNAME
#define CONFIG_BOOTP_MAY_FAIL
#define CONFIG_BOOTP_SERVERIP

/* SPI */
#ifdef CONFIG_ZYNQ_SPI
# define CONFIG_SPI_FLASH_SST
# define CONFIG_CMD_SF
#endif

#if defined(CONFIG_ZYNQ_SDHCI0) || defined(CONFIG_ZYNQ_SDHCI1)
# define CONFIG_MMC
# define CONFIG_GENERIC_MMC
# define CONFIG_SDHCI
# define CONFIG_ZYNQ_SDHCI
# define CONFIG_CMD_MMC
#endif

#if defined(CONFIG_ZYNQ_SDHCI)
# define CONFIG_FAT_WRITE
# define CONFIG_CMD_EXT4_WRITE
#endif

/* Miscellaneous configurable options */
#define CONFIG_SYS_LOAD_ADDR		0x8000000

#if defined(CONFIG_ZYNQMP_USB)
#define CONFIG_USB_DWC3
#define CONFIG_USB_DWC3_GADGET

#define CONFIG_USB_GADGET
#define CONFIG_USB_GADGET_DOWNLOAD
#define CONFIG_USB_GADGET_DUALSPEED
#define CONFIG_USB_GADGET_VBUS_DRAW	2
#define CONFIG_USBDOWNLOAD_GADGET
#define CONFIG_SYS_DFU_DATA_BUF_SIZE	0x1800000
#define DFU_DEFAULT_POLL_TIMEOUT	300
#define CONFIG_USB_FUNCTION_DFU
#define CONFIG_DFU_RAM
#define CONFIG_G_DNL_VENDOR_NUM		0x03FD
#define CONFIG_G_DNL_PRODUCT_NUM	0x0300
#define CONFIG_G_DNL_MANUFACTURER	"Xilinx"
#define CONFIG_USB_CABLE_CHECK
#define CONFIG_CMD_DFU
#define CONFIG_CMD_THOR_DOWNLOAD
#define CONFIG_USB_FUNCTION_THOR
#define CONFIG_THOR_RESET_OFF
#define DFU_ALT_INFO_RAM \
	"dfu_ram_info=" \
	"set dfu_alt_info " \
	"Image ram 0x200000 0x1800000\\\\;" \
	"system.dtb ram 0x7000000 0x40000\0" \
	"dfu_ram=run dfu_ram_info && dfu 0 ram 0\0" \
	"thor_ram=run dfu_ram_info && thordown 0 ram 0\0"

#define DFU_ALT_INFO  \
		DFU_ALT_INFO_RAM
#endif

#if !defined(DFU_ALT_INFO)
# define DFU_ALT_INFO
#endif

/* Initial environment variables */
#define CONFIG_EXTRA_ENV_SETTINGS \
	"kernel_addr=0x80000\0" \
	"fdt_addr=0x7000000\0" \
	"fdt_high=0x10000000\0" \
	"sdboot=mmcinfo && load mmc 0:0 $fdt_addr system.dtb && " \
		"load mmc 0:0 $kernel_addr Image && booti $kernel_addr - $fdt_addr\0" \
	DFU_ALT_INFO

#define CONFIG_BOOTARGS		"setenv bootargs console=ttyPS0,${baudrate} " \
				"earlycon=cdns,mmio,0xff000000,${baudrate}n8"
#define CONFIG_PREBOOT		"run bootargs"
#define CONFIG_BOOTCOMMAND	"run $modeboot"
#define CONFIG_BOOTDELAY	5

#define CONFIG_BOARD_LATE_INIT

/* Do not preserve environment */
#define CONFIG_ENV_IS_NOWHERE		1
#define CONFIG_ENV_SIZE			0x1000

/* Monitor Command Prompt */
/* Console I/O Buffer Size */
#define CONFIG_SYS_CBSIZE		2048
#define CONFIG_SYS_PBSIZE		(CONFIG_SYS_CBSIZE + \
					sizeof(CONFIG_SYS_PROMPT) + 16)
#define CONFIG_SYS_HUSH_PARSER
#define CONFIG_SYS_BARGSIZE		CONFIG_SYS_CBSIZE
#define CONFIG_SYS_LONGHELP
#define CONFIG_CMDLINE_EDITING
#define CONFIG_SYS_MAXARGS		64

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

/* I2C */
#if defined(CONFIG_SYS_I2C_ZYNQ)
# define CONFIG_CMD_I2C
# define CONFIG_SYS_I2C
# define CONFIG_SYS_I2C_ZYNQ_SPEED		100000
# define CONFIG_SYS_I2C_ZYNQ_SLAVE		0
#endif

/* EEPROM */
#ifdef CONFIG_ZYNQMP_EEPROM
# define CONFIG_CMD_EEPROM
# define CONFIG_SYS_I2C_EEPROM_ADDR_LEN		2
# define CONFIG_SYS_I2C_EEPROM_ADDR		0x54
# define CONFIG_SYS_EEPROM_PAGE_WRITE_BITS	4
# define CONFIG_SYS_EEPROM_PAGE_WRITE_DELAY_MS	5
# define CONFIG_SYS_EEPROM_SIZE			(64 * 1024)
#endif

#ifdef CONFIG_AHCI
#define CONFIG_LIBATA
#define CONFIG_SCSI_AHCI
#define CONFIG_SCSI_AHCI_PLAT
#define CONFIG_SYS_SCSI_MAX_SCSI_ID	1
#define CONFIG_SYS_SCSI_MAX_LUN		1
#define CONFIG_SYS_SCSI_MAX_DEVICE	(CONFIG_SYS_SCSI_MAX_SCSI_ID * \
					 CONFIG_SYS_SCSI_MAX_LUN)
#define CONFIG_CMD_SCSI
#endif

#define CONFIG_FIT
#define CONFIG_FIT_VERBOSE       /* enable fit_format_{error,warning}() */

#define CONFIG_SYS_BOOTM_LEN	(60 * 1024 * 1024)

#define CONFIG_CMD_BOOTI
#define CONFIG_CMD_UNZIP

#define CONFIG_BOARD_EARLY_INIT_R
#define CONFIG_CLOCKS

#endif /* __XILINX_ZYNQMP_H */
