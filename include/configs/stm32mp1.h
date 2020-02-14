/* SPDX-License-Identifier: GPL-2.0+ OR BSD-3-Clause */
/*
 * Copyright (C) 2018, STMicroelectronics - All Rights Reserved
 *
 * Configuration settings for the STM32MP15x CPU
 */

#ifndef __CONFIG_H
#define __CONFIG_H
#include <linux/sizes.h>
#include <asm/arch/stm32.h>

#ifndef CONFIG_STM32MP1_TRUSTED
/* PSCI support */
#define CONFIG_ARMV7_PSCI_1_0
#define CONFIG_ARMV7_SECURE_BASE		STM32_SYSRAM_BASE
#define CONFIG_ARMV7_SECURE_MAX_SIZE		STM32_SYSRAM_SIZE
#endif

/*
 * Configuration of the external SRAM memory used by U-Boot
 */
#define CONFIG_SYS_SDRAM_BASE			STM32_DDR_BASE
#define CONFIG_SYS_INIT_SP_ADDR			CONFIG_SYS_TEXT_BASE

#ifdef CONFIG_STM32MP1_OPTEE
#define CONFIG_SYS_MEM_TOP_HIDE			SZ_32M
#endif /* CONFIG_STM32MP1_OPTEE */

/*
 * Console I/O buffer size
 */
#define CONFIG_SYS_CBSIZE			SZ_1K

/*
 * default load address used for command tftp,  bootm , loadb, ...
 */
#define CONFIG_LOADADDR			0xc2000000
#define CONFIG_SYS_LOAD_ADDR		CONFIG_LOADADDR

/* ATAGs */
#define CONFIG_CMDLINE_TAG
#define CONFIG_SETUP_MEMORY_TAGS
#define CONFIG_INITRD_TAG

/*
 * For booting Linux, use the first 256 MB of memory, since this is
 * the maximum mapped by the Linux kernel during initialization.
 */
#define CONFIG_SYS_BOOTMAPSZ		SZ_256M

/* Extend size of kernel image for uncompression */
#define CONFIG_SYS_BOOTM_LEN		SZ_32M

/* SPL support */
#ifdef CONFIG_SPL
/* SPL use DDR */
#define CONFIG_SPL_BSS_START_ADDR	0xC0200000
#define CONFIG_SPL_BSS_MAX_SIZE		0x00100000
#define CONFIG_SYS_SPL_MALLOC_START	0xC0300000
#define CONFIG_SYS_SPL_MALLOC_SIZE	0x00100000

/* limit SYSRAM usage to first 128 KB */
#define CONFIG_SPL_MAX_SIZE		0x00020000
#define CONFIG_SPL_STACK		(STM32_SYSRAM_BASE + \
					 STM32_SYSRAM_SIZE)
#endif /* #ifdef CONFIG_SPL */

#define CONFIG_SYS_MEMTEST_START	STM32_DDR_BASE
#define CONFIG_SYS_MEMTEST_END		(CONFIG_SYS_MEMTEST_START + SZ_64M)
#define CONFIG_SYS_MEMTEST_SCRATCH	(CONFIG_SYS_MEMTEST_END + 4)

/*MMC SD*/
#define CONFIG_SYS_MMC_MAX_DEVICE	3

/* NAND support */
#define CONFIG_SYS_NAND_ONFI_DETECTION
#define CONFIG_SYS_MAX_NAND_DEVICE	1

/* Ethernet need */
#ifdef CONFIG_DWC_ETH_QOS
#define CONFIG_SYS_NONCACHED_MEMORY	(1 * SZ_1M)	/* 1M */
#define CONFIG_SERVERIP                 192.168.1.1
#define CONFIG_BOOTP_SERVERIP
#define CONFIG_SYS_AUTOLOAD		"no"
#endif

/* Dynamic MTD partition support */
#if defined(CONFIG_STM32_QSPI) || defined(CONFIG_NAND_STM32_FMC2)
#define CONFIG_SYS_MTDPARTS_RUNTIME
#endif

#define CONFIG_SET_DFU_ALT_INFO

#ifdef CONFIG_DM_VIDEO
#define CONFIG_VIDEO_BMP_RLE8
#define CONFIG_BMP_16BPP
#define CONFIG_BMP_24BPP
#define CONFIG_BMP_32BPP
#endif

/*****************************************************************************/
#ifdef CONFIG_DISTRO_DEFAULTS
/*****************************************************************************/

#if !defined(CONFIG_SPL_BUILD)

#ifdef CONFIG_CMD_MMC
#define BOOT_TARGET_MMC0(func)	func(MMC, mmc, 0)
#define BOOT_TARGET_MMC1(func)	func(MMC, mmc, 1)
#define BOOT_TARGET_MMC2(func)	func(MMC, mmc, 2)
#else
#define BOOT_TARGET_MMC0(func)
#define BOOT_TARGET_MMC1(func)
#define BOOT_TARGET_MMC2(func)
#endif

#ifdef CONFIG_NET
#define BOOT_TARGET_PXE(func)	func(PXE, pxe, na)
#else
#define BOOT_TARGET_PXE(func)
#endif

#ifdef CONFIG_CMD_UBIFS
#define BOOT_TARGET_UBIFS(func)	func(UBIFS, ubifs, 0)
#else
#define BOOT_TARGET_UBIFS(func)
#endif

#define BOOT_TARGET_DEVICES(func)	\
	BOOT_TARGET_MMC1(func)		\
	BOOT_TARGET_UBIFS(func)		\
	BOOT_TARGET_MMC0(func)		\
	BOOT_TARGET_MMC2(func)		\
	BOOT_TARGET_PXE(func)

