/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2015
 * Heiko Schocher, DENX Software Engineering, hs@denx.de.
 *
 * Based on:
 * Copyright (C) 2012 Freescale Semiconductor, Inc.
 *
 * Configuration settings for the Freescale i.MX6DL aristainetos2 board.
 */
#ifndef __ARISTAINETOS2_CONFIG_H
#define __ARISTAINETOS2_CONFIG_H

#define CONFIG_HOSTNAME		"aristainetos2"

#define CONFIG_MXC_UART_BASE	UART2_BASE
#define CONSOLE_DEV	"ttymxc1"

#define CONFIG_FEC_XCV_TYPE		RGMII

/* Framebuffer */
#define CONFIG_SYS_LDB_CLOCK	28341000
#define CONFIG_LG4573

#include "mx6_common.h"

#define CONFIG_MACH_TYPE	4501
#define CONFIG_MMCROOT		"/dev/mmcblk0p1"

/* MMC Configs */
#define CONFIG_SYS_FSL_ESDHC_ADDR      USDHC1_BASE_ADDR

#define IMX_FEC_BASE			ENET_BASE_ADDR
#define CONFIG_ETHPRIME			"FEC"
#define CONFIG_FEC_MXC_PHYADDR		0

#define CONFIG_SYS_SPI_ST_ENABLE_WP_PIN

#ifdef CONFIG_IMX_HAB
#define HAB_EXTRA_SETTINGS \
	"hab_check_addr=" \
		"if hab_auth_img ${check_addr} ${filesize} ; then " \
			"true;" \
		"else " \
			"echo \"HAB checks ${hab_check_filetype} " \
			"failed!\"; " \
			"false; " \
		"fi;\0" \
	"hab_check_file_fit=" \
		"if env exists enable_hab_check && test " \
			"${enable_hab_check} -eq 1 ; then " \
			"setenv hab_check_filetype \"FIT file on SD card " \
			"or eMMC\";" \
			"env set check_addr ${fit_addr_r};" \
			"run hab_check_addr;" \
		"else " \
			"true; "\
		"fi;\0" \
	"hab_check_file_bootscript=" \
		"if env exists enable_hab_check && test " \
			"${enable_hab_check} -eq 1 ; then " \
			"setenv hab_check_filetype \"Bootscript file\";" \
			"env set check_addr ${loadaddr};" \
			"run hab_check_addr;" \
		"else " \
			"true; "\
		"fi;\0" \
	"hab_check_flash_fit=" \
		"if env exists enable_hab_check && test " \
			"${enable_hab_check} -eq 1 ; then " \
			"setenv hab_check_filetype \"FIT files on flash\";" \
			"env set check_addr ${fit_addr_r};" \
			"run hab_check_addr;" \
		"else " \
			"true; "\
		"fi;\0" \
	"enable_hab_check=1\0"
#else
#define HAB_EXTRA_SETTINGS \
	"hab_check_file_fit=echo HAB check FIT file always returns " \
		"true;true\0" \
	"hab_check_flash_fit=echo HAB check flash FIT always returns " \
		"true;true\0" \
	"hab_check_file_bootscript=echo HAB check bootscript always " \
		"returns true;true\0" \
	"enable_hab_check=0\0"
#endif

#if (CONFIG_SYS_BOARD_VERSION == 3)
#define CONFIG_EXTRA_ENV_BOARD_SETTINGS \
	"dead=led led_red on\0" \
	"mtdids=nand0=gpmi-nand,nor0=spi0.0\0" \
	"mtdparts=mtdparts=spi0.0:832k(u-boot),64k(env),64k(env-red)," \
		"-(ubi-nor);gpmi-nand:-(ubi)\0" \
	"addmisc=setenv bootargs ${bootargs} net.ifnames=0 consoleblank=0 " \
		"bootmode=${bootmode} mmcpart=${mmcpart}\0" \
	"mainboot=echo Booting from SD-card ...; " \
		"run mainargs addmtd addmisc;" \
		"if test -n ${addmiscM}; then run addmiscM;fi;" \
		"if test -n ${addmiscC}; then run addmiscC;fi;" \
		"if test -n ${addmiscD}; then run addmiscD;fi;" \
		"run boot_board_type;" \
		"bootm ${fit_addr_r}\0" \
	"mainargs=setenv bootargs console=${console},${baudrate} " \
		"root=${mmcroot}\0" \
	"main_load_fit=ext4load mmc ${mmcdev}:${mmcpart} ${fit_addr_r} " \
		"${fit_file}\0" \
	"rescue_load_fit=ext4load mmc ${mmcdev}:${mmcrescuepart} " \
		"${fit_addr_r} ${rescue_fit_file}\0"
