/*
 * Sunxi usb-phy code
 *
 * Copyright (C) 2015 Hans de Goede <hdegoede@redhat.com>
 * Copyright (C) 2014 Roman Byshko <rbyshko@gmail.com>
 *
 * Based on code from
 * Allwinner Technology Co., Ltd. <www.allwinnertech.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <asm/arch/clock.h>
#include <asm/arch/cpu.h>
#include <asm/arch/usb_phy.h>
#include <asm/gpio.h>
#include <asm/io.h>
#include <errno.h>

#define SUNXI_USB_PMU_IRQ_ENABLE	0x800
#ifdef CONFIG_MACH_SUN8I_A33
#define SUNXI_USB_CSR			0x410
#else
#define SUNXI_USB_CSR			0x404
#endif
#define SUNXI_USB_PASSBY_EN		1

#define SUNXI_EHCI_AHB_ICHR8_EN		(1 << 10)
#define SUNXI_EHCI_AHB_INCR4_BURST_EN	(1 << 9)
#define SUNXI_EHCI_AHB_INCRX_ALIGN_EN	(1 << 8)
#define SUNXI_EHCI_ULPI_BYPASS_EN	(1 << 0)

#define REG_PHY_UNK_H3			0x420
#define REG_PMU_UNK_H3			0x810

static struct sunxi_usb_phy {
	int usb_rst_mask;
	int gpio_vbus;
	int gpio_vbus_det;
	int gpio_id_det;
	int id;
	int init_count;
	int power_on_count;
	int base;
} sunxi_usb_phy[] = {
	{
		.usb_rst_mask = CCM_USB_CTRL_PHY0_RST | CCM_USB_CTRL_PHY0_CLK,
		.id = 0,
		.base = SUNXI_USB0_BASE,
	},
	{
		.usb_rst_mask = CCM_USB_CTRL_PHY1_RST | CCM_USB_CTRL_PHY1_CLK,
		.id = 1,
		.base = SUNXI_USB1_BASE,
	},
#if CONFIG_SUNXI_USB_PHYS >= 3
	{
		.usb_rst_mask = CCM_USB_CTRL_PHY2_RST | CCM_USB_CTRL_PHY2_CLK,
		.id = 2,
		.base = SUNXI_USB2_BASE,
	},
#endif
#if CONFIG_SUNXI_USB_PHYS >= 4
	{
		.usb_rst_mask = CCM_USB_CTRL_PHY3_RST | CCM_USB_CTRL_PHY3_CLK,
		.id = 3,
		.base = SUNXI_USB3_BASE,
	}
#endif
};

static int get_vbus_gpio(int index)
{
	switch (index) {
	case 0: return sunxi_name_to_gpio(CONFIG_USB0_VBUS_PIN);
	case 1: return sunxi_name_to_gpio(CONFIG_USB1_VBUS_PIN);
	case 2: return sunxi_name_to_gpio(CONFIG_USB2_VBUS_PIN);
	}
	return -EINVAL;
}

static int get_vbus_detect_gpio(int index)
{
	switch (index) {
	case 0: return sunxi_name_to_gpio(CONFIG_USB0_VBUS_DET);
	}
	return -EINVAL;
}

static int get_id_detect_gpio(int index)
{
	switch (index) {
	case 0: return sunxi_name_to_gpio(CONFIG_USB0_ID_DET);
	}
	return -EINVAL;
}

static void usb_phy_write(struct sunxi_usb_phy *phy, int addr,
			  int data, int len)
{
	int j = 0, usbc_bit = 0;
	void *dest = (void *)SUNXI_USB0_BASE + SUNXI_USB_CSR;

#ifdef CONFIG_MACH_SUN8I_A33
	/* CSR needs to be explicitly initialized to 0 on A33 */
	writel(0, dest);
#endif

	usbc_bit = 1 << (phy->id * 2);
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

#if defined CONFIG_MACH_SUN8I_H3
static void sunxi_usb_phy_config(struct sunxi_usb_phy *phy)
{
	if (phy->id == 0)
		clrbits_le32(SUNXI_USBPHY_BASE + REG_PHY_UNK_H3, 0x01);

	clrbits_le32(phy->base + REG_PMU_UNK_H3, 0x02);
}
#else
static void sunxi_usb_phy_config(struct sunxi_usb_phy *phy)
{
	/* The following comments are machine
	 * translated from Chinese, you have been warned!
	 */

	/* Regulation 45 ohms */
	if (phy->id == 0)
		usb_phy_write(phy, 0x0c, 0x01, 1);

	/* adjust PHY's magnitude and rate */
	usb_phy_write(phy, 0x20, 0x14, 5);

	/* threshold adjustment disconnect */
#if defined CONFIG_MACH_SUN5I || defined CONFIG_MACH_SUN7I
	usb_phy_write(phy, 0x2a, 2, 2);
#else
	usb_phy_write(phy, 0x2a, 3, 2);
#endif

	return;
}
#endif

