/*
 * Aries MA5D4 configuration
 * Copyright (C) 2015 Marek Vasut <marex@denx.de>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __MA5D4EVK_CONFIG_H__
#define __MA5D4EVK_CONFIG_H__

#define CONFIG_TIMESTAMP		/* Print image info with timestamp */

#include "at91-sama5_common.h"
#define CONFIG_SYS_USE_SERIALFLASH	1
#define CONFIG_BOARD_LATE_INIT

/* Timer */
#define CONFIG_SYS_TIMER_COUNTER	0xfc06863c

/*
 * Memory configurations
 */
#define CONFIG_NR_DRAM_BANKS		1
#define CONFIG_SYS_SDRAM_BASE		0x20000000
#define CONFIG_SYS_SDRAM_SIZE		0x10000000

#ifdef CONFIG_SPL_BUILD
#define CONFIG_SYS_INIT_SP_ADDR		0x210000
#else
#define CONFIG_SYS_INIT_SP_ADDR \
	(CONFIG_SYS_SDRAM_BASE + 4 * 1024 - GENERATED_GBL_DATA_SIZE)
#endif

/*
 * Environment
 */
#define CONFIG_SYS_CONSOLE_OVERWRITE_ROUTINE
#define CONFIG_SYS_CONSOLE_ENV_OVERWRITE
#define CONFIG_ENV_SIZE			0x4000
#define CONFIG_SYS_MMC_ENV_DEV		0	/* eMMC */
#define CONFIG_ENV_OFFSET		512	/* just after the MBR */

/*
 * U-Boot general configurations
 */

/*
 * Serial Driver
 */
#define CONFIG_ATMEL_USART
#define CONFIG_USART_BASE		0xf802c000
#define CONFIG_USART_ID			6

/*
 * Ethernet
 */
#ifdef CONFIG_CMD_NET
#define CONFIG_MACB
#define CONFIG_RMII
#define CONFIG_NET_RETRY_COUNT		20
#define CONFIG_MACB_SEARCH_PHY
#define CONFIG_ARP_TIMEOUT		200UL
#define CONFIG_IP_DEFRAG
#endif

/*
 * LCD
 */
#ifdef CONFIG_LCD
#define CONFIG_BMP_16BPP
#define CONFIG_BMP_24BPP
#define CONFIG_BMP_32BPP
#define LCD_BPP				LCD_COLOR16
#define LCD_OUTPUT_BPP                  24
#define CONFIG_ATMEL_HLCD
#endif

/*
 * SD/MMC
 */
#ifdef CONFIG_CMD_MMC
#define CONFIG_GENERIC_ATMEL_MCI
#endif

/*
 * SPI NOR (boot memory)
 */
#ifdef CONFIG_CMD_SF
#define CONFIG_ATMEL_SPI
#define CONFIG_ATMEL_SPI0
#define CONFIG_SPI_FLASH_ATMEL
#define CONFIG_SF_DEFAULT_BUS		0
#define CONFIG_SF_DEFAULT_CS		0
#define CONFIG_SF_DEFAULT_SPEED		30000000
#endif

/*
 * USB
 */
#ifdef CONFIG_CMD_USB

/* USB device */
#define CONFIG_USB_FUNCTION_MASS_STORAGE
#define CONFIG_SYS_DFU_DATA_BUF_SIZE	(1 * 1024 * 1024)
#define DFU_DEFAULT_POLL_TIMEOUT	300
#endif

/*
 * Boot Linux
 */
#define CONFIG_CMDLINE_TAG
#define CONFIG_INITRD_TAG
#define CONFIG_SETUP_MEMORY_TAGS
#define CONFIG_BOOTFILE		"fitImage"
#define CONFIG_LOADADDR		0x20800000
#define CONFIG_BOOTCOMMAND	"run mmc_mmc"
#define CONFIG_SYS_LOAD_ADDR	CONFIG_LOADADDR

/*
 * Extra Environments
 */
#define CONFIG_PREBOOT		"run try_bootscript"
#define CONFIG_HOSTNAME		ma5d4evk

