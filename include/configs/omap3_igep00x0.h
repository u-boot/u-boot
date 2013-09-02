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

#include <asm/sizes.h>

/*
 * High Level Configuration Options
 */
#define CONFIG_OMAP		1	/* in a TI OMAP core */
#define CONFIG_OMAP34XX		1	/* which is a 34XX */
#define CONFIG_OMAP_GPIO
#define CONFIG_OMAP_COMMON

#define CONFIG_SDRC	/* The chip has SDRC controller */

#include <asm/arch/cpu.h>
#include <asm/arch/omap3.h>
#include <asm/mach-types.h>

/*
 * Display CPU and Board information
 */
#define CONFIG_DISPLAY_CPUINFO		1
#define CONFIG_DISPLAY_BOARDINFO	1

/* Clock Defines */
#define V_OSCK			26000000	/* Clock output from T2 */
#define V_SCLK			(V_OSCK >> 1)

#define CONFIG_MISC_INIT_R

#define CONFIG_CMDLINE_TAG		1	/* enable passing of ATAGs */
#define CONFIG_SETUP_MEMORY_TAGS	1
#define CONFIG_INITRD_TAG		1
#define CONFIG_REVISION_TAG		1

#define CONFIG_OF_LIBFDT
#define CONFIG_CMD_BOOTZ
#define CONFIG_SUPPORT_RAW_INITRD

/*
 * NS16550 Configuration
 */

#define V_NS16550_CLK			48000000	/* 48MHz (APLL96/2) */

#define CONFIG_SYS_NS16550
#define CONFIG_SYS_NS16550_SERIAL
#define CONFIG_SYS_NS16550_REG_SIZE	(-4)
#define CONFIG_SYS_NS16550_CLK		V_NS16550_CLK

/* select serial console configuration */
#define CONFIG_CONS_INDEX		3
#define CONFIG_SYS_NS16550_COM3		OMAP34XX_UART3
#define CONFIG_SERIAL3			3

/* allow to overwrite serial and ethaddr */
#define CONFIG_ENV_OVERWRITE
#define CONFIG_BAUDRATE			115200
#define CONFIG_SYS_BAUDRATE_TABLE	{4800, 9600, 19200, 38400, 57600, \
					115200}
#define CONFIG_GENERIC_MMC		1
#define CONFIG_MMC			1
#define CONFIG_OMAP_HSMMC		1
#define CONFIG_DOS_PARTITION		1

/* define to enable boot progress via leds */
#if (CONFIG_MACH_TYPE == MACH_TYPE_IGEP0020) || \
    (CONFIG_MACH_TYPE == MACH_TYPE_IGEP0030)
#define CONFIG_SHOW_BOOT_PROGRESS
#endif

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

/* commands to include */
#include <config_cmd_default.h>

#define CONFIG_CMD_CACHE
#define CONFIG_CMD_EXT4
#define CONFIG_CMD_FAT		/* FAT support			*/
#define CONFIG_CMD_FS_GENERIC
#define CONFIG_CMD_I2C		/* I2C serial bus support	*/
#define CONFIG_CMD_MMC		/* MMC support			*/
#ifdef CONFIG_BOOT_ONENAND
#define CONFIG_CMD_ONENAND	/* ONENAND support		*/
#endif
#ifdef CONFIG_BOOT_NAND
#define CONFIG_CMD_NAND
#endif
#if (CONFIG_MACH_TYPE == MACH_TYPE_IGEP0020) || \
    (CONFIG_MACH_TYPE == MACH_TYPE_IGEP0032)
#define CONFIG_CMD_NET		/* bootp, tftpboot, rarpboot	*/
#endif
#define CONFIG_CMD_DHCP
#define CONFIG_CMD_PING
#define CONFIG_CMD_NFS		/* NFS support			*/
#define CONFIG_CMD_MTDPARTS	/* Enable MTD parts commands	*/
#define CONFIG_MTD_DEVICE

#undef CONFIG_CMD_FLASH		/* flinfo, erase, protect	*/
#undef CONFIG_CMD_IMLS		/* List all found images	*/

#define CONFIG_SYS_NO_FLASH
#define CONFIG_HARD_I2C			1
#define CONFIG_SYS_I2C_SPEED		100000
#define CONFIG_SYS_I2C_SLAVE		1
#define CONFIG_DRIVER_OMAP34XX_I2C	1

/*
 * TWL4030
 */
#define CONFIG_TWL4030_POWER		1

#define CONFIG_BOOTDELAY		3

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

#define CONFIG_AUTO_COMPLETE		1

/*
 * Miscellaneous configurable options
 */
#define CONFIG_SYS_LONGHELP		/* undef to save memory */
#define CONFIG_SYS_HUSH_PARSER		/* use "hush" command parser */
#define CONFIG_SYS_PROMPT		"U-Boot # "
#define CONFIG_SYS_CBSIZE		256	/* Console I/O Buffer Size */
/* Print Buffer Size */
#define CONFIG_SYS_PBSIZE		(CONFIG_SYS_CBSIZE + \
					sizeof(CONFIG_SYS_PROMPT) + 16)
#define CONFIG_SYS_MAXARGS		16	/* max number of command args */
/* Boot Argument Buffer Size */
#define CONFIG_SYS_BARGSIZE		(CONFIG_SYS_CBSIZE)

#define CONFIG_SYS_MEMTEST_START	(OMAP34XX_SDRC_CS0)	/* memtest */
								/* works on */
#define CONFIG_SYS_MEMTEST_END		(OMAP34XX_SDRC_CS0 + \
					0x01F00000) /* 31MB */

#define CONFIG_SYS_LOAD_ADDR		(OMAP34XX_SDRC_CS0)	/* default */
							/* load address */

