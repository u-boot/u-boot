/*
 * LG Optimus Black (P970) codename sniper config
 *
 * Copyright (C) 2015 Paul Kocialkowski <contact@paulk.fr>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __CONFIG_H
#define __CONFIG_H

#include <asm/arch/cpu.h>
#include <asm/arch/omap.h>

/*
 * CPU
 */

#define CONFIG_SYS_CACHELINE_SIZE	64

#define CONFIG_ARM_ARCH_CP15_ERRATA
#define CONFIG_ARM_ERRATA_454179
#define CONFIG_ARM_ERRATA_430973
#define CONFIG_ARM_ERRATA_621766

/*
 * Platform
 */

#define CONFIG_OMAP
#define CONFIG_OMAP_COMMON

/*
 * Board
 */

#define CONFIG_MISC_INIT_R

/*
 * Clocks
 */

#define CONFIG_SYS_TIMERBASE	OMAP34XX_GPT2
#define CONFIG_SYS_PTV		2

#define V_NS16550_CLK		48000000
#define V_OSCK			26000000
#define V_SCLK			(V_OSCK >> 1)

/*
 * DRAM
 */

#define CONFIG_SDRC
#define CONFIG_NR_DRAM_BANKS	2
#define PHYS_SDRAM_1		OMAP34XX_SDRC_CS0
#define PHYS_SDRAM_2		OMAP34XX_SDRC_CS1

/*
 * Memory
 */

#define CONFIG_SYS_TEXT_BASE		0x80100000
#define CONFIG_SYS_SDRAM_BASE		OMAP34XX_SDRC_CS0
#define CONFIG_SYS_INIT_RAM_ADDR	0x4020F800
#define CONFIG_SYS_INIT_RAM_SIZE	0x800
#define CONFIG_SYS_INIT_SP_ADDR		(CONFIG_SYS_INIT_RAM_ADDR + \
					 CONFIG_SYS_INIT_RAM_SIZE - \
					 GENERATED_GBL_DATA_SIZE)

#define CONFIG_SYS_MALLOC_LEN		(1024 * 1024 + CONFIG_ENV_SIZE)

/*
 * GPIO
 */

#define CONFIG_OMAP_GPIO
#define CONFIG_OMAP3_GPIO_2
#define CONFIG_OMAP3_GPIO_3
#define CONFIG_OMAP3_GPIO_4
#define CONFIG_OMAP3_GPIO_5
#define CONFIG_OMAP3_GPIO_6

/*
 * I2C
 */

#define CONFIG_SYS_I2C
#define CONFIG_SYS_OMAP24_I2C_SPEED	400000
#define CONFIG_SYS_OMAP24_I2C_SLAVE	1
#define CONFIG_SYS_I2C_OMAP34XX
#define CONFIG_I2C_MULTI_BUS

#define CONFIG_CMD_I2C

/*
 * Flash
 */

#define CONFIG_SYS_NO_FLASH

/*
 * MMC
 */

#define CONFIG_GENERIC_MMC
#define CONFIG_MMC
#define CONFIG_OMAP_HSMMC

#define CONFIG_CMD_MMC

/*
 * Power
 */

#define CONFIG_TWL4030_POWER

/*
 * Input
 */

#define CONFIG_TWL4030_INPUT

/*
 * Partitions
 */

#define CONFIG_PARTITION_UUIDS
#define CONFIG_DOS_PARTITION
#define CONFIG_EFI_PARTITION

#define CONFIG_CMD_PART

/*
 * Filesystems
 */

#define CONFIG_CMD_FS_GENERIC
#define CONFIG_CMD_EXT2
#define CONFIG_CMD_EXT4
#define CONFIG_CMD_FAT

/*
 * SPL
 */

#define CONFIG_SPL_FRAMEWORK

#define CONFIG_SPL_TEXT_BASE		0x40200000
#define CONFIG_SPL_MAX_SIZE		(54 * 1024)
#define CONFIG_SPL_BSS_START_ADDR	0x80000000
#define CONFIG_SPL_BSS_MAX_SIZE		(512 * 1024)
#define CONFIG_SYS_SPL_MALLOC_START	0x80208000
#define CONFIG_SYS_SPL_MALLOC_SIZE	(1024 * 1024)
#define CONFIG_SPL_STACK		LOW_LEVEL_SRAM_STACK

#define CONFIG_SPL_LDSCRIPT		"$(CPUDIR)/omap-common/u-boot-spl.lds"
#define CONFIG_SPL_BOARD_INIT

#define CONFIG_SPL_LIBGENERIC_SUPPORT
#define CONFIG_SPL_LIBCOMMON_SUPPORT
#define CONFIG_SPL_LIBDISK_SUPPORT
#define CONFIG_SPL_SERIAL_SUPPORT
#define CONFIG_SPL_POWER_SUPPORT
#define CONFIG_SPL_GPIO_SUPPORT
#define CONFIG_SPL_I2C_SUPPORT
#define CONFIG_SPL_MMC_SUPPORT
#define CONFIG_SPL_FAT_SUPPORT

