/*
 * (C) Copyright 2007
 * Michael Schwingen, michael@schwingen.org
 *
 * hardware register definitions for the AcTux-4 board.
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

#ifndef _ACTUX4_HW_H
#define _ACTUX4_HW_H

/*
 * GPIO settings
 */
#define CONFIG_SYS_GPIO_USBINTA		0
#define CONFIG_SYS_GPIO_USBINTB		1
#define CONFIG_SYS_GPIO_USBINTC		2
#define CONFIG_SYS_GPIO_nPWRON			3	/* Out */
#define CONFIG_SYS_GPIO_I2C_SCL		4
#define CONFIG_SYS_GPIO_I2C_SDA		5
#define CONFIG_SYS_GPIO_PCI_INTB		6
#define CONFIG_SYS_GPIO_BUTTON1		7
#define CONFIG_SYS_GPIO_LED1			8	/* Out */
#define CONFIG_SYS_GPIO_RTCINT			9
#define CONFIG_SYS_GPIO_LED2			10	/* Out */
#define CONFIG_SYS_GPIO_PCI_INTA		11
#define CONFIG_SYS_GPIO_IORST			12	/* Out */
#define CONFIG_SYS_GPIO_LED3			13	/* Out */
#define CONFIG_SYS_GPIO_PCI_CLK		14	/* Out */
#define CONFIG_SYS_GPIO_EXTBUS_CLK		15	/* Out */

#endif
