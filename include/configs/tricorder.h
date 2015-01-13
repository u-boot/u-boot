/*
 * (C) Copyright 2006-2008
 * Texas Instruments.
 * Richard Woodruff <r-woodruff2@ti.com>
 * Syed Mohammed Khasim <x0khasim@ti.com>
 *
 * (C) Copyright 2012
 * Corscience GmbH & Co. KG
 * Thomas Weber <weber@corscience.de>
 *
 * Configuration settings for the Tricorder board.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __CONFIG_H
#define __CONFIG_H

/* High Level Configuration Options */
#define CONFIG_OMAP			/* in a TI OMAP core */
#define CONFIG_OMAP_COMMON

#define CONFIG_MACH_TYPE		MACH_TYPE_TRICORDER
/*
 * 1MB into the SDRAM to allow for SPL's bss at the beginning of SDRAM
 * 64 bytes before this address should be set aside for u-boot.img's
 * header. That is 0x800FFFC0--0x80100000 should not be used for any
 * other needs.
 */
#define CONFIG_SYS_TEXT_BASE		0x80100000

#define CONFIG_SDRC			/* The chip has SDRC controller */

#include <asm/arch/cpu.h>		/* get chip and board defs */
#include <asm/arch/omap3.h>

#define CONFIG_SYS_GENERIC_BOARD

/* Display CPU and Board information */
#define CONFIG_DISPLAY_CPUINFO
#define CONFIG_DISPLAY_BOARDINFO

#define CONFIG_SILENT_CONSOLE
#define CONFIG_ZERO_BOOTDELAY_CHECK

/* Clock Defines */
#define V_OSCK				26000000 /* Clock output from T2 */
#define V_SCLK				(V_OSCK >> 1)

#define CONFIG_MISC_INIT_R

#define CONFIG_CMDLINE_TAG		/* enable passing of ATAGs */
#define CONFIG_SETUP_MEMORY_TAGS
#define CONFIG_INITRD_TAG
#define CONFIG_REVISION_TAG

#define CONFIG_OF_LIBFDT

/* Size of malloc() pool */
#define CONFIG_SYS_MALLOC_LEN		(1024*1024)

/* Hardware drivers */

/* GPIO support */
#define CONFIG_OMAP_GPIO

/* GPIO banks */
#define CONFIG_OMAP3_GPIO_2		/* GPIO32..63 are in GPIO bank 2 */

/* LED support */
#define CONFIG_STATUS_LED
#define CONFIG_BOARD_SPECIFIC_LED
#define CONFIG_CMD_LED			/* LED command */
#define STATUS_LED_BIT			(1 << 0)
#define STATUS_LED_STATE		STATUS_LED_ON
#define STATUS_LED_PERIOD		(CONFIG_SYS_HZ / 2)
#define STATUS_LED_BIT1			(1 << 1)
#define STATUS_LED_STATE1		STATUS_LED_ON
#define STATUS_LED_PERIOD1		(CONFIG_SYS_HZ / 2)
#define STATUS_LED_BIT2			(1 << 2)
#define STATUS_LED_STATE2		STATUS_LED_ON
#define STATUS_LED_PERIOD2		(CONFIG_SYS_HZ / 2)

/* NS16550 Configuration */
#define CONFIG_SYS_NS16550
#define CONFIG_SYS_NS16550_SERIAL
#define CONFIG_SYS_NS16550_REG_SIZE	(-4)
#define CONFIG_SYS_NS16550_CLK		48000000 /* 48MHz (APLL96/2) */

/* select serial console configuration */
#define CONFIG_CONS_INDEX		3
#define CONFIG_SYS_NS16550_COM3		OMAP34XX_UART3
#define CONFIG_SERIAL3			3
#define CONFIG_BAUDRATE			115200
#define CONFIG_SYS_BAUDRATE_TABLE	{4800, 9600, 19200, 38400, 57600,\
					115200}

/* MMC */
#define CONFIG_GENERIC_MMC
#define CONFIG_MMC
#define CONFIG_OMAP_HSMMC
#define CONFIG_DOS_PARTITION

/* I2C */
#define CONFIG_SYS_I2C
#define CONFIG_SYS_OMAP24_I2C_SPEED	100000
#define CONFIG_SYS_OMAP24_I2C_SLAVE	1
#define CONFIG_SYS_I2C_OMAP34XX
 

