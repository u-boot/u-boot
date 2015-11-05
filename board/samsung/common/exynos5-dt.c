/*
 * Copyright (C) 2012 Samsung Electronics
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <dm.h>
#include <dwc3-uboot.h>
#include <fdtdec.h>
#include <asm/io.h>
#include <errno.h>
#include <i2c.h>
#include <mmc.h>
#include <netdev.h>
#include <samsung-usb-phy-uboot.h>
#include <spi.h>
#include <usb.h>
#include <video_bridge.h>
#include <asm/gpio.h>
#include <asm/arch/cpu.h>
#include <asm/arch/dwmmc.h>
#include <asm/arch/mmc.h>
#include <asm/arch/pinmux.h>
#include <asm/arch/power.h>
#include <asm/arch/sromc.h>
#include <power/pmic.h>
#include <power/max77686_pmic.h>
#include <power/regulator.h>
#include <power/s2mps11.h>
#include <power/s5m8767.h>
#include <samsung/exynos5-dt-types.h>
#include <samsung/misc.h>
#include <tmu.h>

DECLARE_GLOBAL_DATA_PTR;

static void board_enable_audio_codec(void)
{
	int node, ret;
	struct gpio_desc en_gpio;

	node = fdtdec_next_compatible(gd->fdt_blob, 0,
		COMPAT_SAMSUNG_EXYNOS5_SOUND);
	if (node <= 0)
		return;

	ret = gpio_request_by_name_nodev(gd->fdt_blob, node,
					 "codec-enable-gpio", 0, &en_gpio,
					 GPIOD_IS_OUT | GPIOD_IS_OUT_ACTIVE);
	if (ret == -FDT_ERR_NOTFOUND)
		return;

	/* Turn on the GPIO which connects to the codec's "enable" line. */
	gpio_set_pull(gpio_get_number(&en_gpio), S5P_GPIO_PULL_NONE);

#ifdef CONFIG_SOUND_MAX98095
	/* Enable MAX98095 Codec */
	gpio_request(EXYNOS5_GPIO_X17, "max98095_enable");
	gpio_direction_output(EXYNOS5_GPIO_X17, 1);
	gpio_set_pull(EXYNOS5_GPIO_X17, S5P_GPIO_PULL_NONE);
#endif
}

int exynos_init(void)
{
	board_enable_audio_codec();

	return 0;
}

static int exynos_set_regulator(const char *name, uint uv)
{
	struct udevice *dev;
	int ret;

	ret = regulator_get_by_platname(name, &dev);
	if (ret) {
		debug("%s: Cannot find regulator %s\n", __func__, name);
		return ret;
	}
	ret = regulator_set_value(dev, uv);
	if (ret) {
		debug("%s: Cannot set regulator %s\n", __func__, name);
		return ret;
	}

	return 0;
}

int exynos_power_init(void)
{
	struct udevice *dev;
	int ret;

	ret = pmic_get("max77686", &dev);
	if (!ret) {
		/* TODO(sjg@chromium.org): Move into the clock/pmic API */
		ret = pmic_clrsetbits(dev, MAX77686_REG_PMIC_32KHZ, 0,
				MAX77686_32KHCP_EN);
		if (ret)
			return ret;
		ret = pmic_clrsetbits(dev, MAX77686_REG_PMIC_BBAT, 0,
				MAX77686_BBCHOSTEN | MAX77686_BBCVS_3_5V);
		if (ret)
			return ret;
	} else {
		ret = pmic_get("s5m8767-pmic", &dev);
		/* TODO(sjg@chromium.org): Use driver model to access clock */
#ifdef CONFIG_PMIC_S5M8767
		if (!ret)
			s5m8767_enable_32khz_cp(dev);
#endif
	}
	if (ret == -ENODEV)
		return 0;

	ret = regulators_enable_boot_on(false);
	if (ret)
		return ret;

	ret = exynos_set_regulator("vdd_mif", 1100000);
	if (ret)
		return ret;

	/*
	 * This would normally be 1.3V, but since we are running slowly 1.1V
	 * is enough. For spring it helps reduce CPU temperature and avoid
	 * hangs with the case open. 1.1V is minimum voltage borderline for
	 * chained bootloaders.
	 */
	ret = exynos_set_regulator("vdd_arm", 1100000);
	if (ret)
		return ret;
	ret = exynos_set_regulator("vdd_int", 1012500);
	if (ret)
		return ret;
	ret = exynos_set_regulator("vdd_g3d", 1200000);
	if (ret)
		return ret;

	return 0;
}

int board_get_revision(void)
{
	return 0;
}

#ifdef CONFIG_LCD

