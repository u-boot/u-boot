/*
 * (C) Copyright 2012, Stefano Babic <sbabic@denx.de>
 *
 * Copyright (C) 2010 Freescale Semiconductor, Inc.
 *
 * Configuration settings for the MX53-EVK Freescale board.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc.
 */

#ifndef __CONFIG_H
#define __CONFIG_H

/* SOC type must be included before imx-regs.h */
#define CONFIG_MX53
#include <asm/arch/imx-regs.h>
#include <asm/arch/mx5x_pins.h>

#define CONFIG_SYS_MX5_HCLK		24000000
#define CONFIG_SYS_MX5_CLK32		32768

#define CONFIG_DISPLAY_CPUINFO
#define CONFIG_DISPLAY_BOARDINFO

#define CONFIG_CMDLINE_TAG		/* enable passing of ATAGs */
#define CONFIG_SETUP_MEMORY_TAGS
#define CONFIG_INITRD_TAG

#define CONFIG_OF_LIBFDT

/* Size of malloc() pool */
#define CONFIG_SYS_MALLOC_LEN		(CONFIG_ENV_SIZE + 2 * 1024 * 1024)

#define CONFIG_BOARD_EARLY_INIT_F

/* Enable GPIOs */
#define CONFIG_MXC_GPIO

/* UART */
#define CONFIG_MXC_UART
#define CONFIG_MXC_UART_BASE	UART4_BASE_ADDR

/* MMC */
#define CONFIG_FSL_ESDHC
#define CONFIG_SYS_FSL_ESDHC_ADDR	0
#define CONFIG_SYS_FSL_ESDHC_NUM	1

#define CONFIG_MMC
#define CONFIG_GENERIC_MMC
#define CONFIG_DOS_PARTITION

/* Ethernet on FEC */
#define CONFIG_NET_MULTI
#define CONFIG_MII

#define CONFIG_FEC_MXC
#define IMX_FEC_BASE			FEC_BASE_ADDR
#define CONFIG_FEC_MXC_PHYADDR		0x01
#define CONFIG_PHY_ADDR			CONFIG_FEC_MXC_PHYADDR
#define CONFIG_RESET_PHY_R
#define CONFIG_FEC_MXC_NO_ANEG
#define CONFIG_ETHPRIME			"FEC0"

/* SPI */
#define CONFIG_HARD_SPI
#define CONFIG_MXC_SPI
#define CONFIG_DEFAULT_SPI_BUS		1
#define CONFIG_DEFAULT_SPI_MODE		SPI_MODE_0

/* SPI FLASH - not used for environment */
#define CONFIG_SPI_FLASH
#define CONFIG_SPI_FLASH_STMICRO
#define CONFIG_SPI_FLASH_CS		(IOMUX_TO_GPIO(MX53_PIN_CSI0_D11) \
						 << 8) | 0
#define CONFIG_SF_DEFAULT_MODE		SPI_MODE_0
#define CONFIG_SF_DEFAULT_SPEED		25000000

/* allow to overwrite serial and ethaddr */
#define CONFIG_ENV_OVERWRITE
#define CONFIG_CONS_INDEX		1
#define CONFIG_BAUDRATE			115200

/* Command definition */
#include <config_cmd_default.h>
#define CONFIG_CMD_PING
#define CONFIG_CMD_DHCP
#define CONFIG_CMD_MII
#define CONFIG_CMD_MMC
#define CONFIG_CMD_FAT
#define CONFIG_CMD_EXT2
#define CONFIG_CMD_MTDPARTS
#define CONFIG_CMD_SPI
#define CONFIG_CMD_SF
#define CONFIG_CMD_GPIO

#define CONFIG_BOOTDELAY	3

#define CONFIG_LOADADDR		0x70800000	/* loadaddr env var */
#define CONFIG_SYS_TEXT_BASE    0xf0001400 /* uboot in nor flash */

#define CONFIG_ARP_TIMEOUT	200UL

/* Miscellaneous configurable options */
#define CONFIG_SYS_LONGHELP		/* undef to save memory */
#define CONFIG_SYS_HUSH_PARSER		/* use "hush" command parser */
#define CONFIG_SYS_PROMPT		"IMA3 MX53 U-Boot > "
#define CONFIG_AUTO_COMPLETE
#define CONFIG_SYS_CBSIZE		256	/* Console I/O Buffer Size */

