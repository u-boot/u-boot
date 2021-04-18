// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2020 Sean Anderson <seanga2@gmail.com>
 */

#include <common.h>
#include <dm.h>
#include <dm/pinctrl.h>
#include <dm/test.h>
#include <test/ut.h>

static char buf[64];
#define test_muxing(selector, expected) do { \
	ut_assertok(pinctrl_get_pin_muxing(dev, selector, buf, sizeof(buf))); \
	ut_asserteq_str(expected, (char *)&buf); \
} while (0)

#define test_name(selector, expected) do { \
	ut_assertok(pinctrl_get_pin_name(dev, selector, buf, sizeof(buf))); \
	ut_asserteq_str(expected, (char *)&buf); \
} while (0)

static int dm_test_pinmux(struct unit_test_state *uts)
{
	struct udevice *dev;

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

static int dm_test_pinctrl_single(struct unit_test_state *uts)
{
	struct udevice *dev;
	int ret;

	ret = uclass_get_device_by_name(UCLASS_PINCTRL,
					"pinctrl-single-no-width", &dev);
	ut_asserteq(-EINVAL, ret);
	ut_assertok(uclass_get_device_by_name(UCLASS_PWM, "pwm", &dev));
	ut_assertok(uclass_get_device_by_name(UCLASS_SERIAL, "serial", &dev));
	ut_assertok(uclass_get_device_by_name(UCLASS_SPI, "spi@0", &dev));
	ut_assertok(uclass_get_device_by_name(UCLASS_PINCTRL,
					      "pinctrl-single-pins", &dev));
	ut_asserteq(142, pinctrl_get_pins_count(dev));
	test_name(0, "PIN0");
	test_name(141, "PIN141");
	test_name(142, "Error");
	test_muxing(0, "0x00000000 0x00000000 UNCLAIMED");
	test_muxing(18, "0x00000048 0x00000006 pinmux_pwm_pins");
	test_muxing(28, "0x00000070 0x00000030 pinmux_uart0_pins");
	test_muxing(29, "0x00000074 0x00000000 pinmux_uart0_pins");
	test_muxing(100, "0x00000190 0x0000000c pinmux_spi0_pins");
	test_muxing(101, "0x00000194 0x0000000c pinmux_spi0_pins");
	test_muxing(102, "0x00000198 0x00000023 pinmux_spi0_pins");
	test_muxing(103, "0x0000019c 0x0000000c pinmux_spi0_pins");
	ret = pinctrl_get_pin_muxing(dev, 142, buf, sizeof(buf));
	ut_asserteq(-EINVAL, ret);
	ut_assertok(uclass_get_device_by_name(UCLASS_I2C, "i2c@0", &dev));
	ut_assertok(uclass_get_device_by_name(UCLASS_VIDEO, "lcd", &dev));
	ut_assertok(uclass_get_device_by_name(UCLASS_PINCTRL,
					      "pinctrl-single-bits", &dev));
	ut_asserteq(160, pinctrl_get_pins_count(dev));
	test_name(0, "PIN0");
	test_name(159, "PIN159");
	test_name(160, "Error");
	test_muxing(0, "0x00000000 0x00000000 UNCLAIMED");
	test_muxing(34, "0x00000010 0x00000200 pinmux_i2c0_pins");
	test_muxing(35, "0x00000010 0x00002000 pinmux_i2c0_pins");
	test_muxing(130, "0x00000040 0x00000200 pinmux_lcd_pins");
	test_muxing(131, "0x00000040 0x00002000 pinmux_lcd_pins");
	test_muxing(132, "0x00000040 0x00020000 pinmux_lcd_pins");
	test_muxing(133, "0x00000040 0x00200000 pinmux_lcd_pins");
	test_muxing(134, "0x00000040 0x02000000 pinmux_lcd_pins");
	test_muxing(135, "0x00000040 0x20000000 pinmux_lcd_pins");
	test_muxing(136, "0x00000044 0x00000002 pinmux_lcd_pins");
	test_muxing(137, "0x00000044 0x00000020 pinmux_lcd_pins");
	test_muxing(138, "0x00000044 0x00000200 pinmux_lcd_pins");
	test_muxing(139, "0x00000044 0x00002000 pinmux_lcd_pins");
	test_muxing(140, "0x00000044 0x00020000 pinmux_lcd_pins");
	test_muxing(141, "0x00000044 0x00200000 pinmux_lcd_pins");
	test_muxing(142, "0x00000044 0x02000000 pinmux_lcd_pins");
	test_muxing(143, "0x00000044 0x20000000 pinmux_lcd_pins");
	test_muxing(144, "0x00000048 0x00000002 pinmux_lcd_pins");
	test_muxing(145, "0x00000048 0x00000020 pinmux_lcd_pins");
	test_muxing(146, "0x00000048 0x00000000 UNCLAIMED");
	test_muxing(147, "0x00000048 0x00000000 UNCLAIMED");
	test_muxing(148, "0x00000048 0x00000000 UNCLAIMED");
	test_muxing(149, "0x00000048 0x00000000 UNCLAIMED");
	test_muxing(150, "0x00000048 0x02000000 pinmux_lcd_pins");
	test_muxing(151, "0x00000048 0x00000000 UNCLAIMED");
	test_muxing(152, "0x0000004c 0x00000002 pinmux_lcd_pins");
	test_muxing(153, "0x0000004c 0x00000020 pinmux_lcd_pins");
	test_muxing(154, "0x0000004c 0x00000000 UNCLAIMED");
	test_muxing(155, "0x0000004c 0x00000000 UNCLAIMED");
	test_muxing(156, "0x0000004c 0x00000000 UNCLAIMED");
	test_muxing(157, "0x0000004c 0x00000000 UNCLAIMED");
	test_muxing(158, "0x0000004c 0x02000000 pinmux_lcd_pins");
	test_muxing(159, "0x0000004c 0x00000000 UNCLAIMED");
	ret = pinctrl_get_pin_muxing(dev, 160, buf, sizeof(buf));
	ut_asserteq(-EINVAL, ret);
	return 0;
}

DM_TEST(dm_test_pinctrl_single, UT_TESTF_SCAN_FDT);
