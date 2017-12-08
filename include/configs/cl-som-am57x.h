/*
 * Configuration settings for CompuLab CL-SOM-AM57x board
 *
 * (C) Copyright 2016 CompuLab, Ltd. http://compulab.co.il/
 *
 * Author: Dmitry Lifshitz <lifshitz@compulab.co.il>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __CONFIG_CL_SOM_AM57X_H
#define __CONFIG_CL_SOM_AM57X_H

#define CONFIG_NR_DRAM_BANKS		2

#define CONSOLEDEV			"ttyO2"
#define CONFIG_SYS_NS16550_COM3		UART3_BASE	/* UART3 */
#define CONFIG_CONS_INDEX		3

#define CONFIG_SYS_OMAP_ABE_SYSCK

#include <configs/ti_omap5_common.h>

/* misc */
#define CONFIG_MISC_INIT_R
#define CONFIG_REVISION_TAG

/* PMIC I2C bus number */
#define CONFIG_SYS_SPD_BUS_NUM 3

/* SPI Flash support */
#define CONFIG_TI_SPI_MMAP
#define CONFIG_SF_DEFAULT_SPEED		48000000
#define CONFIG_DEFAULT_SPI_MODE		SPI_MODE_3

/* SPI SPL defines */
/* Offsets: 0K - SPL1, 64K - SPL2, 128K - SPL3, 192K - SPL4, 256K - U-Boot */
#define CONFIG_SYS_SPI_U_BOOT_OFFS	(256 * 1024)
#define CONFIG_SPL_SPI_SUPPORT
#define CONFIG_SPL_SPI_FLASH_SUPPORT
#define CONFIG_SPL_SPI_LOAD

/* SD/MMC RAW/FS boot */
#define CONFIG_SPL_FS_LOAD_PAYLOAD_NAME "u-boot.img"
#define CONFIG_SYS_MMCSD_FS_BOOT_PARTITION 1

/* Environment */
#define CONFIG_ENV_SIZE			(16 << 10) /* 16 KiB env size */
#define CONFIG_ENV_VARS_UBOOT_RUNTIME_CONFIG

#define CONFIG_ENV_SECT_SIZE		(64 * 1024)
#define CONFIG_ENV_OFFSET		(768 * 1024)
#define CONFIG_ENV_SPI_MAX_HZ		48000000

/* EEPROM */
#define CONFIG_SYS_I2C_EEPROM_ADDR      0x50
#define CONFIG_SYS_I2C_EEPROM_ADDR_LEN  1
#define CONFIG_SYS_I2C_EEPROM_BUS       3

#define CONFIG_ENV_EEPROM_IS_ON_I2C
#define CONFIG_SYS_EEPROM_SIZE		256

#ifndef CONFIG_SPL_BUILD
/* SATA */
#define CONFIG_SCSI_AHCI_PLAT
#define CONFIG_SYS_SCSI_MAX_SCSI_ID	1
#define CONFIG_SYS_SCSI_MAX_LUN		1
#define CONFIG_SYS_SCSI_MAX_DEVICE	(CONFIG_SYS_SCSI_MAX_SCSI_ID * \
						CONFIG_SYS_SCSI_MAX_LUN)
/* PCA9555 GPIO expander support */
#define CONFIG_PCA953X
#define CONFIG_SYS_I2C_PCA953X_ADDR     0x20
#define CONFIG_SYS_I2C_PCA953X_WIDTH    { {0x20, 16} }

#endif /* !CONFIG_SPL_BUILD */

/* USB xHCI HOST */
#define CONFIG_USB_XHCI_OMAP

#define CONFIG_OMAP_USB_PHY
#define CONFIG_OMAP_USB3PHY1_HOST

/* USB Networking options */

/* CPSW Ethernet */
#define CONFIG_DRIVER_TI_CPSW
#define CONFIG_MII
#define CONFIG_BOOTP_DEFAULT
#define CONFIG_BOOTP_SEND_HOSTNAME
#define CONFIG_BOOTP_GATEWAY
#define CONFIG_PHY_ATHEROS
#define CONFIG_SYS_RX_ETH_BUFFER	64
#define PHY_ANEG_TIMEOUT		8000

