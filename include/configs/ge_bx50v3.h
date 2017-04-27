/*
 * Copyright (C) 2015 Timesys Corporation
 * Copyright (C) 2015 General Electric Company
 * Copyright (C) 2014 Advantech
 * Copyright (C) 2012 Freescale Semiconductor, Inc.
 *
 * Configuration settings for the GE MX6Q Bx50v3 boards.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __GE_BX50V3_CONFIG_H
#define __GE_BX50V3_CONFIG_H

#include <asm/arch/imx-regs.h>
#include <asm/imx-common/gpio.h>

#define BX50V3_BOOTARGS_EXTRA
#if defined(CONFIG_TARGET_GE_B450V3)
#define CONFIG_BOARD_NAME	"General Electric B450v3"
#elif defined(CONFIG_TARGET_GE_B650V3)
#define CONFIG_BOARD_NAME	"General Electric B650v3"
#elif defined(CONFIG_TARGET_GE_B850V3)
#define CONFIG_BOARD_NAME	"General Electric B850v3"
#undef BX50V3_BOOTARGS_EXTRA
#define BX50V3_BOOTARGS_EXTRA	"video=DP-1:1024x768@60 " \
				"video=HDMI-A-1:1024x768@60 "
#else
#define CONFIG_BOARD_NAME	"General Electric BA16 Generic"
#endif

#define CONFIG_MXC_UART_BASE	UART3_BASE
#define CONSOLE_DEV	"ttymxc2"

#define CONFIG_SUPPORT_EMMC_BOOT


#include "mx6_common.h"
#include <linux/sizes.h>

#define CONFIG_CMDLINE_TAG
#define CONFIG_SETUP_MEMORY_TAGS
#define CONFIG_INITRD_TAG
#define CONFIG_REVISION_TAG
#define CONFIG_SYS_MALLOC_LEN		(10 * SZ_1M)

#define CONFIG_MXC_GPIO
#define CONFIG_MXC_UART

#define CONFIG_CMD_FUSE
#define CONFIG_MXC_OCOTP

/* SATA Configs */
#ifdef CONFIG_CMD_SATA
#define CONFIG_DWC_AHSATA
#define CONFIG_SYS_SATA_MAX_DEVICE	1
#define CONFIG_DWC_AHSATA_PORT_ID	0
#define CONFIG_DWC_AHSATA_BASE_ADDR	SATA_ARB_BASE_ADDR
#define CONFIG_LBA48
#define CONFIG_LIBATA
#endif

/* MMC Configs */
#define CONFIG_FSL_ESDHC
#define CONFIG_FSL_USDHC
#define CONFIG_SYS_FSL_ESDHC_ADDR      0
#define CONFIG_BOUNCE_BUFFER

/* USB Configs */
#ifdef CONFIG_USB
#define CONFIG_USB_EHCI
#define CONFIG_USB_EHCI_MX6
#define CONFIG_USB_MAX_CONTROLLER_COUNT 2
#define CONFIG_EHCI_HCD_INIT_AFTER_RESET
#define CONFIG_MXC_USB_PORTSC	(PORT_PTS_UTMI | PORT_PTS_PTW)
#define CONFIG_MXC_USB_FLAGS	0
#define CONFIG_SYS_USB_EVENT_POLL_VIA_CONTROL_EP

#define CONFIG_CI_UDC
#define CONFIG_USBD_HS
#define CONFIG_USB_GADGET_DUALSPEED
#define CONFIG_USB_GADGET
#define CONFIG_USB_GADGET_DOWNLOAD
#define CONFIG_USB_GADGET_MASS_STORAGE
#define CONFIG_USB_FUNCTION_MASS_STORAGE
#define CONFIG_USB_GADGET_VBUS_DRAW 2
#define CONFIG_G_DNL_VENDOR_NUM   0x0525
#define CONFIG_G_DNL_PRODUCT_NUM  0xa4a5
#define CONFIG_G_DNL_MANUFACTURER "Advantech"
#endif

/* Networking Configs */
#ifdef CONFIG_NET
#define CONFIG_FEC_MXC
#define CONFIG_MII
#define IMX_FEC_BASE			ENET_BASE_ADDR
#define CONFIG_FEC_XCV_TYPE		RGMII
#define CONFIG_ETHPRIME		"FEC"
#define CONFIG_FEC_MXC_PHYADDR		4
#define CONFIG_PHYLIB
#define CONFIG_PHY_ATHEROS
#endif

/* Serial Flash */
#ifdef CONFIG_CMD_SF
#define CONFIG_MXC_SPI
#define CONFIG_SF_DEFAULT_BUS		0
#define CONFIG_SF_DEFAULT_CS		0
#define CONFIG_SF_DEFAULT_SPEED	20000000
#define CONFIG_SF_DEFAULT_MODE		SPI_MODE_0
#endif

