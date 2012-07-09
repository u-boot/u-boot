/*
 * Copyright (C) 2012 Samsung Electronics
 * Donghwa Lee <dh09.lee@samsung.com>
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <common.h>
#include <asm/io.h>
#include <asm/arch/system.h>

static void exynos5_set_usbhost_mode(unsigned int mode)
{
	struct exynos5_sysreg *sysreg =
		(struct exynos5_sysreg *)samsung_get_base_sysreg();

	/* Setting USB20PHY_CONFIG register to USB 2.0 HOST link */
	if (mode == USB20_PHY_CFG_HOST_LINK_EN) {
		setbits_le32(&sysreg->usb20phy_cfg,
				USB20_PHY_CFG_HOST_LINK_EN);
	} else {
		clrbits_le32(&sysreg->usb20phy_cfg,
				USB20_PHY_CFG_HOST_LINK_EN);
	}
}

void set_usbhost_mode(unsigned int mode)
{
	if (cpu_is_exynos5())
		exynos5_set_usbhost_mode(mode);
}

static void exynos4_set_system_display(void)
{
	struct exynos4_sysreg *sysreg =
	    (struct exynos4_sysreg *)samsung_get_base_sysreg();
	unsigned int cfg = 0;

	/*
	 * system register path set
	 * 0: MIE/MDNIE
	 * 1: FIMD Bypass
	 */
	cfg = readl(&sysreg->display_ctrl);
	cfg |= (1 << 1);
	writel(cfg, &sysreg->display_ctrl);
}

void set_system_display_ctrl(void)
{
	if (cpu_is_exynos4())
		exynos4_set_system_display();
}
