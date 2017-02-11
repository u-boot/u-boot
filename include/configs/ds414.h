/*
 * Copyright (C) 2014 Stefan Roese <sr@denx.de>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef _CONFIG_SYNOLOGY_DS414_H
#define _CONFIG_SYNOLOGY_DS414_H

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
#define CONFIG_CMD_ENV

/* I2C */
#define CONFIG_SYS_I2C
#define CONFIG_SYS_I2C_MVTWSI
#define CONFIG_I2C_MVTWSI_BASE0		MVEBU_TWSI_BASE
#define CONFIG_SYS_I2C_SLAVE		0x0
#define CONFIG_SYS_I2C_SPEED		100000

/* SPI NOR flash default params, used by sf commands */
#define CONFIG_SF_DEFAULT_SPEED		1000000
#define CONFIG_SF_DEFAULT_MODE		SPI_MODE_3

/* Environment in SPI NOR flash */
#define CONFIG_ENV_IS_IN_SPI_FLASH
#define CONFIG_ENV_OFFSET		0x7E0000   /* RedBoot config partition in DTS */
#define CONFIG_ENV_SIZE			(64 << 10) /* 64KiB */
#define CONFIG_ENV_SECT_SIZE		(64 << 10) /* 64KiB sectors */

#define CONFIG_PHY_MARVELL		/* there is a marvell phy */
#define CONFIG_PHY_ADDR			{ 0x1, 0x0 }
#define CONFIG_SYS_NETA_INTERFACE_TYPE	PHY_INTERFACE_MODE_RGMII

#define CONFIG_SYS_ALT_MEMTEST

/* PCIe support */
#ifndef CONFIG_SPL_BUILD
#define CONFIG_CMD_PCI
#define CONFIG_CMD_PCI_ENUM
#define CONFIG_PCI_MVEBU
#define CONFIG_PCI_SCAN_SHOW
#endif

/* USB/EHCI/XHCI configuration */

#define CONFIG_DM_USB
#define CONFIG_USB_MAX_CONTROLLER_COUNT 2

/* FIXME: broken XHCI support
 * Below defines should enable support for the two rear USB3 ports. Sadly, this
 * does not work because:
 * - xhci-pci seems to not support DM_USB, so with that enabled it is not
 *   found.
 * - USB init fails, controller does not respond in time */
#if 0
#undef CONFIG_DM_USB
#define CONFIG_USB_XHCI_PCI
#define CONFIG_SYS_USB_XHCI_MAX_ROOT_PORTS 2
#endif

#if !defined(CONFIG_USB_XHCI_HCD)
#define CONFIG_USB_EHCI
#define CONFIG_USB_EHCI_MARVELL
#define CONFIG_EHCI_IS_TDI
#endif

/* why is this only defined in mv-common.h if CONFIG_DM is undefined? */
#define CONFIG_SUPPORT_VFAT
#define CONFIG_SYS_MVFS

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
#define CONFIG_SYS_SPI_U_BOOT_OFFS	0x24000

/* DS414 bus width is 32bits */
#define CONFIG_DDR_32BIT

/* Use random ethernet address if not configured */
#define CONFIG_LIB_RAND
#define CONFIG_NET_RANDOM_ETHADDR

/* Default Environment */
#define CONFIG_BOOTCOMMAND	"sf read ${loadaddr} 0xd0000 0x700000; bootm"
#define CONFIG_BOOTARGS		"console=ttyS0,115200"
#define CONFIG_LOADADDR		0x80000
#undef CONFIG_PREBOOT		/* override preboot for USB and SPI flash init */
#define CONFIG_PREBOOT		"usb start; sf probe"

#endif /* _CONFIG_SYNOLOGY_DS414_H */
