/*
 * Common configuration settings for IGEP technology based boards
 *
 * (C) Copyright 2012
 * ISEE 2007 SL, <www.iseebcn.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __IGEP00X0_H
#define __IGEP00X0_H

#ifdef CONFIG_BOOT_NAND
#define CONFIG_NAND
#endif

#define CONFIG_NR_DRAM_BANKS            2

#include <configs/ti_omap3_common.h>
#include <asm/mach-types.h>

/*
 * Display CPU and Board information
 */
#define CONFIG_DISPLAY_CPUINFO		1
#define CONFIG_DISPLAY_BOARDINFO	1

#define CONFIG_MISC_INIT_R

#define CONFIG_REVISION_TAG		1

#define CONFIG_SUPPORT_RAW_INITRD

/* define to enable boot progress via leds */
#if (CONFIG_MACH_TYPE == MACH_TYPE_IGEP0020) || \
    (CONFIG_MACH_TYPE == MACH_TYPE_IGEP0030)
#define CONFIG_SHOW_BOOT_PROGRESS
#endif

/* GPIO banks */
#define CONFIG_OMAP3_GPIO_3		/* GPIO64 .. 95 is in GPIO bank 3 */
#define CONFIG_OMAP3_GPIO_5		/* GPIO128..159 is in GPIO bank 5 */
#define CONFIG_OMAP3_GPIO_6		/* GPIO160..191 is in GPIO bank 6 */

/* USB */
#define CONFIG_MUSB_UDC			1
#define CONFIG_USB_OMAP3		1
#define CONFIG_TWL4030_USB		1

/* USB device configuration */
#define CONFIG_USB_DEVICE		1
#define CONFIG_USB_TTY			1
#define CONFIG_SYS_CONSOLE_IS_IN_ENV	1

/* Change these to suit your needs */
#define CONFIG_USBD_VENDORID		0x0451
#define CONFIG_USBD_PRODUCTID		0x5678
#define CONFIG_USBD_MANUFACTURER	"Texas Instruments"
#define CONFIG_USBD_PRODUCT_NAME	"IGEP"

#define CONFIG_CMD_CACHE
#ifdef CONFIG_BOOT_ONENAND
#define CONFIG_CMD_ONENAND	/* ONENAND support		*/
#endif
#if (CONFIG_MACH_TYPE == MACH_TYPE_IGEP0020) || \
    (CONFIG_MACH_TYPE == MACH_TYPE_IGEP0032)
#define CONFIG_CMD_NET		/* bootp, tftpboot, rarpboot	*/
#endif
#define CONFIG_CMD_DHCP
#define CONFIG_CMD_PING
#define CONFIG_CMD_NFS		/* NFS support			*/

/*#undef CONFIG_ENV_IS_NOWHERE*/

#define CONFIG_EXTRA_ENV_SETTINGS \
	"usbtty=cdc_acm\0" \
	"loadaddr=0x82000000\0" \
	"dtbaddr=0x81600000\0" \
	"bootdir=/boot\0" \
	"bootfile=zImage\0" \
	"usbtty=cdc_acm\0" \
	"console=ttyO2,115200n8\0" \
	"mpurate=auto\0" \
	"vram=12M\0" \
	"dvimode=1024x768MR-16@60\0" \
	"defaultdisplay=dvi\0" \
	"mmcdev=0\0" \
	"mmcroot=/dev/mmcblk0p2 rw\0" \
	"mmcrootfstype=ext4 rootwait\0" \
	"nandroot=/dev/mtdblock4 rw\0" \
	"nandrootfstype=jffs2\0" \
	"mmcargs=setenv bootargs console=${console} " \
		"mpurate=${mpurate} " \
		"vram=${vram} " \
		"omapfb.mode=dvi:${dvimode} " \
		"omapfb.debug=y " \
		"omapdss.def_disp=${defaultdisplay} " \
		"root=${mmcroot} " \
		"rootfstype=${mmcrootfstype}\0" \
	"nandargs=setenv bootargs console=${console} " \
		"mpurate=${mpurate} " \
		"vram=${vram} " \
		"omapfb.mode=dvi:${dvimode} " \
		"omapfb.debug=y " \
		"omapdss.def_disp=${defaultdisplay} " \
		"root=${nandroot} " \
		"rootfstype=${nandrootfstype}\0" \
	"loadbootenv=load mmc ${mmcdev} ${loadaddr} uEnv.txt\0" \
	"importbootenv=echo Importing environment from mmc ...; " \
		"env import -t $loadaddr $filesize\0" \
	"loadzimage=load mmc ${mmcdev}:2 ${loadaddr} ${bootdir}/${bootfile}\0" \
	"loadfdt=load mmc ${mmcdev}:2 ${dtbaddr} ${bootdir}/${dtbfile}\0" \
	"mmcboot=echo Booting from mmc ...; " \
		"run mmcargs; " \
		"bootz ${loadaddr}\0" \
	"mmcbootfdt=echo Booting with DT from mmc ...; " \
		"bootz ${loadaddr} - ${dtbaddr}\0" \
	"nandboot=echo Booting from onenand ...; " \
		"run nandargs; " \
		"onenand read ${loadaddr} 280000 400000; " \
		"bootz ${loadaddr}\0" \

