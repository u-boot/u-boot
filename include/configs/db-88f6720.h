/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2016 Stefan Roese <sr@denx.de>
 */

#ifndef _CONFIG_DB_88F6720_H
#define _CONFIG_DB_88F6720_H

/*
 * High Level Configuration Options (easy to change)
 */

/*
 * TEXT_BASE needs to be below 16MiB, since this area is scrubbed
 * for DDR ECC byte filling in the SPL before loading the main
 * U-Boot into it.
 */

/* I2C */
#define CONFIG_I2C_MVTWSI_BASE0		MVEBU_TWSI_BASE

/* USB/EHCI configuration */
#define CONFIG_USB_MAX_CONTROLLER_COUNT 3

/* Environment in SPI NOR flash */

#define PHY_ANEG_TIMEOUT	8000	/* PHY needs a longer aneg time */

/*
 * mv-common.h should be defined after CMD configs since it used them
 * to enable certain macros
 */
#include "mv-common.h"

/* SPL */
/* Defines for SPL */
#define CONFIG_SPL_MAX_SIZE		((128 << 10) - (CONFIG_SPL_TEXT_BASE - 0x40000000))

#define CONFIG_SPL_BSS_START_ADDR	(0x40000000 + (128 << 10))
#define CONFIG_SPL_BSS_MAX_SIZE		(16 << 10)

#ifdef CONFIG_SPL_BUILD
#define CONFIG_SYS_MALLOC_SIMPLE
#endif

#define CONFIG_SPL_STACK		(0x40000000 + ((192 - 16) << 10))
#define CONFIG_SPL_BOOTROM_SAVE		(CONFIG_SPL_STACK + 4)

#endif /* _CONFIG_DB_88F6720_H */
