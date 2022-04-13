/* SPDX-License-Identifier: GPL-2.0+ */
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
 */

#ifndef __CONFIG_H
#define __CONFIG_H

/* High Level Configuration Options */

/*
 * 1MB into the SDRAM to allow for SPL's bss at the beginning of SDRAM
 * 64 bytes before this address should be set aside for u-boot.img's
 * header. That is 0x800FFFC0--0x80100000 should not be used for any
 * other needs.
 */

#define CONFIG_SPL_BSS_START_ADDR       0x80000500 /* leave space for bootargs*/
#define CONFIG_SPL_BSS_MAX_SIZE		0x80000

#define CONFIG_SYS_SPL_MALLOC_START	0x80208000
#define CONFIG_SYS_SPL_MALLOC_SIZE	0x100000	/* 1 MB */

/*  Physical Memory Map  */

#include <configs/ti_omap3_common.h>

/* Hardware drivers */
/* DM9000 */
#define	CONFIG_DM9000_BASE		0x2c000000
#define	DM9000_IO			CONFIG_DM9000_BASE
#define	DM9000_DATA			(CONFIG_DM9000_BASE + 0x400)
#define	CONFIG_DM9000_USE_16BIT		1
#define CONFIG_DM9000_NO_SROM		1
#undef	CONFIG_DM9000_DEBUG

/* TWL4030 */

/* BOOTP/DHCP options */

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

/* Boot Argument Buffer Size */

/* Defines for SPL */

/* NAND boot config */
#define CONFIG_SYS_NAND_ECCPOS		{2, 3, 4, 5, 6, 7, 8, 9,\
						10, 11, 12, 13}

#define CONFIG_SYS_NAND_ECCSIZE		512
#define CONFIG_SYS_NAND_ECCBYTES	3

#define CONFIG_SYS_NAND_U_BOOT_SIZE	0x200000

/* SPL OS boot options */
#define CONFIG_SYS_NAND_SPL_KERNEL_OFFS 0x280000

#undef CONFIG_SYS_MMCSD_RAW_MODE_ARGS_SECTOR
#undef CONFIG_SYS_MMCSD_RAW_MODE_ARGS_SECTORS
#define CONFIG_SYS_MMCSD_RAW_MODE_ARGS_SECTOR	0x8   /* address 0x1000 */
#define CONFIG_SYS_MMCSD_RAW_MODE_ARGS_SECTORS	8     /* 4KB */

#undef CONFIG_SYS_SPL_ARGS_ADDR
#define CONFIG_SYS_SPL_ARGS_ADDR        (PHYS_SDRAM_1 + 0x100)

#endif /* __CONFIG_H */
