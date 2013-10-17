/*
 * (C) Copyright 2007
 * Michael Schwingen, michael@schwingen.org
 *
 * hardware register definitions for the AcTux-3 board.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef _ACTUX3_HW_H
#define _ACTUX3_HW_H

/* 0 = LED off,1 = ON */
#define ACTUX3_LED1_RT(a)	writeb((a), IXP425_EXP_BUS_CS7_BASE_PHYS + 0)
#define ACTUX3_LED1_GN(a)	writeb((a), IXP425_EXP_BUS_CS7_BASE_PHYS + 1)
#define ACTUX3_LED2_RT(a)	writeb((a), IXP425_EXP_BUS_CS7_BASE_PHYS + 2)
#define ACTUX3_LED2_GN(a)	writeb((a), IXP425_EXP_BUS_CS7_BASE_PHYS + 3)
#define ACTUX3_LED3_RT(a)	writeb((a), IXP425_EXP_BUS_CS7_BASE_PHYS + 4)
#define ACTUX3_LED3_GN(a)	writeb((a), IXP425_EXP_BUS_CS7_BASE_PHYS + 5)
#define ACTUX3_LED4_GN(a)	writeb((a)^1, IXP425_EXP_BUS_CS7_BASE_PHYS + 6)
#define ACTUX3_LED5_RT(a)	writeb((a), IXP425_EXP_BUS_CS7_BASE_PHYS + 7)

#define ACTUX3_DBG_PORT		IXP425_EXP_BUS_CS5_BASE_PHYS
#define ACTUX3_BOARDREL		(readb(IXP425_EXP_BUS_CS6_BASE_PHYS) & 0x0F)
#define ACTUX3_OPTION		(readb(IXP425_EXP_BUS_CS6_BASE_PHYS) & 0xF0)

/* GPIO settings */
#define CONFIG_SYS_GPIO_DBGINT			0
#define CONFIG_SYS_GPIO_ETHINT			1
#define CONFIG_SYS_GPIO_ETHRST			2	/* Out */
#define CONFIG_SYS_GPIO_LED5_GN		3	/* Out */
#define CONFIG_SYS_GPIO_LED6_RT		4	/* Out */
#define CONFIG_SYS_GPIO_LED6_GN		5	/* Out */
#define CONFIG_SYS_GPIO_DSR			6	/* Out */
#define CONFIG_SYS_GPIO_DCD			7	/* Out */
#define CONFIG_SYS_GPIO_DBGJUMPER		9
#define CONFIG_SYS_GPIO_BUTTON1		10
#define CONFIG_SYS_GPIO_DBGSENSE		11
#define CONFIG_SYS_GPIO_DTR			12
#define CONFIG_SYS_GPIO_IORST			13	/* Out */
#define CONFIG_SYS_GPIO_PCI_CLK		14	/* Out */
#define CONFIG_SYS_GPIO_EXTBUS_CLK		15	/* Out */

#endif
