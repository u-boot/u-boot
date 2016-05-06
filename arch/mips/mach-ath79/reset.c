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

u32 get_bootstrap(void)
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

	return -EINVAL;
}