#define CONFIG_EXTRA_ENV_SETTINGS					\
	"consdev=ttyS3\0"						\
	"baudrate=115200\0"						\
	"bootscript=boot.scr\0"						\
	"bootdev=/dev/mmcblk0p1\0"					\
	"bootpart=0:1\0"						\
	"rootdev=/dev/mmcblk0p2\0"					\
	"netdev=eth0\0"							\
	"dfu_alt_info=mmc raw 0 3867148288\0"				\
	"kernel_addr_r=0x22000000\0"					\
	"update_spi_firmware_spl_addr=0x21000000\0"			\
	"update_spi_firmware_spl_filename=boot.bin\0"			\
	"update_spi_firmware_addr=0x22000000\0"				\
	"update_spi_firmware_filename=u-boot.img\0"			\
	"update_spi_firmware="	/* Update the SPI flash firmware */	\
		"if sf probe ; then "					\
		"if tftp ${update_spi_firmware_spl_addr} "		\
			"${update_spi_firmware_spl_filename} ; then "	\
		"setenv update_spi_firmware_spl_filesize ${filesize} ; "\
		"if tftp ${update_spi_firmware_addr} "			\
			"${update_spi_firmware_filename} ; then "	\
		"setenv update_spi_firmware_filesize ${filesize} ; "	\
		"sf update ${update_spi_firmware_spl_addr} 0x0 "	\
			"${update_spi_firmware_spl_filesize} ; "	\
		"sf update ${update_spi_firmware_addr} 0x10000 "	\
			"${update_spi_firmware_filesize} ; "		\
		"fi ; "							\
		"fi ; "							\
		"fi\0"							\
	"addcons="							\
		"setenv bootargs ${bootargs} "				\
		"console=${consdev},${baudrate}\0"			\
	"addip="							\
		"setenv bootargs ${bootargs} "				\
		"ip=${ipaddr}:${serverip}:${gatewayip}:"		\
			"${netmask}:${hostname}:${netdev}:off\0"	\
	"addmisc="							\
		"setenv bootargs ${bootargs} ${miscargs}\0"		\
	"addargs=run addcons addmisc\0"					\
	"mmcload="							\
		"mmc rescan ; "						\
		"load mmc ${bootpart} ${kernel_addr_r} ${bootfile}\0"	\
	"netload="							\
		"tftp ${kernel_addr_r} ${hostname}/${bootfile}\0"	\
	"miscargs=nohlt panic=1\0"					\
	"mmcargs=setenv bootargs root=${rootdev} rw rootwait\0"		\
	"nfsargs="							\
		"setenv bootargs root=/dev/nfs rw "			\
			"nfsroot=${serverip}:${rootpath},v3,tcp\0"	\
	"fdtimg=if test ${bootmode} = \"sf\" ; then "			\
			"setenv kernel_fdt 1 ; "			\
		"else ; "						\
			"setenv kernel_fdt 2 ; "			\
		"fi\0"							\
	"mmc_mmc="							\
		"run fdtimg mmcload mmcargs addargs ; "			\
		"bootm ${kernel_addr_r}:kernel@1 - ${kernel_addr_r}:fdt@${kernel_fdt}\0" \
	"mmc_nfs="							\
		"run fdtimg mmcload nfsargs addip addargs ; "			\
		"bootm ${kernel_addr_r}:kernel@1 - ${kernel_addr_r}:fdt@${kernel_fdt}\0" \
	"net_mmc="							\
		"run fdtimg netload mmcargs addargs ; "			\
		"bootm ${kernel_addr_r}:kernel@1 - ${kernel_addr_r}:fdt@${kernel_fdt}\0" \
	"net_nfs="							\
		"run fdtimg netload nfsargs addip addargs ; "			\
		"bootm ${kernel_addr_r}:kernel@1 - ${kernel_addr_r}:fdt@${kernel_fdt}\0" \
	"try_bootscript="						\
		"mmc rescan;"						\
		"if test -e mmc 1:1 ${bootscript} ; then "		\
		"if load mmc 1:1 ${kernel_addr_r} ${bootscript};"	\
		"then ; "						\
			"echo Running bootscript... ; "			\
			"source ${kernel_addr_r} ; "			\
		"fi ; "							\
		"fi\0"
/* SPL */
#define CONFIG_SPL_FRAMEWORK
#define CONFIG_SPL_TEXT_BASE		0x200000
#define CONFIG_SPL_MAX_SIZE		0x10000
#define CONFIG_SPL_BSS_START_ADDR	0x20000000
#define CONFIG_SPL_BSS_MAX_SIZE		0x80000
#define CONFIG_SYS_SPL_MALLOC_START	0x20080000
#define CONFIG_SYS_SPL_MALLOC_SIZE	0x80000

#define CONFIG_SYS_MONITOR_LEN		(512 << 10)

#define CONFIG_SPL_SPI_LOAD
#define CONFIG_SYS_SPI_U_BOOT_OFFS	0x10000

#define CONFIG_SYS_USE_MMC
#define CONFIG_SPL_MMC_SUPPORT
#define CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_SECTOR 0x200
#define CONFIG_SYS_MMCSD_FS_BOOT_PARTITION	1
#define CONFIG_SPL_FS_LOAD_PAYLOAD_NAME		"u-boot.img"
#define CONFIG_SPL_FAT_SUPPORT
#define CONFIG_SPL_LIBDISK_SUPPORT

#endif	/* __MA5D4EVK_CONFIG_H__ */
