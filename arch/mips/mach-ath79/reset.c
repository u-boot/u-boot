/*
 * Copyright (C) 2015-2016 Wills Wang <wills.wang@live.com>
 *
 * SPDX-License-Identifier: GPL-2.0+
 */

#include <common.h>
#include <asm/errno.h>
#include <asm/io.h>
#include <asm/addrspace.h>
#include <asm/types.h>
#include <mach/ath79.h>
#include <mach/ar71xx_regs.h>

void _machine_restart(void)
{
	void __iomem *base;
	u32 reg = 0;

	base = map_physmem(AR71XX_RESET_BASE, AR71XX_RESET_SIZE,
			   MAP_NOCACHE);
	if (soc_is_ar71xx())
		reg = AR71XX_RESET_REG_RESET_MODULE;
	else if (soc_is_ar724x())
		reg = AR724X_RESET_REG_RESET_MODULE;
	else if (soc_is_ar913x())
		reg = AR913X_RESET_REG_RESET_MODULE;
	else if (soc_is_ar933x())
		reg = AR933X_RESET_REG_RESET_MODULE;
	else if (soc_is_ar934x())
		reg = AR934X_RESET_REG_RESET_MODULE;
	else if (soc_is_qca953x())
		reg = QCA953X_RESET_REG_RESET_MODULE;
	else if (soc_is_qca955x())
		reg = QCA955X_RESET_REG_RESET_MODULE;
	else if (soc_is_qca956x())
		reg = QCA956X_RESET_REG_RESET_MODULE;
	else
		puts("Reset register not defined for this SOC\n");

	if (reg)
		setbits_be32(base + reg, AR71XX_RESET_FULL_CHIP);

	while (1)
		/* NOP */;
}

u32 ath79_get_bootstrap(void)
{
	void __iomem *base;
	u32 reg = 0;

	base = map_physmem(AR71XX_RESET_BASE, AR71XX_RESET_SIZE,
			   MAP_NOCACHE);
	if (soc_is_ar933x())
		reg = AR933X_RESET_REG_BOOTSTRAP;
	else if (soc_is_ar934x())
		reg = AR934X_RESET_REG_BOOTSTRAP;
	else if (soc_is_qca953x())
		reg = QCA953X_RESET_REG_BOOTSTRAP;
	else if (soc_is_qca955x())
		reg = QCA955X_RESET_REG_BOOTSTRAP;
	else if (soc_is_qca956x())
		reg = QCA956X_RESET_REG_BOOTSTRAP;
	else
		puts("Bootstrap register not defined for this SOC\n");

	if (reg)
		return readl(base + reg);

	return 0;
}

static int eth_init_ar933x(void)
{
	void __iomem *rregs = map_physmem(AR71XX_RESET_BASE, AR71XX_RESET_SIZE,
					  MAP_NOCACHE);
	void __iomem *pregs = map_physmem(AR71XX_PLL_BASE, AR71XX_PLL_SIZE,
					  MAP_NOCACHE);
	void __iomem *gregs = map_physmem(AR933X_GMAC_BASE, AR933X_GMAC_SIZE,
					  MAP_NOCACHE);
	const u32 mask = AR933X_RESET_GE0_MAC | AR933X_RESET_GE0_MDIO |
			 AR933X_RESET_GE1_MAC | AR933X_RESET_GE1_MDIO |
			 AR933X_RESET_ETH_SWITCH |
			 AR933X_RESET_ETH_SWITCH_ANALOG;

	/* Clear MDIO slave EN bit. */
	clrbits_be32(rregs + AR933X_RESET_REG_BOOTSTRAP, BIT(17));
	mdelay(10);

	/* Get Atheros S26 PHY out of reset. */
	clrsetbits_be32(pregs + AR933X_PLL_SWITCH_CLOCK_CONTROL_REG,
			0x1f, 0x10);
	mdelay(10);

	setbits_be32(rregs + AR933X_RESET_REG_RESET_MODULE, mask);
	mdelay(10);
	clrbits_be32(rregs + AR933X_RESET_REG_RESET_MODULE, mask);
	mdelay(10);

	/* Configure AR93xx GMAC register. */
	clrsetbits_be32(gregs + AR933X_GMAC_REG_ETH_CFG,
			AR933X_ETH_CFG_MII_GE0_MASTER |
			AR933X_ETH_CFG_MII_GE0_SLAVE,
			AR933X_ETH_CFG_MII_GE0_SLAVE);
	return 0;
}

static int eth_init_ar934x(void)
{
	void __iomem *rregs = map_physmem(AR71XX_RESET_BASE, AR71XX_RESET_SIZE,
					  MAP_NOCACHE);
	void __iomem *pregs = map_physmem(AR71XX_PLL_BASE, AR71XX_PLL_SIZE,
					  MAP_NOCACHE);
	void __iomem *gregs = map_physmem(AR934X_GMAC_BASE, AR934X_GMAC_SIZE,
					  MAP_NOCACHE);
	const u32 mask = AR934X_RESET_GE0_MAC | AR934X_RESET_GE0_MDIO |
			 AR934X_RESET_GE1_MAC | AR934X_RESET_GE1_MDIO |
			 AR934X_RESET_ETH_SWITCH_ANALOG;
	u32 reg;

	reg = readl(rregs + AR934X_RESET_REG_BOOTSTRAP);
	if (reg & AR934X_BOOTSTRAP_REF_CLK_40)
		writel(0x570, pregs + AR934X_PLL_SWITCH_CLOCK_CONTROL_REG);
	else
		writel(0x271, pregs + AR934X_PLL_SWITCH_CLOCK_CONTROL_REG);
	writel(BIT(26) | BIT(25), pregs + AR934X_PLL_ETH_XMII_CONTROL_REG);

	setbits_be32(rregs + AR934X_RESET_REG_RESET_MODULE, mask);
	mdelay(1);
	clrbits_be32(rregs + AR934X_RESET_REG_RESET_MODULE, mask);
	mdelay(1);

	/* Configure AR934x GMAC register. */
	writel(AR934X_ETH_CFG_RGMII_GMAC0, gregs + AR934X_GMAC_REG_ETH_CFG);
	return 0;
}

