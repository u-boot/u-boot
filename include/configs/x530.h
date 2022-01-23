/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2018 Allied Telesis Labs
 */

#ifndef _CONFIG_X530_H
#define _CONFIG_X530_H

/*
 * High Level Configuration Options (easy to change)
 */

/*
 * NS16550 Configuration
 */
#define CONFIG_SYS_NS16550_SERIAL
#define CONFIG_SYS_NS16550_CLK		CONFIG_SYS_TCLK
#if !defined(CONFIG_DM_SERIAL)
#define CONFIG_SYS_NS16550_REG_SIZE	(-4)
#define CONFIG_SYS_NS16550_COM1		MV_UART_CONSOLE_BASE
#endif

/*
 * Serial Port configuration
 * The following definitions let you select what serial you want to use
 * for your console driver.
 */

/* NAND */
#define CONFIG_SYS_MAX_NAND_DEVICE 1

#define BBT_CUSTOM_SCAN
#define BBT_CUSTOM_SCAN_PAGE 0
#define BBT_CUSTOM_SCAN_POSITION 2048

/* SPI NOR flash default params, used by sf commands */

#define MTDIDS_DEFAULT			"nand0=nand"
#define MTDPARTS_DEFAULT		"mtdparts=nand:240M(user),8M(errlog),8M(nand-bbt)"
#define MTDPARTS_MTDOOPS		"errlog"

/* Partition support */

/* Additional FS support/configuration */

/* Environment in SPI NOR flash */

#define PHY_ANEG_TIMEOUT	8000	/* PHY needs a longer aneg time */

/* PCIe support */
#ifndef CONFIG_SPL_BUILD
#define CONFIG_PCI_SCAN_SHOW
#endif

/* NAND */

#include <asm/arch/config.h>

/* Keep device tree and initrd in low memory so the kernel can access them */
#define CONFIG_EXTRA_ENV_SETTINGS	\
	"fdt_high=0x10000000\0"		\
	"initrd_high=0x10000000\0"

#define CONFIG_UBI_PART			user
#define CONFIG_UBIFS_VOLUME		user

/* SPL */

/* Defines for SPL */
#define CONFIG_SPL_SIZE			(140 << 10)
#define CONFIG_SPL_MAX_SIZE		(CONFIG_SPL_SIZE - (CONFIG_SPL_TEXT_BASE - 0x40000000))

#define CONFIG_SPL_BSS_START_ADDR	(0x40000000 + CONFIG_SPL_SIZE)
#define CONFIG_SPL_BSS_MAX_SIZE		(16 << 10)

#ifdef CONFIG_SPL_BUILD
#define CONFIG_SYS_MALLOC_SIMPLE
#endif

#define CONFIG_SPL_STACK		(0x40000000 + ((192 - 16) << 10))
#define CONFIG_SPL_BOOTROM_SAVE		(CONFIG_SPL_STACK + 4)

#endif /* _CONFIG_X530_H */
