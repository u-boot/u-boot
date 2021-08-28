/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2017 Álvaro Fernández Rojas <noltari@gmail.com>
 */

#ifndef __CONFIG_BMIPS_BCM6358_H
#define __CONFIG_BMIPS_BCM6358_H

#include <linux/sizes.h>

/* CPU */
#define CONFIG_SYS_MIPS_TIMER_FREQ	150000000

/* RAM */
#define CONFIG_SYS_SDRAM_BASE		0x80000000

/* USB */
#define CONFIG_EHCI_DESC_BIG_ENDIAN
#define CONFIG_EHCI_MMIO_BIG_ENDIAN
#define CONFIG_SYS_OHCI_SWAP_REG_ACCESS
#define CONFIG_SYS_USB_OHCI_MAX_ROOT_PORTS 2
#if defined(CONFIG_USB_OHCI_HCD)
#define CONFIG_USB_OHCI_NEW
#endif /* CONFIG_USB_OHCI_HCD */

/* U-Boot */

#if defined(CONFIG_BMIPS_BOOT_RAM)
#define CONFIG_SYS_INIT_SP_OFFSET	SZ_8K
#endif

#define CONFIG_SYS_FLASH_BASE			0xbe000000
#define CONFIG_SYS_FLASH_EMPTY_INFO
#define CONFIG_SYS_MAX_FLASH_BANKS_DETECT	1

#endif /* __CONFIG_BMIPS_BCM6358_H */