static int eth_init_qca953x(void)
{
	void __iomem *rregs = map_physmem(AR71XX_RESET_BASE, AR71XX_RESET_SIZE,
					  MAP_NOCACHE);
	const u32 mask = QCA953X_RESET_GE0_MAC | QCA953X_RESET_GE0_MDIO |
			 QCA953X_RESET_GE1_MAC | QCA953X_RESET_GE1_MDIO |
			 QCA953X_RESET_ETH_SWITCH_ANALOG |
			 QCA953X_RESET_ETH_SWITCH;

	setbits_be32(rregs + AR934X_RESET_REG_RESET_MODULE, mask);
	mdelay(1);
	clrbits_be32(rregs + AR934X_RESET_REG_RESET_MODULE, mask);
	mdelay(1);

	return 0;
}

int ath79_eth_reset(void)
{
	/*
	 * Un-reset ethernet. DM still doesn't have any notion of reset
	 * framework, so we do it by hand here.
	 */
	if (soc_is_ar933x())
		return eth_init_ar933x();
	if (soc_is_ar934x())
		return eth_init_ar934x();
	if (soc_is_qca953x())
		return eth_init_qca953x();

	return -EINVAL;
}

static int usb_reset_ar933x(void __iomem *reset_regs)
{
	/* Ungate the USB block */
	setbits_be32(reset_regs + AR933X_RESET_REG_RESET_MODULE,
		     AR933X_RESET_USBSUS_OVERRIDE);
	mdelay(1);
	clrbits_be32(reset_regs + AR933X_RESET_REG_RESET_MODULE,
		     AR933X_RESET_USB_HOST);
	mdelay(1);
	clrbits_be32(reset_regs + AR933X_RESET_REG_RESET_MODULE,
		     AR933X_RESET_USB_PHY);
	mdelay(1);

	return 0;
}

static int usb_reset_ar934x(void __iomem *reset_regs)
{
	/* Ungate the USB block */
	setbits_be32(reset_regs + AR934X_RESET_REG_RESET_MODULE,
		     AR934X_RESET_USBSUS_OVERRIDE);
	mdelay(1);
	clrbits_be32(reset_regs + AR934X_RESET_REG_RESET_MODULE,
		     AR934X_RESET_USB_PHY);
	mdelay(1);
	clrbits_be32(reset_regs + AR934X_RESET_REG_RESET_MODULE,
		     AR934X_RESET_USB_PHY_ANALOG);
	mdelay(1);
	clrbits_be32(reset_regs + AR934X_RESET_REG_RESET_MODULE,
		     AR934X_RESET_USB_HOST);
	mdelay(1);

	return 0;
}

static int usb_reset_qca953x(void __iomem *reset_regs)
{
	void __iomem *pregs = map_physmem(AR71XX_PLL_BASE, AR71XX_PLL_SIZE,
					  MAP_NOCACHE);

	clrsetbits_be32(pregs + QCA953X_PLL_SWITCH_CLOCK_CONTROL_REG,
			0xf00, 0x200);
	mdelay(10);

	/* Ungate the USB block */
	setbits_be32(reset_regs + QCA953X_RESET_REG_RESET_MODULE,
		     QCA953X_RESET_USBSUS_OVERRIDE);
	mdelay(1);
	clrbits_be32(reset_regs + QCA953X_RESET_REG_RESET_MODULE,
		     QCA953X_RESET_USB_PHY);
	mdelay(1);
	clrbits_be32(reset_regs + QCA953X_RESET_REG_RESET_MODULE,
		     QCA953X_RESET_USB_PHY_ANALOG);
	mdelay(1);
	clrbits_be32(reset_regs + QCA953X_RESET_REG_RESET_MODULE,
		     QCA953X_RESET_USB_HOST);
	mdelay(1);
	clrbits_be32(reset_regs + QCA953X_RESET_REG_RESET_MODULE,
		     QCA953X_RESET_USB_PHY_PLL_PWD_EXT);
	mdelay(1);

	return 0;
}

int ath79_usb_reset(void)
{
	void __iomem *usbc_regs = map_physmem(AR71XX_USB_CTRL_BASE,
					      AR71XX_USB_CTRL_SIZE,
					      MAP_NOCACHE);
	void __iomem *reset_regs = map_physmem(AR71XX_RESET_BASE,
					       AR71XX_RESET_SIZE,
					       MAP_NOCACHE);
	/*
	 * Turn on the Buff and Desc swap bits.
	 * NOTE: This write into an undocumented register in mandatory to
	 *       get the USB controller operational in BigEndian mode.
	 */
	writel(0xf0000, usbc_regs + AR71XX_USB_CTRL_REG_CONFIG);

	if (soc_is_ar933x())
		return usb_reset_ar933x(reset_regs);
	if (soc_is_ar934x())
		return usb_reset_ar934x(reset_regs);
	if (soc_is_qca953x())
		return usb_reset_qca953x(reset_regs);

	return -EINVAL;
}
