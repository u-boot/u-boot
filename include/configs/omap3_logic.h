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

#include <configs/ti_omap3_common.h>

/*
 * We are only ever GP parts and will utilize all of the "downloaded image"
 * area in SRAM which starts at 0x40200000 and ends at 0x4020FFFF (64KB) in
 * order to allow for BCH8 to fit in.
 */
#undef CONFIG_SPL_TEXT_BASE
#define CONFIG_SPL_TEXT_BASE		0x40200000

#define CONFIG_BOARD_LATE_INIT
#define CONFIG_MISC_INIT_R		/* misc_init_r dumps the die id */
#define CONFIG_CMDLINE_TAG		/* enable passing of ATAGs */
#define CONFIG_SETUP_MEMORY_TAGS
#define CONFIG_INITRD_TAG
#define CONFIG_REVISION_TAG
#define CONFIG_CMDLINE_EDITING		/* cmd line edit/history */

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

/* TWL4030 */
#define CONFIG_TWL4030_PWM
#define CONFIG_TWL4030_USB

/* Board NAND Info. */
#ifdef CONFIG_NAND
#define CONFIG_NAND_OMAP_GPMC

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
#define MTDPARTS_DEFAULT	"mtdparts=omap2-nand.0:"\
							"512k(MLO),"\
							"1792k(u-boot),"\
							"128k(spl-os)," \
							"128k(u-boot-env),"\
							"6m(kernel),-(fs)"
#endif

/* Environment information */

#define CONFIG_PREBOOT \
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
	"nfsargs=run setconsole; setenv serverip ${tftpserver}; " \
		"setenv bootargs console=${console} root=/dev/nfs " \
		"nfsroot=${nfsrootpath} " \
		"ip=${ipaddr}:${tftpserver}:${gatewayip}:${netmask}::eth0:off\0" \
	"nfsrootpath=/opt/nfs-exports/omap\0" \
	"autoload=no\0" \
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
		"bootz ${loadaddr} ${ramdiskaddr} ${fdtaddr};\0" \
	"tftpboot=echo 'Booting kernel/ramdisk rootfs from tftp...'; " \
		"run ramargs; " \
		"run common_bootargs; " \
		"run dump_bootargs; " \
		"tftpboot ${loadaddr} ${zimage}; " \
		"tftpboot ${ramdiskaddr} ${ramdiskimage}; " \
		"bootm ${loadaddr} ${ramdiskaddr}\0" \
	"tftpbootz=echo 'Booting kernel NFS rootfs...'; " \
		"dhcp;" \
		"run nfsargs;" \
		"run common_bootargs;" \
		"run dump_bootargs;" \
		"tftpboot $loadaddr zImage;" \
		"bootz $loadaddr\0"

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
