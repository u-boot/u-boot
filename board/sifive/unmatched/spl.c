// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2020-2021 SiFive, Inc
 *
 * Authors:
 *   Pragnesh Patel <pragnesh.patel@sifive.com>
 */

#include <init.h>
#include <spl.h>
#include <misc.h>
#include <log.h>
#include <linux/delay.h>
#include <linux/io.h>
#include <asm/gpio.h>
#include <asm/arch/gpio.h>
#include <asm/arch/spl.h>

#define UBRDG_RESET	SIFIVE_GENERIC_GPIO_NR(0, 7)
#define ULPI_RESET	SIFIVE_GENERIC_GPIO_NR(0, 9)
#define UHUB_RESET	SIFIVE_GENERIC_GPIO_NR(0, 11)
#define GEM_PHY_RESET	SIFIVE_GENERIC_GPIO_NR(0, 12)

#define MODE_SELECT_REG		0x1000
#define MODE_SELECT_SPI		0x6
#define MODE_SELECT_SD		0xb
#define MODE_SELECT_MASK	GENMASK(3, 0)

static inline int spl_reset_device_by_gpio(const char *label, int pin, int low_width)
{
	int ret;

	ret = gpio_request(pin, label);
	if (ret) {
		debug("%s gpio request failed: %d\n", label, ret);
		return ret;
	}

	ret = gpio_direction_output(pin, 1);
	if (ret) {
		debug("%s gpio direction set failed: %d\n", label, ret);
		return ret;
	}

	udelay(1);

	gpio_set_value(pin, 0);
	udelay(low_width);
	gpio_set_value(pin, 1);

	return ret;
}

static inline int spl_gemgxl_init(void)
{
	int ret;
	/*
	 * GEMGXL init VSC8541 PHY reset sequence;
	 * leave pull-down active for 2ms
	 */
	udelay(2000);
	ret = spl_reset_device_by_gpio("gem_phy_reset", GEM_PHY_RESET, 1);
	mdelay(15);

	return ret;
}

static inline int spl_usb_pcie_bridge_init(void)
{
	return spl_reset_device_by_gpio("usb_pcie_bridge_reset", UBRDG_RESET, 3000);
}

static inline int spl_usb_hub_init(void)
{
	return spl_reset_device_by_gpio("usb_hub_reset", UHUB_RESET, 100);
}

static inline int spl_ulpi_init(void)
{
	return spl_reset_device_by_gpio("ulpi_reset", ULPI_RESET, 1);
}

int spl_board_init_f(void)
{
	int ret;

	ret = spl_soc_init();
	if (ret) {
		debug("HiFive Unmatched FU740 SPL init failed: %d\n", ret);
		goto end;
	}

	ret = spl_gemgxl_init();
	if (ret) {
		debug("Gigabit ethernet PHY (VSC8541) init failed: %d\n", ret);
		goto end;
	}

	ret = spl_usb_pcie_bridge_init();
	if (ret) {
		debug("USB Bridge (ASM1042A) init failed: %d\n", ret);
		goto end;
	}

	ret = spl_usb_hub_init();
	if (ret) {
		debug("USB Hub (ASM1074) init failed: %d\n", ret);
		goto end;
	}

	ret = spl_ulpi_init();
	if (ret) {
		debug("USB 2.0 PHY (USB3320C) init failed: %d\n", ret);
		goto end;
	}

end:
	return ret;
}

u32 spl_boot_device(void)
{
	u32 mode_select = readl((void *)MODE_SELECT_REG);
	u32 boot_device = mode_select & MODE_SELECT_MASK;

	switch (boot_device) {
	case MODE_SELECT_SPI:
		return BOOT_DEVICE_SPI;
	case MODE_SELECT_SD:
		return BOOT_DEVICE_MMC1;
	default:
		debug("Unsupported boot device 0x%x but trying MMC1\n",
		      boot_device);
		return BOOT_DEVICE_MMC1;
	}
}

#ifdef CONFIG_SPL_LOAD_FIT
int board_fit_config_name_match(const char *name)
{
	/* boot using first FIT config */
	return 0;
}
#endif