/* allow to overwrite serial and ethaddr */
#define CONFIG_ENV_OVERWRITE
#define CONFIG_CONS_INDEX	1

#define CONFIG_LOADADDR	0x12000000
#define CONFIG_SYS_TEXT_BASE	0x17800000

#define CONFIG_EXTRA_ENV_SETTINGS \
	"script=boot.scr\0" \
	"image=/boot/uImage\0" \
	"uboot=u-boot.imx\0" \
	"fdt_file=" CONFIG_DEFAULT_FDT_FILE "\0" \
	"fdt_addr=0x18000000\0" \
	"boot_fdt=yes\0" \
	"ip_dyn=yes\0" \
	"console=" CONSOLE_DEV "\0" \
	"fdt_high=0xffffffff\0"	  \
	"initrd_high=0xffffffff\0" \
	"sddev=0\0" \
	"emmcdev=1\0" \
	"partnum=1\0" \
	"update_sd_firmware=" \
		"if test ${ip_dyn} = yes; then " \
			"setenv get_cmd dhcp; " \
		"else " \
			"setenv get_cmd tftp; " \
		"fi; " \
		"if mmc dev ${mmcdev}; then "	\
			"if ${get_cmd} ${update_sd_firmware_filename}; then " \
				"setexpr fw_sz ${filesize} / 0x200; " \
				"setexpr fw_sz ${fw_sz} + 1; "	\
				"mmc write ${loadaddr} 0x2 ${fw_sz}; " \
			"fi; "	\
		"fi\0" \
	"update_sf_uboot=" \
		"if tftp $loadaddr $uboot; then " \
			"sf probe; " \
			"sf erase 0 0xC0000; " \
			"sf write $loadaddr 0x400 $filesize; " \
			"echo 'U-Boot upgraded. Please reset'; " \
		"fi\0" \
	"setargs=setenv bootargs console=${console},${baudrate} " \
		"root=/dev/${rootdev} rw rootwait cma=128M " \
		BX50V3_BOOTARGS_EXTRA "\0" \
	"loadbootscript=" \
		"ext2load ${dev} ${devnum}:${partnum} ${loadaddr} ${script};\0" \
	"bootscript=echo Running bootscript from ${dev}:${devnum}:${partnum};" \
		" source\0" \
	"loadimage=" \
		"ext2load ${dev} ${devnum}:${partnum} ${loadaddr} ${image}\0" \
	"loadfdt=ext2load ${dev} ${devnum}:${partnum} ${fdt_addr} ${fdt_file}\0" \
	"tryboot=" \
		"if run loadbootscript; then " \
			"run bootscript; " \
		"else " \
			"if run loadimage; then " \
				"run doboot; " \
			"fi; " \
		"fi;\0" \
	"doboot=echo Booting from ${dev}:${devnum}:${partnum} ...; " \
		"run setargs; " \
		"if test ${boot_fdt} = yes || test ${boot_fdt} = try; then " \
			"if run loadfdt; then " \
				"bootm ${loadaddr} - ${fdt_addr}; " \
			"else " \
				"if test ${boot_fdt} = try; then " \
					"bootm; " \
				"else " \
					"echo WARN: Cannot load the DT; " \
				"fi; " \
			"fi; " \
		"else " \
			"bootm; " \
		"fi;\0" \
	"netargs=setenv bootargs console=${console},${baudrate} " \
		"root=/dev/nfs " \
		"ip=dhcp nfsroot=${serverip}:${nfsroot},v3,tcp\0" \
	"netboot=echo Booting from net ...; " \
		"run netargs; " \
		"if test ${ip_dyn} = yes; then " \
			"setenv get_cmd dhcp; " \
		"else " \
			"setenv get_cmd tftp; " \
		"fi; " \
		"${get_cmd} ${image}; " \
		"if test ${boot_fdt} = yes || test ${boot_fdt} = try; then " \
			"if ${get_cmd} ${fdt_addr} ${fdt_file}; then " \
				"bootm ${loadaddr} - ${fdt_addr}; " \
			"else " \
				"if test ${boot_fdt} = try; then " \
					"bootm; " \
				"else " \
					"echo WARN: Cannot load the DT; " \
				"fi; " \
			"fi; " \
		"else " \
			"bootm; " \
		"fi;\0" \

#define CONFIG_MMCBOOTCOMMAND \
	"setenv dev mmc; " \
	"setenv rootdev mmcblk0p${partnum}; " \
	\
	"setenv devnum ${sddev}; " \
	"if mmc dev ${devnum}; then " \
		"run tryboot; " \
		"setenv rootdev mmcblk1p${partnum}; " \
	"fi; " \
	\
	"setenv devnum ${emmcdev}; " \
	"if mmc dev ${devnum}; then " \
		"run tryboot; " \
	"fi; " \

