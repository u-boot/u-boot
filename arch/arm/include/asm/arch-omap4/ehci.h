/* SPDX-License-Identifier: GPL-2.0 */
/*
 * OMAP EHCI port support
 * Based on LINUX KERNEL
 * drivers/usb/host/ehci-omap.c and drivers/mfd/omap-usb-host.c
 *
 * Copyright (C) 2011 Texas Instruments Incorporated - https://www.ti.com
 * Author: Govindraj R <govindraj.raja@ti.com>
 */

#ifndef _OMAP4_EHCI_H_
#define _OMAP4_EHCI_H_

#define OMAP_EHCI_BASE				(OMAP44XX_L4_CORE_BASE + 0x64C00)
#define OMAP_UHH_BASE				(OMAP44XX_L4_CORE_BASE + 0x64000)
#define OMAP_USBTLL_BASE			(OMAP44XX_L4_CORE_BASE + 0x62000)

/* UHH, TLL and opt clocks */
#define CM_L3INIT_HSUSBHOST_CLKCTRL		0x4A009358UL

#define HSUSBHOST_CLKCTRL_CLKSEL_UTMI_P1_MASK	BIT(24)

/* TLL Register Set */
#define OMAP_USBTLL_SYSCONFIG_SIDLEMODE		BIT(3)
#define OMAP_USBTLL_SYSCONFIG_ENAWAKEUP		BIT(2)
#define OMAP_USBTLL_SYSCONFIG_SOFTRESET		BIT(1)
#define OMAP_USBTLL_SYSCONFIG_CACTIVITY		BIT(8)
#define OMAP_USBTLL_SYSSTATUS_RESETDONE		1

#define OMAP_UHH_SYSCONFIG_SOFTRESET		1
#define OMAP_UHH_SYSSTATUS_EHCI_RESETDONE	BIT(2)
#define OMAP_UHH_SYSCONFIG_NOIDLE		BIT(2)
#define OMAP_UHH_SYSCONFIG_NOSTDBY		BIT(4)

#define OMAP_UHH_SYSCONFIG_VAL	(OMAP_UHH_SYSCONFIG_NOIDLE | \
					OMAP_UHH_SYSCONFIG_NOSTDBY)

#endif /* _OMAP4_EHCI_H_ */