#define CONFIG_BOOTP_DNS
#define CONFIG_BOOTP_DNS2
#define CONFIG_BOOTP_SUBNETMASK
#define CONFIG_NET_RETRY_COUNT		10

/* Default environment */
#undef CONFIG_EXTRA_ENV_SETTINGS
#define CONFIG_EXTRA_ENV_SETTINGS \
	DEFAULT_LINUX_BOOT_ENV \
	"autoload=no\0" \
	"baudrate=115200\0" \
	"console=ttyO2,115200n8\0" \
	"bootdelay=3\0" \
	"fdtfile=am57xx-sbc-am57x.dtb\0" \
	"kernel=zImage-cl-som-am57x\0" \
	"bootscr=bootscr.img\0" \
	"displaytype=hdmi\0" \
	"bootkernel=bootz ${loadaddr} - ${fdtaddr}\0" \
	"mmcloadfdt=load mmc ${mmcdev} ${fdtaddr} ${fdtfile}\0" \
	"mmcloadkernel=load mmc ${mmcdev} ${loadaddr} ${kernel}\0" \
	"emmcloadfdt=load mmc ${mmcdev} ${fdtaddr} ${fdtfile}\0" \
	"emmcloadkernel=load mmc ${mmcdev} ${loadaddr} ${kernel}\0" \
	"load_mmc=mmc dev ${mmcdev} && mmc rescan && " \
		"run mmcloadkernel && run mmcloadfdt\0" \
	"mmcroot=/dev/mmcblk1p2\0" \
	"mmcrootfstype=ext4 rw rootwait\0" \
	"mmcargs=setenv bootargs console=${console} root=${mmcroot} " \
		"rootfstype=${mmcrootfstype}\0" \
	"mmcbootscript=setenv mmcdev 0; mmc dev ${mmcdev} && mmc rescan && " \
		"load mmc ${mmcdev} ${loadaddr} ${bootscr} && " \
		"echo Running bootscript from MMC/SD Card ... && " \
		"source ${loadaddr}\0" \
	"mmcboot=setenv mmcdev 0 && run load_mmc && " \
		"run mmcargs && echo Booting from MMC/SD Card ... && " \
		"run bootkernel\0" \
	"emmcroot=/dev/mmcblk0p2\0" \
	"emmcrootfstype=ext4 rw rootwait\0" \
	"emmcargs=setenv bootargs console=${console} " \
		"root=${emmcroot} " \
		"rootfstype=${emmcrootfstype}\0" \
	"emmcbootscript=setenv mmcdev 1; mmc dev ${mmcdev} && mmc rescan && " \
		"load mmc ${mmcdev} ${loadaddr} ${bootscr} && " \
		"echo Running bootscript from eMMC ... && " \
		"source ${loadaddr}\0" \
	"emmcboot=setenv mmcdev 1 && run load_mmc && " \
		"run emmcargs && echo Booting from eMMC ... && " \
		"run bootkernel\0" \
	"sataroot=/dev/sda2\0" \
	"satarootfstype=ext4 rw rootwait\0" \
	"load_sata=load scsi 0 ${loadaddr} ${kernel} && " \
		"load scsi 0 ${fdtaddr} ${fdtfile}\0" \
	"sataargs=setenv bootargs console=${console} " \
		"root=${sataroot} " \
		"rootfstype=${satarootfstype}\0" \
	"satabootscript=load scsi 0 ${loadaddr} ${bootscr} && " \
		"echo Running bootscript from SATA ... && " \
		"source ${loadaddr}\0" \
	"sataboot=run load_sata && run sataargs && " \
		"echo Booting from SATA ... && " \
		"run bootkernel\0"

#undef CONFIG_BOOTCOMMAND
#define CONFIG_BOOTCOMMAND \
	"run mmcbootscript || run mmcboot || " \
	"run satabootscript || run sataboot || " \
	"run emmcbootscript || run emmcboot"


#endif /* __CONFIG_CL_SOM_AM57X_H */
