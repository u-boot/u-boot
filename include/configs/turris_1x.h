/* SPDX-License-Identifier: GPL-2.0+ */
/* (C) 2022 Pali Rohár <pali@kernel.org> */

#ifndef _CONFIG_TURRIS_1X_H
#define _CONFIG_TURRIS_1X_H

#include <linux/sizes.h>

/*
 * Turris 1.x memory map:
 *
 * 0x0000_0000 - 0x7fff_ffff    2 GB  DDR                 cacheable
 * 0x8000_0000 - 0xbfff_ffff    1 GB  PCIe MEM (bus 1-2)  non-cacheable
 * 0xc000_0000 - 0xc01f_ffff    2 MB  PCIe MEM (bus 3)    non-cacheable
 * 0xc020_0000 - 0xeeff_ffff  750 MB  unused
 * 0xef00_0000 - 0xefff_ffff   16 MB  NOR (CS0)           non-cacheable
 * 0xf000_0000 - 0xf8f7_ffff  143 MB  unused
 * 0xf8f8_0000 - 0xf8ff_ffff  512 kB  L2 SRAM             cacheable (early boot, SD card only)
 * 0xf900_0000 - 0xff6f_ffff  103 MB  unused
 * 0xff70_0000 - 0xff7f_ffff    1 MB  CCSR                non-cacheable (SPL only)
 * 0xff80_0000 - 0xff83_ffff  256 kB  NAND (CS1)          non-cacheable
 * 0xffa0_0000 - 0xffa1_ffff  128 kB  CPLD (CS3)          non-cacheable
 * 0xffc0_0000 - 0xffc2_ffff  192 kB  PCIe IO             non-cacheable
 * 0xffd0_0000 - 0xffd0_3fff   16 kB  L1 stack            cacheable (early boot)
 * 0xffe0_0000 - 0xffef_ffff    1 MB  CCSR                non-cacheable (not in SPL)
 * 0xffff_f000 - 0xffff_ffff    4 kB  Boot page           non-cacheable
 */

/*
 * Global settings
 */

/*
 * CONFIG_ENABLE_36BIT_PHYS needs to be always defined when processor supports
 * 36-bit addressing (which is case for P2020), also when only 32-bit addressing
 * mode is used. Name of this config option is misleading and should have been
 * called SUPPORT instead of ENABLE.
 * When CONFIG_PHYS_64BIT is set then 36-bit addressing is used, when unset then
 * 32-bit addressing is used. Name of this config option is misleading too and
 * should have been called 36BIT and ENABLED, not 64BIT.
 * Due to performance reasons (see document AN4064), Turris 1.x boards use only
 * 32-bit addressing. Also all config options are currently defined only for
 * 32-bit addressing, so compiling U-Boot for 36-bit addressing is not possible
 * yet.
 */
#ifdef CONFIG_PHYS_64BIT
#error "36-bit addressing is not implemented for this board"
#endif

/*
 * Boot settings
 */

/*
 * Booting from SD card
 * BootROM configures L2 cache as SRAM, loads image from SD card into L2 SRAM
 * and starts executing directly _start entry point in L2 SRAM. Therefore reset
 * vector is not used and maximal size of the image is L2 cache size. For builds
 * with SPL there is no limit of U-Boot proper as BootROM loads SPL which then
 * loads U-Boot proper directly into DDR.
 */

/*
 * For SD card builds without SPL it is needed to set CONFIG_SYS_RAMBOOT
 *
 * if CONFIG_SPL_BUILD
 *     if CONFIG_FSL_PREPBL_ESDHC_BOOT_SECTOR
 *         define CONFIG_SPL_MAX_SIZE = (CONFIG_SYS_L2_SIZE+CONFIG_FSL_PREPBL_ESDHC_BOOT_SECTOR_DATA
 *                                      * SZ_512)
 *     else
 *         define CONFIG_SPL_MAX_SIZE = CONFIG_SYS_L2_SIZE
 */

#ifdef CONFIG_SDCARD
#define CFG_SYS_MMC_U_BOOT_SIZE	CONFIG_BOARD_SIZE_LIMIT
#define CFG_SYS_MMC_U_BOOT_DST	CONFIG_TEXT_BASE
#define CFG_SYS_MMC_U_BOOT_START	CONFIG_TEXT_BASE
#endif

