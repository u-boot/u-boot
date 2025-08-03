/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Common definitions for LPC32XX board configurations
 *
 * Copyright (C) 2011-2015 Vladimir Zapolskiy <vz@mleia.com>
 */

#ifndef _LPC32XX_CONFIG_H
#define _LPC32XX_CONFIG_H

/* Basic CPU architecture */

#if !defined(CFG_SYS_NS16550_CLK)
#define CFG_SYS_NS16550_CLK		13000000
#endif

#define CFG_SYS_BAUDRATE_TABLE	\
		{ 9600, 19200, 38400, 57600, 115200, 230400, 460800 }

/* NOR Flash */

/* USB OHCI */
#if defined(CONFIG_USB_OHCI_LPC32XX)
#define CFG_SYS_USB_OHCI_REGS_BASE		USB_BASE
#endif

#endif /* _LPC32XX_CONFIG_H */
