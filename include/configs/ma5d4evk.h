/*
 * DENX MA5D4 configuration
 * Copyright (C) 2015 Marek Vasut <marex@denx.de>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __MA5D4EVK_CONFIG_H__
#define __MA5D4EVK_CONFIG_H__

#define CONFIG_TIMESTAMP		/* Print image info with timestamp */

#include "at91-sama5_common.h"
#undef CONFIG_BOOTARGS
#define CONFIG_SYS_USE_SERIALFLASH	1

/*
 * U-Boot Commands
 */
#define CONFIG_FAT_WRITE

/*
 * Memory configurations
 */
#define CONFIG_NR_DRAM_BANKS		1
#define CONFIG_SYS_SDRAM_BASE           ATMEL_BASE_DDRCS
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
#define CONFIG_ENV_IS_IN_SPI_FLASH
#define CONFIG_SYS_REDUNDAND_ENVIRONMENT
#define CONFIG_ENV_OFFSET		0x8000
#define CONFIG_ENV_SIZE			0x4000
#define CONFIG_ENV_OFFSET_REDUND	(CONFIG_ENV_OFFSET + CONFIG_ENV_SIZE)
#define CONFIG_ENV_SIZE_REDUND		CONFIG_ENV_SIZE
#define CONFIG_ENV_SECT_SIZE		0x1000

/*
 * U-Boot general configurations
 */

/*
 * Serial Driver
 */
#define CONFIG_ATMEL_USART
#define CONFIG_USART_BASE		ATMEL_BASE_USART0
#define CONFIG_USART_ID			ATMEL_ID_USART0

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
#define CONFIG_USB_EHCI
#define CONFIG_USB_EHCI_ATMEL
#define CONFIG_SYS_USB_EHCI_MAX_ROOT_PORTS	3

/* USB device */
#define CONFIG_USB_ETHER
#define CONFIG_USB_ETH_RNDIS
#define CONFIG_USBNET_MANUFACTURER      "DENX"
#endif

/*
 * Boot Linux
 */
#define CONFIG_CMDLINE_TAG
#define CONFIG_INITRD_TAG
#define CONFIG_SETUP_MEMORY_TAGS
#define CONFIG_BOOTFILE		"fitImage"
#define CONFIG_BOOTARGS		"console=ttyS3,115200"
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
	"bootdev=/dev/mmcblk1p1\0"					\
	"bootpart=1:1\0"						\
	"rootdev=/dev/mmcblk1p2\0"					\
	"netdev=eth0\0"							\
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
	"mmc_mmc="							\
		"run mmcload mmcargs addargs ; "			\
		"bootm ${kernel_addr_r}\0"				\
	"mmc_nfs="							\
		"run mmcload nfsargs addip addargs ; "			\
		"bootm ${kernel_addr_r}\0"				\
	"net_mmc="							\
		"run netload mmcargs addargs ; "			\
		"bootm ${kernel_addr_r}\0"				\
	"net_nfs="							\
		"run netload nfsargs addip addargs ; "			\
		"bootm ${kernel_addr_r}\0"				\
	"try_bootscript="						\
		"mmc rescan;"						\
		"if test -e mmc ${bootpart} ${bootscript} ; then "	\
		"if load mmc ${bootpart} ${kernel_addr_r} ${bootscript};"\
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

#define CONFIG_SPL_BOARD_INIT
#define CONFIG_SYS_MONITOR_LEN		(512 << 10)

#define CONFIG_SPL_SPI_LOAD
#define CONFIG_SYS_SPI_U_BOOT_OFFS	0x10000

#endif	/* __MA5D4EVK_CONFIG_H__ */
