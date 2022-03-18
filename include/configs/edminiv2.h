/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2010 Albert ARIBAUD <albert.u.boot@aribaud.net>
 *
 * Based on original Kirkwood support which is
 * (C) Copyright 2009
 * Marvell Semiconductor <www.marvell.com>
 * Written-by: Prafulla Wadaskar <prafulla@marvell.com>
 */

#ifndef _CONFIG_EDMINIV2_H
#define _CONFIG_EDMINIV2_H

/*
 * SPL
 */

#define CONFIG_SPL_MAX_SIZE		0x0000fff0
#define CONFIG_SPL_STACK		0x00020000
#define CONFIG_SPL_BSS_START_ADDR	0x00020000
#define CONFIG_SPL_BSS_MAX_SIZE		0x0001ffff
#define CONFIG_SYS_SPL_MALLOC_START	0x00040000
#define CONFIG_SYS_SPL_MALLOC_SIZE	0x0001ffff
#define CONFIG_SYS_UBOOT_BASE		0xfff90000
#define CONFIG_SYS_UBOOT_START		0x00800000

/*
 * High Level Configuration Options (easy to change)
 */

#include <asm/arch/orion5x.h>
/*
 * CLKs configurations
 */

/*
 * Board-specific values for Orion5x MPP low level init:
 * - MPPs 12 to 15 are SATA LEDs (mode 5)
 * - Others are GPIO/unused (mode 3 for MPP0, mode 5 for
 *   MPP16 to MPP19, mode 0 for others
 */

#define ORION5X_MPP0_7		0x00000003
#define ORION5X_MPP8_15		0x55550000
#define ORION5X_MPP16_23	0x00005555

/*
 * Board-specific values for Orion5x GPIO low level init:
 * - GPIO3 is input (RTC interrupt)
 * - GPIO16 is Power LED control (0 = on, 1 = off)
 * - GPIO17 is Power LED source select (0 = CPLD, 1 = GPIO16)
 * - GPIO18 is Power Button status (0 = Released, 1 = Pressed)
 * - GPIO19 is SATA disk power toggle (toggles on 0-to-1)
 * - GPIO22 is SATA disk power status ()
 * - GPIO23 is supply status for SATA disk ()
 * - GPIO24 is supply control for board (write 1 to power off)
 * Last GPIO is 25, further bits are supposed to be 0.
 * Enable mask has ones for INPUT, 0 for OUTPUT.
 * Default is LED ON, board ON :)
 */

#define ORION5X_GPIO_OUT_ENABLE		0xfef4f0ca
#define ORION5X_GPIO_OUT_VALUE		0x00000000
#define ORION5X_GPIO_IN_POLARITY	0x000000d0

/*
 * NS16550 Configuration
 */

#define CONFIG_SYS_NS16550_SERIAL
#define CONFIG_SYS_NS16550_REG_SIZE	(-4)
#define CONFIG_SYS_NS16550_CLK		CONFIG_SYS_TCLK
#define CONFIG_SYS_NS16550_COM1		ORION5X_UART0_BASE

/*
 * Serial Port configuration
 * The following definitions let you select what serial you want to use
 * for your console driver.
 */

#define CONFIG_SYS_BAUDRATE_TABLE \
	{ 9600, 19200, 38400, 57600, 115200, 230400, 460800, 921600 }

/*
 * FLASH configuration
 */

#define CONFIG_SYS_MAX_FLASH_SECT	11 /* max num of sects on one chip */
#define CONFIG_SYS_FLASH_BASE		0xfff80000

/* auto boot */

#define	CONFIG_SYS_CBSIZE	1024	/* Console I/O Buff Size */

/*
 * Network
 */

#ifdef CONFIG_CMD_NET
#define CONFIG_MVGBE_PORTS	{1}		/* enable port 0 only */
#define CONFIG_SKIP_LOCAL_MAC_RANDOMIZATION	/* don't randomize MAC */
#define CONFIG_PHY_BASE_ADR	0x8
#endif

/*
 * IDE
 */
#ifdef CONFIG_IDE
#define __io
/* Data, registers and alternate blocks are at the same offset */
/* Each 8-bit ATA register is aligned to a 4-bytes address */
/* Controller supports 48-bits LBA addressing */
#define CONFIG_LBA48
/* A single bus, a single device */
/* ATA registers base is at SATA controller base */
/* ATA bus 0 is orion5x port 1 on ED Mini V2 */
/* end of IDE defines */
#endif /* CMD_IDE */

/*
 * Common USB/EHCI configuration
 */
#ifdef CONFIG_CMD_USB
#define ORION5X_USB20_HOST_PORT_BASE ORION5X_USB20_PORT0_BASE
#endif /* CONFIG_CMD_USB */

/*
 * I2C related stuff
 */
#ifdef CONFIG_CMD_I2C
#define CONFIG_I2C_MVTWSI_BASE0		ORION5X_TWSI_BASE
#endif

/*
 *  Environment variables configurations
 */

/* Enable command line editing */

/* provide extensive help */

/* additions for new relocation code, must be added to all boards */
#define CONFIG_SYS_SDRAM_BASE		0
#define CONFIG_SYS_INIT_SP_ADDR	\
	(CONFIG_SYS_SDRAM_BASE + 0x1000 - GENERATED_GBL_DATA_SIZE)

#endif /* _CONFIG_EDMINIV2_H */
