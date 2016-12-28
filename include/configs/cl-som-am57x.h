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

#define CONFIG_DRA7XX

#define CONFIG_NR_DRAM_BANKS		2

#define CONSOLEDEV			"ttyO2"
#define CONFIG_SYS_NS16550_COM3		UART3_BASE	/* UART3 */
#define CONFIG_CONS_INDEX		3
#define CONFIG_BAUDRATE			115200

#define CONFIG_SYS_OMAP_ABE_SYSCK

#include <configs/ti_omap5_common.h>

/* Status LED */
#define CONFIG_STATUS_LED		/* Status LED enabled */
#define CONFIG_GPIO_LED
#define CONFIG_BOARD_SPECIFIC_LED
#define GREEN_LED_DEV			0
					/* cl_som_am57x Green LED is GPIO2_5 */
#define GREEN_LED_GPIO			37
#define STATUS_LED_BIT			GREEN_LED_GPIO
#define STATUS_LED_STATE		STATUS_LED_ON
#define STATUS_LED_PERIOD		(CONFIG_SYS_HZ / 2)

/* PMIC I2C bus number */
#define CONFIG_SYS_SPD_BUS_NUM 3

/* SPI Flash support */
#undef  CONFIG_OMAP3_SPI

#define CONFIG_TI_SPI_MMAP
#define CONFIG_SF_DEFAULT_SPEED		48000000
#define CONFIG_DEFAULT_SPI_MODE		SPI_MODE_3

/* SPI SPL defines */
/* Offsets: 0K - SPL1, 64K - SPL2, 128K - SPL3, 192K - SPL4, 256K - U-Boot */
#define CONFIG_SYS_SPI_U_BOOT_OFFS	(256 * 1024)
#define CONFIG_SPL_SPI_SUPPORT
#define CONFIG_SPL_SPI_FLASH_SUPPORT
#define CONFIG_SPL_SPI_LOAD

/* SD/MMC RAW boot */
#undef CONFIG_SPL_FS_LOAD_PAYLOAD_NAME
#undef CONFIG_SYS_MMCSD_FS_BOOT_PARTITION

/* Environment */
#define CONFIG_ENV_SIZE			(16 << 10) /* 16 KiB env size */
#define CONFIG_ENV_VARS_UBOOT_RUNTIME_CONFIG

#define CONFIG_ENV_IS_IN_SPI_FLASH
#define CONFIG_ENV_SECT_SIZE		(64 * 1024)
#define CONFIG_ENV_OFFSET		(768 * 1024)
#define CONFIG_ENV_SPI_MAX_HZ		48000000

#ifndef CONFIG_SPL_BUILD
/* SATA */
#define CONFIG_CMD_SCSI
#define CONFIG_LIBATA
#define CONFIG_SCSI_AHCI
#define CONFIG_SCSI_AHCI_PLAT
#define CONFIG_SYS_SCSI_MAX_SCSI_ID	1
#define CONFIG_SYS_SCSI_MAX_LUN		1
#define CONFIG_SYS_SCSI_MAX_DEVICE	(CONFIG_SYS_SCSI_MAX_SCSI_ID * \
						CONFIG_SYS_SCSI_MAX_LUN)
/* PCA9555 GPIO expander support */
#define CONFIG_PCA953X
#define CONFIG_CMD_PCA953X
#define CONFIG_CMD_PCA953X_INFO
#define CONFIG_SYS_I2C_PCA953X_ADDR     0x20
#define CONFIG_SYS_I2C_PCA953X_WIDTH    { {0x20, 16} }

/* GPT */
#define CONFIG_CMD_GPT
#define CONFIG_EFI_PARTITION

/* USB xHCI HOST */
#define CONFIG_USB_XHCI_OMAP
#define CONFIG_SYS_USB_XHCI_MAX_ROOT_PORTS 2

#define CONFIG_OMAP_USB_PHY
#define CONFIG_OMAP_USB3PHY1_HOST

/* USB Networking options */
#define CONFIG_USB_HOST_ETHER
#define CONFIG_USB_ETHER_SMSC95XX
#define CONFIG_USB_ETHER_RNDIS
#define CONFIG_USB_ETHER_ASIX
#define CONFIG_USB_ETHER_MCS7830

#endif /* !CONFIG_SPL_BUILD */

#endif /* __CONFIG_CL_SOM_AM57X_H */
