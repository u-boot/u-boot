/*
 * (C) Copyright 2011
 * eInfochips Ltd. <www.einfochips.com>
 * Written-by: Ajay Bhargav <ajay.bhargav@einfochips.com>
 *
 * Based on Aspenite:
 * (C) Copyright 2010
 * Marvell Semiconductor <www.marvell.com>
 * Written-by: Prafulla Wadaskar <prafulla@marvell.com>
 * Contributor: Mahavir Jain <mjain@marvell.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __CONFIG_GPLUGD_H
#define __CONFIG_GPLUGD_H

/*
 * FIXME: fix for error caused due to recent update to mach-types.h
 */
#include <asm/mach-types.h>
#ifdef MACH_TYPE_SHEEVAD
#error "MACH_TYPE_SHEEVAD has been defined properly, please remove this."
#else
#define MACH_TYPE_SHEEVAD	2625
#endif

/*
 * Version number information
 */
#define CONFIG_IDENT_STRING	"\nMarvell-gplugD"

/*
 * High Level Configuration Options
 */
#define CONFIG_SHEEVA_88SV331xV5	1	/* CPU Core subversion */
#define CONFIG_ARMADA100		1	/* SOC Family Name */
#define CONFIG_ARMADA168		1	/* SOC Used on this Board */
#define CONFIG_MACH_TYPE		MACH_TYPE_SHEEVAD /* Machine type */
#define CONFIG_SKIP_LOWLEVEL_INIT	/* disable board lowlevel_init */

#define	CONFIG_SYS_TEXT_BASE	0x00f00000

/*
 * There is no internal RAM in ARMADA100, using DRAM
 * TBD: dcache to be used for this
 */
#define CONFIG_SYS_INIT_SP_ADDR		(CONFIG_SYS_TEXT_BASE - 0x00200000)
#define CONFIG_NR_DRAM_BANKS_MAX	2

/*
 * Commands configuration
 */
#define CONFIG_SYS_NO_FLASH		/* Declare no flash (NOR/SPI) */
#include <config_cmd_default.h>
#define CONFIG_CMD_I2C
#undef CONFIG_CMD_FPGA
#define CONFIG_CMD_USB
#define CONFIG_CMD_EXT2
#define CONFIG_CMD_FAT

/* Disable DCACHE */
#define CONFIG_SYS_DCACHE_OFF

/* Network configuration */
#ifdef CONFIG_CMD_NET
#define CONFIG_CMD_PING
#define CONFIG_ARMADA100_FEC

/* DHCP Support */
#define CONFIG_CMD_DHCP
#define CONFIG_BOOTP_DHCP_REQUEST_DELAY		50000
#endif /* CONFIG_CMD_NET */

/* GPIO Support */
#define CONFIG_MARVELL_GPIO

/* PHY configuration */
#define CONFIG_MII
#define CONFIG_CMD_MII
#define CONFIG_RESET_PHY_R
/* 88E3015 register definition */
#define PHY_LED_PAR_SEL_REG		22
#define PHY_LED_MAN_REG			25
#define PHY_LED_VAL			0x5b	/* LINK LED1, ACT LED2 */
/* GPIO Configuration for PHY */
#define CONFIG_SYS_GPIO_PHY_RST		104	/* GPIO104 */

/* SPI Support */
#define CONFIG_ARMADA100_SPI
#define CONFIG_ENV_SPI_CS		110
#define CONFIG_SYS_SSP_PORT		2

/* Flash Support */
#define CONFIG_CMD_SF
#define CONFIG_SPI_FLASH_ATMEL

/*
 * mv-common.h should be defined after CMD configs since it used them
 * to enable certain macros
 */
#include "mv-common.h"
#undef CONFIG_ARCH_MISC_INIT

#ifdef CONFIG_SYS_NS16550_COM1
#undef CONFIG_SYS_NS16550_COM1
#endif /* CONFIG_SYS_NS16550_COM1 */

#define CONFIG_SYS_NS16550_COM1 ARMD1_UART3_BASE

/*
 * Environment variables configurations
 */
#define CONFIG_ENV_IS_IN_SPI_FLASH
#define CONFIG_ENV_SECT_SIZE		0x4000
#define CONFIG_ENV_SIZE			0x4000
#define CONFIG_ENV_OFFSET		0x07C000

#define CONFIG_CMD_ASKENV
#define CONFIG_CMD_EDITENV
#define CONFIG_CMD_SAVEENV

#ifdef CONFIG_CMD_USB
#define CONFIG_USB_EHCI
#define CONFIG_USB_EHCI_ARMADA100
#define CONFIG_EHCI_IS_TDI
#define CONFIG_USB_STORAGE
#endif /* CONFIG_CMD_USB */

#define CONFIG_DOS_PARTITION
#define CONFIG_ISO_PARTITION
#define CONFIG_SUPPORT_VFAT

#endif	/* __CONFIG_GPLUGD_H */
