/*
 * (C) Copyright 2011 Logic Product Development <www.logicpd.com>
 *	Peter Barada <peter.barada@logicpd.com>
 *
 * Configuration settings for the Logic OMAP35x/DM37x SOM LV/Torpedo
 * reference boards.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __CONFIG_H
#define __CONFIG_H

/* High Level Configuration Options */

#define CONFIG_NR_DRAM_BANKS	2	/* CS1 may or may not be populated */

/*
 * 1MB into the SDRAM to allow for SPL's bss at the beginning of SDRAM
 * 64 bytes before this address should be set aside for u-boot.img's
 * header. That is 0x800FFFC0--0x80100000 should not be used for any
 * other needs.  We use this rather than the inherited defines from
 * ti_armv7_common.h for backwards compatibility.
 */
#define CONFIG_SYS_TEXT_BASE		0x80100000
#define CONFIG_SPL_BSS_START_ADDR	0x80000000
#define CONFIG_SPL_BSS_MAX_SIZE		(512 << 10)	/* 512 KB */
#define CONFIG_SYS_SPL_MALLOC_START	0x80208000
#define CONFIG_SYS_SPL_MALLOC_SIZE	0x100000

#include <configs/ti_omap3_common.h>

/* Override default SPL info to minimize empty space and allow BCH8 in SPL */
#undef CONFIG_SPL_TEXT_BASE
#undef CONFIG_SPL_MAX_SIZE
#define CONFIG_SPL_TEXT_BASE   0x40200000
#define CONFIG_SPL_MAX_SIZE    (SRAM_SCRATCH_SPACE_ADDR - CONFIG_SPL_TEXT_BASE)

/* Display CPU and Board information */

#define CONFIG_DISPLAY_CPUINFO
#define CONFIG_DISPLAY_BOARDINFO
#define CONFIG_BOARD_LATE_INIT
#define CONFIG_MISC_INIT_R		/* misc_init_r dumps the die id */
#define CONFIG_CMDLINE_TAG		/* enable passing of ATAGs */
#define CONFIG_SETUP_MEMORY_TAGS
#define CONFIG_INITRD_TAG
#define CONFIG_REVISION_TAG
#define CONFIG_CMDLINE_EDITING		/* cmd line edit/history */
#define CONFIG_ZERO_BOOTDELAY_CHECK	/* check keypress w/no delay */

/* Hardware drivers */

/* GPIO banks */
#define CONFIG_OMAP3_GPIO_6		/* GPIO160..191 is in GPIO bank 6 */

#define CONFIG_USB_OMAP3

/* select serial console configuration */
#undef CONFIG_CONS_INDEX
#define CONFIG_CONS_INDEX		1
#define CONFIG_SYS_NS16550_COM1		OMAP34XX_UART1
#define CONFIG_SERIAL1			1	/* UART1 on OMAP Logic boards */

/* commands to include */
#define CONFIG_CMD_NAND
#define CONFIG_CMD_MTDPARTS
#define CONFIG_CMD_NAND_LOCK_UNLOCK	/* nand (un)lock commands	*/

/* I2C */
#define CONFIG_SYS_I2C_OMAP34XX
#define CONFIG_SYS_I2C_EEPROM_ADDR	0x50	/* EEPROM AT24C64      */
#define EXPANSION_EEPROM_I2C_BUS	2	/* I2C Bus for AT24C64 */
#define CONFIG_OMAP3_LOGIC_USE_NEW_PRODUCT_ID

/* USB */
#define CONFIG_USB_MUSB_OMAP2PLUS
#define CONFIG_USB_MUSB_PIO_ONLY
#define CONFIG_USB_ETHER
#define CONFIG_USB_ETHER_RNDIS
#define CONFIG_USB_FUNCTION_FASTBOOT
#define CONFIG_CMD_FASTBOOT
#define CONFIG_ANDROID_BOOT_IMAGE
#define CONFIG_FASTBOOT_BUF_ADDR	CONFIG_SYS_LOAD_ADDR
#define CONFIG_FASTBOOT_BUF_SIZE	0x07000000
#define CONFIG_SYS_CACHELINE_SIZE	64