/* Print Buffer Size */
#define CONFIG_SYS_PBSIZE (CONFIG_SYS_CBSIZE + sizeof(CONFIG_SYS_PROMPT) + 16)
#define CONFIG_SYS_MAXARGS	16	/* max number of command args */
#define CONFIG_SYS_BARGSIZE CONFIG_SYS_CBSIZE /* Boot Argument Buffer Size */

#define CONFIG_SYS_MEMTEST_START       0x70000000
#define CONFIG_SYS_MEMTEST_END         0x10000

#define CONFIG_SYS_LOAD_ADDR		CONFIG_LOADADDR

#define CONFIG_SYS_HZ		1000
#define CONFIG_CMDLINE_EDITING

/* Physical Memory Map */
#define CONFIG_NR_DRAM_BANKS	1
#define PHYS_SDRAM_1		CSD0_BASE_ADDR
#define PHYS_SDRAM_1_SIZE	(1024 * 1024 * 1024)

#define CONFIG_SYS_SDRAM_BASE		(PHYS_SDRAM_1)
#define CONFIG_SYS_INIT_RAM_ADDR	(IRAM_BASE_ADDR)
#define CONFIG_SYS_INIT_RAM_SIZE	(IRAM_SIZE)

#define CONFIG_SYS_INIT_SP_OFFSET \
	(CONFIG_SYS_INIT_RAM_SIZE - GENERATED_GBL_DATA_SIZE)
#define CONFIG_SYS_INIT_SP_ADDR \
	(CONFIG_SYS_INIT_RAM_ADDR + CONFIG_SYS_INIT_SP_OFFSET)

#define CONFIG_MTD_DEVICE		/* needed for mtdparts commands */
#define MTDIDS_DEFAULT		"nor0=f0000000.flash"

/* FLASH and environment organization */

#define CONFIG_SYS_FLASH_BASE		0xF0000000
#define CONFIG_SYS_FLASH_CFI		/* Flash is CFI conformant */
#define CONFIG_FLASH_CFI_DRIVER		/* Use the common driver */
#define CONFIG_FLASH_CFI_MTD		/* with MTD support */
#define CONFIG_SYS_FLASH_BANKS_LIST	{ CONFIG_SYS_FLASH_BASE }
#define CONFIG_SYS_MAX_FLASH_BANKS	1
#define CONFIG_SYS_MAX_FLASH_SECT	1024

#define CONFIG_SYS_FLASH_EMPTY_INFO
#define CONFIG_SYS_FLASH_USE_BUFFER_WRITE

#define CONFIG_SYS_MONITOR_BASE		CONFIG_SYS_FLASH_BASE
#define CONFIG_SYS_MONITOR_LEN		(512 * 1024)

#define CONFIG_ENV_SIZE        (8 * 1024)
#define CONFIG_ENV_IS_IN_FLASH
#define CONFIG_ENV_ADDR		(CONFIG_SYS_MONITOR_BASE + \
				CONFIG_SYS_MONITOR_LEN)
#define CONFIG_ENV_SECT_SIZE	0x20000
#define CONFIG_ENV_OFFSET_REDUND	(CONFIG_ENV_OFFSET + \
					CONFIG_ENV_SECT_SIZE)
#define CONFIG_ENV_SIZE_REDUND	CONFIG_ENV_SIZE

/*
 * Default environment and default scripts
 * to update uboot and load kernel
 */

#define HOSTNAME ima3-mx53
#define xstr(s)	str(s)
#define str(s)	#s

