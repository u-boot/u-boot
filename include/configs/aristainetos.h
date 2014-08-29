/*
 * (C) Copyright 2014
 * Heiko Schocher, DENX Software Engineering, hs@denx.de.
 *
 * Based on:
 * Copyright (C) 2012 Freescale Semiconductor, Inc.
 *
 * Configuration settings for the Freescale i.MX6Q SabreSD board.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */
#ifndef __ARISTAINETOS_CONFIG_H
#define __ARISTAINETOS_CONFIG_H

#define CONFIG_MX6

#include "mx6_common.h"
#include <linux/sizes.h>

#define CONFIG_DISPLAY_CPUINFO
#define CONFIG_DISPLAY_BOARDINFO

#include <asm/arch/imx-regs.h>
#include <asm/imx-common/gpio.h>

#define CONFIG_MACH_TYPE	4501
#define CONFIG_MMCROOT		"/dev/mmcblk0p2"
#define CONFIG_DEFAULT_FDT_FILE	"aristainetos.dtb"
#define CONFIG_HOSTNAME		aristainetos
#define PHYS_SDRAM_SIZE		(1u * 1024 * 1024 * 1024)

#define CONFIG_SYS_GENERIC_BOARD

/* Size of malloc() pool */
#define CONFIG_SYS_MALLOC_LEN		(64 * SZ_1M)

#define CONFIG_BOARD_EARLY_INIT_F
#define CONFIG_MXC_GPIO

#define CONFIG_MXC_UART
#define CONFIG_MXC_UART_BASE	UART5_BASE
#define CONFIG_CONSOLE_DEV	"ttymxc4"

#define CONFIG_CMD_FUSE
#define CONFIG_MXC_OCOTP

/* MMC Configs */
#define CONFIG_FSL_ESDHC
#define CONFIG_FSL_USDHC
#define CONFIG_SYS_FSL_ESDHC_ADDR      0

#define CONFIG_MMC
#define CONFIG_CMD_MMC
#define CONFIG_GENERIC_MMC
#define CONFIG_BOUNCE_BUFFER
#define CONFIG_CMD_EXT2
#define CONFIG_CMD_FAT
#define CONFIG_DOS_PARTITION

#define CONFIG_CMD_PING
#define CONFIG_CMD_DHCP
#define CONFIG_CMD_MII
#define CONFIG_CMD_NET
#define CONFIG_FEC_MXC
#define CONFIG_MII
#define IMX_FEC_BASE			ENET_BASE_ADDR
#define CONFIG_FEC_XCV_TYPE		RMII
#define CONFIG_ETHPRIME			"FEC"
#define CONFIG_FEC_MXC_PHYADDR		0

#define CONFIG_PHYLIB
#define CONFIG_PHY_MICREL

#define CONFIG_CMD_SF
#define CONFIG_SPI_FLASH
#define CONFIG_SPI_FLASH_MTD
#define CONFIG_SPI_FLASH_STMICRO
#define CONFIG_MXC_SPI
#define CONFIG_SF_DEFAULT_BUS		3
#define CONFIG_SF_DEFAULT_CS		(0|(IMX_GPIO_NR(3, 20)<<8))
#define CONFIG_SF_DEFAULT_SPEED		20000000
#define CONFIG_SF_DEFAULT_MODE		SPI_MODE_0
#define CONFIG_SYS_SPI_ST_ENABLE_WP_PIN

/* allow to overwrite serial and ethaddr */
#define CONFIG_ENV_OVERWRITE
#define CONFIG_CONS_INDEX		1
#define CONFIG_BAUDRATE			115200

/* Command definition */
#include <config_cmd_default.h>

#define CONFIG_CMD_BMODE
#define CONFIG_CMD_BOOTZ
#define CONFIG_CMD_SETEXPR
#undef CONFIG_CMD_IMLS

#define CONFIG_BOOTDELAY		3

#define CONFIG_LOADADDR			0x12000000
#define CONFIG_SYS_TEXT_BASE		0x17800000

