/*
 * (C) Copyright 2007
 * Michael Schwingen, michael@schwingen.org
 *
 * hardware register definitions for the AcTux-1 board.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef _ACTUX1_HW_H
#define _ACTUX1_HW_H

/* 0 = LED off,1 = green, 2 = red, 3 = orange */
#define ACTUX1_LED1(a)	writeb((a),   IXP425_EXP_BUS_CS7_BASE_PHYS + 0)
#define ACTUX1_LED2(a)	writeb((a),   IXP425_EXP_BUS_CS7_BASE_PHYS + 1)
#define ACTUX1_LED3(a)	writeb((a),   IXP425_EXP_BUS_CS7_BASE_PHYS + 2)
#define ACTUX1_LED4(a)	writeb((a)^3, IXP425_EXP_BUS_CS7_BASE_PHYS + 3)
#define ACTUX1_LED5(a)	writeb((a)^3, IXP425_EXP_BUS_CS7_BASE_PHYS + 4)
#define ACTUX1_LED6(a)	writeb((a)^3, IXP425_EXP_BUS_CS7_BASE_PHYS + 5)
#define ACTUX1_LED7(a)	writeb((a)^3, IXP425_EXP_BUS_CS7_BASE_PHYS + 6)
#define ACTUX1_HS(a)	writeb((a),   IXP425_EXP_BUS_CS7_BASE_PHYS + 7)
#define ACTUX1_HS_DCD	0x01
#define ACTUX1_HS_DSR	0x02

#define ACTUX1_DBG_PORT	IXP425_EXP_BUS_CS5_BASE_PHYS
#define ACTUX1_BOARDREL	(readb(IXP425_EXP_BUS_CS6_BASE_PHYS) & 0x0F)

/* GPIO settings */
#define CONFIG_SYS_GPIO_PCI1_INTA		2
#define CONFIG_SYS_GPIO_PCI2_INTA		3
#define CONFIG_SYS_GPIO_I2C_SDA		4
#define CONFIG_SYS_GPIO_I2C_SCL		5
#define CONFIG_SYS_GPIO_DBGJUMPER		9
#define CONFIG_SYS_GPIO_BUTTON1		10
#define CONFIG_SYS_GPIO_DBGSENSE		11
#define CONFIG_SYS_GPIO_DTR			12
#define CONFIG_SYS_GPIO_IORST			13	/* Out */
#define CONFIG_SYS_GPIO_PCI_CLK		14	/* Out */
#define CONFIG_SYS_GPIO_EXTBUS_CLK		15	/* Out */

#endif
