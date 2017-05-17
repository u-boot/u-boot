/*
 * Copyright (C) 2015-2016 Stefan Roese <sr@denx.de>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef _CONFIG_THEADORABLE_H
#define _CONFIG_THEADORABLE_H

/*
 * High Level Configuration Options (easy to change)
 */
#define CONFIG_DISPLAY_BOARDINFO_LATE

/*
 * TEXT_BASE needs to be below 16MiB, since this area is scrubbed
 * for DDR ECC byte filling in the SPL before loading the main
 * U-Boot into it.
 */
#define	CONFIG_SYS_TEXT_BASE	0x00800000
#define CONFIG_SYS_TCLK		250000000	/* 250MHz */

/*
 * Commands configuration
 */
#define CONFIG_CMD_SATA

/*
 * The debugging version enables USB support via defconfig.
 * This version should also enable all other non-production
 * interfaces / features.
 */
#ifdef CONFIG_USB
#define CONFIG_CMD_PCI
#endif

/* I2C */
#define CONFIG_SYS_I2C
#define CONFIG_SYS_I2C_MVTWSI
#define CONFIG_I2C_MVTWSI_BASE0		MVEBU_TWSI_BASE
#define CONFIG_I2C_MVTWSI_BASE1		MVEBU_TWSI1_BASE
#define CONFIG_SYS_I2C_SLAVE		0x0
#define CONFIG_SYS_I2C_SPEED		100000

/* USB/EHCI configuration */
#define CONFIG_EHCI_IS_TDI
#define CONFIG_USB_MAX_CONTROLLER_COUNT 3

/* SPI NOR flash default params, used by sf commands */
#define CONFIG_SF_DEFAULT_SPEED		27777777 /* for fast SPL booting */
#define CONFIG_SF_DEFAULT_MODE		SPI_MODE_3

/* Environment in SPI NOR flash */
#define CONFIG_ENV_IS_IN_SPI_FLASH
#define CONFIG_ENV_OFFSET		(1 << 20) /* 1MiB in */
#define CONFIG_ENV_SIZE			(64 << 10) /* 64KiB */
#define CONFIG_ENV_SECT_SIZE		(256 << 10) /* 256KiB sectors */
#define CONFIG_ENV_OVERWRITE

#define CONFIG_PHY_MARVELL		/* there is a marvell phy */
#define PHY_ANEG_TIMEOUT	8000	/* PHY needs a longer aneg time */

#define CONFIG_SYS_ALT_MEMTEST
#define CONFIG_PREBOOT

/* Keep device tree and initrd in lower memory so the kernel can access them */
#define CONFIG_EXTRA_ENV_SETTINGS	\
	"fdt_high=0x10000000\0"		\
	"initrd_high=0x10000000\0"

/* SATA support */
#define CONFIG_SYS_SATA_MAX_DEVICE	1
#define CONFIG_SATA_MV
#define CONFIG_LIBATA
#define CONFIG_LBA48

/* Additional FS support/configuration */
#define CONFIG_SUPPORT_VFAT

/* PCIe support */
#ifdef CONFIG_CMD_PCI
#ifndef CONFIG_SPL_BUILD
#define CONFIG_PCI_MVEBU
#endif
#endif

/* Enable LCD and reserve 512KB from top of memory*/
#define CONFIG_SYS_MEM_TOP_HIDE		0x80000

/* FPGA programming support */
#define CONFIG_FPGA
#define CONFIG_FPGA_ALTERA
#define CONFIG_FPGA_STRATIX_V

/*
 * Bootcounter
 */
#define CONFIG_BOOTCOUNT_LIMIT
#define CONFIG_BOOTCOUNT_RAM
/* Max size of RAM minus BOOTCOUNT_ADDR is the bootcounter address */
#define BOOTCOUNT_ADDR			0x1000

/*
 * mv-common.h should be defined after CMD configs since it used them
 * to enable certain macros
 */
#include "mv-common.h"

/*
 * Memory layout while starting into the bin_hdr via the
 * BootROM:
 *
 * 0x4000.4000 - 0x4003.4000	headers space (192KiB)
 * 0x4000.4030			bin_hdr start address
 * 0x4003.4000 - 0x4004.7c00	BootROM memory allocations (15KiB)
 * 0x4007.fffc			BootROM stack top
 *
 * The address space between 0x4007.fffc and 0x400f.fff is not locked in
 * L2 cache thus cannot be used.
 */

/* SPL */
/* Defines for SPL */
#define CONFIG_SPL_FRAMEWORK
#define CONFIG_SPL_TEXT_BASE		0x40004030
#define CONFIG_SPL_MAX_SIZE		((128 << 10) - 0x4030)

#define CONFIG_SPL_BSS_START_ADDR	(0x40000000 + (128 << 10))
#define CONFIG_SPL_BSS_MAX_SIZE		(16 << 10)

#ifdef CONFIG_SPL_BUILD
#define CONFIG_SYS_MALLOC_SIMPLE
#endif

#define CONFIG_SPL_STACK		(0x40000000 + ((192 - 16) << 10))
#define CONFIG_SPL_BOOTROM_SAVE		(CONFIG_SPL_STACK + 4)

/* SPL related SPI defines */
#define CONFIG_SPL_SPI_LOAD
#define CONFIG_SYS_SPI_U_BOOT_OFFS	0x1a000
#define CONFIG_SYS_U_BOOT_OFFS		CONFIG_SYS_SPI_U_BOOT_OFFS

/* Enable DDR support in SPL (DDR3 training from Marvell bin_hdr) */
#define CONFIG_DDR_FIXED_SIZE		(2 << 20)	/* 2GiB */

#endif /* _CONFIG_THEADORABLE_H */
