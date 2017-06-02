/*
 * (C) Copyright 2006-2008
 * Texas Instruments.
 * Richard Woodruff <r-woodruff2@ti.com>
 * Syed Mohammed Khasim <x0khasim@ti.com>
 *
 * (C) Copyright 2009
 * Frederik Kriewitz <frederik@kriewitz.eu>
 *
 * Configuration settings for the DevKit8000 board.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __CONFIG_H
#define __CONFIG_H

/* High Level Configuration Options */
#define CONFIG_MACH_TYPE	MACH_TYPE_DEVKIT8000

/*
 * 1MB into the SDRAM to allow for SPL's bss at the beginning of SDRAM
 * 64 bytes before this address should be set aside for u-boot.img's
 * header. That is 0x800FFFC0--0x80100000 should not be used for any
 * other needs.
 */
#define CONFIG_SYS_TEXT_BASE	0x80100000

#define CONFIG_SPL_BSS_START_ADDR       0x80000500 /* leave space for bootargs*/
#define CONFIG_SPL_BSS_MAX_SIZE		0x80000

#define CONFIG_SYS_SPL_MALLOC_START	0x80208000
#define CONFIG_SYS_SPL_MALLOC_SIZE	0x100000	/* 1 MB */

#define CONFIG_NAND

/*  Physical Memory Map  */
#define CONFIG_NR_DRAM_BANKS		2 /* CS1 may or may not be populated */

#include <configs/ti_omap3_common.h>

#define CONFIG_MISC_INIT_R

#define CONFIG_REVISION_TAG		1

/* Size of malloc() pool */
#define CONFIG_ENV_SIZE			(128 << 10)	/* 128 KiB */
						/* Sector */
#undef CONFIG_SYS_MALLOC_LEN
#define CONFIG_SYS_MALLOC_LEN		(CONFIG_ENV_SIZE + (128 << 10))

/* Hardware drivers */
/* DM9000 */
#define CONFIG_NET_RETRY_COUNT		20
#define	CONFIG_DRIVER_DM9000		1
#define	CONFIG_DM9000_BASE		0x2c000000
#define	DM9000_IO			CONFIG_DM9000_BASE
#define	DM9000_DATA			(CONFIG_DM9000_BASE + 0x400)
#define	CONFIG_DM9000_USE_16BIT		1
#define CONFIG_DM9000_NO_SROM		1
#undef	CONFIG_DM9000_DEBUG

/* SPI */
#undef CONFIG_SPI
#undef CONFIG_OMAP3_SPI

/* I2C */
#undef CONFIG_SYS_I2C_OMAP24XX
#define CONFIG_SYS_I2C_OMAP34XX

/* TWL4030 */
#define CONFIG_TWL4030_LED		1

/* Board NAND Info */
#define MTDIDS_DEFAULT			"nand0=nand"
#define MTDPARTS_DEFAULT		"mtdparts=nand:" \
						"512k(x-loader)," \
						"1920k(u-boot)," \
						"128k(u-boot-env)," \
						"4m(kernel)," \
						"-(fs)"

#define CONFIG_SYS_NAND_ADDR		NAND_BASE	/* physical address */
							/* to access nand */
#define CONFIG_JFFS2_NAND
/* nand device jffs2 lives on */
#define CONFIG_JFFS2_DEV		"nand0"
/* start of jffs2 partition */
#define CONFIG_JFFS2_PART_OFFSET	0x680000
#define CONFIG_JFFS2_PART_SIZE		0xf980000	/* size of jffs2 */
							/* partition */

/* commands to include */
#define CONFIG_CMD_NAND_LOCK_UNLOCK	/* nand (un)lock commands	*/

#undef CONFIG_SUPPORT_RAW_INITRD

/* BOOTP/DHCP options */
#define CONFIG_BOOTP_SUBNETMASK
#define CONFIG_BOOTP_GATEWAY
#define CONFIG_BOOTP_HOSTNAME
#define CONFIG_BOOTP_NISDOMAIN
#define CONFIG_BOOTP_BOOTPATH
#define CONFIG_BOOTP_BOOTFILESIZE
#define CONFIG_BOOTP_DNS
#define CONFIG_BOOTP_DNS2
#define CONFIG_BOOTP_SEND_HOSTNAME
#define CONFIG_BOOTP_NTPSERVER
#define CONFIG_BOOTP_TIMEOFFSET
#undef CONFIG_BOOTP_VENDOREX

