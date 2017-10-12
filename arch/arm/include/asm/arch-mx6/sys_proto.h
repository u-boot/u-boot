/*
 * (C) Copyright 2009
 * Stefano Babic, DENX Software Engineering, sbabic@denx.de.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <asm/mach-imx/sys_proto.h>

#define USBPHY_PWD		0x00000000

#define USBPHY_PWD_RXPWDRX	(1 << 20) /* receiver block power down */

#define is_usbotg_phy_active(void) (!(readl(USB_PHY0_BASE_ADDR + USBPHY_PWD) & \
				   USBPHY_PWD_RXPWDRX))

int imx6_pcie_toggle_power(void);
int imx6_pcie_toggle_reset(void);
