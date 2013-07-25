/*
 * (C) Copyright 2009
 * Michael Schwingen, michael@schwingen.org
 *
 * hardware register definitions for the
 * dLAN200 AV Wireless G ("dvlhost") board.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef _DVLHOST_HW_H
#define _DVLHOST_HW_H

/*
 * GPIO settings
 */
#define CONFIG_SYS_GPIO_WDGTRIGGER	0 /* Out */
#define CONFIG_SYS_GPIO_BTN_WLAN	1
#define CONFIG_SYS_GPIO_BTN_PAIRING	6
#define CONFIG_SYS_GPIO_DLAN_PAIRING	7 /* Out */
#define CONFIG_SYS_GPIO_BTN_RESET	9
#define CONFIG_SYS_GPIO_IRQB		10
#define CONFIG_SYS_GPIO_IRQA		11
#define CONFIG_SYS_GPIO_WDG_LED_EN	12 /* Out */
#define CONFIG_SYS_GPIO_PCIRST		13 /* Out */
#define CONFIG_SYS_GPIO_PCI_CLK		14 /* Out */
#define CONFIG_SYS_GPIO_EXTBUS_CLK	15 /* Out */

#define DVLHOST_LED_LATCH	IXP425_EXP_BUS_CS1_BASE_PHYS

#endif