static int board_dp_bridge_init(struct udevice *dev)
{
	const int max_tries = 10;
	int num_tries;
	int ret;

	debug("%s\n", __func__);
	ret = video_bridge_attach(dev);
	if (ret) {
		debug("video bridge init failed: %d\n", ret);
		return ret;
	}

	/*
	 * We need to wait for 90ms after bringing up the bridge since there
	 * is a phantom "high" on the HPD chip during its bootup.  The phantom
	 * high comes within 7ms of de-asserting PD and persists for at least
	 * 15ms.  The real high comes roughly 50ms after PD is de-asserted. The
	 * phantom high makes it hard for us to know when the NXP chip is up.
	 */
	mdelay(90);

	for (num_tries = 0; num_tries < max_tries; num_tries++) {
		/* Check HPD. If it's high, or we don't have it, all is well */
		ret = video_bridge_check_attached(dev);
		if (!ret || ret == -ENOENT)
			return 0;

		debug("%s: eDP bridge failed to come up; try %d of %d\n",
		      __func__, num_tries, max_tries);
	}

	/* Immediately go into bridge reset if the hp line is not high */
	return -EIO;
}

static int board_dp_bridge_setup(const void *blob)
{
	const int max_tries = 2;
	int num_tries;
	struct udevice *dev;
	int ret;

	/* Configure I2C registers for Parade bridge */
	ret = uclass_get_device(UCLASS_VIDEO_BRIDGE, 0, &dev);
	if (ret) {
		debug("video bridge init failed: %d\n", ret);
		return ret;
	}

	if (strncmp(dev->driver->name, "parade", 6)) {
		/* Mux HPHPD to the special hotplug detect mode */
		exynos_pinmux_config(PERIPH_ID_DPHPD, 0);
	}

	for (num_tries = 0; num_tries < max_tries; num_tries++) {
		ret = board_dp_bridge_init(dev);
		if (!ret)
			return 0;
		if (num_tries == max_tries - 1)
			break;

		/*
		* If we're here, the bridge chip failed to initialise.
		* Power down the bridge in an attempt to reset.
		*/
		video_bridge_set_active(dev, false);

		/*
		* Arbitrarily wait 300ms here with DP_N low.  Don't know for
		* sure how long we should wait, but we're being paranoid.
		*/
		mdelay(300);
	}

	return ret;
}

void exynos_cfg_lcd_gpio(void)
{
	/* For Backlight */
	gpio_request(EXYNOS5_GPIO_B20, "lcd_backlight");
	gpio_cfg_pin(EXYNOS5_GPIO_B20, S5P_GPIO_OUTPUT);
	gpio_set_value(EXYNOS5_GPIO_B20, 1);
}

void exynos_set_dp_phy(unsigned int onoff)
{
	set_dp_phy_ctrl(onoff);
}

static int board_dp_set_backlight(int percent)
{
	struct udevice *dev;
	int ret;

	ret = uclass_get_device(UCLASS_VIDEO_BRIDGE, 0, &dev);
	if (!ret)
		ret = video_bridge_set_backlight(dev, percent);

	return ret;
}

void exynos_backlight_on(unsigned int on)
{
	struct udevice *dev;
	int ret;

	debug("%s(%u)\n", __func__, on);
	if (!on)
		return;

	ret = regulator_get_by_platname("vcd_led", &dev);
	if (!ret)
		ret = regulator_set_enable(dev, true);
	if (ret)
		debug("Failed to enable backlight: ret=%d\n", ret);

	/* T5 in the LCD timing spec (defined as > 10ms) */
	mdelay(10);

	/* board_dp_backlight_pwm */
	gpio_direction_output(EXYNOS5_GPIO_B20, 1);

	/* T6 in the LCD timing spec (defined as > 10ms) */
	mdelay(10);

	/* try to set the backlight in the bridge registers */
	ret = board_dp_set_backlight(80);

	/* if we have no bridge or it does not support backlight, use a GPIO */
	if (ret == -ENODEV || ret == -ENOSYS) {
		gpio_request(EXYNOS5_GPIO_X30, "board_dp_backlight_en");
		gpio_direction_output(EXYNOS5_GPIO_X30, 1);
	}
}

void exynos_lcd_power_on(void)
{
	struct udevice *dev;
	int ret;

	debug("%s\n", __func__);
	ret = regulator_get_by_platname("lcd_vdd", &dev);
	if (!ret)
		ret = regulator_set_enable(dev, true);
	if (ret)
		debug("Failed to enable LCD panel: ret=%d\n", ret);

	ret = board_dp_bridge_setup(gd->fdt_blob);
	if (ret && ret != -ENODEV)
		printf("LCD bridge failed to enable: %d\n", ret);
}

#endif

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
	char *info = "Not supported!";

	if (board_is_odroidxu4())
		return info;

	return getenv("dfu_alt_system");
}

char *get_dfu_alt_boot(char *interface, char *devstr)
{
	char *info = "Not supported!";
	struct mmc *mmc;
	char *alt_boot;
	int dev_num;

	if (board_is_odroidxu4())
		return info;

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