#elif (CONFIG_SYS_BOARD_VERSION == 4)
#define CONFIG_EXTRA_ENV_BOARD_SETTINGS \
	"dead=led led_red on;led led_red2 on;\0" \
	"mtdids=nand0=gpmi-nand,nor0=spi0.0\0" \
	"mtdparts=mtdparts=spi0.0:832k(u-boot),64k(env),64k(env-red)," \
		"-(ubi-nor);gpmi-nand:-(ubi)\0" \
	"addmisc=setenv bootargs ${bootargs} net.ifnames=0 consoleblank=0 " \
		"bootmode=${bootmode} mmcpart=${mmcpart}\0" \
	"mainboot=echo Booting from SD-card ...; " \
		"run mainargs addmtd addmisc;" \
		"if test -n ${addmiscM}; then run addmiscM;fi;" \
		"if test -n ${addmiscC}; then run addmiscC;fi;" \
		"if test -n ${addmiscD}; then run addmiscD;fi;" \
		"run boot_board_type;" \
		"bootm ${fit_addr_r}\0" \
	"mainargs=setenv bootargs console=${console},${baudrate} " \
		"root=${mmcroot}\0" \
	"main_load_fit=ext4load mmc ${mmcdev}:${mmcpart} ${fit_addr_r} " \
		"${fit_file}\0" \
	"rescue_load_fit=ext4load mmc ${mmcdev}:${mmcrescuepart} " \
		"${fit_addr_r} ${rescue_fit_file}\0"
#elif (CONFIG_SYS_BOARD_VERSION == 5)
#define CONFIG_EXTRA_ENV_BOARD_SETTINGS \
	"emmcpart=1\0" \
	"emmc_rescue_part=3\0" \
	"emmcdev=1\0" \
	"emmcroot=/dev/mmcblk1p1 rootwait rw\0" \
	"dead=led led_red on\0" \
	"mtdids=nor0=spi0.0\0" \
	"mtdparts=mtdparts=spi0.0:832k(u-boot),64k(env),64k(env-red)," \
		"-(ubi-nor)\0" \
	"addmisc=setenv bootargs ${bootargs} net.ifnames=0 consoleblank=0 " \
		"bootmode=${bootmode} mmcpart=${mmcpart} " \
		"emmcpart=${emmcpart}\0" \
	"mainboot=echo Booting from eMMC ...; " \
		"run mainargs addmtd addmisc;" \
		"if test -n ${addmiscM}; then run addmiscM;fi;" \
		"if test -n ${addmiscC}; then run addmiscC;fi;" \
		"if test -n ${addmiscD}; then run addmiscD;fi;" \
		"run boot_board_type;" \
		"bootm ${fit_addr_r}\0" \
	"mainargs=setenv bootargs console=${console},${baudrate} " \
		"root=${emmcroot} rootfstype=ext4\0 " \
	"main_load_fit=ext4load mmc ${emmcdev}:${emmcpart} ${fit_addr_r} " \
		"${fit_file}; " \
		"imi ${fit_addr_r}\0 " \
	"rescue_load_fit=ext4load mmc ${emmcdev}:${emmc_rescue_part} " \
		"${fit_addr_r} ${rescue_fit_file};imi ${fit_addr_r}\0"
#else
#define CONFIG_EXTRA_ENV_BOARD_SETTINGS \
	"dead=led led_red on\0" \
	"mtdids=nand0=gpmi-nand,nor0=spi3.1\0" \
	"mtdparts=mtdparts=spi3.1:832k(u-boot),64k(env),64k(env-red)," \
		"-(ubi-nor);gpmi-nand:-(ubi)\0" \
	"addmisc=setenv bootargs ${bootargs} net.ifnames=0 consoleblank=0 " \
		"bootmode=${bootmode} mmcpart=${mmcpart}\0" \
	"mainboot=echo Booting from SD-card ...; " \
		"run mainargs addmtd addmisc;" \
		"if test -n ${addmiscM}; then run addmiscM;fi;" \
		"if test -n ${addmiscC}; then run addmiscC;fi;" \
		"if test -n ${addmiscD}; then run addmiscD;fi;" \
		"run boot_board_type;" \
		"bootm ${fit_addr_r}\0" \
	"mainargs=setenv bootargs console=${console},${baudrate} " \
		"root=${mmcroot}\0" \
	"main_load_fit=ext4load mmc ${mmcdev}:${mmcpart} ${fit_addr_r} " \
		"${fit_file}\0" \
	"rescue_load_fit=ext4load mmc ${mmcdev}:${mmcrescuepart} " \
		"${fit_addr_r} ${rescue_fit_file}\0"
