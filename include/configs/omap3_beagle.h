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

#include <configs/ti_omap3_common.h>

/*
 * We are only ever GP parts and will utilize all of the "downloaded image"
 * area in SRAM which starts at 0x40200000 and ends at 0x4020FFFF (64KB).
 */
#undef CONFIG_SPL_TEXT_BASE
#define CONFIG_SPL_TEXT_BASE            0x40200000

#define CONFIG_MISC_INIT_R
#define CONFIG_CMDLINE_TAG
#define CONFIG_SETUP_MEMORY_TAGS
#define CONFIG_INITRD_TAG
#define CONFIG_REVISION_TAG

/* NAND */
#if defined(CONFIG_NAND)
#define CONFIG_SYS_FLASH_BASE		NAND_BASE
#define CONFIG_SYS_MAX_NAND_DEVICE      1
#define CONFIG_SYS_NAND_5_ADDR_CYCLE
#define CONFIG_SYS_NAND_PAGE_COUNT      64
#define CONFIG_SYS_NAND_PAGE_SIZE       2048
#define CONFIG_SYS_NAND_OOBSIZE         64
#define CONFIG_SYS_NAND_BLOCK_SIZE      (128*1024)
#define CONFIG_SYS_NAND_BAD_BLOCK_POS   NAND_LARGE_BADBLOCK_POS
#define CONFIG_SYS_NAND_ECCPOS          {2, 3, 4, 5, 6, 7, 8, 9,\
                                         10, 11, 12, 13}
#define CONFIG_SYS_NAND_ECCSIZE         512
#define CONFIG_SYS_NAND_ECCBYTES        3
#define CONFIG_NAND_OMAP_ECCSCHEME      OMAP_ECC_BCH8_CODE_HW_DETECTION_SW
#define CONFIG_SYS_NAND_U_BOOT_OFFS     0x80000
#define CONFIG_ENV_IS_IN_NAND           1
#define CONFIG_ENV_SIZE                 (128 << 10) /* 128 KiB */
#define CONFIG_SYS_ENV_SECT_SIZE        (128 << 10) /* 128 KiB */
#define CONFIG_ENV_OFFSET               0x260000
#define CONFIG_ENV_ADDR                 0x260000
#define CONFIG_ENV_OVERWRITE
#define CONFIG_MTD_PARTITIONS           /* required for UBI partition support */
/* NAND: SPL falcon mode configs */
#if defined(CONFIG_SPL_OS_BOOT)
#define CONFIG_SYS_NAND_SPL_KERNEL_OFFS 0x280000
#endif /* CONFIG_SPL_OS_BOOT */
#endif /* CONFIG_NAND */

/* MUSB */
#define CONFIG_USB_OMAP3

/* USB EHCI */
#define CONFIG_OMAP_EHCI_PHY1_RESET_GPIO	147

/* Enable Multi Bus support for I2C */
#define CONFIG_I2C_MULTI_BUS

/* DSS Support */
#define CONFIG_VIDEO_OMAP3

/* TWL4030 LED Support */
#define CONFIG_TWL4030_LED

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
				"run loadfdt;" \
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
	DEFAULT_LINUX_BOOT_ENV \
	"fdt_high=0xffffffff\0" \
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
	"mmcrootfstype=ext4 rootwait\0" \
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
	"mmcboot=echo Booting ${bootfile} with DT from mmc${mmcdev} ...; " \
		"run mmcargs; " \
		"bootm ${loadaddr} - ${fdtaddr}\0" \
	"mmcbootz=echo Booting ${bootfile} with DT from mmc${mmcdev} ...; " \
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

#endif /* __CONFIG_H */
