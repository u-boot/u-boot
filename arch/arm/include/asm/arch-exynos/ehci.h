/*
 * SAMSUNG EXYNOS USB HOST EHCI Controller
 *
 * Copyright (C) 2012 Samsung Electronics Co.Ltd
 *	Vivek Gautam <gautam.vivek@samsung.com>
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301 USA
 */

#ifndef __ASM_ARM_ARCH_EHCI_H__
#define __ASM_ARM_ARCH_EHCI_H__

#define CLK_24MHZ		5

#define HOST_CTRL0_PHYSWRSTALL			(1 << 31)
#define HOST_CTRL0_COMMONON_N			(1 << 9)
#define HOST_CTRL0_SIDDQ			(1 << 6)
#define HOST_CTRL0_FORCESLEEP			(1 << 5)
#define HOST_CTRL0_FORCESUSPEND			(1 << 4)
#define HOST_CTRL0_WORDINTERFACE		(1 << 3)
#define HOST_CTRL0_UTMISWRST			(1 << 2)
#define HOST_CTRL0_LINKSWRST			(1 << 1)
#define HOST_CTRL0_PHYSWRST			(1 << 0)

#define HOST_CTRL0_FSEL_MASK			(7 << 16)

#define EHCICTRL_ENAINCRXALIGN			(1 << 29)
#define EHCICTRL_ENAINCR4			(1 << 28)
#define EHCICTRL_ENAINCR8			(1 << 27)
#define EHCICTRL_ENAINCR16			(1 << 26)

/* Register map for PHY control */
struct exynos_usb_phy {
	unsigned int usbphyctrl0;
	unsigned int usbphytune0;
	unsigned int reserved1[2];
	unsigned int hsicphyctrl1;
	unsigned int hsicphytune1;
	unsigned int reserved2[2];
	unsigned int hsicphyctrl2;
	unsigned int hsicphytune2;
	unsigned int reserved3[2];
	unsigned int ehcictrl;
	unsigned int ohcictrl;
	unsigned int usbotgsys;
	unsigned int reserved4;
	unsigned int usbotgtune;
};

/* Switch on the VBUS power. */
int board_usb_vbus_init(void);

#endif /* __ASM_ARM_ARCH_EHCI_H__ */
