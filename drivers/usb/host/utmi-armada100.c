/*
 * (C) Copyright 2012
 * eInfochips Ltd. <www.einfochips.com>
 * Written-by: Ajay Bhargav <ajay.bhargav@einfochips.com>
 *
 * (C) Copyright 2009
 * Marvell Semiconductor <www.marvell.com>
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301 USA
 */

#include <common.h>
#include <asm/io.h>
#include <usb.h>
#include <asm/arch/cpu.h>
#include <asm/arch/armada100.h>
#include <asm/arch/utmi-armada100.h>

static int utmi_phy_init(void)
{
	struct armd1usb_phy_reg *phy_regs =
		(struct armd1usb_phy_reg *)UTMI_PHY_BASE;
	int timeout;

	setbits_le32(&phy_regs->utmi_ctrl, INPKT_DELAY_SOF | PLL_PWR_UP);
	udelay(1000);
	setbits_le32(&phy_regs->utmi_ctrl, PHY_PWR_UP);

	clrbits_le32(&phy_regs->utmi_pll, PLL_FBDIV_MASK | PLL_REFDIV_MASK);
	setbits_le32(&phy_regs->utmi_pll, N_DIVIDER << PLL_FBDIV | M_DIVIDER);

	setbits_le32(&phy_regs->utmi_tx, PHSEL_VAL << CK60_PHSEL);

	/* Calibrate pll */
	timeout = 10000;
	while (--timeout && ((readl(&phy_regs->utmi_pll) & PLL_READY) == 0))
		;
	if (!timeout)
		return -1;

	udelay(200);
	setbits_le32(&phy_regs->utmi_pll, VCOCAL_START);
	udelay(400);
	clrbits_le32(&phy_regs->utmi_pll, VCOCAL_START);

	udelay(200);
	setbits_le32(&phy_regs->utmi_tx, RCAL_START);
	udelay(400);
	clrbits_le32(&phy_regs->utmi_tx, RCAL_START);

	timeout = 10000;
	while (--timeout && ((readl(&phy_regs->utmi_pll) & PLL_READY) == 0))
		;
	if (!timeout)
		return -1;

	return 0;
}

/*
 * Initialize USB host controller's UTMI Physical interface
 */
int utmi_init(void)
{
	struct armd1mpmu_registers *mpmu_regs =
		(struct armd1mpmu_registers *)ARMD1_MPMU_BASE;

	struct armd1apmu_registers *apmu_regs =
		(struct armd1apmu_registers *)ARMD1_APMU_BASE;

	/* Turn on 26Mhz ref clock for UTMI PLL */
	setbits_le32(&mpmu_regs->acgr, APB2_26M_EN | AP_26M);

	/* USB Clock reset */
	writel(USB_SPH_AXICLK_EN, &apmu_regs->usbcrc);
	writel(USB_SPH_AXICLK_EN | USB_SPH_AXI_RST, &apmu_regs->usbcrc);

	/* Initialize UTMI transceiver */
	return utmi_phy_init();
}
