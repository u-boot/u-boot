/*
 * Sunxi usb-controller code shared between the ehci and musb controllers
 *
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
#include <asm/arch/cpu.h>
#include <asm/arch/usbc.h>
#include <asm/gpio.h>
#include <asm/io.h>
#include <common.h>
#ifdef CONFIG_AXP152_POWER
#include <axp152.h>
#endif
#ifdef CONFIG_AXP209_POWER
#include <axp209.h>
#endif
#ifdef CONFIG_AXP221_POWER
#include <axp221.h>
#endif

#define SUNXI_USB_PMU_IRQ_ENABLE	0x800
#define SUNXI_USB_CSR			0x404
#define SUNXI_USB_PASSBY_EN		1

#define SUNXI_EHCI_AHB_ICHR8_EN		(1 << 10)
#define SUNXI_EHCI_AHB_INCR4_BURST_EN	(1 << 9)
#define SUNXI_EHCI_AHB_INCRX_ALIGN_EN	(1 << 8)
#define SUNXI_EHCI_ULPI_BYPASS_EN	(1 << 0)

static struct sunxi_usbc_hcd {
	struct usb_hcd *hcd;
	int usb_rst_mask;
	int ahb_clk_mask;
	int gpio_vbus;
	int irq;
	int id;
} sunxi_usbc_hcd[] = {
	{
		.usb_rst_mask = CCM_USB_CTRL_PHY0_RST | CCM_USB_CTRL_PHY0_CLK,
		.ahb_clk_mask = 1 << AHB_GATE_OFFSET_USB0,
#if defined CONFIG_MACH_SUN6I || defined CONFIG_MACH_SUN8I
		.irq = 71,
#else
		.irq = 38,
#endif
		.id = 0,
	},
	{
		.usb_rst_mask = CCM_USB_CTRL_PHY1_RST | CCM_USB_CTRL_PHY1_CLK,
		.ahb_clk_mask = 1 << AHB_GATE_OFFSET_USB_EHCI0,
#if defined CONFIG_MACH_SUN6I || defined CONFIG_MACH_SUN8I
		.irq = 72,
#else
		.irq = 39,
#endif
		.id = 1,
	},
#if (CONFIG_USB_MAX_CONTROLLER_COUNT > 1)
	{
		.usb_rst_mask = CCM_USB_CTRL_PHY2_RST | CCM_USB_CTRL_PHY2_CLK,
		.ahb_clk_mask = 1 << AHB_GATE_OFFSET_USB_EHCI1,
#ifdef CONFIG_MACH_SUN6I
		.irq = 74,
#else
		.irq = 40,
#endif
		.id = 2,
	}
#endif
};

static int enabled_hcd_count;

static bool use_axp_drivebus(int index)
{
	return index == 0 &&
	       strcmp(CONFIG_USB0_VBUS_PIN, "axp_drivebus") == 0;
}

void *sunxi_usbc_get_io_base(int index)
{
	switch (index) {
	case 0:
		return (void *)SUNXI_USB0_BASE;
	case 1:
		return (void *)SUNXI_USB1_BASE;
	case 2:
		return (void *)SUNXI_USB2_BASE;
	default:
		return NULL;
	}
}

static int get_vbus_gpio(int index)
{
	if (use_axp_drivebus(index))
		return -1;

	switch (index) {
	case 0: return sunxi_name_to_gpio(CONFIG_USB0_VBUS_PIN);
	case 1: return sunxi_name_to_gpio(CONFIG_USB1_VBUS_PIN);
	case 2: return sunxi_name_to_gpio(CONFIG_USB2_VBUS_PIN);
	}
	return -1;
}

static void usb_phy_write(struct sunxi_usbc_hcd *sunxi_usbc, int addr,
			  int data, int len)
{
	int j = 0, usbc_bit = 0;
	void *dest = sunxi_usbc_get_io_base(0) + SUNXI_USB_CSR;

	usbc_bit = 1 << (sunxi_usbc->id * 2);
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

static void sunxi_usb_phy_init(struct sunxi_usbc_hcd *sunxi_usbc)
{
	/* The following comments are machine
	 * translated from Chinese, you have been warned!
	 */

	/* Regulation 45 ohms */
	if (sunxi_usbc->id == 0)
		usb_phy_write(sunxi_usbc, 0x0c, 0x01, 1);

	/* adjust PHY's magnitude and rate */
	usb_phy_write(sunxi_usbc, 0x20, 0x14, 5);

	/* threshold adjustment disconnect */
