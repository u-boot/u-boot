// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2020 Sean Anderson <seanga2@gmail.com>
 */

#include <common.h>
#include <dm.h>
#include <dm/pinctrl.h>
#include <dm/test.h>
#include <test/ut.h>

static int dm_test_pinmux(struct unit_test_state *uts)
{
	char buf[64];
	struct udevice *dev;

#define test_muxing(selector, expected) do { \
	ut_assertok(pinctrl_get_pin_muxing(dev, selector, buf, sizeof(buf))); \
	ut_asserteq_str(expected, (char *)&buf); \
} while (0)

	ut_assertok(uclass_get_device_by_name(UCLASS_PINCTRL, "pinctrl", &dev));
	test_muxing(0, "UART TX.");
	test_muxing(1, "UART RX.");
	test_muxing(2, "I2S SCK.");
	test_muxing(3, "I2S SD.");
	test_muxing(4, "I2S WS.");
	test_muxing(5, "GPIO0 bias-pull-up input-disable.");
	test_muxing(6, "GPIO1 drive-open-drain.");
	test_muxing(7, "GPIO2 bias-pull-down input-enable.");
	test_muxing(8, "GPIO3 bias-disable.");

	ut_assertok(pinctrl_select_state(dev, "alternate"));
	test_muxing(0, "I2C SCL drive-open-drain.");
	test_muxing(1, "I2C SDA drive-open-drain.");
	test_muxing(2, "SPI SCLK.");
	test_muxing(3, "SPI MOSI.");
	test_muxing(4, "SPI MISO.");
	test_muxing(5, "SPI CS0.");
	test_muxing(6, "SPI CS1.");
	test_muxing(7, "GPIO2 bias-pull-down input-enable.");
	test_muxing(8, "GPIO3 bias-disable.");

	ut_assertok(pinctrl_select_state(dev, "0"));
	test_muxing(0, "I2C SCL drive-open-drain.");
	test_muxing(1, "I2C SDA drive-open-drain.");
	test_muxing(2, "I2S SCK.");
	test_muxing(3, "I2S SD.");
	test_muxing(4, "I2S WS.");
	test_muxing(5, "GPIO0 bias-pull-up input-disable.");
	test_muxing(6, "GPIO1 drive-open-drain.");
	test_muxing(7, "GPIO2 bias-pull-down input-enable.");
	test_muxing(8, "GPIO3 bias-disable.");

	return 0;
}
DM_TEST(dm_test_pinmux, UT_TESTF_SCAN_FDT);
