/*
 * (C) Copyright 2008
 * Matthias Fuchs, esd gmbh, matthias.fuchs@esd-electronics.com
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#define SDR0_USB0		0x0320     /* USB Control Register */

#define CFG_GPIO0_EP_EEP	(0x80000000 >> 23)       /* GPIO0_23 */
#define CFG_GPIO1_DCF77		(0x80000000 >> (42-32))  /* GPIO1_42 */

#define CFG_GPIO1_IORSTN	(0x80000000 >> (55-32))  /* GPIO1_55 */
#define CFG_GPIO1_IORST2N	(0x80000000 >> (47-32))  /* GPIO1_47 */

#define CFG_GPIO1_HWVER_MASK	0x000000f0 /* GPIO1_56-59 */
#define CFG_GPIO1_HWVER_SHIFT	4
#define CFG_GPIO1_LEDUSR1	0x00000008 /* GPIO1_60 */
#define CFG_GPIO1_LEDUSR2	0x00000004 /* GPIO1_61 */
#define CFG_GPIO1_LEDPOST	0x00000002 /* GPIO1_62 */
#define CFG_GPIO1_LEDDU		0x00000001 /* GPIO1_63 */

#define CPLD_VERSION_MASK	0x0f
#define PWR_INT_FLAG		0x80
#define PWR_RDY			0x10

#define CPLD_IRQ		(32+30)

#define PCI_VENDOR_ID_ESDGMBH	0x12fe
#define PCI_DEVICE_ID_DU440	0x0444
