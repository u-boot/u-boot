/*
 * Copyright (c) 2005 freescale semiconductor
 * Copyright (c) 2005 MontaVista Software
 * Copyright (c) 2008 Excito Elektronik i Sk=E5ne AB
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

#ifndef _EHCI_FSL_H
#define _EHCI_FSL_H

/* Global offsets */
#define FSL_SKIP_PCI		0x100

/* offsets for the non-ehci registers in the FSL SOC USB controller */
#define FSL_SOC_USB_ULPIVP	0x170
#define FSL_SOC_USB_PORTSC1	0x184
#define PORT_PTS_MSK		(3 << 30)
#define PORT_PTS_UTMI		(0 << 30)
#define PORT_PTS_ULPI		(2 << 30)
#define PORT_PTS_SERIAL		(3 << 30)
#define PORT_PTS_PTW		(1 << 28)

/* USBMODE Register bits */
#define CM_IDLE			(0 << 0)
#define CM_RESERVED		(1 << 0)
#define CM_DEVICE		(2 << 0)
#define CM_HOST			(3 << 0)
#define USBMODE_RESERVED_2	(0 << 2)
#define SLOM			(1 << 3)
#define SDIS			(1 << 4)

/* CONTROL Register bits */
#define ULPI_INT_EN		(1 << 0)
#define WU_INT_EN		(1 << 1)
#define USB_EN			(1 << 2)
#define LSF_EN			(1 << 3)
#define KEEP_OTG_ON		(1 << 4)
#define OTG_PORT		(1 << 5)
#define REFSEL_12MHZ		(0 << 6)
#define REFSEL_16MHZ		(1 << 6)
#define REFSEL_48MHZ		(2 << 6)
#define PLL_RESET		(1 << 8)
#define UTMI_PHY_EN		(1 << 9)
#define PHY_CLK_SEL_UTMI	(0 << 10)
#define PHY_CLK_SEL_ULPI	(1 << 10)
#define CLKIN_SEL_USB_CLK	(0 << 11)
#define CLKIN_SEL_USB_CLK2	(1 << 11)
#define CLKIN_SEL_SYS_CLK	(2 << 11)
#define CLKIN_SEL_SYS_CLK2	(3 << 11)
#define RESERVED_18		(0 << 13)
#define RESERVED_17		(0 << 14)
#define RESERVED_16		(0 << 15)
#define WU_INT			(1 << 16)
#define PHY_CLK_VALID		(1 << 17)

#define FSL_SOC_USB_PORTSC2	0x188
#define FSL_SOC_USB_USBMODE	0x1a8
#define FSL_SOC_USB_SNOOP1	0x400	/* NOTE: big-endian */
#define FSL_SOC_USB_SNOOP2	0x404	/* NOTE: big-endian */
#define FSL_SOC_USB_AGECNTTHRSH	0x408	/* NOTE: big-endian */
#define FSL_SOC_USB_PRICTRL	0x40c	/* NOTE: big-endian */
#define FSL_SOC_USB_SICTRL	0x410	/* NOTE: big-endian */
#define FSL_SOC_USB_CTRL	0x500	/* NOTE: big-endian */
#define SNOOP_SIZE_2GB		0x1e

/* System Clock Control Register */
#define MPC83XX_SCCR_USB_MASK		0x00f00000
#define MPC83XX_SCCR_USB_DRCM_11	0x00300000
#define MPC83XX_SCCR_USB_DRCM_01	0x00100000
#define MPC83XX_SCCR_USB_DRCM_10	0x00200000

#endif /* _EHCI_FSL_H */