#define CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_PARTITION	2

#define CONFIG_SYS_MMCSD_FS_BOOT_PARTITION		1
#define CONFIG_SPL_FS_LOAD_PAYLOAD_NAME			"u-boot.img"

/*
 * Console
 */

#define CONFIG_SYS_CONSOLE_IS_IN_ENV

#define CONFIG_DISPLAY_CPUINFO
#define CONFIG_DISPLAY_BOARDINFO

#define CONFIG_AUTO_COMPLETE

#define CONFIG_SYS_LONGHELP
#define CONFIG_SYS_HUSH_PARSER

#define CONFIG_SYS_MAXARGS	16
#define CONFIG_SYS_CBSIZE	512
#define CONFIG_SYS_PBSIZE	(CONFIG_SYS_CBSIZE + sizeof(CONFIG_SYS_PROMPT) \
				 + 16)

/*
 * Serial
 */


#ifdef CONFIG_SPL_BUILD
#define CONFIG_SYS_NS16550_SERIAL
#define CONFIG_SYS_NS16550_REG_SIZE	(-4)
#endif

#define CONFIG_SYS_NS16550_CLK		V_NS16550_CLK
#define CONFIG_SYS_NS16550_COM3		OMAP34XX_UART3
#define CONFIG_CONS_INDEX		3
#define CONFIG_SERIAL3			3

#define CONFIG_BAUDRATE			115200
#define CONFIG_SYS_BAUDRATE_TABLE	{ 4800, 9600, 19200, 38400, 57600, \
					  115200 }

/*
 * USB gadget
 */

#define CONFIG_USB_MUSB_PIO_ONLY
#define CONFIG_USB_MUSB_OMAP2PLUS
#define CONFIG_TWL4030_USB

#define CONFIG_USB_GADGET
#define CONFIG_USB_GADGET_DUALSPEED
#define CONFIG_USB_GADGET_VBUS_DRAW	0

/*
 * Download
 */

#define CONFIG_USB_GADGET_DOWNLOAD

#define CONFIG_G_DNL_VENDOR_NUM		0x0451
#define CONFIG_G_DNL_PRODUCT_NUM	0xd022
#define CONFIG_G_DNL_MANUFACTURER	"Texas Instruments"

/*
 * Fastboot
 */

#define CONFIG_USB_FUNCTION_FASTBOOT

#define CONFIG_FASTBOOT_BUF_ADDR	CONFIG_SYS_LOAD_ADDR
#define CONFIG_FASTBOOT_BUF_SIZE	0x2000000

#define CONFIG_FASTBOOT_FLASH
#define CONFIG_FASTBOOT_FLASH_MMC_DEV	0

#define CONFIG_CMD_FASTBOOT

/*
 * Environment
 */

#define CONFIG_ENV_SIZE		(128 * 1024)
#define CONFIG_ENV_IS_NOWHERE

#define CONFIG_ENV_OVERWRITE

#define CONFIG_EXTRA_ENV_SETTINGS \
	"kernel_addr_r=0x82000000\0" \
	"boot_mmc_dev=0\0" \
	"kernel_mmc_part=3\0" \
	"recovery_mmc_part=4\0" \
	"bootargs=console=ttyO2 vram=5M,0x9FA00000 omapfb.vram=0:5M\0"

/*
 * ATAGs / Device Tree
 */

#define CONFIG_OF_LIBFDT
#define CONFIG_SETUP_MEMORY_TAGS
#define CONFIG_CMDLINE_TAG
#define CONFIG_INITRD_TAG
#define CONFIG_REVISION_TAG
#define CONFIG_SERIAL_TAG

/*
 * Boot
 */

#define CONFIG_SYS_LOAD_ADDR	0x82000000
#define CONFIG_BOOTDELAY	1

#define CONFIG_ANDROID_BOOT_IMAGE

#define CONFIG_BOOTCOMMAND \
	"setenv boot_mmc_part ${kernel_mmc_part}; " \
	"if test reboot-${reboot-mode} = reboot-r; then " \
	"echo recovery; setenv boot_mmc_part ${recovery_mmc_part}; fi; " \
	"if test reboot-${reboot-mode} = reboot-b; then " \
	"echo fastboot; fastboot 0; fi; " \
	"part start mmc ${boot_mmc_dev} ${boot_mmc_part} boot_mmc_start; " \
	"part size mmc ${boot_mmc_dev} ${boot_mmc_part} boot_mmc_size; " \
	"mmc dev ${boot_mmc_dev}; " \
	"mmc read ${kernel_addr_r} ${boot_mmc_start} ${boot_mmc_size} && " \
	"bootm ${kernel_addr_r};"

/*
 * Defaults
 */

#include <config_defaults.h>

#endif