/*
 * Booting from NOR
 * Last 4kB page of the NOR is mapped into CPU address space and CPU starts
 * executing last instruction of that page, which is reset vector address.
 * We have 16 MB large NOR memory, so define correct reset vector address.
 *
 * CONFIG_RESET_VECTOR_ADDRESS = (CONFIG_SYS_FLASH_BASE + SZ_16M - 4)
 */

/*
 * CONFIG_BOARD_SIZE_LIMIT must be hex number because it is used in Makefile.
 * For NOR build, size of the U-Boot binary must always be 768 kB.
 * For SD card build with SPL, there is no limit, just broken build system which
 * cannot fill CFG_SYS_MMC_U_BOOT_SIZE and CONFIG_SYS_MONITOR_LEN values
 * automatically. So choose it as lowest value as possible with which build
 * process does not fail, to minimize final binary size.
 * For SD card build without SPL, there is upper limit of L2 cache size.
 *
 * if SDCARD
 *     CONFIG_BOARD_SIZE_LIMIT = 0x000c0000 // 768 kB
 * elif SPL
 *     CONFIG_BOARD_SIZE_LIMIT = 0x00100000 // 1 MB
 * else
 *     CONFIG_BOARD_SIZE_LIMIT = 0x00080000 // 512 kB - must be same as CONFIG_SYS_L2_SIZE
 */

/*
 * Initial stack in L1 cache
 */

#define CFG_SYS_INIT_RAM_ADDR			0xffd00000
#define CFG_SYS_INIT_RAM_ADDR_PHYS		CFG_SYS_INIT_RAM_ADDR
#define CFG_SYS_INIT_RAM_ADDR_PHYS_HIGH	0
#define CFG_SYS_INIT_RAM_ADDR_PHYS_LOW		CFG_SYS_INIT_RAM_ADDR_PHYS
#define CFG_SYS_INIT_RAM_SIZE			SZ_16K

#define CFG_SYS_GBL_DATA_OFFSET			(CFG_SYS_INIT_RAM_SIZE - GENERATED_GBL_DATA_SIZE)
#define CFG_SYS_INIT_SP_OFFSET			CFG_SYS_GBL_DATA_OFFSET

/*
 * Initial SRAM in L2 cache
 */

/* Initial SRAM is used only for SD card boot in first stage image */
#if !defined(CONFIG_SPL) || defined(CONFIG_SPL_BUILD)
#define CFG_SYS_INIT_L2_ADDR		0xf8f80000
#define CFG_SYS_INIT_L2_ADDR_PHYS	CFG_SYS_INIT_L2_ADDR
/*
 * CONFIG_SPL_RELOC_TEXT_BASE =     CONFIG_SYS_MONITOR_BASE
 * CONFIG_SPL_GD_ADDR =             (CFG_SYS_INIT_L2_ADDR + 112 * SZ_1K)
 * CONFIG_SPL_RELOC_STACK =         (CFG_SYS_INIT_L2_ADDR + 116 * SZ_1K)
 * CONFIG_SPL_RELOC_MALLOC_ADDR =   (CFG_SYS_INIT_L2_ADDR + 148 * SZ_1K)
 * CONFIG_SPL_RELOC_MALLOC_SIZE =   (364 * SZ_1K)
 */
#endif

/*
 * CCSR
 */

#define CFG_SYS_CCSRBAR				0xffe00000
#define CFG_SYS_CCSRBAR_PHYS_LOW		CFG_SYS_CCSRBAR
/*
 * CFG_SYS_CCSRBAR_PHYS_HIGH =	0x0
 */

/*
 * U-Boot _start code expects that if CCSRBAR is configured to its default
 * location and automatically relocate it to the new CONFIG_SYS_CCSRBAR_PHYS
 * location. Relocation to the new location can be skipped by defining macro
 * CONFIG_SYS_CCSR_DO_NOT_RELOCATE.
 *
 * All addresses in device tree are set to according the new relocated CCSRBAR.
 * So device tree code cannot be used when CONFIG_SYS_CCSR_DO_NOT_RELOCATE is
 * set.
 *
 * If CCSRBAR is not configured to its default location then _start code hangs
 * or crashes.
 *
 * So relocation of CCSRBAR must be disabled in every code which runs before
 * U-Boot proper (e.g. SPL), otherwise U-Boot proper's _start code crashes.
 */

/*
 * DDR
 */