/* TWL4030 */
#define CONFIG_TWL4030_PWM
#define CONFIG_TWL4030_USB

/* Board NAND Info. */
#ifdef CONFIG_NAND
#define CONFIG_NAND_OMAP_GPMC

#define CONFIG_CMD_UBI			/* UBI-formated MTD partition support */
#define CONFIG_CMD_UBIFS		/* Read-only UBI volume operations */
#define CONFIG_RBTREE			/* required by CONFIG_CMD_UBI */
#define CONFIG_LZO			/* required by CONFIG_CMD_UBIFS */

#define CONFIG_SYS_NAND_ADDR		NAND_BASE /* physical address */
						  /* to access nand */
#define CONFIG_SYS_MAX_NAND_DEVICE	1	  /* Max number of */
						  /* NAND devices */
#define CONFIG_SYS_NAND_BUSWIDTH_16BIT
#define CONFIG_SYS_NAND_5_ADDR_CYCLE
#define CONFIG_SYS_NAND_PAGE_COUNT	64
#define CONFIG_SYS_NAND_PAGE_SIZE	2048
#define CONFIG_SYS_NAND_OOBSIZE		64
#define CONFIG_SYS_NAND_BLOCK_SIZE	(128 * 1024)
#define CONFIG_SYS_NAND_BAD_BLOCK_POS	NAND_LARGE_BADBLOCK_POS
#define CONFIG_SYS_NAND_ECCPOS		{2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, \
					 13, 14, 16, 17, 18, 19, 20, 21, 22, \
					 23, 24, 25, 26, 27, 28, 30, 31, 32, \
					 33, 34, 35, 36, 37, 38, 39, 40, 41, \
					 42, 44, 45, 46, 47, 48, 49, 50, 51, \
					 52, 53, 54, 55, 56}

#define CONFIG_SYS_NAND_ECCSIZE		512
#define CONFIG_SYS_NAND_ECCBYTES	13
#define CONFIG_NAND_OMAP_ECCSCHEME	OMAP_ECC_BCH8_CODE_HW_DETECTION_SW
#define CONFIG_BCH
#define CONFIG_SYS_NAND_MAX_OOBFREE	2
#define CONFIG_SYS_NAND_MAX_ECCPOS	56
#define CONFIG_SYS_NAND_U_BOOT_OFFS	0x80000
#define CONFIG_MTD_DEVICE		/* needed for mtdparts commands */
#define CONFIG_MTD_PARTITIONS		/* required for UBI partition support */
#define MTDIDS_DEFAULT			"nand0=omap2-nand.0"
#define MTDPARTS_DEFAULT		"mtdparts=omap2-nand.0:512k(MLO),"\
					"1920k(u-boot),128k(u-boot-env),"\
					"4m(kernel),-(fs)"
#endif

/* Environment information */

/*
 * PREBOOT assumes the 4.3" display is attached.  User can interrupt
 * and modify display variable to suit their needs.
 */