/* EEPROM */
#define CONFIG_SYS_I2C_MULTI_EEPROMS
#define CONFIG_CMD_EEPROM
#define CONFIG_SYS_I2C_EEPROM_ADDR_LEN	2
#define CONFIG_SYS_EEPROM_BUS_NUM	1

/* TWL4030 */
#define CONFIG_TWL4030_POWER
#define CONFIG_TWL4030_LED

/* Board NAND Info */
#define CONFIG_SYS_NO_FLASH		/* no NOR flash */
#define CONFIG_MTD_DEVICE		/* needed for mtdparts commands */
#define MTDIDS_DEFAULT			"nand0=omap2-nand.0"
#define MTDPARTS_DEFAULT		"mtdparts=omap2-nand.0:" \
						"128k(SPL)," \
						"1m(u-boot)," \
						"384k(u-boot-env1)," \
						"1152k(mtdoops)," \
						"384k(u-boot-env2)," \
						"5m(kernel)," \
						"2m(fdt)," \
						"-(ubi)"

#define CONFIG_NAND_OMAP_GPMC
#define CONFIG_SYS_NAND_ADDR		NAND_BASE	/* physical address */
							/* to access nand */
#define CONFIG_SYS_NAND_BASE		NAND_BASE	/* physical address */
							/* to access nand at */
							/* CS0 */
#define CONFIG_SYS_MAX_NAND_DEVICE	1		/* Max number of NAND */
							/* devices */
#define CONFIG_BCH
#define CONFIG_SYS_NAND_MAX_OOBFREE	2
#define CONFIG_SYS_NAND_MAX_ECCPOS	56

/* commands to include */
#include <config_cmd_default.h>

#define CONFIG_CMD_EXT2			/* EXT2 Support */
#define CONFIG_CMD_FAT			/* FAT support */
#define CONFIG_CMD_I2C			/* I2C serial bus support */
#define CONFIG_CMD_MMC			/* MMC support */
#define CONFIG_CMD_MTDPARTS		/* Enable MTD parts commands */
#define CONFIG_CMD_NAND			/* NAND support */
#define CONFIG_CMD_NAND_LOCK_UNLOCK	/* nand (un)lock commands */
#define CONFIG_CMD_UBI			/* UBI commands */
#define CONFIG_CMD_UBIFS		/* UBIFS commands */
#define CONFIG_LZO			/* LZO is needed for UBIFS */

#undef CONFIG_CMD_NET
#undef CONFIG_CMD_NFS
#undef CONFIG_CMD_FPGA			/* FPGA configuration Support */
#undef CONFIG_CMD_IMI			/* iminfo */
#undef CONFIG_CMD_JFFS2			/* JFFS2 Support */

/* needed for ubi */
#define CONFIG_RBTREE
#define CONFIG_MTD_DEVICE       /* needed for mtdparts commands */
#define CONFIG_MTD_PARTITIONS

/* Environment information (this is the common part) */

#define CONFIG_BOOTDELAY		0

/* hang() the board on panic() */
#define CONFIG_PANIC_HANG

/* environment placement (for NAND), is different for FLASHCARD but does not
 * harm there */
#define CONFIG_ENV_OFFSET		0x120000    /* env start */
#define CONFIG_ENV_OFFSET_REDUND	0x2A0000    /* redundant env start */
#define CONFIG_ENV_SIZE			(16 << 10)  /* use 16KiB for env */
#define CONFIG_ENV_RANGE		(384 << 10) /* allow badblocks in env */

/* the loadaddr is the same as CONFIG_SYS_LOAD_ADDR, unfortunately the defiend
 * value can not be used here! */
#define CONFIG_LOADADDR		0x82000000

#define CONFIG_COMMON_ENV_SETTINGS \
	"console=ttyO2,115200n8\0" \
	"mmcdev=0\0" \
	"vram=3M\0" \
	"defaultdisplay=lcd\0" \
	"kernelopts=mtdoops.mtddev=3\0" \
	"mtdparts=" MTDPARTS_DEFAULT "\0" \
	"mtdids=" MTDIDS_DEFAULT "\0" \
	"commonargs=" \
		"setenv bootargs console=${console} " \
		"${mtdparts} " \
		"${kernelopts} " \
		"vt.global_cursor_default=0 " \
		"vram=${vram} " \
		"omapdss.def_disp=${defaultdisplay}\0"

#define CONFIG_BOOTCOMMAND "run autoboot"

