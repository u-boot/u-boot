/*
 * (C) Copyright 2007
 * Michael Schwingen, michael@schwingen.org
 *
 * hardware register definitions for the AcTux-2 board.
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

#ifndef _ACTUX2_HW_H
#define _ACTUX2_HW_H

/* 0 = LED off,1 = green, 2 = red, 3 = orange */
#define ACTUX2_LED1(a)	writeb((a ? 2 : 0), IXP425_EXP_BUS_CS7_BASE_PHYS + 0)
#define ACTUX2_LED2(a)	writeb((a ? 2 : 0), IXP425_EXP_BUS_CS7_BASE_PHYS + 1)
#define ACTUX2_LED3(a)	writeb((a ? 0 : 2), IXP425_EXP_BUS_CS7_BASE_PHYS + 2)
#define ACTUX2_LED4(a)	writeb((a ? 0 : 2), IXP425_EXP_BUS_CS7_BASE_PHYS + 3)

#define ACTUX2_DBG_PORT	IXP425_EXP_BUS_CS5_BASE_PHYS
#define ACTUX2_BOARDREL	(readb(IXP425_EXP_BUS_CS6_BASE_PHYS) & 0x0F)
#define ACTUX2_OPTION	(readb(IXP425_EXP_BUS_CS6_BASE_PHYS) & 0xF0)

/*
 * GPIO settings
 */
#define CFG_GPIO_DBGINT			0
#define CFG_GPIO_ETHINT			1
#define CFG_GPIO_ETHRST			2	/* Out */
#define CFG_GPIO_LED5_GN		3	/* Out */
#define CFG_GPIO_UNUSED4		4
#define CFG_GPIO_UNUSED5		5
#define CFG_GPIO_DSR			6	/* Out */
#define CFG_GPIO_DCD			7	/* Out */
#define CFG_GPIO_IPAC_INT		8
#define CFG_GPIO_DBGJUMPER		9
#define CFG_GPIO_BUTTON1		10
#define CFG_GPIO_DBGSENSE		11
#define CFG_GPIO_DTR			12
#define CFG_GPIO_IORST			13	/* Out */
#define CFG_GPIO_PCI_CLK		14	/* Out */
#define CFG_GPIO_EXTBUS_CLK		15	/* Out */

#endif
