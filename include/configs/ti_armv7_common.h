/*
 * ti_armv7_common.h
 *
 * Copyright (C) 2013 Texas Instruments Incorporated - http://www.ti.com/
 *
 * SPDX-License-Identifier:	GPL-2.0+
 *
 * The various ARMv7 SoCs from TI all share a number of IP blocks when
 * implementing a given feature.  Rather than define these in every
 * board or even SoC common file, we define a common file to be re-used
 * in all cases.  While technically true that some of these details are
 * configurable at the board design, they are common throughout SoC
 * reference platforms as well as custom designs and become de facto
 * standards.
 */

#ifndef __CONFIG_TI_ARMV7_COMMON_H__
#define __CONFIG_TI_ARMV7_COMMON_H__

/* Common define for many platforms. */
#define CONFIG_OMAP
#define CONFIG_OMAP_COMMON

/*
 * We typically do not contain NOR flash.  In the cases where we do, we
 * undefine this later.
 */
#define CONFIG_SYS_NO_FLASH

/* Support both device trees and ATAGs. */
#define CONFIG_OF_LIBFDT
#define CONFIG_CMDLINE_TAG
#define CONFIG_SETUP_MEMORY_TAGS
#define CONFIG_INITRD_TAG

/*
 * Our DDR memory always starts at 0x80000000 and U-Boot shall have
 * relocated itself to higher in memory by the time this value is used.
 */
#define CONFIG_SYS_LOAD_ADDR		0x80000000

/*
 * Default to a quick boot delay.
 */
#define CONFIG_BOOTDELAY		1

/*
 * DDR information.  We say (for simplicity) that we have 1 bank,
 * always, even when we have more.  We always start at 0x80000000,
 * and we place the initial stack pointer in our SRAM.
 */
#define CONFIG_NR_DRAM_BANKS		1
#define CONFIG_SYS_SDRAM_BASE		0x80000000
#define CONFIG_SYS_INIT_SP_ADDR         (NON_SECURE_SRAM_END - \
						GENERATED_GBL_DATA_SIZE)

/* Timer information. */
#define CONFIG_SYS_PTV			2	/* Divisor: 2^(PTV+1) => 8 */
#define CONFIG_SYS_HZ			1000	/* 1ms clock */

/* I2C IP block */
#define CONFIG_I2C
#define CONFIG_HARD_I2C
#define CONFIG_SYS_I2C_SPEED		100000
#define CONFIG_SYS_I2C_SLAVE		1
#define CONFIG_I2C_MULTI_BUS
#define CONFIG_DRIVER_OMAP24XX_I2C
#define CONFIG_CMD_I2C

/* MMC/SD IP block */
#define CONFIG_MMC
#define CONFIG_GENERIC_MMC
#define CONFIG_OMAP_HSMMC
#define CONFIG_CMD_MMC

/* McSPI IP block */
#define CONFIG_SPI
#define CONFIG_OMAP3_SPI
#define CONFIG_CMD_SPI

/* GPIO block */
#define CONFIG_OMAP_GPIO
#define CONFIG_CMD_GPIO

/*
 * GPMC NAND block.  We support 1 device and the physical address to
 * access CS0 at is 0x8000000.
 */
#ifdef CONFIG_NAND
#define CONFIG_NAND_OMAP_GPMC
#define CONFIG_SYS_NAND_BASE		0x8000000
#define CONFIG_SYS_MAX_NAND_DEVICE	1
#define CONFIG_CMD_NAND
#endif

/*
 * The following are general good-enough settings for U-Boot.  We set a
 * large malloc pool as we generally have a lot of DDR, and we opt for
 * function over binary size in the main portion of U-Boot as this is
 * generally easily constrained later if needed.  We enable the config
 * options that give us information in the environment about what board
 * we are on so we do not need to rely on the command prompt.  We set a
 * console baudrate of 115200 and use the default baud rate table.
 */
#define CONFIG_SYS_MALLOC_LEN		(1024 << 10)
#define CONFIG_SYS_HUSH_PARSER
#define CONFIG_SYS_PROMPT		"U-Boot# "
#define CONFIG_SYS_CONSOLE_INFO_QUIET
#define CONFIG_BAUDRATE			115200
#define CONFIG_ENV_VARS_UBOOT_CONFIG	/* Strongly encouraged */
#define CONFIG_ENV_OVERWRITE		/* Overwrite ethaddr / serial# */

/* As stated above, the following choices are optional. */
#define CONFIG_SYS_LONGHELP
#define CONFIG_AUTO_COMPLETE
#define CONFIG_CMDLINE_EDITING
#define CONFIG_VERSION_VARIABLE

/* We set the max number of command args high to avoid HUSH bugs. */
#define CONFIG_SYS_MAXARGS		64

/* Console I/O Buffer Size */
#define CONFIG_SYS_CBSIZE		512
/* Print Buffer Size */
#define CONFIG_SYS_PBSIZE		(CONFIG_SYS_CBSIZE \
					+ sizeof(CONFIG_SYS_PROMPT) + 16)
/* Boot Argument Buffer Size */
#define CONFIG_SYS_BARGSIZE		CONFIG_SYS_CBSIZE

/*
 * When we have SPI, NOR or NAND flash we expect to be making use of
 * mtdparts, both for ease of use in U-Boot and for passing information
 * on to the Linux kernel.
 */
#if defined(CONFIG_SPI_BOOT) || defined(CONFIG_NOR) || defined(CONFIG_NAND)
#define CONFIG_MTD_DEVICE		/* Required for mtdparts */
#define CONFIG_CMD_MTDPARTS
#endif

