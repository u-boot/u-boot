/* SPDX-License-Identifier: GPL-2.0-or-later OR BSD-3-Clause */
/*
 * Copyright (C) 2025, STMicroelectronics - All Rights Reserved
 *
 * Configuration settings for the STM32MP23x CPU
 */

#ifndef __CONFIG_STM32MP23_COMMMON_H
#define __CONFIG_STM32MP23_COMMMON_H
#include <linux/sizes.h>
#include <asm/arch/stm32.h>

/*
 * Configuration of the external SRAM memory used by U-Boot
 */
#define CFG_SYS_SDRAM_BASE	STM32_DDR_BASE

/*
 * For booting Linux, use the first 256 MB of memory, since this is
 * the maximum mapped by the Linux kernel during initialization.
 */
#define CFG_SYS_BOOTMAPSZ		SZ_256M

#define STM32MP_FIP_IMAGE_GUID \
	EFI_GUID(0x19d5df83, 0x11b0, 0x457b, 0xbe, 0x2c, \
		 0x75, 0x59, 0xc1, 0x31, 0x42, 0xa5)

/*****************************************************************************/
#ifdef CONFIG_DISTRO_DEFAULTS
/*****************************************************************************/

#ifdef CONFIG_NET
#define BOOT_TARGET_PXE(func)	func(PXE, pxe, na)
#else
#define BOOT_TARGET_PXE(func)
#endif

#ifdef CONFIG_CMD_MMC
#define BOOT_TARGET_MMC0(func)	func(MMC, mmc, 0)
#define BOOT_TARGET_MMC1(func)	func(MMC, mmc, 1)
#define BOOT_TARGET_MMC2(func)	func(MMC, mmc, 2)
#else
#define BOOT_TARGET_MMC0(func)
#define BOOT_TARGET_MMC1(func)
#define BOOT_TARGET_MMC2(func)
#endif

#ifdef CONFIG_CMD_UBIFS
#define BOOT_TARGET_UBIFS(func)	func(UBIFS, ubifs, 0, UBI, boot)
#else
#define BOOT_TARGET_UBIFS(func)
#endif

#ifdef CONFIG_CMD_USB
#define BOOT_TARGET_USB(func)	func(USB, usb, 0)
#else
#define BOOT_TARGET_USB(func)
#endif

#define BOOT_TARGET_DEVICES(func)	\
	BOOT_TARGET_MMC1(func)		\
	BOOT_TARGET_UBIFS(func)		\
	BOOT_TARGET_MMC0(func)		\
	BOOT_TARGET_MMC2(func)		\
	BOOT_TARGET_USB(func)		\
	BOOT_TARGET_PXE(func)

/*
 * default bootcmd for stm32mp23:
 * for serial/usb: execute the stm32prog command
 * for mmc boot (eMMC, SD card), distro boot on the same mmc device
 * for NAND or SPI-NAND boot, distro boot with UBIFS on UBI partition
 * for other boot, use the default distro order in ${boot_targets}
 */
#define STM32MP_BOOTCMD "bootcmd_stm32mp=" \
	"echo \"Boot over ${boot_device}${boot_instance}!\";" \
	"if test ${boot_device} = serial || test ${boot_device} = usb;" \
	"then stm32prog ${boot_device} ${boot_instance}; " \
	"else " \
		"run env_check;" \
		"if test ${boot_device} = mmc;" \
		"then env set boot_targets \"mmc${boot_instance}\"; fi;" \
		"if test ${boot_device} = nand ||" \
		  " test ${boot_device} = spi-nand ;" \
		"then env set boot_targets ubifs0; fi;" \
		"run distro_bootcmd;" \
	"fi;\0"

#ifndef STM32MP_BOARD_EXTRA_ENV
#define STM32MP_BOARD_EXTRA_ENV
#endif

#define STM32MP_EXTRA \
	"env_check=if env info -p -d -q; then env save; fi\0" \
	"boot_net_usb_start=true\0"
/*
 * memory layout for 96MB uncompressed/compressed kernel,
 * 1M fdt, 1M script, 1M pxe and 1M for overlay
 * and the ramdisk at the end.
 */
#define __KERNEL_COMP_ADDR_R	__stringify(0x84000000)
#define __KERNEL_COMP_SIZE_R	__stringify(0x04000000)
#define __KERNEL_ADDR_R		__stringify(0x8a000000)
#define __FDT_ADDR_R		__stringify(0x90000000)
#define __SCRIPT_ADDR_R		__stringify(0x90100000)
#define __PXEFILE_ADDR_R	__stringify(0x90200000)
#define __FDTOVERLAY_ADDR_R	__stringify(0x90300000)
#define __RAMDISK_ADDR_R	__stringify(0x90400000)

#define STM32MP_MEM_LAYOUT \
	"kernel_addr_r=" __KERNEL_ADDR_R "\0" \
	"fdt_addr_r=" __FDT_ADDR_R "\0" \
	"scriptaddr=" __SCRIPT_ADDR_R "\0" \
	"pxefile_addr_r=" __PXEFILE_ADDR_R "\0" \
	"fdtoverlay_addr_r=" __FDTOVERLAY_ADDR_R "\0" \
	"ramdisk_addr_r=" __RAMDISK_ADDR_R "\0" \
	"kernel_comp_addr_r=" __KERNEL_COMP_ADDR_R "\0"	\
	"kernel_comp_size=" __KERNEL_COMP_SIZE_R "\0"

#include <config_distro_bootcmd.h>
#define CFG_EXTRA_ENV_SETTINGS \
	STM32MP_MEM_LAYOUT \
	STM32MP_BOOTCMD \
	BOOTENV \
	STM32MP_EXTRA \
	STM32MP_BOARD_EXTRA_ENV

#endif

#endif /* __CONFIG_STM32MP23_COMMMON_H */
