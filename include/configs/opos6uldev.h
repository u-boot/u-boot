/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2017 Armadeus Systems
 *
 * Configuration settings for the OPOS6ULDev board
 */

#ifndef __OPOS6ULDEV_CONFIG_H
#define __OPOS6ULDEV_CONFIG_H

#include "mx6_common.h"

#ifdef CONFIG_SPL
#include "imx6_spl.h"
#endif

/* Miscellaneous configurable options */
#define CONFIG_STANDALONE_LOAD_ADDR	CONFIG_SYS_LOAD_ADDR

/* Physical Memory Map */
#define CONFIG_SYS_SDRAM_BASE		MMDC0_ARB_BASE_ADDR
#define CONFIG_SYS_INIT_RAM_ADDR	IRAM_BASE_ADDR
#define CONFIG_SYS_INIT_RAM_SIZE	IRAM_SIZE

/* USB */
#ifdef CONFIG_USB_EHCI_MX6
#define CONFIG_MXC_USB_PORTSC		(PORT_PTS_UTMI | PORT_PTS_PTW)
#define CONFIG_MXC_USB_FLAGS		0
#endif

/* LCD */
#define MXS_LCDIF_BASE MX6UL_LCDIF1_BASE_ADDR

#define CONFIG_ROOTPATH         "/tftpboot/opos6ul-root"

#endif /* __OPOS6ULDEV_CONFIG_H */