/* specific environment settings for different use cases
 * FLASHCARD: used to run a rdimage from sdcard to program the device
 * 'NORMAL': used to boot kernel from sdcard, nand, ...
 *
 * The main aim for the FLASHCARD skin is to have an embedded environment
 * which will not be influenced by any data already on the device.
 */
#ifdef CONFIG_FLASHCARD

#define CONFIG_ENV_IS_NOWHERE

/* the rdaddr is 16 MiB before the loadaddr */
#define CONFIG_ENV_RDADDR	"rdaddr=0x81000000\0"

#define CONFIG_EXTRA_ENV_SETTINGS \
	CONFIG_COMMON_ENV_SETTINGS \
	CONFIG_ENV_RDADDR \
	"autoboot=" \
	"run commonargs; " \
	"setenv bootargs ${bootargs} " \
		"flashy_updateimg=/dev/mmcblk0p1:corscience_update.img " \
		"rdinit=/sbin/init; " \
	"mmc dev ${mmcdev}; mmc rescan; " \
	"fatload mmc ${mmcdev} ${loadaddr} uImage; " \
	"fatload mmc ${mmcdev} ${rdaddr} uRamdisk; " \
	"bootm ${loadaddr} ${rdaddr}\0"

#else /* CONFIG_FLASHCARD */

#define CONFIG_ENV_OVERWRITE /* allow to overwrite serial and ethaddr */

#define CONFIG_ENV_IS_IN_NAND

#define CONFIG_EXTRA_ENV_SETTINGS \
	CONFIG_COMMON_ENV_SETTINGS \
	"mmcargs=" \
		"run commonargs; " \
		"setenv bootargs ${bootargs} " \
		"root=/dev/mmcblk0p2 " \
		"rootwait " \
		"rw\0" \
	"nandargs=" \
		"run commonargs; " \
		"setenv bootargs ${bootargs} " \
		"root=ubi0:root " \
		"ubi.mtd=7 " \
		"rootfstype=ubifs " \
		"ro\0" \
	"loadbootscript=fatload mmc ${mmcdev} ${loadaddr} boot.scr\0" \
	"bootscript=echo Running bootscript from mmc ...; " \
		"source ${loadaddr}\0" \
	"loaduimage=fatload mmc ${mmcdev} ${loadaddr} uImage\0" \
	"mmcboot=echo Booting from mmc ...; " \
		"run mmcargs; " \
		"bootm ${loadaddr}\0" \
	"loaduimage_ubi=ubi part ubi; " \
		"ubifsmount ubi:root; " \
		"ubifsload ${loadaddr} /boot/uImage\0" \
	"loaduimage_nand=nand read ${loadaddr} kernel 0x500000\0" \
	"nandboot=echo Booting from nand ...; " \
		"run nandargs; " \
		"run loaduimage_nand; " \
		"bootm ${loadaddr}\0" \
	"autoboot=mmc dev ${mmcdev}; if mmc rescan; then " \
			"if run loadbootscript; then " \
				"run bootscript; " \
			"else " \
				"if run loaduimage; then " \
					"run mmcboot; " \
				"else run nandboot; " \
				"fi; " \
			"fi; " \
		"else run nandboot; fi\0"

#endif /* CONFIG_FLASHCARD */

/* Miscellaneous configurable options */
#define CONFIG_SYS_LONGHELP		/* undef to save memory */
#define CONFIG_SYS_HUSH_PARSER		/* use "hush" command parser */
#define CONFIG_CMDLINE_EDITING		/* enable cmdline history */
#define CONFIG_AUTO_COMPLETE
#define CONFIG_SYS_PROMPT		"OMAP3 Tricorder # "
#define CONFIG_SYS_CBSIZE		512	/* Console I/O Buffer Size */
/* Print Buffer Size */
#define CONFIG_SYS_PBSIZE		(CONFIG_SYS_CBSIZE + \
					sizeof(CONFIG_SYS_PROMPT) + 16)
#define CONFIG_SYS_MAXARGS		16	/* max number of command args */

/* Boot Argument Buffer Size */
#define CONFIG_SYS_BARGSIZE		(CONFIG_SYS_CBSIZE)

#define CONFIG_SYS_MEMTEST_START	(OMAP34XX_SDRC_CS0 + 0x00000000)
#define CONFIG_SYS_MEMTEST_END		(CONFIG_SYS_MEMTEST_START + \
					0x07000000) /* 112 MB */

#define CONFIG_SYS_LOAD_ADDR		(OMAP34XX_SDRC_CS0 + 0x02000000)

