/*
 * OMAP EHCI port support
 * Based on LINUX KERNEL
 * drivers/usb/host/ehci-omap.c and drivers/mfd/omap-usb-host.c
 *
 * Copyright (C) 2011 Texas Instruments Incorporated - http://www.ti.com
 * Author: Govindraj R <govindraj.raja@ti.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2  of
 * the License as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _OMAP4_EHCI_H_
#define _OMAP4_EHCI_H_

#define OMAP_EHCI_BASE				(OMAP44XX_L4_CORE_BASE + 0x64C00)
#define OMAP_UHH_BASE				(OMAP44XX_L4_CORE_BASE + 0x64000)
#define OMAP_USBTLL_BASE			(OMAP44XX_L4_CORE_BASE + 0x62000)

/* UHH, TLL and opt clocks */
#define CM_L3INIT_HSUSBHOST_CLKCTRL		0x4A009358UL

#define HSUSBHOST_CLKCTRL_CLKSEL_UTMI_P1_MASK	(1 << 24)

/* TLL Register Set */
#define OMAP_USBTLL_SYSCONFIG_SIDLEMODE		(1 << 3)
#define OMAP_USBTLL_SYSCONFIG_ENAWAKEUP		(1 << 2)
#define OMAP_USBTLL_SYSCONFIG_SOFTRESET		(1 << 1)
#define OMAP_USBTLL_SYSCONFIG_CACTIVITY		(1 << 8)
#define OMAP_USBTLL_SYSSTATUS_RESETDONE		1

#define OMAP_UHH_SYSCONFIG_SOFTRESET		1
#define OMAP_UHH_SYSSTATUS_EHCI_RESETDONE	(1 << 2)
#define OMAP_UHH_SYSCONFIG_NOIDLE		(1 << 2)
#define OMAP_UHH_SYSCONFIG_NOSTDBY		(1 << 4)

#define OMAP_UHH_SYSCONFIG_VAL	(OMAP_UHH_SYSCONFIG_NOIDLE | \
					OMAP_UHH_SYSCONFIG_NOSTDBY)

#endif /* _OMAP4_EHCI_H_ */
