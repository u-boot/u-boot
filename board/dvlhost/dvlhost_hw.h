/*
 * (C) Copyright 2009
 * Michael Schwingen, michael@schwingen.org
 *
 * hardware register definitions for the
 * dLAN200 AV Wireless G ("dvlhost") board.
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
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