#endif

#define CONFIG_EXTRA_ENV_SETTINGS \
	"disable_giga=yes\0" \
	"usb_pgood_delay=2000\0" \
	"nor_bootdelay=-2\0" \
	"script=u-boot.scr\0" \
	"fit_file=/boot/system.itb\0" \
	"rescue_fit_file=/boot/rescue.itb\0" \
	"loadaddr=0x12000000\0" \
	"fit_addr_r=0x14000000\0" \
	"uboot=/boot/u-boot.imx\0" \
	"uboot_sz=d0000\0" \
	"panel=lb07wv8\0" \
	"splashpos=m,m\0" \
	"console=" CONSOLE_DEV "\0" \
	"fdt_high=0xffffffff\0"	  \
	"initrd_high=0xffffffff\0" \
	"addmtd=setenv bootargs ${bootargs} ${mtdparts}\0" \
	"boot_board_type=bootm ${fit_addr_r}#${board_type}\0" \
	"get_env=mw ${loadaddr} 0 0x20000;" \
		"mmc rescan;" \
		"ext4load mmc ${mmcdev}:${mmcpart} ${loadaddr} env.txt;" \
		"env import -t ${loadaddr}\0" \
	"default_env=gpio set wp_spi_nor.gpio-hog;" \
		"sf probe;" \
		"sf protect unlock 0 0x1000000;" \
		"mw ${loadaddr} 0 0x20000;" \
		"env export -t ${loadaddr} serial# ethaddr " \
		"board_type panel addmisc addmiscM addmiscC addmiscD;" \
		"env default -a;" \
		"env import -t ${loadaddr}\0" \
	"loadbootscript=" \
		"ext4load mmc ${mmcdev}:${mmcpart} ${loadaddr} " \
		"${script};\0" \
	"loadbootscriptUSB=" \
		"ext4load usb 0 ${loadaddr} ${script};\0" \
	"loadbootscriptUSBf=" \
		"fatload usb 0 ${loadaddr} ${script};\0" \
	"bootscriptUSB=echo Running bootscript from usb-stick ...; " \
		"source\0" \
	"bootscript=echo Running bootscript from mmc ...; " \
		"source\0" \
	"mmcpart=1\0" \
	"mmcrescuepart=3\0" \
	"mmcdev=0\0" \
	"mmcroot=" CONFIG_MMCROOT " rootwait rw\0" \
	"mmcargs=setenv bootargs console=${console},${baudrate} " \
		"root=${mmcroot}\0" \
	"mmcboot=echo Booting from mmc ...; " \
		"run mmcargs addmtd addmisc;" \
		"if test -n ${addmiscM}; then run addmiscM;fi;" \
		"if test -n ${addmiscC}; then run addmiscC;fi;" \
		"if test -n ${addmiscD}; then run addmiscD;fi;" \
		"run boot_board_type;" \
		"bootm ${fit_addr_r}\0" \
	"mmc_load_fit=ext4load mmc ${mmcdev}:${mmcpart} ${fit_addr_r} " \
		"${fit_file}\0" \
	"mmc_load_uboot=ext4load mmc ${mmcdev}:${mmcpart} ${loadaddr} " \
		"${uboot}\0" \
	"mmc_rescue_load_fit=ext4load mmc ${mmcdev}:${mmcrescuepart} " \
		"${fit_addr_r} ${rescue_fit_file}\0" \
	"mmc_upd_uboot=mw.b ${loadaddr} 0xff ${uboot_sz};" \
		"setexpr cmp_buf ${loadaddr} + ${uboot_sz};" \
		"setexpr uboot_maxsize ${uboot_sz} - 400;" \
		"mw.b ${cmp_buf} 0x00 ${uboot_sz};" \
		"run mmc_load_uboot;sf probe;sf erase 0 ${uboot_sz};" \
		"sf write ${loadaddr} 400 ${filesize};" \
		"sf read ${cmp_buf} 400 ${uboot_sz};" \
		"cmp.b ${loadaddr} ${cmp_buf} ${uboot_maxsize}\0" \
	"rescueargs=setenv bootargs console=${console},${baudrate} " \
		"root=/dev/ram rw\0 " \
	"rescueboot=echo Booting rescue system ...; " \
		"run rescueargs addmtd addmisc;" \
		"if test -n ${rescue_reason}; then run rescue_reason;fi;" \
		"if test -n ${addmiscM}; then run addmiscM;fi;" \
		"if test -n ${addmiscC}; then run addmiscC;fi;" \
		"if test -n ${addmiscD}; then run addmiscD;fi;" \
		"run boot_board_type;" \
		"if bootm ${fit_addr_r}; then ; " \
		"else " \
			"run dead; " \
		"fi; \0" \
	"r_reason_syserr=setenv rescue_reason setenv bootargs " \
		"\\\\${bootargs} " \
	"rescueReason=18\0 " \
	"usb_load_fit=ext4load usb 0 ${fit_addr_r} ${fit_file}\0" \
	"usb_load_fitf=fatload usb 0 ${fit_addr_r} ${fit_file}\0" \
	"usb_load_rescuefit=ext4load usb 0 ${fit_addr_r} " \
		"${rescue_fit_file}\0" \
	"usb_load_rescuefitf=fatload usb 0 ${fit_addr_r} " \
		"${rescue_fit_file}\0" \
	"usbroot=/dev/sda1 rootwait rw\0" \
	"usbboot=echo Booting from usb-stick ...; " \
		"run usbargs addmtd addmisc;" \
		"if test -n ${addmiscM}; then run addmiscM;fi;" \
		"if test -n ${addmiscC}; then run addmiscC;fi;" \
		"if test -n ${addmiscD}; then run addmiscD;fi;" \
		"run boot_board_type;" \
		"bootm ${fit_addr_r}\0" \
	"usbargs=setenv bootargs console=${console},${baudrate} " \
		"root=${usbroot}\0" \
	"mmc_rescue_boot=" \
		"run r_reason_syserr;" \
		"if run mmc_rescue_load_fit hab_check_file_fit; then " \
			"run rescueboot; " \
		"else " \
			"run dead; " \
			"echo RESCUE SYSTEM FROM SD-CARD BOOT FAILURE;" \
		"fi;\0" \
	"main_rescue_boot=" \
		"if run main_load_fit hab_check_flash_fit; then " \
			"if run mainboot; then ; " \
			"else " \
				"run r_reason_syserr;" \
				"if run rescue_load_fit hab_check_file_fit;" \
					"then run rescueboot; " \
				"else " \
					"run dead; " \
					"echo RESCUE SYSTEM BOOT FAILURE;" \
				"fi; " \
			"fi; " \
		"else " \
			"run r_reason_syserr;" \
			"if run rescue_load_fit hab_check_file_fit; then " \
				"run rescueboot; " \
			"else " \
				"run dead; " \
				"echo RESCUE SYSTEM BOOT FAILURE;" \
			"fi; " \
		"fi;\0" \
	"usb_mmc_rescue_boot=" \
		"usb start;" \
		"if usb storage; then " \
			"if run loadbootscriptUSB " \
				"hab_check_file_bootscript;" \
				"then run bootscriptUSB; " \
			"fi; " \
			"if run loadbootscriptUSBf " \
				"hab_check_file_bootscript;" \
				"then run bootscriptUSB; " \
			"fi; " \
			"if run usb_load_fit hab_check_file_fit; then " \
				"run usbboot; " \
			"fi; " \
			"if run usb_load_fitf hab_check_file_fit; then " \
				"run usbboot; " \
			"fi; "\
			"if run usb_load_rescuefit hab_check_file_fit;" \
				"then run r_reason_syserr rescueboot;" \
			"fi; " \
			"if run usb_load_rescuefitf hab_check_file_fit;" \
				"then run r_reason_syserr rescueboot;" \
			"fi; " \
			"run mmc_rescue_boot;" \
		"fi; "\
		"run mmc_rescue_boot;\0" \
	"rescue_xload_boot=" \
		"run r_reason_syserr;" \
		"if test ${bootmode} -ne 0 ; then " \
			"mmc dev ${mmcdev};" \
			"if mmc rescan; then " \
				"if run mmc_rescue_load_fit " \
					"hab_check_file_fit; then " \
					"run rescueboot; " \
				"else " \
					"usb start;" \
					"if usb storage; then " \
						"if run usb_load_rescuefit " \
							"hab_check_file_fit;"\
							"then " \
							"run rescueboot;" \
						"fi; " \
						"if run usb_load_rescuefitf "\
							"hab_check_file_fit;"\
							"then " \
							"run rescueboot;" \
						"fi; " \
					"fi;" \
				"fi;" \
				"run dead; " \
				"echo RESCUE SYSTEM ON SD OR " \
					"USB BOOT FAILURE;" \
			"else " \
				"usb start;" \
				"if usb storage; then " \
					"if run usb_load_rescuefit " \
						"hab_check_file_fit; then " \
						"run rescueboot;" \
					"fi; " \
					"if run usb_load_rescuefitf " \
						"hab_check_file_fit; then " \
						"run rescueboot;" \
					"fi; " \
				"fi;" \
				"run dead; " \
				"echo RESCUE SYSTEM ON USB BOOT FAILURE;" \
			"fi; " \
		"else "\
			"if run rescue_load_fit hab_check_file_fit; then " \
				"run rescueboot; " \
			"else " \
				"run dead; " \
				"echo RESCUE SYSTEM ON BOARD BOOT FAILURE;" \
			"fi; " \
		"fi;\0" \
	"ari_boot=if test ${bootmode} -ne 0 ; then " \
		"mmc dev ${mmcdev};" \
		"if mmc rescan; then " \
			"if run loadbootscript hab_check_file_bootscript;" \
				"then run bootscript; " \
			"fi; " \
			"if run mmc_load_fit hab_check_file_fit; then " \
				"if run mmcboot; then ; " \
				"else " \
					"run mmc_rescue_boot;" \
				"fi; " \
			"else " \
				"run usb_mmc_rescue_boot;" \
			"fi; " \
		"else " \
			"run usb_mmc_rescue_boot;" \
		"fi; " \
	"else "\
		"run main_rescue_boot;" \
	"fi; \0"\
	HAB_EXTRA_SETTINGS \
	CONFIG_EXTRA_ENV_BOARD_SETTINGS