/*
 * OMAP3 has 12 GP timers, they can be driven by the system clock
 * (12/13/16.8/19.2/38.4MHz) or by 32KHz clock. We use 13MHz (V_SCLK).
 * This rate is divided by a local divisor.
 */
#define CONFIG_SYS_TIMERBASE		(OMAP34XX_GPT2)
#define CONFIG_SYS_PTV			2 /* Divisor: 2^(PTV+1) => 8 */

/*  Physical Memory Map  */
#define CONFIG_NR_DRAM_BANKS		2 /* CS1 may or may not be populated */
#define PHYS_SDRAM_1			OMAP34XX_SDRC_CS0
#define PHYS_SDRAM_2			OMAP34XX_SDRC_CS1

/* NAND and environment organization  */
#define CONFIG_SYS_MONITOR_LEN		(256 << 10)	/* Reserve 2 sectors */

#define CONFIG_SYS_SDRAM_BASE		PHYS_SDRAM_1
#define CONFIG_SYS_INIT_RAM_ADDR	0x4020f800
#define CONFIG_SYS_INIT_RAM_SIZE	0x800
#define CONFIG_SYS_INIT_SP_ADDR	(CONFIG_SYS_INIT_RAM_ADDR + \
						CONFIG_SYS_INIT_RAM_SIZE - \
						GENERATED_GBL_DATA_SIZE)

/* SRAM config */
#define CONFIG_SYS_SRAM_START		0x40200000
#define CONFIG_SYS_SRAM_SIZE		0x10000

/* Defines for SPL */
#define CONFIG_SPL_FRAMEWORK
#define CONFIG_SPL_NAND_SIMPLE

#define CONFIG_SPL_BOARD_INIT
#define CONFIG_SPL_GPIO_SUPPORT
#define CONFIG_SPL_LIBCOMMON_SUPPORT
#define CONFIG_SPL_LIBDISK_SUPPORT
#define CONFIG_SPL_I2C_SUPPORT
#define CONFIG_SPL_LIBGENERIC_SUPPORT
#define CONFIG_SPL_SERIAL_SUPPORT
#define CONFIG_SPL_POWER_SUPPORT
#define CONFIG_SPL_NAND_SUPPORT
#define CONFIG_SPL_NAND_BASE
#define CONFIG_SPL_NAND_DRIVERS
#define CONFIG_SPL_NAND_ECC
#define CONFIG_SPL_MMC_SUPPORT
#define CONFIG_SPL_FAT_SUPPORT
#define CONFIG_SPL_LDSCRIPT		"$(CPUDIR)/omap-common/u-boot-spl.lds"
#define CONFIG_SPL_FS_LOAD_PAYLOAD_NAME        "u-boot.img"
#define CONFIG_SYS_MMCSD_FS_BOOT_PARTITION     1
#define CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_SECTOR 0x300 /* address 0x60000 */

#define CONFIG_SPL_TEXT_BASE		0x40200000 /*CONFIG_SYS_SRAM_START*/
#define CONFIG_SPL_MAX_SIZE		(57 * 1024)	/* 7 KB for stack */
#define CONFIG_SPL_STACK		LOW_LEVEL_SRAM_STACK

#define CONFIG_SPL_BSS_START_ADDR	0x80000000 /*CONFIG_SYS_SDRAM_BASE*/
#define CONFIG_SPL_BSS_MAX_SIZE		0x80000

/* NAND boot config */
#define CONFIG_SYS_NAND_5_ADDR_CYCLE
#define CONFIG_SYS_NAND_PAGE_COUNT	64
#define CONFIG_SYS_NAND_PAGE_SIZE	2048
#define CONFIG_SYS_NAND_OOBSIZE		64
#define CONFIG_SYS_NAND_BLOCK_SIZE	(128*1024)
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

#define CONFIG_SYS_NAND_U_BOOT_START	CONFIG_SYS_TEXT_BASE

#define CONFIG_SYS_NAND_U_BOOT_OFFS	0x20000
#define CONFIG_SYS_NAND_U_BOOT_SIZE	0x100000

#define CONFIG_SYS_SPL_MALLOC_START	0x80208000
#define CONFIG_SYS_SPL_MALLOC_SIZE	0x100000	/* 1 MB */

#define CONFIG_SYS_ALT_MEMTEST
#define CONFIG_SYS_MEMTEST_SCRATCH	0x81000000
#endif /* __CONFIG_H */