static void sunxi_usb_phy_passby(struct sunxi_usb_phy *phy, int enable)
{
	unsigned long bits = 0;
	void *addr;

	addr = (void *)phy->base + SUNXI_USB_PMU_IRQ_ENABLE;

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

void sunxi_usb_phy_enable_squelch_detect(int index, int enable)
{
	struct sunxi_usb_phy *phy = &sunxi_usb_phy[index];

	usb_phy_write(phy, 0x3c, enable ? 0 : 2, 2);
}

void sunxi_usb_phy_init(int index)
{
	struct sunxi_usb_phy *phy = &sunxi_usb_phy[index];
	struct sunxi_ccm_reg *ccm = (struct sunxi_ccm_reg *)SUNXI_CCM_BASE;

	phy->init_count++;
	if (phy->init_count != 1)
		return;

	setbits_le32(&ccm->usb_clk_cfg, phy->usb_rst_mask);

	sunxi_usb_phy_config(phy);

	if (phy->id != 0)
		sunxi_usb_phy_passby(phy, SUNXI_USB_PASSBY_EN);
}

void sunxi_usb_phy_exit(int index)
{
	struct sunxi_usb_phy *phy = &sunxi_usb_phy[index];
	struct sunxi_ccm_reg *ccm = (struct sunxi_ccm_reg *)SUNXI_CCM_BASE;

	phy->init_count--;
	if (phy->init_count != 0)
		return;

	if (phy->id != 0)
		sunxi_usb_phy_passby(phy, !SUNXI_USB_PASSBY_EN);

	clrbits_le32(&ccm->usb_clk_cfg, phy->usb_rst_mask);
}

void sunxi_usb_phy_power_on(int index)
{
	struct sunxi_usb_phy *phy = &sunxi_usb_phy[index];

	phy->power_on_count++;
	if (phy->power_on_count != 1)
		return;

	if (phy->gpio_vbus >= 0)
		gpio_set_value(phy->gpio_vbus, 1);
}

void sunxi_usb_phy_power_off(int index)
{
	struct sunxi_usb_phy *phy = &sunxi_usb_phy[index];

	phy->power_on_count--;
	if (phy->power_on_count != 0)
		return;

	if (phy->gpio_vbus >= 0)
		gpio_set_value(phy->gpio_vbus, 0);
}

int sunxi_usb_phy_power_is_on(int index)
{
	struct sunxi_usb_phy *phy = &sunxi_usb_phy[index];

	return phy->power_on_count > 0;
}

int sunxi_usb_phy_vbus_detect(int index)
{
	struct sunxi_usb_phy *phy = &sunxi_usb_phy[index];
	int err, retries = 3;

	if (phy->gpio_vbus_det < 0)
		return phy->gpio_vbus_det;

	err = gpio_get_value(phy->gpio_vbus_det);
	/*
	 * Vbus may have been provided by the board and just been turned of
	 * some milliseconds ago on reset, what we're measuring then is a
	 * residual charge on Vbus, sleep a bit and try again.
	 */
	while (err > 0 && retries--) {
		mdelay(100);
		err = gpio_get_value(phy->gpio_vbus_det);
	}

	return err;
}

int sunxi_usb_phy_id_detect(int index)
{
	struct sunxi_usb_phy *phy = &sunxi_usb_phy[index];

	if (phy->gpio_id_det < 0)
		return phy->gpio_id_det;

	return gpio_get_value(phy->gpio_id_det);
}

int sunxi_usb_phy_probe(void)
{
	struct sunxi_ccm_reg *ccm = (struct sunxi_ccm_reg *)SUNXI_CCM_BASE;
	struct sunxi_usb_phy *phy;
	int i, ret = 0;

	for (i = 0; i < CONFIG_SUNXI_USB_PHYS; i++) {
		phy = &sunxi_usb_phy[i];

		phy->gpio_vbus = get_vbus_gpio(i);
		if (phy->gpio_vbus >= 0) {
			ret = gpio_request(phy->gpio_vbus, "usb_vbus");
			if (ret)
				return ret;
			ret = gpio_direction_output(phy->gpio_vbus, 0);
			if (ret)
				return ret;
		}

		phy->gpio_vbus_det = get_vbus_detect_gpio(i);
		if (phy->gpio_vbus_det >= 0) {
			ret = gpio_request(phy->gpio_vbus_det, "usb_vbus_det");
			if (ret)
				return ret;
			ret = gpio_direction_input(phy->gpio_vbus_det);
			if (ret)
				return ret;
		}

		phy->gpio_id_det = get_id_detect_gpio(i);
		if (phy->gpio_id_det >= 0) {
			ret = gpio_request(phy->gpio_id_det, "usb_id_det");
			if (ret)
				return ret;
			ret = gpio_direction_input(phy->gpio_id_det);
			if (ret)
				return ret;
			sunxi_gpio_set_pull(phy->gpio_id_det,
					    SUNXI_GPIO_PULL_UP);
		}
	}

	setbits_le32(&ccm->usb_clk_cfg, CCM_USB_CTRL_PHYGATE);

	return 0;
}

int sunxi_usb_phy_remove(void)
{
	struct sunxi_ccm_reg *ccm = (struct sunxi_ccm_reg *)SUNXI_CCM_BASE;
	struct sunxi_usb_phy *phy;
	int i;

	clrbits_le32(&ccm->usb_clk_cfg, CCM_USB_CTRL_PHYGATE);

	for (i = 0; i < CONFIG_SUNXI_USB_PHYS; i++) {
		phy = &sunxi_usb_phy[i];

		if (phy->gpio_vbus >= 0)
			gpio_free(phy->gpio_vbus);

		if (phy->gpio_vbus_det >= 0)
			gpio_free(phy->gpio_vbus_det);

		if (phy->gpio_id_det >= 0)
			gpio_free(phy->gpio_id_det);
	}

	return 0;
}