#define CONFIG_EXTRA_ENV_SETTINGS \
	"uimage=uImage\0" \
	"fdt_file=" CONFIG_DEFAULT_FDT_FILE "\0" \
	"fdt_addr_r=0x11000000\0" \
	"kernel_addr_r=0x12000000\0" \
	"kernel_file=uImage\0" \
	"boot_fdt=try\0" \
	"ip_dyn=yes\0" \
	"console=" CONFIG_CONSOLE_DEV "\0" \
	"fdt_high=0xffffffff\0"	  \
	"initrd_high=0xffffffff\0" \
	"mmcpart=1\0" \
	"mmcdev=" __stringify(CONFIG_SYS_MMC_ENV_DEV) "\0" \
	"mmcroot=" CONFIG_MMCROOT " rootwait rw\0" \
	"mmcargs=setenv bootargs console=${console},${baudrate} " \
		"root=${mmcroot}\0" \
	"loadimage=fatload mmc ${mmcdev}:${mmcpart} ${kernel_addr_r} " \
		"${uimage}\0" \
	"loadfdt=fatload mmc ${mmcdev}:${mmcpart} ${fdt_addr_r} " \
		"${fdt_file}\0" \
	"mmcboot=echo Booting from mmc ...; " \
		"run mmcargs;run loadimage loadfdt fdt_setup;" \
		"bootm ${kernel_addr_r} - ${fdt_addr_r};\0" \
	"rootpath=/opt/eldk-5.5/armv7a-hf/rootfs-sato-sdk\0" \
	"nfsopts=nfsvers=3 nolock rw\0" \
	"netdev=eth0\0" \
	"fdt_setup=fdt addr ${fdt_addr_r};fdt resize;fdt chosen;fdt board\0"\
	"load_fdt=tftp ${fdt_addr_r} ${fdt_file}\0" \
	"load_kernel=tftp ${kernel_addr_r} ${kernel_file}\0" \
	"addmtd=setenv bootargs ${bootargs} ${mtdparts}\0" \
	"get_env=mw ${loadaddr} 0x00000000 0x20000;" \
		"tftp ${loadaddr} /tftpboot/aristainetos/env.txt;" \
		"env import -t ${loadaddr}\0" \
	"addmisc=setenv bootargs ${bootargs} maxcpus=1 loglevel=8\0" \
	"bootargs_defaults=setenv bootargs ${console} ${mtdoops} " \
		"${optargs}\0" \
	"net_args=run bootargs_defaults;setenv bootargs ${bootargs} " \
		"root=/dev/nfs nfsroot=${serverip}:${rootpath},${nfsopts} " \
		"ip=${ipaddr}:${serverip}:${gatewayip}:${netmask}:" \
		"${hostname}:${netdev}:off\0" \
	"net_nfs=run load_kernel load_fdt;run net_args addmtd addmisc;" \
		"run fdt_setup;bootm ${kernel_addr_r} - ${fdt_addr_r}\0" \
	"uboot=/tftpboot/aristainetos/u-boot.imx\0" \
	"load_uboot=tftp ${loadaddr} ${uboot}\0" \
	"uboot_sz=c0000\0" \
	"upd_uboot=mw.b ${loadaddr} 0xff ${uboot_sz};" \
		"mw.b 10200000 0x00 ${uboot_sz};" \
		"run load_uboot;sf probe;sf erase 0 ${uboot_sz};" \
		"sf write ${loadaddr} 400 ${filesize};" \
		"sf read 10200000 400 ${uboot_sz};" \
		"cmp.b ${loadaddr} 10200000 bc000\0" \
	"ubi_prep=ubi part ubi 2048;ubifsmount ubi:kernel\0" \
	"load_kernel_ubi=ubifsload ${kernel_addr_r} uImage\0" \
	"load_fdt_ubi=ubifsload ${fdt_addr_r} aristainetos.dtb\0" \
	"ubi_nfs=run ubiprep load_kernel_ubi load_fdt_ubi;" \
		"run net_args addmtd addmisc;run fdt_setup;" \
		"bootm ${kernel_addr_r} - ${fdt_addr_r}\0" \
	"rootfsname=rootfs\0" \
	"ubi_args=run bootargs_defaults;setenv bootargs ${bootargs} " \
		"ubi.mtd=0,2048 root=ubi0:${rootfsname} rootfstype=ubifs " \
		"ip=${ipaddr}:${serverip}:${gatewayip}:${netmask}:" \
		"${hostname}:${netdev}:off\0" \
	"ubi_ubi=run ubi_prep load_kernel_ubi load_fdt_ubi;" \
		"run bootargs_defaults ubi_args addmtd addmisc;" \
		"run fdt_setup;bootm ${kernel_addr_r} - ${fdt_addr_r}\0" \
	"ubirootfs_file=/tftpboot/aristainetos/rootfs-minimal.ubifs\0" \
	"upd_ubirootfs=run ubi_prep;tftp ${loadaddr} ${ubirootfs_file};" \
		"ubi write ${loadaddr} rootfs ${filesize}\0" \
	"ksz=800000\0" \
	"rootsz=2000000\0" \
	"usersz=8000000\0" \
	"ubi_make=run ubi_prep;ubi create kernel ${ksz};" \
		"ubi create rootfs ${rootsz};ubi create userfs ${usersz}\0"