#define CFG_SYS_DDR_SDRAM_BASE			0x00000000
#define CFG_SYS_SDRAM_BASE				CFG_SYS_DDR_SDRAM_BASE

#define CFG_SYS_I2C_PCA9557_ADDR	0x18
#define SPD_EEPROM_ADDRESS		0x52

/*
 * NOR
 */

#define CFG_SYS_FLASH_BASE		0xef000000
#define CFG_SYS_FLASH_BASE_PHYS	CFG_SYS_FLASH_BASE
#define CFG_RESET_VECTOR_ADDRESS	(CFG_SYS_FLASH_BASE + SZ_16M - 4)

/*
 * CONFIG_SYS_BR0_PRELIM = (BR_PHYS_ADDR(CFG_SYS_FLASH_BASE_PHYS) | BR_PS_16 | BR_MS_GPCM | BR_V)
 * CONFIG_SYS_OR0_PRELIM = (OR_AM_16MB | OR_GPCM_CSNT | OR_GPCM_ACS_DIV2 | OR_GPCM_XACS
 *                         | OR_GPCM_SCY_15 | OR_GPCM_TRLX | OR_GPCM_EHTR | OR_GPCM_EAD)
 */

/*
 * NAND
 */

#define CFG_SYS_NAND_BASE		0xff800000
#define CFG_SYS_NAND_BASE_PHYS	CFG_SYS_NAND_BASE

/*
 * CONFIG_SYS_BR1_PRELIM = BR_PHYS_ADDR(CFG_SYS_NAND_BASE_PHYS) | BR_PS_8 | BR_MS_FCM | BR_V)
 * CONFIG_SYS_OR1_PRELIM = (OR_AM_256KB | OR_FCM_PGS | OR_FCM_CSCT | OR_FCM_CST | OR_FCM_CHT
 *                         | OR_FCM_SCY_1 | OR_FCM_TRLX | OR_FCM_EHTR)
 */

#define CFG_SYS_NAND_BASE_LIST	{ CFG_SYS_NAND_BASE }
#define CFG_SYS_NAND_OR_PRELIM	CONFIG_SYS_OR1_PRELIM

/*
 * CPLD
 */

#define CFG_SYS_CPLD_BASE		0xffa00000
#define CFG_SYS_CPLD_BASE_PHYS	CFG_SYS_CPLD_BASE

/*
 * CONFIG_SYS_BR3_PRELIM = (BR_PHYS_ADDR(CFG_SYS_CPLD_BASE_PHYS) | BR_PS_8 | BR_MS_GPCM | BR_V)
 * CONFIG_SYS_OR3_PRELIM = (OR_AM_128KB | OR_GPCM_CSNT | OR_GPCM_XACS | OR_GPCM_SCY_15
 *                         | OR_GPCM_TRLX | OR_GPCM_EHTR | OR_GPCM_EAD)
 */

/*
 * Serial Port
 */

#if !CONFIG_IS_ENABLED(DM_SERIAL)
#define CFG_SYS_NS16550_CLK			get_bus_freq(0)
#define CFG_SYS_NS16550_COM1		(CFG_SYS_CCSRBAR + 0x4500)
#define CFG_SYS_NS16550_COM2		(CFG_SYS_CCSRBAR + 0x4600)
#endif

/*
 * PCIe
 */

/* PCIe bus on mPCIe slot 1 (CN5) for expansion mPCIe card */
#define CFG_SYS_PCIE1_MEM_VIRT	0x80000000
#define CFG_SYS_PCIE1_IO_VIRT	0xffc00000
#define CFG_SYS_PCIE1_MEM_PHYS	CFG_SYS_PCIE1_MEM_VIRT
#define CFG_SYS_PCIE1_IO_PHYS	CFG_SYS_PCIE1_IO_VIRT

/* PCIe bus on mPCIe slot 2 (CN6) for expansion mPCIe card */
#define CFG_SYS_PCIE2_MEM_VIRT	0xa0000000
#define CFG_SYS_PCIE2_IO_VIRT	0xffc10000
#define CFG_SYS_PCIE2_MEM_PHYS	CFG_SYS_PCIE2_MEM_VIRT
#define CFG_SYS_PCIE2_IO_PHYS	CFG_SYS_PCIE2_IO_VIRT