#define CONFIG_USBBOOTCOMMAND \
	"usb start; " \
	"setenv dev usb; " \
	"setenv devnum 0; " \
	"setenv rootdev sda${partnum}; " \
	"run tryboot; " \
	\
	CONFIG_MMCBOOTCOMMAND \
	"bmode usb; " \

#ifdef CONFIG_CMD_USB
#define CONFIG_BOOTCOMMAND CONFIG_USBBOOTCOMMAND
#else
#define CONFIG_BOOTCOMMAND CONFIG_MMCBOOTCOMMAND
#endif

#define CONFIG_ARP_TIMEOUT     200UL

/* Miscellaneous configurable options */
#define CONFIG_SYS_LONGHELP
#define CONFIG_AUTO_COMPLETE

/* Print Buffer Size */
#define CONFIG_SYS_PBSIZE (CONFIG_SYS_CBSIZE + sizeof(CONFIG_SYS_PROMPT) + 16)
#define CONFIG_SYS_BARGSIZE CONFIG_SYS_CBSIZE

#define CONFIG_SYS_MEMTEST_START       0x10000000
#define CONFIG_SYS_MEMTEST_END         0x10010000
#define CONFIG_SYS_MEMTEST_SCRATCH     0x10800000

#define CONFIG_SYS_LOAD_ADDR           CONFIG_LOADADDR

#define CONFIG_CMDLINE_EDITING

/* Physical Memory Map */
#define CONFIG_NR_DRAM_BANKS           1
#define PHYS_SDRAM                     MMDC0_ARB_BASE_ADDR

#define CONFIG_SYS_SDRAM_BASE          PHYS_SDRAM
#define CONFIG_SYS_INIT_RAM_ADDR       IRAM_BASE_ADDR
#define CONFIG_SYS_INIT_RAM_SIZE       IRAM_SIZE

#define CONFIG_SYS_INIT_SP_OFFSET \
	(CONFIG_SYS_INIT_RAM_SIZE - GENERATED_GBL_DATA_SIZE)
#define CONFIG_SYS_INIT_SP_ADDR \
	(CONFIG_SYS_INIT_RAM_ADDR + CONFIG_SYS_INIT_SP_OFFSET)

/* environment organization */
#define CONFIG_ENV_IS_IN_SPI_FLASH
#define CONFIG_ENV_SIZE		(8 * 1024)
#define CONFIG_ENV_OFFSET		(768 * 1024)
#define CONFIG_ENV_SECT_SIZE		(64 * 1024)
#define CONFIG_ENV_SPI_BUS		CONFIG_SF_DEFAULT_BUS
#define CONFIG_ENV_SPI_CS		CONFIG_SF_DEFAULT_CS
#define CONFIG_ENV_SPI_MODE		CONFIG_SF_DEFAULT_MODE
#define CONFIG_ENV_SPI_MAX_HZ		CONFIG_SF_DEFAULT_SPEED

#ifndef CONFIG_SYS_DCACHE_OFF
#endif

#define CONFIG_SYS_FSL_USDHC_NUM	3

/* Framebuffer */
#ifdef CONFIG_VIDEO
#define CONFIG_VIDEO_IPUV3
#define CONFIG_VIDEO_BMP_RLE8
#define CONFIG_SPLASH_SCREEN
#define CONFIG_SPLASH_SCREEN_ALIGN
#define CONFIG_BMP_16BPP
#define CONFIG_VIDEO_LOGO
#define CONFIG_VIDEO_BMP_LOGO
#define CONFIG_IPUV3_CLK 260000000
#define CONFIG_IMX_HDMI
#define CONFIG_IMX_VIDEO_SKIP
#endif

#define CONFIG_PWM_IMX
#define CONFIG_IMX6_PWM_PER_CLK	66000000

#undef CONFIG_CMD_PCI
#ifdef CONFIG_CMD_PCI
#define CONFIG_PCI_SCAN_SHOW
#define CONFIG_PCIE_IMX
#define CONFIG_PCIE_IMX_PERST_GPIO	IMX_GPIO_NR(7, 12)
#define CONFIG_PCIE_IMX_POWER_GPIO	IMX_GPIO_NR(1, 5)
#endif

/* I2C Configs */
#define CONFIG_SYS_I2C
#define CONFIG_SYS_I2C_MXC
#define CONFIG_SYS_I2C_SPEED		  100000
#define CONFIG_SYS_I2C_MXC_I2C1
#define CONFIG_SYS_I2C_MXC_I2C2
#define CONFIG_SYS_I2C_MXC_I2C3

#endif	/* __GE_BX50V3_CONFIG_H */