#define CONFIG_ARP_TIMEOUT		200UL

#define CONFIG_SYS_MEMTEST_START	PHYS_SDRAM
#define CONFIG_SYS_MEMTEST_END		(CONFIG_SYS_MEMTEST_START + 0x100000)
#define CONFIG_SYS_MEMTEST_SCRATCH	0x10800000

/* Physical Memory Map */
#define PHYS_SDRAM			MMDC0_ARB_BASE_ADDR

#define CONFIG_SYS_SDRAM_BASE		PHYS_SDRAM
#define CONFIG_SYS_INIT_RAM_ADDR	IRAM_BASE_ADDR
#define CONFIG_SYS_INIT_RAM_SIZE	IRAM_SIZE

#define CONFIG_SYS_INIT_SP_OFFSET \
	(CONFIG_SYS_INIT_RAM_SIZE - GENERATED_GBL_DATA_SIZE)
#define CONFIG_SYS_INIT_SP_ADDR \
	(CONFIG_SYS_INIT_RAM_ADDR + CONFIG_SYS_INIT_SP_OFFSET)

#define CONFIG_SYS_FSL_USDHC_NUM	2

/* NAND stuff */
#define CONFIG_SYS_MAX_NAND_DEVICE	1
#define CONFIG_SYS_NAND_BASE		0x40000000
#define CONFIG_SYS_NAND_5_ADDR_CYCLE
#define CONFIG_SYS_NAND_ONFI_DETECTION

/* DMA stuff, needed for GPMI/MXS NAND support */

/* USB Configs */
#define CONFIG_EHCI_HCD_INIT_AFTER_RESET	/* For OTG port */
#define CONFIG_MXC_USB_PORTSC	(PORT_PTS_UTMI | PORT_PTS_PTW)
#define CONFIG_MXC_USB_FLAGS	0

/* UBI support */

/* Framebuffer */
/* check this console not needed, after test remove it */
#define CONFIG_SPLASH_SCREEN
#define CONFIG_SPLASH_SCREEN_ALIGN
#define CONFIG_IMX_VIDEO_SKIP
#define CONFIG_VIDEO_LOGO
#define CONFIG_VIDEO_BMP_LOGO
#define CONFIG_BMP_16BPP
#define CONFIG_VIDEO_BMP_RLE8

#define CONFIG_IMX6_PWM_PER_CLK	66000000

#endif                         /* __ARISTAINETOS2_CONFIG_H */
