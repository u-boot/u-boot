/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2024 Kontron Electronics GmbH
 *
 * Configuration settings for the Kontron OSM-S/BL i.MX93 boards and modules.
 */
#ifndef __KONTRON_MX93_CONFIG_H
#define __KONTRON_MX93_CONFIG_H

#include <asm/arch/imx-regs.h>
#include <linux/sizes.h>

/* RAM */
#define PHYS_SDRAM			0x80000000
#define PHYS_SDRAM_SIZE			(SZ_2G)
#define CFG_SYS_SDRAM_BASE		PHYS_SDRAM

#define CFG_SYS_INIT_RAM_ADDR	0x80000000
#define CFG_SYS_INIT_RAM_SIZE	SZ_2M

/* Board and environment settings */

#ifdef CONFIG_USB_EHCI_HCD
#define CFG_MXC_USB_PORTSC		(PORT_PTS_UTMI | PORT_PTS_PTW)
#define CFG_MXC_USB_FLAGS		0
#endif

#define WDOG_BASE_ADDR          WDG3_BASE_ADDR

#endif /* __KONTRON_MX93_CONFIG_H */