/*
 * bootcmd for stm32mp1:
 * for serial/usb: execute the stm32prog command
 * for mmc boot (eMMC, SD card), boot only on the same device
 * for nand boot, boot with on ubifs partition on nand
 * for nor boot, use the default order
 */
#define STM32MP_BOOTCMD "bootcmd_stm32mp=" \
	"echo \"Boot over ${boot_device}${boot_instance}!\";" \
	"if test ${boot_device} = serial || test ${boot_device} = usb;" \
	"then stm32prog ${boot_device} ${boot_instance}; " \
	"else " \
		"run env_check;" \
		"if test ${boot_device} = mmc;" \
		"then env set boot_targets \"mmc${boot_instance}\"; fi;" \
		"if test ${boot_device} = nand;" \
		"then env set boot_targets ubifs0; fi;" \
		"run distro_bootcmd;" \
	"fi;\0"

#include <config_distro_bootcmd.h>

#ifdef CONFIG_STM32MP1_OPTEE
/* with OPTEE: define specific MTD partitions = teeh, teed, teex */
#define STM32MP_MTDPARTS \
	"mtdparts_nor0=256k(fsbl1),256k(fsbl2),2m(ssbl),256k(u-boot-env),256k(teeh),256k(teed),256k(teex),-(nor_user)\0" \
	"mtdparts_nand0=2m(fsbl),2m(ssbl1),2m(ssbl2),512k(teeh),512k(teed),512k(teex),-(UBI)\0" \
	"mtdparts_spi-nand0=2m(fsbl),2m(ssbl1),2m(ssbl2),"\
		"512k(teeh),512k(teed),512k(teex),-(UBI)\0"

#else /* CONFIG_STM32MP1_OPTEE */
#define STM32MP_MTDPARTS \
	"mtdparts_nor0=256k(fsbl1),256k(fsbl2),2m(ssbl),256k(u-boot-env),-(nor_user)\0" \
	"mtdparts_nand0=2m(fsbl),2m(ssbl1),2m(ssbl2),-(UBI)\0" \
	"mtdparts_spi-nand0=2m(fsbl),2m(ssbl1),2m(ssbl2),-(UBI)\0"

#endif /* CONFIG_STM32MP1_OPTEE */

#ifndef CONFIG_SYS_MTDPARTS_RUNTIME
#undef STM32MP_MTDPARTS
#define STM32MP_MTDPARTS
#endif

#define STM32MP_DFU_ALT_RAM \
	"dfu_alt_info_ram=ram 0=" \
		"uImage ram ${kernel_addr_r} 0x2000000;" \
		"devicetree.dtb ram ${fdt_addr_r} 0x100000;" \
		"uramdisk.image.gz ram ${ramdisk_addr_r} 0x10000000\0"

#ifdef CONFIG_SET_DFU_ALT_INFO
#define STM32MP_DFU_ALT_INFO \
	"dfu_alt_info_nor0=mtd nor0=" \
		"nor_fsbl1 part 1;nor_fsbl2 part 2;" \
		"nor_ssbl part 3;nor_env part 4\0" \
	"dfu_alt_info_nand0=mtd nand0="\
		"nand_fsbl part 1;nand_ssbl1 part 2;" \
		"nand_ssbl2 part 3;nand_UBI partubi 4\0" \
	"dfu_alt_info_spi-nand0=mtd spi-nand0="\
		"spi-nand_fsbl part 1;spi-nand_ssbl1 part 2;" \
		"spi-nand_ssbl2 part 3;spi-nand_UBI partubi 4\0" \
	"dfu_alt_info_mmc0=mmc 0=" \
		"sdcard_fsbl1 part 0 1;sdcard_fsbl2 part 0 2;" \
		"sdcard_ssbl part 0 3;sdcard_bootfs part 0 4;" \
		"sdcard_vendorfs part 0 5;sdcard_rootfs part 0 6;" \
		"sdcard_userfs part 0 7\0" \
	"dfu_alt_info_mmc1=mmc 1=" \
		"emmc_fsbl1 raw 0x0 0x200 mmcpart 1;" \
		"emmc_fsbl2 raw 0x0 0x200 mmcpart 2;emmc_ssbl part 1 1;" \
		"emmc_bootfs part 1 2;emmc_vendorfs part 1 3;" \
		"emmc_rootfs part 1 4;emmc_userfs part 1 5\0"
#else
#define STM32MP_DFU_ALT_INFO
#endif

/*
 * memory layout for 32M uncompressed/compressed kernel,
 * 1M fdt, 1M script, 1M pxe and 1M for splashimage
 * and the ramdisk at the end.
 */
#define CONFIG_EXTRA_ENV_SETTINGS \
	"bootdelay=1\0" \
	"kernel_addr_r=0xc2000000\0" \
	"fdt_addr_r=0xc4000000\0" \
	"scriptaddr=0xc4100000\0" \
	"pxefile_addr_r=0xc4200000\0" \
	"splashimage=0xc4300000\0"  \
	"ramdisk_addr_r=0xc4400000\0" \
	"altbootcmd=run bootcmd\0" \
	"env_default=1\0" \
	"env_check=if test $env_default -eq 1;"\
		" then env set env_default 0;env save;fi\0" \
	STM32MP_BOOTCMD \
	STM32MP_MTDPARTS \
	STM32MP_DFU_ALT_RAM \
	STM32MP_DFU_ALT_INFO \
	BOOTENV \
	"boot_net_usb_start=true\0"

#endif /* ifndef CONFIG_SPL_BUILD */
#endif /* ifdef CONFIG_DISTRO_DEFAULTS*/

#endif /* __CONFIG_H */
