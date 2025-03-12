// SPDX-License-Identifier: GPL-2.0+
/*
 * Test for video bridge uclass
 *
 * Copyright (c) 2025 Svyatoslav Ryhel <clamor95@gmail.com>
 */

#include <backlight.h>
#include <dm.h>
#include <panel.h>
#include <video.h>
#include <video_bridge.h>
#include <asm/gpio.h>
#include <asm/test.h>
#include <dm/test.h>
#include <power/regulator.h>
#include <test/test.h>
#include <test/ut.h>

/* Basic test of the video uclass, test is based on driven panel */
static int dm_test_video_bridge(struct unit_test_state *uts)
{
	struct udevice *dev, *pwm, *gpio, *reg;
	uint period_ns, duty_ns;
	bool enable, polarity;
	struct display_timing timing;

	ut_assertok(uclass_first_device_err(UCLASS_VIDEO_BRIDGE, &dev));
	ut_assertok(uclass_get_device_by_name(UCLASS_PWM, "pwm", &pwm));
	ut_assertok(uclass_get_device(UCLASS_GPIO, 1, &gpio));
	ut_assertok(regulator_get_by_platname("VDD_EMMC_1.8V", &reg));
	ut_assertok(sandbox_pwm_get_config(pwm, 0, &period_ns, &duty_ns,
					   &enable, &polarity));
	ut_asserteq(false, enable);
	ut_asserteq(true, regulator_get_enable(reg));

	/* bridge calls panel_enable_backlight() of panel */
	ut_assertok(video_bridge_attach(dev));
	ut_assertok(sandbox_pwm_get_config(pwm, 0, &period_ns, &duty_ns,
					   &enable, &polarity));
	ut_asserteq(1000, period_ns);
	ut_asserteq(170 * 1000 / 255, duty_ns);
	ut_asserteq(true, enable);
	ut_asserteq(false, polarity);
	ut_asserteq(1, sandbox_gpio_get_value(gpio, 1));
	ut_asserteq(true, regulator_get_enable(reg));

	/* bridge calls panel_set_backlight() of panel */
	ut_assertok(video_bridge_set_backlight(dev, BACKLIGHT_DEFAULT));
	ut_assertok(sandbox_pwm_get_config(pwm, 0, &period_ns, &duty_ns,
					   &enable, &polarity));
	ut_asserteq(true, enable);
	ut_asserteq(170 * 1000 / 255, duty_ns);

	/* bridge should be active */
	ut_assertok(video_bridge_set_active(dev, true));

	/* bridge is internal and has no hotplug gpio */
	ut_asserteq(-ENOENT, video_bridge_check_attached(dev));

	/* check passing timings and EDID */
	ut_assertok(video_bridge_get_display_timing(dev, &timing));
	ut_assertok(video_bridge_read_edid(dev, NULL, 0));

	return 0;
}
DM_TEST(dm_test_video_bridge, UTF_SCAN_PDATA | UTF_SCAN_FDT);