#define CONFIG_BOOTCOMMAND \
	"mmc dev ${mmcdev};" \
	"if mmc rescan; then " \
		"run mmcboot;" \
	"else run ubi_ubi; fi"

#define CONFIG_ARP_TIMEOUT		200UL

/* Miscellaneous configurable options */
#define CONFIG_SYS_LONGHELP
#define CONFIG_SYS_HUSH_PARSER
#define CONFIG_SYS_PROMPT_HUSH_PS2	"> "
#define CONFIG_AUTO_COMPLETE
#define CONFIG_SYS_CBSIZE		256

/* Print Buffer Size */
#define CONFIG_SYS_PBSIZE (CONFIG_SYS_CBSIZE + sizeof(CONFIG_SYS_PROMPT) + 16)
#define CONFIG_SYS_MAXARGS             16
#define CONFIG_SYS_BARGSIZE CONFIG_SYS_CBSIZE

#define CONFIG_SYS_MEMTEST_START	PHYS_SDRAM
#define CONFIG_SYS_MEMTEST_END		(CONFIG_SYS_MEMTEST_START + 0x100000)
#define CONFIG_SYS_MEMTEST_SCRATCH	0x10800000

#define CONFIG_SYS_LOAD_ADDR		CONFIG_LOADADDR

#define CONFIG_CMDLINE_EDITING
#define CONFIG_STACKSIZE		(128 * 1024)

/* Physical Memory Map */
#define CONFIG_NR_DRAM_BANKS		1
#define PHYS_SDRAM			MMDC0_ARB_BASE_ADDR

#define CONFIG_SYS_SDRAM_BASE		PHYS_SDRAM
#define CONFIG_SYS_INIT_RAM_ADDR	IRAM_BASE_ADDR
#define CONFIG_SYS_INIT_RAM_SIZE	IRAM_SIZE

#define CONFIG_SYS_INIT_SP_OFFSET \
	(CONFIG_SYS_INIT_RAM_SIZE - GENERATED_GBL_DATA_SIZE)
#define CONFIG_SYS_INIT_SP_ADDR \
	(CONFIG_SYS_INIT_RAM_ADDR + CONFIG_SYS_INIT_SP_OFFSET)

/* FLASH and environment organization */
#define CONFIG_SYS_NO_FLASH

#define CONFIG_ENV_SIZE			(12 * 1024)
#define CONFIG_ENV_IS_IN_SPI_FLASH
#define CONFIG_SYS_REDUNDAND_ENVIRONMENT
#define CONFIG_ENV_SPI_BUS		CONFIG_SF_DEFAULT_BUS
#define CONFIG_ENV_SPI_CS		CONFIG_SF_DEFAULT_CS
#define CONFIG_ENV_SPI_MAX_HZ		CONFIG_SF_DEFAULT_SPEED
#define CONFIG_ENV_SPI_MODE		CONFIG_SF_DEFAULT_MODE
#define CONFIG_ENV_SECT_SIZE		(0x010000)
#define CONFIG_ENV_OFFSET		(0x0c0000)
#define CONFIG_ENV_OFFSET_REDUND	(0x0d0000)

#define CONFIG_OF_LIBFDT