#define CONFIG_PREBOOT \
	"echo ======================NOTICE============================;"\
	"echo \"The u-boot environment is not set.\";"			\
	"echo \"If using a display a valid display variable for your panel\";" \
	"echo \"needs to be set.\";"					\
	"echo \"Valid display options are:\";"				\
	"echo \"  2 == LQ121S1DG31     TFT SVGA    (12.1)  Sharp\";"	\
	"echo \"  3 == LQ036Q1DA01     TFT QVGA    (3.6)   Sharp w/ASIC\";" \
	"echo \"  5 == LQ064D343       TFT VGA     (6.4)   Sharp\";"	\
	"echo \"  7 == LQ10D368        TFT VGA     (10.4)  Sharp\";"	\
	"echo \" 15 == LQ043T1DG01     TFT WQVGA   (4.3)   Sharp (DEFAULT)\";" \
	"echo \" vga[-dvi or -hdmi]    LCD VGA     640x480\";"          \
	"echo \" svga[-dvi or -hdmi]   LCD SVGA    800x600\";"          \
	"echo \" xga[-dvi or -hdmi]    LCD XGA     1024x768\";"         \
	"echo \" 720p[-dvi or -hdmi]   LCD 720P    1280x720\";"         \
	"echo \"Defaulting to 4.3 LCD panel (display=15).\";"		\
	"setenv display 15;"						\
	"setenv preboot;"						\
	"nand unlock;"							\
	"saveenv;"

#define CONFIG_EXTRA_ENV_SETTINGS \
	"loadaddr=0x81000000\0" \
	"uimage=uImage\0" \
	"zimage=zImage\0" \
	"mtdids=" MTDIDS_DEFAULT "\0"	\
	"mtdparts=" MTDPARTS_DEFAULT "\0" \
	"mmcdev=0\0" \
	"mmcroot=/dev/mmcblk0p2 rw\0" \
	"mmcrootfstype=ext4 rootwait\0" \
	"nandroot=ubi0:rootfs rw ubi.mtd=fs noinitrd\0" \
	"nandrootfstype=ubifs rootwait\0" \
	"autoboot=mmc dev ${mmcdev}; if mmc rescan; then " \
			"if run loadbootscript; then " \
				"run bootscript; " \
			"else " \
				"run defaultboot;" \
			"fi; " \
		"else run defaultboot; fi\0" \
	"defaultboot=run mmcramboot\0" \
	"consoledevice=ttyO0\0" \
	"display=15\0" \
	"setconsole=setenv console ${consoledevice},${baudrate}n8\0" \
	"dump_bootargs=echo 'Bootargs: '; echo $bootargs\0" \
	"rotation=0\0" \
	"vrfb_arg=if itest ${rotation} -ne 0; then " \
		"setenv bootargs ${bootargs} omapfb.vrfb=y " \
		"omapfb.rotate=${rotation}; " \
		"fi\0" \
	"optargs=ignore_loglevel early_printk no_console_suspend\0" \
	"addmtdparts=setenv bootargs ${bootargs} ${mtdparts}\0" \
	"common_bootargs=setenv bootargs ${bootargs} display=${display} " \
		"${optargs};" \
		"run addmtdparts; " \
		"run vrfb_arg\0" \
	"loadbootscript=fatload mmc ${mmcdev} ${loadaddr} boot.scr\0" \
	"bootscript=echo 'Running bootscript from mmc ...'; " \
		"source ${loadaddr}\0" \
	"loaduimage=mmc rescan; " \
		"fatload mmc ${mmcdev} ${loadaddr} ${uimage}\0" \
	"loadzimage=mmc rescan; " \
		"fatload mmc ${mmcdev} ${loadaddr} ${zimage}\0" \
	"ramdisksize=64000\0" \
	"ramdiskaddr=0x82000000\0" \
	"ramdiskimage=rootfs.ext2.gz.uboot\0" \
	"loadramdisk=mmc rescan; " \
		"fatload mmc ${mmcdev} ${ramdiskaddr} ${ramdiskimage}\0" \
	"ramargs=run setconsole; setenv bootargs console=${console} " \
		"root=/dev/ram rw ramdisk_size=${ramdisksize}\0" \
	"mmcargs=run setconsole; setenv bootargs console=${console} " \
		"${optargs} " \
		"root=${mmcroot} " \
		"rootfstype=${mmcrootfstype}\0" \
	"nandargs=run setconsole; setenv bootargs console=${console} " \
		"${optargs} " \
		"root=${nandroot} " \
		"rootfstype=${nandrootfstype}\0" \
	"fdtaddr=0x86000000\0" \
	"loadfdtimage=mmc rescan; " \
		"fatload mmc ${mmcdev} ${fdtaddr} ${fdtimage}\0" \
	"mmcbootz=echo Booting with DT from mmc${mmcdev} ...; " \
		"run mmcargs; " \
		"run common_bootargs; " \
		"run dump_bootargs; " \
		"run loadzimage; " \
		"run loadfdtimage; " \
		"bootz ${loadaddr} - ${fdtaddr}\0" \
	"mmcramboot=echo 'Booting uImage kernel from mmc w/ramdisk...'; " \
		"run ramargs; " \
		"run common_bootargs; " \
		"run dump_bootargs; " \
		"run loaduimage; " \
		"run loadramdisk; " \
		"bootm ${loadaddr} ${ramdiskaddr}\0" \
	"mmcrambootz=echo 'Booting zImage kernel from mmc w/ramdisk...'; " \
		"run ramargs; " \
		"run common_bootargs; " \
		"run dump_bootargs; " \
		"run loadzimage; " \
		"run loadramdisk; " \
		"run loadfdtimage; " \
		"bootz ${loadaddr} ${ramdiskaddr} ${fdtaddr}\0; " \
	"tftpboot=echo 'Booting kernel/ramdisk rootfs from tftp...'; " \
		"run ramargs; " \
		"run common_bootargs; " \
		"run dump_bootargs; " \
		"tftpboot ${loadaddr} ${uimage}; " \
		"tftpboot ${ramdiskaddr} ${ramdiskimage}; " \
		"bootm ${loadaddr} ${ramdiskaddr}\0"