#define CONFIG_BOOTCOMMAND \
	"mmc dev ${mmcdev}; if mmc rescan; then " \
		"echo SD/MMC found on device ${mmcdev};" \
		"if run loadbootenv; then " \
			"run importbootenv;" \
		"fi;" \
		"if test -n $uenvcmd; then " \
			"echo Running uenvcmd ...;" \
			"run uenvcmd;" \
		"fi;" \
		"if run loadzimage; then " \
			"if test -n $dtbfile; then " \
				"if run loadfdt; then " \
					"run mmcbootfdt;" \
				"fi;" \
			"fi;" \
			"run mmcboot;" \
		"fi;" \
	"fi;" \
	"run nandboot;" \

/*
 * FLASH and environment organization
 */

#ifdef CONFIG_BOOT_ONENAND
#define PISMO1_ONEN_SIZE		GPMC_SIZE_128M /* Configure the PISMO */

#define CONFIG_SYS_ONENAND_BASE		ONENAND_MAP

#define ONENAND_ENV_OFFSET		0x260000 /* environment starts here */

#define CONFIG_ENV_IS_IN_ONENAND	1
#define CONFIG_ENV_SIZE			(512 << 10) /* Total Size Environment */
#define CONFIG_ENV_ADDR			ONENAND_ENV_OFFSET
#endif

#ifdef CONFIG_NAND
#define PISMO1_NAND_SIZE		GPMC_SIZE_128M /* Configure the PISMO */
#define CONFIG_ENV_OFFSET		0x260000 /* environment starts here */
#define CONFIG_ENV_IS_IN_NAND	        1
#define CONFIG_ENV_SIZE			(512 << 10) /* Total Size Environment */
#define CONFIG_ENV_ADDR			NAND_ENV_OFFSET
#endif

/*
 * SMSC911x Ethernet
 */
#if defined(CONFIG_CMD_NET)
#define CONFIG_SMC911X
#define CONFIG_SMC911X_32_BIT
#define CONFIG_SMC911X_BASE	0x2C000000
#endif /* (CONFIG_CMD_NET) */

/* OneNAND boot config */
#ifdef CONFIG_BOOT_ONENAND
#define CONFIG_SPL_ONENAND_SUPPORT
#define CONFIG_SYS_ONENAND_U_BOOT_OFFS  0x80000
#define CONFIG_SYS_ONENAND_PAGE_SIZE	2048
#define CONFIG_SPL_ONENAND_LOAD_ADDR    0x80000
#define CONFIG_SPL_ONENAND_LOAD_SIZE    \
	(512 * 1024 - CONFIG_SPL_ONENAND_LOAD_ADDR)

#endif

/* NAND boot config */
#ifdef CONFIG_NAND
#define CONFIG_SYS_NAND_5_ADDR_CYCLE
#define CONFIG_SYS_NAND_PAGE_COUNT	64
#define CONFIG_SYS_NAND_PAGE_SIZE	2048
#define CONFIG_SYS_NAND_OOBSIZE		64
#define CONFIG_SYS_NAND_BLOCK_SIZE	(128*1024)
#define CONFIG_SYS_NAND_BAD_BLOCK_POS	0
#define CONFIG_SYS_NAND_ECCPOS		{2, 3, 4, 5, 6, 7, 8, 9,\
						10, 11, 12, 13}
#define CONFIG_SYS_NAND_ECCSIZE		512
#define CONFIG_SYS_NAND_ECCBYTES	3
#define CONFIG_NAND_OMAP_ECCSCHEME	OMAP_ECC_HAM1_CODE_HW
#endif

#endif /* __IGEP00X0_H */