#define CONFIG_CMD_CACHE

#define CONFIG_SYS_FSL_USDHC_NUM	2

#define CONFIG_CMD_I2C
#define CONFIG_SYS_I2C
#define CONFIG_SYS_I2C_MXC
#define CONFIG_SYS_I2C_SPEED		100000
#define CONFIG_SYS_I2C_SLAVE		0x7f
#define CONFIG_SYS_I2C_NOPROBES		{ {0, 0x00} }

#define CONFIG_CMD_GPIO
#define CONFIG_GPIO_ENABLE_SPI_FLASH	IMX_GPIO_NR(2, 15)

/* NAND stuff */
#define CONFIG_CMD_NAND
#define CONFIG_CMD_NAND_TRIMFFS
#define CONFIG_NAND_MXS
#define CONFIG_SYS_MAX_NAND_DEVICE	1
#define CONFIG_SYS_NAND_BASE		0x40000000
#define CONFIG_SYS_NAND_5_ADDR_CYCLE
#define CONFIG_SYS_NAND_ONFI_DETECTION

/* DMA stuff, needed for GPMI/MXS NAND support */
#define CONFIG_APBH_DMA
#define CONFIG_APBH_DMA_BURST
#define CONFIG_APBH_DMA_BURST8

/* RTC */
#define CONFIG_SYS_I2C_RTC_ADDR	0x68
#define CONFIG_SYS_RTC_BUS_NUM	2
#define CONFIG_RTC_M41T11
#define CONFIG_CMD_DATE

/* USB Configs */
#define CONFIG_CMD_USB
#define CONFIG_CMD_FAT
#define CONFIG_USB_EHCI
#define CONFIG_USB_EHCI_MX6
#define CONFIG_USB_STORAGE
#define CONFIG_USB_MAX_CONTROLLER_COUNT 2
#define CONFIG_EHCI_HCD_INIT_AFTER_RESET	/* For OTG port */
#define CONFIG_MXC_USB_PORTSC	(PORT_PTS_UTMI | PORT_PTS_PTW)
#define CONFIG_MXC_USB_FLAGS	0

#define ARISTAINETOS_USB_OTG_PWR	IMX_GPIO_NR(4, 15)
#define ARISTAINETOS_USB_H1_PWR		IMX_GPIO_NR(3, 31)

/* UBI support */
#define CONFIG_CMD_MTDPARTS
#define CONFIG_MTD_PARTITIONS
#define CONFIG_MTD_DEVICE
#define CONFIG_RBTREE
#define CONFIG_LZO
#define CONFIG_CMD_UBI
#define CONFIG_CMD_UBIFS

#define MTDIDS_DEFAULT                  "nand0=gpmi-nand"
#define MTDPARTS_DEFAULT                "mtdparts=gpmi-nand:-(ubi)"

#define CONFIG_MTD_UBI_FASTMAP
#define CONFIG_MTD_UBI_FASTMAP_AUTOCONVERT	1

#define CONFIG_HW_WATCHDOG
#define CONFIG_IMX_WATCHDOG

#define CONFIG_FIT

/* Framebuffer */
#define CONFIG_VIDEO
#define CONFIG_VIDEO_IPUV3
/* check this console not needed, after test remove it */
#define CONFIG_CFB_CONSOLE
#define CONFIG_VGA_AS_SINGLE_DEVICE
#define CONFIG_SYS_CONSOLE_IS_IN_ENV
#define CONFIG_SYS_CONSOLE_OVERWRITE_ROUTINE
#define CONFIG_VIDEO_BMP_RLE8
#define CONFIG_SPLASH_SCREEN
#define CONFIG_SPLASH_SCREEN_ALIGN
#define CONFIG_BMP_16BPP
#define CONFIG_VIDEO_LOGO
#define CONFIG_VIDEO_BMP_LOGO
#define CONFIG_IPUV3_CLK 198000000
#define CONFIG_IMX_VIDEO_SKIP

#define CONFIG_CMD_BMP

#define CONFIG_PWM_IMX
#define CONFIG_IMX6_PWM_PER_CLK	66000000

#endif                         /* __ARISTAINETOS_CONFIG_H */