#if defined CONFIG_MACH_SUN4I || defined CONFIG_MACH_SUN6I
	usb_phy_write(sunxi_usbc, 0x2a, 3, 2);
#else
	usb_phy_write(sunxi_usbc, 0x2a, 2, 2);
#endif

	return;
}

static void sunxi_usb_passby(struct sunxi_usbc_hcd *sunxi_usbc, int enable)
{
	unsigned long bits = 0;
	void *addr = sunxi_usbc_get_io_base(sunxi_usbc->id) +
		     SUNXI_USB_PMU_IRQ_ENABLE;

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

int sunxi_usbc_request_resources(int index)
{
	struct sunxi_usbc_hcd *sunxi_usbc = &sunxi_usbc_hcd[index];

	sunxi_usbc->gpio_vbus = get_vbus_gpio(index);
	if (sunxi_usbc->gpio_vbus != -1)
		return gpio_request(sunxi_usbc->gpio_vbus, "usbc_vbus");

	return 0;
}

int sunxi_usbc_free_resources(int index)
{
	struct sunxi_usbc_hcd *sunxi_usbc = &sunxi_usbc_hcd[index];

	if (sunxi_usbc->gpio_vbus != -1)
		return gpio_free(sunxi_usbc->gpio_vbus);

	return 0;
}

void sunxi_usbc_enable(int index)
{
	struct sunxi_usbc_hcd *sunxi_usbc = &sunxi_usbc_hcd[index];
	struct sunxi_ccm_reg *ccm = (struct sunxi_ccm_reg *)SUNXI_CCM_BASE;

	/* enable common PHY only once */
	if (enabled_hcd_count == 0)
		setbits_le32(&ccm->usb_clk_cfg, CCM_USB_CTRL_PHYGATE);

	setbits_le32(&ccm->usb_clk_cfg, sunxi_usbc->usb_rst_mask);
	setbits_le32(&ccm->ahb_gate0, sunxi_usbc->ahb_clk_mask);
#if defined CONFIG_MACH_SUN6I || defined CONFIG_MACH_SUN8I
	setbits_le32(&ccm->ahb_reset0_cfg, sunxi_usbc->ahb_clk_mask);
#endif

	sunxi_usb_phy_init(sunxi_usbc);

	if (sunxi_usbc->id != 0)
		sunxi_usb_passby(sunxi_usbc, SUNXI_USB_PASSBY_EN);

	enabled_hcd_count++;
}

void sunxi_usbc_disable(int index)
{
	struct sunxi_usbc_hcd *sunxi_usbc = &sunxi_usbc_hcd[index];
	struct sunxi_ccm_reg *ccm = (struct sunxi_ccm_reg *)SUNXI_CCM_BASE;

	if (sunxi_usbc->id != 0)
		sunxi_usb_passby(sunxi_usbc, !SUNXI_USB_PASSBY_EN);

#if defined CONFIG_MACH_SUN6I || defined CONFIG_MACH_SUN8I
	clrbits_le32(&ccm->ahb_reset0_cfg, sunxi_usbc->ahb_clk_mask);
#endif
	clrbits_le32(&ccm->ahb_gate0, sunxi_usbc->ahb_clk_mask);
	clrbits_le32(&ccm->usb_clk_cfg, sunxi_usbc->usb_rst_mask);

	/* disable common PHY only once, for the last enabled hcd */
	if (enabled_hcd_count == 1)
		clrbits_le32(&ccm->usb_clk_cfg, CCM_USB_CTRL_PHYGATE);

	enabled_hcd_count--;
}

void sunxi_usbc_vbus_enable(int index)
{
	struct sunxi_usbc_hcd *sunxi_usbc = &sunxi_usbc_hcd[index];

#ifdef AXP_DRIVEBUS
	if (use_axp_drivebus(index))
		axp_drivebus_enable();
#endif
	if (sunxi_usbc->gpio_vbus != -1)
		gpio_direction_output(sunxi_usbc->gpio_vbus, 1);
}

void sunxi_usbc_vbus_disable(int index)
{
	struct sunxi_usbc_hcd *sunxi_usbc = &sunxi_usbc_hcd[index];

#ifdef AXP_DRIVEBUS
	if (use_axp_drivebus(index))
		axp_drivebus_disable();
#endif
	if (sunxi_usbc->gpio_vbus != -1)
		gpio_direction_output(sunxi_usbc->gpio_vbus, 0);
}
