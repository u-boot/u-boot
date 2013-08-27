/*
 * Copyright (C) 2013 Texas Instruments Incorporated - http://www.ti.com*
 * Author: Govindraj R <govindraj.raja@ti.com>
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

#ifndef _EHCI_H
#define _EHCI_H

#define OMAP_EHCI_BASE				(OMAP54XX_L4_CORE_BASE + 0x64C00)
#define OMAP_UHH_BASE				(OMAP54XX_L4_CORE_BASE + 0x64000)
#define OMAP_USBTLL_BASE			(OMAP54XX_L4_CORE_BASE + 0x62000)

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

#endif /* _EHCI_H */
