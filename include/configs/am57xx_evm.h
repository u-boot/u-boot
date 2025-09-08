/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2014
 * Texas Instruments Incorporated.
 * Felipe Balbi <balbi@ti.com>
 *
 * Configuration settings for the TI Beagle x15 board.
 * See ti_omap5_common.h for omap5 common settings.
 */

#ifndef __CONFIG_AM57XX_EVM_H
#define __CONFIG_AM57XX_EVM_H

#include <linux/sizes.h>

#define CFG_SYS_NS16550_COM1		UART1_BASE	/* Base EVM has UART0 */
#define CFG_SYS_NS16550_COM2		UART2_BASE	/* UART2 */
#define CFG_SYS_NS16550_COM3		UART3_BASE	/* UART3 */

/* CPSW Ethernet */

#include <configs/ti_omap5_common.h>
#endif /* __CONFIG_AM57XX_EVM_H */