#define CONFIG_BOOTCOMMAND \
	"run autoboot"

/* Miscellaneous configurable options */
#define CONFIG_AUTO_COMPLETE

/* memtest works on */
#define CONFIG_SYS_MEMTEST_START	(OMAP34XX_SDRC_CS0)
#define CONFIG_SYS_MEMTEST_END		(OMAP34XX_SDRC_CS0 + \
					0x01F00000) /* 31MB */

/* FLASH and environment organization */

/* **** PISMO SUPPORT *** */
#if defined(CONFIG_CMD_NAND)
#define CONFIG_SYS_FLASH_BASE		NAND_BASE
#elif defined(CONFIG_CMD_ONENAND)
#define CONFIG_SYS_FLASH_BASE		ONENAND_MAP
#endif

/* Monitor at start of flash */
#define CONFIG_SYS_MONITOR_BASE		CONFIG_SYS_FLASH_BASE

#define CONFIG_ENV_IS_IN_NAND		1
#define CONFIG_ENV_SIZE			(128 << 10)	/* 128 KiB */
#define ONENAND_ENV_OFFSET		0x260000 /* environment starts here */
#define SMNAND_ENV_OFFSET		0x260000 /* environment starts here */

#define CONFIG_SYS_ENV_SECT_SIZE	(128 << 10)	/* 128 KiB */
#define CONFIG_ENV_OFFSET		SMNAND_ENV_OFFSET
#define CONFIG_ENV_ADDR			SMNAND_ENV_OFFSET

/* SMSC922x Ethernet */
#if defined(CONFIG_CMD_NET)
#define CONFIG_SMC911X
#define CONFIG_SMC911X_32_BIT
#define CONFIG_SMC911X_BASE	0x08000000
#endif /* (CONFIG_CMD_NET) */

/* Defines for SPL */

#define CONFIG_SPL_OMAP3_ID_NAND

/* NAND: SPL falcon mode configs */
#ifdef CONFIG_SPL_OS_BOOT
#define CONFIG_CMD_SPL_NAND_OFS		0x240000
#define CONFIG_SYS_NAND_SPL_KERNEL_OFFS	0x280000
#define CONFIG_CMD_SPL_WRITE_SIZE	0x2000
#endif

#endif /* __CONFIG_H */
