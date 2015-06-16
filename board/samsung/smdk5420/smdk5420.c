/*
 * Copyright (C) 2013 Samsung Electronics
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <fdtdec.h>
#include <errno.h>
#include <asm/io.h>
#include <asm/gpio.h>
#include <asm/arch/cpu.h>
#include <asm/arch/board.h>
#include <asm/arch/power.h>
#include <asm/arch/system.h>
#include <asm/arch/pinmux.h>
#include <asm/arch/dp_info.h>
#include <asm/arch/xhci-exynos.h>
#include <power/tps65090_pmic.h>
#include <i2c.h>
#include <lcd.h>
#include <mmc.h>
#include <parade.h>
#include <spi.h>
#include <usb.h>
#include <dwc3-uboot.h>
#include <samsung-usb-phy-uboot.h>

DECLARE_GLOBAL_DATA_PTR;

int exynos_init(void)
{
	return 0;
}

#ifdef CONFIG_LCD
static int has_edp_bridge(void)
{
	int node;

	node = fdtdec_next_compatible(gd->fdt_blob, 0, COMPAT_PARADE_PS8625);

	/* No node for bridge in device tree. */
	if (node <= 0)
		return 0;

	/* Default is with bridge ic */
	return 1;
}

void exynos_lcd_power_on(void)
{
	int ret;

#ifdef CONFIG_POWER_TPS65090
	ret = tps65090_init();
	if (ret < 0) {
		printf("%s: tps65090_init() failed\n", __func__);
		return;
	}

	tps65090_fet_enable(6);
#endif

	mdelay(5);

	if (has_edp_bridge())
		if (parade_init(gd->fdt_blob))
			printf("%s: ps8625_init() failed\n", __func__);
}

void exynos_backlight_on(unsigned int onoff)
{
#ifdef CONFIG_POWER_TPS65090
	tps65090_fet_enable(1);
#endif
}
#endif

int board_get_revision(void)
{
	return 0;
}

#ifdef CONFIG_USB_DWC3
static struct dwc3_device dwc3_device_data = {
	.maximum_speed = USB_SPEED_SUPER,
	.base = 0x12400000,
	.dr_mode = USB_DR_MODE_PERIPHERAL,
	.index = 0,
};

int usb_gadget_handle_interrupts(void)
{
	dwc3_uboot_handle_interrupt(0);
	return 0;
}

int board_usb_init(int index, enum usb_init_type init)
{
	struct exynos_usb3_phy *phy = (struct exynos_usb3_phy *)
		samsung_get_base_usb3_phy();

	if (!phy) {
		error("usb3 phy not supported");
		return -ENODEV;
	}

	set_usbdrd_phy_ctrl(POWER_USB_DRD_PHY_CTRL_EN);
	exynos5_usb3_phy_init(phy);

	return dwc3_uboot_init(&dwc3_device_data);
}
#endif
#ifdef CONFIG_SET_DFU_ALT_INFO
char *get_dfu_alt_system(char *interface, char *devstr)
{
	return getenv("dfu_alt_system");
}

char *get_dfu_alt_boot(char *interface, char *devstr)
{
	struct mmc *mmc;
	char *alt_boot;
	int dev_num;

	dev_num = simple_strtoul(devstr, NULL, 10);

	mmc = find_mmc_device(dev_num);
	if (!mmc)
		return NULL;

	if (mmc_init(mmc))
		return NULL;

	if (IS_SD(mmc))
		alt_boot = CONFIG_DFU_ALT_BOOT_SD;
	else
		alt_boot = CONFIG_DFU_ALT_BOOT_EMMC;

	return alt_boot;
}
#endif