/* PCIe bus for on-board TUSB7340RKM USB 3.0 xHCI controller */
#define CFG_SYS_PCIE3_MEM_VIRT	0xc0000000
#define CFG_SYS_PCIE3_IO_VIRT	0xffc20000
#define CFG_SYS_PCIE3_MEM_PHYS	CFG_SYS_PCIE3_MEM_VIRT
#define CFG_SYS_PCIE3_IO_PHYS	CFG_SYS_PCIE3_IO_VIRT

/*
 * eSDHC
 */

#define CFG_SYS_FSL_ESDHC_ADDR	CFG_SYS_MPC85xx_ESDHC_ADDR
#define SDHC_WP_IS_GPIO		/* SDHC_WP pin is not connected to SD card slot, it is GPIO pin */

/*
 * For booting Linux, the board info and command line data
 * have to be in the first 64 MB of memory, since this is
 * the maximum mapped by the Linux kernel during initialization.
 */
#define CFG_SYS_BOOTMAPSZ		SZ_64M /* Initial Memory for Linux */

/*
 * Environment Configuration
 */

#ifdef CONFIG_SDCARD
#define BOOT_TARGET_DEVICES(func) \
	func(MMC, mmc, 0) \
	func(NVME, nvme, 0) \
	func(SCSI, scsi, 0) \
	func(USB, usb, 0) \
	func(USB, usb, 1) \
	func(USB, usb, 2) \
	func(USB, usb, 3) \
	func(USB, usb, 4) \
	func(UBIFS, ubifs, 0, rootfs, rootfs, 512) \
	func(UBIFS, ubifs, 1, rootfs, rootfs, 2048) \
	func(DHCP, dhcp, na)
#else
#define BOOT_TARGET_DEVICES(func) \
	func(MMC, mmc, 0) \
	func(NVME, nvme, 0) \
	func(SCSI, scsi, 0) \
	func(USB, usb, 0) \
	func(USB, usb, 1) \
	func(USB, usb, 2) \
	func(USB, usb, 3) \
	func(USB, usb, 4) \
	func(DHCP, dhcp, na)
#endif

#include <config_distro_bootcmd.h>

/* These boot source switches macros must be constant numbers as they are stringified */
#define __SW_BOOT_MASK			0x03
#define __SW_BOOT_NOR			0xc8
#define __SW_BOOT_SPI			0x28
#define __SW_BOOT_SD			0x68
#define __SW_BOOT_SD2			0x18
#define __SW_BOOT_NAND			0xe8
#define __SW_BOOT_PCIE			0xa8
#define __SW_NOR_BANK_MASK		0xfd
#define __SW_NOR_BANK_UP		0x00
#define __SW_NOR_BANK_LO		0x02
#define __SW_BOOT_NOR_BANK_UP		0xc8 /* (__SW_BOOT_NOR | __SW_NOR_BANK_UP) */
#define __SW_BOOT_NOR_BANK_LO		0xca /* (__SW_BOOT_NOR | __SW_NOR_BANK_LO) */
#define __SW_BOOT_NOR_BANK_MASK		0x01 /* (__SW_BOOT_MASK & __SW_NOR_BANK_MASK) */

#include "p1_p2_bootsrc.h"

#define REBOOT_ENV_SETTINGS \
	RST_NOR_UP_CMD(reboot_to_nor, echo Rebooting to NOR bootloader;) \
	RST_SD_CMD(reboot_to_sd, echo Rebooting to SD bootloader;) \
	RST_DEF_CMD(reboot_to_def, echo Rebooting to default bootloader;) \
	""

#define BOOTCMD_RESCUE \
	"setenv bootargs root=mtd2 ro rootfstype=jffs2 console=ttyS0,115200; " \
	"mw.b 0xffa00002 0x03; " \
	"bootm 0xef020000 - 0xef000000" \
	""

#define CFG_EXTRA_ENV_SETTINGS \
	"fdt_addr_r=0x2000000\0" \
	"kernel_addr_r=0x2100000\0" \
	"scriptaddr=0x3000000\0" \
	"pxefile_addr_r=0x3100000\0" \
	"ramdisk_addr_r=0x4000000\0" \
	"fdtfile=" CONFIG_DEFAULT_DEVICE_TREE ".dtb\0" \
	"fdt_addr=0xef000000\0" \
	"bootcmd_rescue=" BOOTCMD_RESCUE "\0" \
	REBOOT_ENV_SETTINGS \
	BOOTENV

#endif /* _CONFIG_TURRIS_1X_H */
