/*
 * (C) Copyright 2006-2008
 * Texas Instruments.
 * Richard Woodruff <r-woodruff2@ti.com>
 * Syed Mohammed Khasim <x0khasim@ti.com>
 *
 * Configuration settings for the TI OMAP3530 Beagle board.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __CONFIG_H
#define __CONFIG_H

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

#define CONFIG_MISC_INIT_R

#define CONFIG_REVISION_TAG		1
#define CONFIG_ENV_OVERWRITE

/* Status LED */

/* Enable Multi Bus support for I2C */
#define CONFIG_I2C_MULTI_BUS		1

/* Probe all devices */
#define CONFIG_SYS_I2C_NOPROBES		{{0x0, 0x0}}

/* USB */
#define CONFIG_USB_MUSB_OMAP2PLUS
#define CONFIG_USB_MUSB_PIO_ONLY
#define CONFIG_TWL4030_USB		1

/* USB EHCI */

#define CONFIG_OMAP_EHCI_PHY1_RESET_GPIO	147

/* commands to include */

#define CONFIG_VIDEO_OMAP3	/* DSS Support			*/

/*
 * TWL4030
 */
#define CONFIG_TWL4030_LED		1

/*
 * Board NAND Info.
 */
#define CONFIG_SYS_MAX_NAND_DEVICE	1		/* Max number of NAND */
							/* devices */

#define BOOT_TARGET_DEVICES(func) \
	func(MMC, mmc, 0)

#define CONFIG_BOOTCOMMAND \
	"run findfdt; " \
	"run distro_bootcmd; " \
	"mmc dev ${mmcdev}; if mmc rescan; then " \
		"if run userbutton; then " \
			"setenv bootenv uEnv.txt;" \
		"else " \
			"setenv bootenv user.txt;" \
		"fi;" \
		"echo SD/MMC found on device ${mmcdev};" \
		"if run loadbootenv; then " \
			"echo Loaded environment from ${bootenv};" \
			"run importbootenv;" \
		"fi;" \
		"if test -n $uenvcmd; then " \
			"echo Running uenvcmd ...;" \
			"run uenvcmd;" \
		"fi;" \
		"if run loadbootscript; then " \
			"run bootscript; " \
		"else " \
			"if run loadimage; then " \
				"run mmcboot;" \
			"fi;" \
		"fi; " \
	"fi;" \
	"run nandboot;" \
	"setenv bootfile zImage;" \
	"if run loadimage; then " \
		"run loadfdt;" \
		"run mmcbootz; " \
	"fi; " \

#include <config_distro_bootcmd.h>