/* Environment information */
#define CONFIG_EXTRA_ENV_SETTINGS \
	"loadaddr=0x82000000\0" \
	"console=ttyO2,115200n8\0" \
	"mmcdev=0\0" \
	"vram=12M\0" \
	"dvimode=1024x768MR-16@60\0" \
	"defaultdisplay=dvi\0" \
	"nfsopts=hard,tcp,rsize=65536,wsize=65536\0" \
	"kernelopts=rw\0" \
	"commonargs=" \
		"setenv bootargs console=${console} " \
		"vram=${vram} " \
		"omapfb.mode=dvi:${dvimode} " \
		"omapdss.def_disp=${defaultdisplay}\0" \
	"mmcargs=" \
		"run commonargs; " \
		"setenv bootargs ${bootargs} " \
		"root=/dev/mmcblk0p2 " \
		"rootwait " \
		"${kernelopts}\0" \
	"nandargs=" \
		"run commonargs; " \
		"setenv bootargs ${bootargs} " \
		"omapfb.mode=dvi:${dvimode} " \
		"omapdss.def_disp=${defaultdisplay} " \
		"root=/dev/mtdblock4 " \
		"rootfstype=jffs2 " \
		"${kernelopts}\0" \
	"netargs=" \
		"run commonargs; " \
		"setenv bootargs ${bootargs} " \
		"root=/dev/nfs " \
		"nfsroot=${serverip}:${rootpath},${nfsopts} " \
		"ip=${ipaddr}:${serverip}:${gatewayip}:${netmask}:${hostname}::off " \
		"${kernelopts} " \
		"dnsip1=${dnsip} " \
		"dnsip2=${dnsip2}\0" \
	"loadbootscript=fatload mmc ${mmcdev} ${loadaddr} boot.scr\0" \
	"bootscript=echo Running bootscript from mmc ...; " \
		"source ${loadaddr}\0" \
	"loaduimage=fatload mmc ${mmcdev} ${loadaddr} uImage\0" \
	"eraseenv=nand unlock 0x260000 0x20000; nand erase 0x260000 0x20000\0" \
	"mmcboot=echo Booting from mmc ...; " \
		"run mmcargs; " \
		"bootm ${loadaddr}\0" \
	"nandboot=echo Booting from nand ...; " \
		"run nandargs; " \
		"nand read ${loadaddr} 280000 400000; " \
		"bootm ${loadaddr}\0" \
	"netboot=echo Booting from network ...; " \
		"dhcp ${loadaddr}; " \
		"run netargs; " \
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

#define CONFIG_BOOTCOMMAND "run autoboot"

/* Boot Argument Buffer Size */
#define CONFIG_SYS_MEMTEST_START	(OMAP34XX_SDRC_CS0 + 0x07000000)
#define CONFIG_SYS_MEMTEST_END		(CONFIG_SYS_MEMTEST_START + \
					0x01000000) /* 16MB */

/* NAND and environment organization  */
#define CONFIG_ENV_IS_IN_NAND		1
#define SMNAND_ENV_OFFSET		0x260000 /* environment starts here */

#define CONFIG_ENV_OFFSET		SMNAND_ENV_OFFSET

/* SRAM config */
#define CONFIG_SYS_SRAM_START              0x40200000
#define CONFIG_SYS_SRAM_SIZE               0x10000

/* Defines for SPL */

#undef CONFIG_SPL_TEXT_BASE
#define CONFIG_SPL_TEXT_BASE		0x40200000 /*CONFIG_SYS_SRAM_START*/

/* NAND boot config */
#define CONFIG_SYS_NAND_BUSWIDTH_16BIT
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

#define CONFIG_SYS_NAND_U_BOOT_OFFS	0x80000
#define CONFIG_SYS_NAND_U_BOOT_SIZE	0x200000

/* SPL OS boot options */
#define CONFIG_CMD_SPL_WRITE_SIZE       0x400 /* 1024 byte */
#define CONFIG_CMD_SPL_NAND_OFS (CONFIG_SYS_NAND_SPL_KERNEL_OFFS+\
					0x400000)
#define CONFIG_SYS_NAND_SPL_KERNEL_OFFS 0x280000

#undef CONFIG_SYS_MMCSD_RAW_MODE_KERNEL_SECTOR
#undef CONFIG_SYS_MMCSD_RAW_MODE_ARGS_SECTOR
#undef CONFIG_SYS_MMCSD_RAW_MODE_ARGS_SECTORS
#define CONFIG_SYS_MMCSD_RAW_MODE_KERNEL_SECTOR	0x500 /* address 0xa0000 */
#define CONFIG_SYS_MMCSD_RAW_MODE_ARGS_SECTOR	0x8   /* address 0x1000 */
#define CONFIG_SYS_MMCSD_RAW_MODE_ARGS_SECTORS	8     /* 4KB */

#undef CONFIG_SYS_SPL_ARGS_ADDR
#define CONFIG_SYS_SPL_ARGS_ADDR        (PHYS_SDRAM_1 + 0x100)

#endif /* __CONFIG_H */