#define CONFIG_HOSTNAME ima3-mx53
#define	CONFIG_EXTRA_ENV_SETTINGS					\
	"netdev=eth0\0"							\
	"nfsargs=setenv bootargs root=/dev/nfs rw "			\
		"nfsroot=${serverip}:${rootpath}\0"			\
	"ramargs=setenv bootargs root=/dev/ram0 rw\0"			\
	"addip_sta=setenv bootargs ${bootargs} "			\
		"ip=${ipaddr}:${serverip}:${gatewayip}:${netmask}"	\
		":${hostname}:${netdev}:off panic=1\0"			\
	"addip_dyn=setenv bootargs ${bootargs} ip=dhcp\0"		\
	"addip=if test -n ${ipdyn};then run addip_dyn;"			\
		"else run addip_sta;fi\0"	\
	"addmtd=setenv bootargs ${bootargs} ${mtdparts}\0"		\
	"addtty=setenv bootargs ${bootargs}"				\
		" console=${console},${baudrate}\0"			\
	"addmisc=setenv bootargs ${bootargs} ${misc}\0"			\
	"console=ttymxc3\0"						\
	"loadaddr=70800000\0"						\
	"kernel_addr_r=70800000\0"					\
	"ramdisk_addr_r=71000000\0"					\
	"hostname=" xstr(CONFIG_HOSTNAME) "\0"				\
	"bootfile=" xstr(CONFIG_HOSTNAME) "/uImage\0"			\
	"ramdisk_file=" xstr(CONFIG_HOSTNAME) "/uRamdisk\0"		\
	"mmcargs=setenv bootargs root=${mmcroot} "			\
		"rootfstype=${mmcrootfstype}\0"				\
	"mmcroot=/dev/mmcblk0p3 rw\0"					\
	"mmcboot=echo Booting from mmc ...; "				\
		"run mmcargs addip addtty addmtd addmisc mmcload;"	\
		"bootm\0"						\
	"mmcload=fatload mmc ${mmcdev}:${mmcpart} "			\
		"${loadaddr} ${uimage}\0"				\
	"mmcrootfstype=ext3 rootwait\0"					\
	"flash_self=run ramargs addip addtty addmtd addmisc;"		\
		"bootm ${kernel_addr} ${ramdisk_addr}\0"		\
	"flash_nfs=run nfsargs addip addtty addmtd addmisc;"		\
		"bootm ${kernel_addr}\0"				\
	"net_nfs=tftp ${kernel_addr_r} ${bootfile}; "			\
		"run nfsargs addip addtty addmtd addmisc;"		\
		"bootm ${kernel_addr_r}\0"				\
	"net_self_load=tftp ${ramdisk_addr_r} ${ramdisk_file};"		\
		"tftp ${kernel_addr_r} ${bootfile}\0"			\
	"net_self=if run net_self_load;then "				\
		"run ramargs addip addtty addmtd addmisc;"		\
		"bootm ${kernel_addr_r} ${ramdisk_addr_r};"		\
		"else echo Images not loades;fi\0"			\
	"satargs=setenv bootargs root=/dev/sda1\0"			\
	"satafile=boot/uImage\0"					\
	"ssdboot=echo Booting from ssd ...; "				\
		"run satargs addip addtty addmtd addmisc;"		\
		"sata init;ext2load sata 0:1 ${kernel_addr_r} "		\
		"${satafile};bootm\0"					\
	"u-boot=" xstr(CONFIG_HOSTNAME) "/u-boot.imx\0"			\
	"uimage=uImage\0"						\
	"load=tftp ${loadaddr} ${u-boot}\0"				\
	"uboot_addr=0xf0001000\0"					\
	"update=protect off 0xf0000000 +60000;"				\
		"erase ${uboot_addr} +60000;"				\
		"cp.b ${loadaddr} ${uboot_addr} ${filesize}\0"		\
	"upd=if run load;then echo Updating u-boot;if run update;"	\
		"then echo U-Boot updated;"				\
			"else echo Error updating u-boot !;"		\
			"echo Board without bootloader !!;"		\
		"fi;"							\
		"else echo U-Boot not downloaded..exiting;fi\0"		\
	"bootcmd=run net_nfs\0"


#define CONFIG_CMD_SATA
#ifdef CONFIG_CMD_SATA
	#define CONFIG_DWC_AHSATA
	#define CONFIG_SYS_SATA_MAX_DEVICE      1
	#define CONFIG_DWC_AHSATA_PORT_ID       0
	#define CONFIG_DWC_AHSATA_BASE_ADDR     SATA_BASE_ADDR
	#define CONFIG_LBA48
	#define CONFIG_LIBATA
#endif

#endif				/* __CONFIG_H */