#define CONFIG_SYS_MONITOR_LEN		(256 << 10)

/*
 * OMAP3 has 12 GP timers, they can be driven by the system clock
 * (12/13/16.8/19.2/38.4MHz) or by 32KHz clock. We use 13MHz (V_SCLK).
 * This rate is divided by a local divisor.
 */
#define CONFIG_SYS_TIMERBASE		(OMAP34XX_GPT2)
#define CONFIG_SYS_PTV			2       /* Divisor: 2^(PTV+1) => 8 */
#define CONFIG_SYS_HZ			1000

/*
 * Physical Memory Map
 *
 */
#define CONFIG_NR_DRAM_BANKS	2	/* CS1 may or may not be populated */
#define PHYS_SDRAM_1		OMAP34XX_SDRC_CS0
#define PHYS_SDRAM_2		OMAP34XX_SDRC_CS1

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

#ifdef CONFIG_BOOT_NAND
#define PISMO1_NAND_SIZE		GPMC_SIZE_128M /* Configure the PISMO */
#define CONFIG_NAND_OMAP_GPMC
#define CONFIG_SYS_NAND_BASE		NAND_BASE
#define GPMC_NAND_ECC_LP_x16_LAYOUT	1
#define CONFIG_ENV_OFFSET		0x260000 /* environment starts here */
#define CONFIG_ENV_IS_IN_NAND	        1
#define CONFIG_ENV_SIZE			(512 << 10) /* Total Size Environment */
#define CONFIG_ENV_ADDR			NAND_ENV_OFFSET
#define CONFIG_SYS_MAX_NAND_DEVICE      1
#endif

/*
 * Size of malloc() pool
 */
#define CONFIG_SYS_MALLOC_LEN		(CONFIG_ENV_SIZE + (128 << 10))

/*
 * SMSC911x Ethernet
 */
#if defined(CONFIG_CMD_NET)
#define CONFIG_SMC911X
#define CONFIG_SMC911X_32_BIT
#define CONFIG_SMC911X_BASE	0x2C000000
#endif /* (CONFIG_CMD_NET) */

/*
 * Leave it at 0x80008000 to allow booting new u-boot.bin with X-loader
 * and older u-boot.bin with the new U-Boot SPL.
 */
#define CONFIG_SYS_TEXT_BASE		0x80008000
#define CONFIG_SYS_SDRAM_BASE		PHYS_SDRAM_1
#define CONFIG_SYS_INIT_RAM_ADDR	0x4020f800
#define CONFIG_SYS_INIT_RAM_SIZE	0x800
#define CONFIG_SYS_INIT_SP_ADDR		(CONFIG_SYS_INIT_RAM_ADDR + \
					 CONFIG_SYS_INIT_RAM_SIZE - \
					 GENERATED_GBL_DATA_SIZE)

/* SPL */
#define CONFIG_SPL
#define CONFIG_SPL_FRAMEWORK
#define CONFIG_SPL_NAND_SIMPLE
#define CONFIG_SPL_TEXT_BASE		0x40200800
#define CONFIG_SPL_MAX_SIZE		(54 * 1024)
#define CONFIG_SPL_STACK		LOW_LEVEL_SRAM_STACK

/* move malloc and bss high to prevent clashing with the main image */
#define CONFIG_SYS_SPL_MALLOC_START	0x87000000
#define CONFIG_SYS_SPL_MALLOC_SIZE	0x80000
#define CONFIG_SPL_BSS_START_ADDR	0x87080000	/* end of minimum RAM */
#define CONFIG_SPL_BSS_MAX_SIZE		0x80000		/* 512 KB */

/* MMC boot config */
#define CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_SECTOR	0x300 /* address 0x60000 */
#define CONFIG_SYS_U_BOOT_MAX_SIZE_SECTORS	0x200 /* 256 KB */
#define CONFIG_SYS_MMC_SD_FAT_BOOT_PARTITION	1
#define CONFIG_SPL_FAT_LOAD_PAYLOAD_NAME	"u-boot.img"

#define CONFIG_SPL_BOARD_INIT
#define CONFIG_SPL_LIBCOMMON_SUPPORT
#define CONFIG_SPL_LIBDISK_SUPPORT
#define CONFIG_SPL_I2C_SUPPORT
#define CONFIG_SPL_LIBGENERIC_SUPPORT
#define CONFIG_SPL_MMC_SUPPORT
#define CONFIG_SPL_FAT_SUPPORT
#define CONFIG_SPL_SERIAL_SUPPORT

#define CONFIG_SPL_POWER_SUPPORT
#define CONFIG_SPL_LDSCRIPT		"$(CPUDIR)/omap-common/u-boot-spl.lds"

#ifdef CONFIG_BOOT_ONENAND
#define CONFIG_SPL_ONENAND_SUPPORT

/* OneNAND boot config */
#define CONFIG_SYS_ONENAND_U_BOOT_OFFS  0x80000
#define CONFIG_SYS_ONENAND_PAGE_SIZE	2048
#define CONFIG_SPL_ONENAND_LOAD_ADDR    0x80000
#define CONFIG_SPL_ONENAND_LOAD_SIZE    \
	(512 * 1024 - CONFIG_SPL_ONENAND_LOAD_ADDR)

#endif

#ifdef CONFIG_BOOT_NAND
#define CONFIG_SPL_NAND_SUPPORT
#define CONFIG_SPL_NAND_BASE
#define CONFIG_SPL_NAND_DRIVERS
#define CONFIG_SPL_NAND_ECC

/* NAND boot config */
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
#define CONFIG_SYS_NAND_U_BOOT_START	CONFIG_SYS_TEXT_BASE
#define CONFIG_SYS_NAND_U_BOOT_OFFS	0x80000
#endif

#endif /* __IGEP00X0_H */
