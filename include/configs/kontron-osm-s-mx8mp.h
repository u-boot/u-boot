/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2023 Kontron Electronics GmbH
 *
 * Configuration settings for the Kontron OSM-S/BL i.MX8M Plus boards and modules.
 */
#ifndef __KONTRON_MX8MP_CONFIG_H
#define __KONTRON_MX8MP_CONFIG_H

#include <asm/arch/imx-regs.h>
#include <linux/sizes.h>

/* RAM */
#define PHYS_SDRAM			DDR_CSD1_BASE_ADDR
#define PHYS_SDRAM_SIZE			(SZ_2G + SZ_1G)
#define PHYS_SDRAM_2			0x100000000
#define PHYS_SDRAM_2_SIZE		(SZ_1G + SZ_4G)
#define CFG_SYS_SDRAM_BASE		PHYS_SDRAM

#define CFG_SYS_INIT_RAM_ADDR	0x40000000
#define CFG_SYS_INIT_RAM_SIZE	SZ_512K

/* Board and environment settings */

#ifdef CONFIG_USB_EHCI_HCD
#define CFG_MXC_USB_PORTSC		(PORT_PTS_UTMI | PORT_PTS_PTW)
#define CFG_MXC_USB_FLAGS		0
#endif

#endif /* __KONTRON_MX8MP_CONFIG_H */