/*
 * For commands to use, we take the default list and add a few other
 * useful commands.  Note that we must have set CONFIG_SYS_NO_FLASH
 * prior to this include, in order to skip a few commands.  When we do
 * have flash, if we expect these commands they must be enabled in that
 * config.  If desired, a specific list of desired commands can be used
 * instead.
 */
#include <config_cmd_default.h>
#define CONFIG_CMD_ASKENV
#define CONFIG_CMD_ECHO
#define CONFIG_CMD_BOOTZ

/*
 * Common filesystems support.  When we have removable storage we
 * enabled a number of useful commands and support.
 */
#if defined(CONFIG_MMC) || defined(CONFIG_USB_STORAGE)
#define CONFIG_DOS_PARTITION
#define CONFIG_CMD_FAT
#define CONFIG_FAT_WRITE
#define CONFIG_CMD_EXT2
#define CONFIG_CMD_EXT4
#define CONFIG_CMD_FS_GENERIC
#endif

/*
 * Our platforms make use of SPL to initalize the hardware (primarily
 * memory) enough for full U-Boot to be loaded.  We also support Falcon
 * Mode so that the Linux kernel can be booted directly from SPL
 * instead, if desired.  We make use of the general SPL framework found
 * under common/spl/.  Given our generally common memory map, we set a
 * number of related defaults and sizes here.
 */
#ifndef CONFIG_NOR_BOOT
#define CONFIG_SPL
#define CONFIG_SPL_FRAMEWORK
#define CONFIG_SPL_OS_BOOT

/*
 * Place the image at the start of the ROM defined image space.
 * We limit our size to the ROM-defined downloaded image area, and use the
 * rest of the space for stack.  We load U-Boot itself into memory at
 * 0x80800000 for legacy reasons (to not conflict with older SPLs).  We
 * have our BSS be placed 1MiB after this, to allow for the default
 * Linux kernel address of 0x80008000 to work, in the Falcon Mode case.
 * We have the SPL malloc pool at the end of the BSS area.
 */
#define CONFIG_SPL_STACK		CONFIG_SYS_INIT_SP_ADDR
#define CONFIG_SYS_TEXT_BASE		0x80800000
#define CONFIG_SPL_BSS_START_ADDR	0x80a00000
#define CONFIG_SPL_BSS_MAX_SIZE		0x80000		/* 512 KB */
#define CONFIG_SYS_SPL_MALLOC_START	(CONFIG_SPL_BSS_START_ADDR + \
					 CONFIG_SPL_BSS_MAX_SIZE)
#define CONFIG_SYS_SPL_MALLOC_SIZE	CONFIG_SYS_MALLOC_LEN

/* RAW SD card / eMMC locations. */
#define CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_SECTOR	0x300 /* address 0x60000 */
#define CONFIG_SYS_U_BOOT_MAX_SIZE_SECTORS	0x200 /* 256 KB */

/* FAT sd card locations. */
#define CONFIG_SYS_MMC_SD_FAT_BOOT_PARTITION	1
#define CONFIG_SPL_FAT_LOAD_PAYLOAD_NAME	"u-boot.img"

#ifdef CONFIG_SPL_OS_BOOT
#define CONFIG_SYS_SPL_ARGS_ADDR		(CONFIG_SYS_SDRAM_BASE + 0x100)

/* FAT */
#define CONFIG_SPL_FAT_LOAD_KERNEL_NAME		"uImage"
#define CONFIG_SPL_FAT_LOAD_ARGS_NAME		"args"

/* RAW SD card / eMMC */
#define CONFIG_SYS_MMCSD_RAW_MODE_KERNEL_SECTOR	0x900	/* address 0x120000 */
#define CONFIG_SYS_MMCSD_RAW_MODE_ARGS_SECTOR	0x80	/* address 0x10000 */
#define CONFIG_SYS_MMCSD_RAW_MODE_ARGS_SECTORS	0x80	/* 64KiB */

/* NAND */
#ifdef CONFIG_NAND
#define CONFIG_CMD_SPL_NAND_OFS			0x240000 /* end of u-boot */
#define CONFIG_SYS_NAND_SPL_KERNEL_OFFS		0x280000
#define CONFIG_CMD_SPL_WRITE_SIZE		0x2000
#endif

/* spl export command */
#define CONFIG_CMD_SPL
#endif

#ifdef CONFIG_MMC
#define CONFIG_SPL_LIBDISK_SUPPORT
#define CONFIG_SPL_MMC_SUPPORT
#define CONFIG_SPL_FAT_SUPPORT
#endif

/* General parts of the framework, required. */
#define CONFIG_SPL_I2C_SUPPORT
#define CONFIG_SPL_LIBCOMMON_SUPPORT
#define CONFIG_SPL_LIBGENERIC_SUPPORT
#define CONFIG_SPL_SERIAL_SUPPORT
#define CONFIG_SPL_GPIO_SUPPORT
#define CONFIG_SPL_BOARD_INIT

#ifdef CONFIG_NAND
#define CONFIG_SPL_NAND_AM33XX_BCH	/* OMAP4 and later ELM support */
#define CONFIG_SPL_NAND_SUPPORT
#define CONFIG_SPL_NAND_BASE
#define CONFIG_SPL_NAND_DRIVERS
#define CONFIG_SPL_NAND_ECC
#define CONFIG_SYS_NAND_U_BOOT_START	CONFIG_SYS_TEXT_BASE
#define CONFIG_SYS_NAND_U_BOOT_OFFS	0x80000
#endif
#endif /* !CONFIG_NOR_BOOT */

#endif	/* __CONFIG_TI_ARMV7_COMMON_H__ */