#define CONFIG_EXTRA_ENV_SETTINGS \
	"loadaddr=0x80200000\0" \
	"kernel_addr_r=0x80200000\0" \
	"rdaddr=0x81000000\0" \
	"initrd_addr_r=0x81000000\0" \
	"fdt_high=0xffffffff\0" \
	"fdtaddr=0x80f80000\0" \
	"fdt_addr_r=0x80f80000\0" \
	"usbtty=cdc_acm\0" \
	"bootfile=uImage\0" \
	"ramdisk=ramdisk.gz\0" \
	"bootdir=/boot\0" \
	"bootpart=0:2\0" \
	"console=ttyO2,115200n8\0" \
	"mpurate=auto\0" \
	"buddy=none\0" \
	"optargs=\0" \
	"camera=none\0" \
	"vram=12M\0" \
	"dvimode=640x480MR-16@60\0" \
	"defaultdisplay=dvi\0" \
	"mmcdev=0\0" \
	"mmcroot=/dev/mmcblk0p2 rw\0" \
	"mmcrootfstype=ext3 rootwait\0" \
	"nandroot=ubi0:rootfs ubi.mtd=4\0" \
	"nandrootfstype=ubifs\0" \
	"ramroot=/dev/ram0 rw ramdisk_size=65536 initrd=0x81000000,64M\0" \
	"ramrootfstype=ext2\0" \
	"mmcargs=setenv bootargs console=${console} " \
		"${optargs} " \
		"mpurate=${mpurate} " \
		"buddy=${buddy} "\
		"camera=${camera} "\
		"vram=${vram} " \
		"omapfb.mode=dvi:${dvimode} " \
		"omapdss.def_disp=${defaultdisplay} " \
		"root=${mmcroot} " \
		"rootfstype=${mmcrootfstype}\0" \
	"nandargs=setenv bootargs console=${console} " \
		"${optargs} " \
		"mpurate=${mpurate} " \
		"buddy=${buddy} "\
		"camera=${camera} "\
		"vram=${vram} " \
		"omapfb.mode=dvi:${dvimode} " \
		"omapdss.def_disp=${defaultdisplay} " \
		"root=${nandroot} " \
		"rootfstype=${nandrootfstype}\0" \
	"findfdt=" \
		"if test $beaglerev = AxBx; then " \
			"setenv fdtfile omap3-beagle.dtb; fi; " \
		"if test $beaglerev = Cx; then " \
			"setenv fdtfile omap3-beagle.dtb; fi; " \
		"if test $beaglerev = C4; then " \
			"setenv fdtfile omap3-beagle.dtb; fi; " \
		"if test $beaglerev = xMAB; then " \
			"setenv fdtfile omap3-beagle-xm-ab.dtb; fi; " \
		"if test $beaglerev = xMC; then " \
			"setenv fdtfile omap3-beagle-xm.dtb; fi; " \
		"if test $fdtfile = undefined; then " \
			"echo WARNING: Could not determine device tree to use; fi; \0" \
	"validatefdt=" \
		"if test $beaglerev = xMAB; then " \
			"if test ! -e mmc ${bootpart} ${bootdir}/${fdtfile}; then " \
				"setenv fdtfile omap3-beagle-xm.dtb; " \
			"fi; " \
		"fi; \0" \
	"bootenv=uEnv.txt\0" \
	"loadbootenv=fatload mmc ${mmcdev} ${loadaddr} ${bootenv}\0" \
	"importbootenv=echo Importing environment from mmc ...; " \
		"env import -t -r $loadaddr $filesize\0" \
	"ramargs=setenv bootargs console=${console} " \
		"${optargs} " \
		"mpurate=${mpurate} " \
		"buddy=${buddy} "\
		"vram=${vram} " \
		"omapfb.mode=dvi:${dvimode} " \
		"omapdss.def_disp=${defaultdisplay} " \
		"root=${ramroot} " \
		"rootfstype=${ramrootfstype}\0" \
	"loadramdisk=load mmc ${bootpart} ${rdaddr} ${bootdir}/${ramdisk}\0" \
	"loadimage=load mmc ${bootpart} ${loadaddr} ${bootdir}/${bootfile}\0" \
	"loadbootscript=load mmc ${mmcdev} ${loadaddr} boot.scr\0" \
	"bootscript=echo Running bootscript from mmc${mmcdev} ...; " \
		"source ${loadaddr}\0" \
	"loadfdt=run validatefdt; load mmc ${bootpart} ${fdtaddr} ${bootdir}/${fdtfile}\0" \
	"mmcboot=echo Booting from mmc ...; " \
		"run mmcargs; " \
		"bootm ${loadaddr}\0" \
	"mmcbootz=echo Booting with DT from mmc${mmcdev} ...; " \
		"run mmcargs; " \
		"bootz ${loadaddr} - ${fdtaddr}\0" \
	"nandboot=echo Booting from nand ...; " \
		"run nandargs; " \
		"nand read ${loadaddr} 280000 400000; " \
		"bootm ${loadaddr}\0" \
	"ramboot=echo Booting from ramdisk ...; " \
		"run ramargs; " \
		"bootm ${loadaddr}\0" \
	"userbutton=if gpio input 173; then run userbutton_xm; " \
		"else run userbutton_nonxm; fi;\0" \
	"userbutton_xm=gpio input 4;\0" \
	"userbutton_nonxm=gpio input 7;\0" \
	BOOTENV

/*
 * OMAP3 has 12 GP timers, they can be driven by the system clock
 * (12/13/16.8/19.2/38.4MHz) or by 32KHz clock. We use 13MHz (V_SCLK).
 * This rate is divided by a local divisor.
 */
#define CONFIG_SYS_PTV			2       /* Divisor: 2^(PTV+1) => 8 */

/*-----------------------------------------------------------------------
 * FLASH and environment organization
 */

/* **** PISMO SUPPORT *** */
#if defined(CONFIG_CMD_NAND)
#define CONFIG_SYS_FLASH_BASE		NAND_BASE
#endif

/* Monitor at start of flash */
#define CONFIG_SYS_MONITOR_BASE		CONFIG_SYS_FLASH_BASE
#define CONFIG_SYS_ONENAND_BASE		ONENAND_MAP

#define CONFIG_ENV_SIZE			(128 << 10)	/* 128 KiB */
#define ONENAND_ENV_OFFSET		0x260000 /* environment starts here */

#define CONFIG_SYS_ENV_SECT_SIZE	(128 << 10)	/* 128 KiB */
#define CONFIG_ENV_OFFSET		0x260000
#define CONFIG_ENV_ADDR			0x260000

/* Defines for SPL */

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
#define CONFIG_NAND_OMAP_ECCSCHEME	OMAP_ECC_HAM1_CODE_HW
#define CONFIG_SYS_NAND_U_BOOT_OFFS	0x80000
/* NAND: SPL falcon mode configs */
#ifdef CONFIG_SPL_OS_BOOT
#define CONFIG_SYS_NAND_SPL_KERNEL_OFFS	0x280000
#endif

#endif /* __CONFIG_H */
