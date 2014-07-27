/*
 * Copyright (C) 2014 Roman Byshko
 *
 * Roman Byshko <rbyshko@gmail.com>
 *
 * Based on code from
 * Allwinner Technology Co., Ltd. <www.allwinnertech.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <asm/arch/clock.h>
#include <asm/gpio.h>
#include <asm/io.h>
#include <common.h>
#include "ehci.h"

#define SUNXI_USB1_IO_BASE		0x01c14000
#define SUNXI_USB2_IO_BASE		0x01c1c000

#define SUNXI_USB_PMU_IRQ_ENABLE	0x800
#define SUNXI_USB_CSR			0x01c13404
#define SUNXI_USB_PASSBY_EN		1

#define SUNXI_EHCI_AHB_ICHR8_EN		(1 << 10)
#define SUNXI_EHCI_AHB_INCR4_BURST_EN	(1 << 9)
#define SUNXI_EHCI_AHB_INCRX_ALIGN_EN	(1 << 8)
#define SUNXI_EHCI_ULPI_BYPASS_EN	(1 << 0)

static struct sunxi_ehci_hcd {
	struct usb_hcd *hcd;
	int usb_rst_mask;
	int ahb_clk_mask;
	int gpio_vbus;
	void *csr;
	int irq;
	int id;
} sunxi_echi_hcd[] = {
	{
		.usb_rst_mask = CCM_USB_CTRL_PHY1_RST,
		.ahb_clk_mask = 1 << AHB_GATE_OFFSET_USB_EHCI0,
		.gpio_vbus = CONFIG_SUNXI_USB_VBUS0_GPIO,
		.csr = (void *)SUNXI_USB_CSR,
		.irq = 39,
		.id = 1,
	},
#if (CONFIG_USB_MAX_CONTROLLER_COUNT > 1)
	{
		.usb_rst_mask = CCM_USB_CTRL_PHY2_RST,
		.ahb_clk_mask = 1 << AHB_GATE_OFFSET_USB_EHCI1,
		.gpio_vbus = CONFIG_SUNXI_USB_VBUS1_GPIO,
		.csr = (void *)SUNXI_USB_CSR,
		.irq = 40,
		.id = 2,
	}
#endif
};

static int enabled_hcd_count;

static void *get_io_base(int hcd_id)
{
	if (hcd_id == 1)
		return (void *)SUNXI_USB1_IO_BASE;
	else if (hcd_id == 2)
		return (void *)SUNXI_USB2_IO_BASE;
	else
		return NULL;
}

static void usb_phy_write(struct sunxi_ehci_hcd *sunxi_ehci, int addr,
			  int data, int len)
{
	int j = 0, usbc_bit = 0;
	void *dest = sunxi_ehci->csr;

	usbc_bit = 1 << (sunxi_ehci->id * 2);
	for (j = 0; j < len; j++) {
		/* set the bit address to be written */
		clrbits_le32(dest, 0xff << 8);
		setbits_le32(dest, (addr + j) << 8);

		clrbits_le32(dest, usbc_bit);
		/* set data bit */
		if (data & 0x1)
			setbits_le32(dest, 1 << 7);
		else
			clrbits_le32(dest, 1 << 7);

		setbits_le32(dest, usbc_bit);

		clrbits_le32(dest, usbc_bit);

		data >>= 1;
	}
}

static void sunxi_usb_phy_init(struct sunxi_ehci_hcd *sunxi_ehci)
{
	/* The following comments are machine
	 * translated from Chinese, you have been warned!
	 */

	/* adjust PHY's magnitude and rate */
	usb_phy_write(sunxi_ehci, 0x20, 0x14, 5);

	/* threshold adjustment disconnect */
#ifdef CONFIG_SUN4I
	usb_phy_write(sunxi_ehci, 0x2a, 3, 2);
#else
	usb_phy_write(sunxi_ehci, 0x2a, 2, 2);
#endif

	return;
}

static void sunxi_usb_passby(struct sunxi_ehci_hcd *sunxi_ehci, int enable)
{
	unsigned long bits = 0;
	void *addr = get_io_base(sunxi_ehci->id) + SUNXI_USB_PMU_IRQ_ENABLE;

	bits = SUNXI_EHCI_AHB_ICHR8_EN |
		SUNXI_EHCI_AHB_INCR4_BURST_EN |
		SUNXI_EHCI_AHB_INCRX_ALIGN_EN |
		SUNXI_EHCI_ULPI_BYPASS_EN;

	if (enable)
		setbits_le32(addr, bits);
	else
		clrbits_le32(addr, bits);

	return;
}

static void sunxi_ehci_enable(struct sunxi_ehci_hcd *sunxi_ehci)
{
	struct sunxi_ccm_reg *ccm = (struct sunxi_ccm_reg *)SUNXI_CCM_BASE;

	setbits_le32(&ccm->usb_clk_cfg, sunxi_ehci->usb_rst_mask);
	setbits_le32(&ccm->ahb_gate0, sunxi_ehci->ahb_clk_mask);

	sunxi_usb_phy_init(sunxi_ehci);

	sunxi_usb_passby(sunxi_ehci, SUNXI_USB_PASSBY_EN);

	gpio_direction_output(sunxi_ehci->gpio_vbus, 1);
}

static void sunxi_ehci_disable(struct sunxi_ehci_hcd *sunxi_ehci)
{
	struct sunxi_ccm_reg *ccm = (struct sunxi_ccm_reg *)SUNXI_CCM_BASE;

	gpio_direction_output(sunxi_ehci->gpio_vbus, 0);

	sunxi_usb_passby(sunxi_ehci, !SUNXI_USB_PASSBY_EN);

	clrbits_le32(&ccm->ahb_gate0, sunxi_ehci->ahb_clk_mask);
	clrbits_le32(&ccm->usb_clk_cfg, sunxi_ehci->usb_rst_mask);
}

int ehci_hcd_init(int index, enum usb_init_type init, struct ehci_hccr **hccr,
		struct ehci_hcor **hcor)
{
	struct sunxi_ccm_reg *ccm = (struct sunxi_ccm_reg *)SUNXI_CCM_BASE;
	struct sunxi_ehci_hcd *sunxi_ehci = &sunxi_echi_hcd[index];

	/* enable common PHY only once */
	if (index == 0)
		setbits_le32(&ccm->usb_clk_cfg, CCM_USB_CTRL_PHYGATE);

	sunxi_ehci_enable(sunxi_ehci);

	*hccr = get_io_base(sunxi_ehci->id);

	*hcor = (struct ehci_hcor *)((uint32_t) *hccr
				+ HC_LENGTH(ehci_readl(&(*hccr)->cr_capbase)));

	debug("sunxi-ehci: init hccr %x and hcor %x hc_length %d\n",
	      (uint32_t)*hccr, (uint32_t)*hcor,
	      (uint32_t)HC_LENGTH(ehci_readl(&(*hccr)->cr_capbase)));

	enabled_hcd_count++;

	return 0;
}

int ehci_hcd_stop(int index)
{
	struct sunxi_ccm_reg *ccm = (struct sunxi_ccm_reg *)SUNXI_CCM_BASE;
	struct sunxi_ehci_hcd *sunxi_ehci = &sunxi_echi_hcd[index];

	sunxi_ehci_disable(sunxi_ehci);

	/* disable common PHY only once, for the last enabled hcd */
	if (enabled_hcd_count == 1)
		clrbits_le32(&ccm->usb_clk_cfg, CCM_USB_CTRL_PHYGATE);

	enabled_hcd_count--;

	return 0;
}
