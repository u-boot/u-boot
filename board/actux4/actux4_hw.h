/*
 * (C) Copyright 2007
 * Michael Schwingen, michael@schwingen.org
 *
 * hardware register definitions for the AcTux-4 board.
 *
 * SPDX-License-Identifier:	GPL-2.0+
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
