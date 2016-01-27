/*
 * Copyright 2013 Broadcom Corporation.
 *
 * SPDX-License-Identifier:      GPL-2.0+
 */

#ifndef __BCM28155_AP_H
#define __BCM28155_AP_H

#include <linux/sizes.h>
#include <asm/arch/sysmap.h>

/* CPU, chip, mach, etc */
#define CONFIG_KONA
#define CONFIG_SKIP_LOWLEVEL_INIT

/*
 * Memory configuration
 */
#define CONFIG_SYS_TEXT_BASE		0xae000000

#define CONFIG_SYS_SDRAM_BASE		0x80000000
#define CONFIG_SYS_SDRAM_SIZE		0x80000000
#define CONFIG_NR_DRAM_BANKS		1

#define CONFIG_SYS_MALLOC_LEN		SZ_4M	/* see armv7/start.S. */
#define CONFIG_STACKSIZE		SZ_256K

/* GPIO Driver */
#define CONFIG_KONA_GPIO

/* MMC/SD Driver */
#define CONFIG_SDHCI
#define CONFIG_MMC_SDMA
#define CONFIG_KONA_SDHCI
#define CONFIG_MMC
#define CONFIG_GENERIC_MMC

#define CONFIG_SYS_SDIO_BASE0 SDIO1_BASE_ADDR
#define CONFIG_SYS_SDIO_BASE1 SDIO2_BASE_ADDR
#define CONFIG_SYS_SDIO_BASE2 SDIO3_BASE_ADDR
#define CONFIG_SYS_SDIO_BASE3 SDIO4_BASE_ADDR
#define CONFIG_SYS_SDIO0_MAX_CLK 48000000
#define CONFIG_SYS_SDIO1_MAX_CLK 48000000
#define CONFIG_SYS_SDIO2_MAX_CLK 48000000
#define CONFIG_SYS_SDIO3_MAX_CLK 48000000
#define CONFIG_SYS_SDIO0 "sdio1"
#define CONFIG_SYS_SDIO1 "sdio2"
#define CONFIG_SYS_SDIO2 "sdio3"
#define CONFIG_SYS_SDIO3 "sdio4"

/* I2C Driver */
#define CONFIG_SYS_I2C
#define CONFIG_SYS_I2C_KONA
#define CONFIG_SYS_SPD_BUS_NUM	3	/* Start with PMU bus */
#define CONFIG_SYS_MAX_I2C_BUS	4
#define CONFIG_SYS_I2C_BASE0	BSC1_BASE_ADDR
#define CONFIG_SYS_I2C_BASE1	BSC2_BASE_ADDR
#define CONFIG_SYS_I2C_BASE2	BSC3_BASE_ADDR
#define CONFIG_SYS_I2C_BASE3	PMU_BSC_BASE_ADDR

/* Timer Driver */
#define CONFIG_SYS_TIMER_RATE		32000
#define CONFIG_SYS_TIMER_COUNTER	(TIMER_BASE_ADDR + 4) /* STCLO offset */

/* Init functions */
#define CONFIG_MISC_INIT_R	/* board's misc_init_r function */

/* Some commands use this as the default load address */
#define CONFIG_SYS_LOAD_ADDR		CONFIG_SYS_SDRAM_BASE

/* No mtest functions as recommended */

/*
 * This is the initial SP which is used only briefly for relocating the u-boot
 * image to the top of SDRAM. After relocation u-boot moves the stack to the
 * proper place.
 */
#define CONFIG_SYS_INIT_SP_ADDR		CONFIG_SYS_TEXT_BASE

/* Serial Info */
#define CONFIG_SYS_NS16550_SERIAL
/* Post pad 3 bytes after each reg addr */
#define CONFIG_SYS_NS16550_REG_SIZE	(-4)
#define CONFIG_SYS_NS16550_CLK		13000000
#define CONFIG_CONS_INDEX		1
#define CONFIG_SYS_NS16550_COM1		0x3e000000

#define CONFIG_BAUDRATE			115200

#define CONFIG_ENV_SIZE			0x10000
#define CONFIG_ENV_IS_NOWHERE

#define CONFIG_SYS_NO_FLASH	/* Not using NAND/NOR unmanaged flash */

/* console configuration */
#define CONFIG_SYS_CBSIZE		1024	/* Console buffer size */
#define CONFIG_SYS_PBSIZE		(CONFIG_SYS_CBSIZE + \
			sizeof(CONFIG_SYS_PROMPT) + 16)	/* Printbuffer size */
#define CONFIG_SYS_MAXARGS		64
#define CONFIG_SYS_BARGSIZE		CONFIG_SYS_CBSIZE

/*
 * One partition type must be defined for part.c
 * This is necessary for the fatls command to work on an SD card
 * for example.
 */
#define CONFIG_DOS_PARTITION
#define CONFIG_EFI_PARTITION

/* version string, parser, etc */
#define CONFIG_VERSION_VARIABLE
#define CONFIG_AUTO_COMPLETE
#define CONFIG_SYS_HUSH_PARSER
#define CONFIG_CMDLINE_EDITING
#define CONFIG_SYS_LONGHELP

#define CONFIG_CRC32_VERIFY
#define CONFIG_MX_CYCLIC

/* Initial upstream - boot to cmd prompt only */
#define CONFIG_BOOTCOMMAND		""

/* Commands */
#define CONFIG_CMD_ASKENV
#define CONFIG_CMD_CACHE
#define CONFIG_CMD_FAT
#define CONFIG_CMD_I2C
#define CONFIG_CMD_MMC
#define CONFIG_CMD_BOOTZ
#define CONFIG_FAT_WRITE

/* Fastboot and USB OTG */
#define CONFIG_USB_FUNCTION_FASTBOOT
#define CONFIG_CMD_FASTBOOT
#define CONFIG_FASTBOOT_FLASH
#define CONFIG_FASTBOOT_FLASH_MMC_DEV	0
#define CONFIG_SYS_CACHELINE_SIZE	64
#define CONFIG_FASTBOOT_BUF_SIZE	(CONFIG_SYS_SDRAM_SIZE - SZ_1M)
#define CONFIG_FASTBOOT_BUF_ADDR	CONFIG_SYS_SDRAM_BASE
#define CONFIG_USB_GADGET
#define CONFIG_USB_GADGET_DUALSPEED
#define CONFIG_USB_GADGET_VBUS_DRAW	0
#define CONFIG_USB_GADGET_DWC2_OTG
#define CONFIG_USB_GADGET_BCM_UDC_OTG_PHY
#define CONFIG_USB_GADGET_DOWNLOAD
#define CONFIG_USBID_ADDR		0x34052c46
#define CONFIG_G_DNL_VENDOR_NUM		0x18d1	/* google */
#define CONFIG_G_DNL_PRODUCT_NUM	0x0d02	/* nexus one */
#define CONFIG_G_DNL_MANUFACTURER	"Broadcom Corporation"

#endif /* __BCM28155_AP_H */
