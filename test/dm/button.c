// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2020 Philippe Reynes <philippe.reynes@softathome.com>
 *
 * Based on led.c
 */

#include <dm.h>
#include <adc.h>
#include <button.h>
#include <env.h>
#include <power/regulator.h>
#include <power/sandbox_pmic.h>
#include <asm/gpio.h>
#include <dm/test.h>
#include <dt-bindings/input/input.h>
#include <test/ut.h>

/* Base test of the button uclass */
static int dm_test_button_base(struct unit_test_state *uts)
{
	struct udevice *dev;

	/* Get the top-level gpio buttons device */
	ut_assertok(uclass_get_device(UCLASS_BUTTON, 0, &dev));
	/* Get the 2 gpio buttons */
	ut_assertok(uclass_get_device(UCLASS_BUTTON, 1, &dev));
	ut_assertok(uclass_get_device(UCLASS_BUTTON, 2, &dev));

	/* Get the top-level adc buttons device */
	ut_assertok(uclass_get_device(UCLASS_BUTTON, 3, &dev));
	/* Get the 3 adc buttons */
	ut_assertok(uclass_get_device(UCLASS_BUTTON, 4, &dev));
	ut_assertok(uclass_get_device(UCLASS_BUTTON, 5, &dev));
	ut_assertok(uclass_get_device(UCLASS_BUTTON, 6, &dev));

	ut_asserteq(-ENODEV, uclass_get_device(UCLASS_BUTTON, 7, &dev));

	return 0;
}
DM_TEST(dm_test_button_base, UTF_SCAN_PDATA | UTF_SCAN_FDT);

/* Test of the button uclass using the button_gpio driver */
static int dm_test_button_gpio(struct unit_test_state *uts)
{
	const int offset = 3;
	struct udevice *dev, *gpio;

	/*
	 * Check that we can manipulate a BUTTON. BUTTON 1 is connected to GPIO
	 * bank gpio_a, offset 3.
	 */
	ut_assertok(uclass_get_device(UCLASS_BUTTON, 1, &dev));
	ut_assertok(uclass_get_device(UCLASS_GPIO, 1, &gpio));

	ut_asserteq(0, sandbox_gpio_set_value(gpio, offset, 0));
	ut_asserteq(0, sandbox_gpio_get_value(gpio, offset));
	ut_asserteq(BUTTON_OFF, button_get_state(dev));

	ut_asserteq(0, sandbox_gpio_set_value(gpio, offset, 1));
	ut_asserteq(1, sandbox_gpio_get_value(gpio, offset));
	ut_asserteq(BUTTON_ON, button_get_state(dev));

	return 0;
}
DM_TEST(dm_test_button_gpio, UTF_SCAN_PDATA | UTF_SCAN_FDT);

/* Test obtaining a BUTTON by label */
static int dm_test_button_label(struct unit_test_state *uts)
{
	struct udevice *dev, *cmp;

	ut_assertok(button_get_by_label("button1", &dev));
	ut_asserteq(1, device_active(dev));
	ut_assertok(uclass_get_device(UCLASS_BUTTON, 1, &cmp));
	ut_asserteq_ptr(dev, cmp);

	ut_assertok(button_get_by_label("button2", &dev));
	ut_asserteq(1, device_active(dev));
	ut_assertok(uclass_get_device(UCLASS_BUTTON, 2, &cmp));
	ut_asserteq_ptr(dev, cmp);

	ut_asserteq(-ENODEV, button_get_by_label("nobutton", &dev));

	return 0;
}
DM_TEST(dm_test_button_label, UTF_SCAN_PDATA | UTF_SCAN_FDT);

/* Test button has linux,code */
static int dm_test_button_linux_code(struct unit_test_state *uts)
{
	struct udevice *dev;

	ut_assertok(uclass_get_device(UCLASS_BUTTON, 1, &dev));
	ut_asserteq(BTN_1, button_get_code(dev));

	return 0;
}
DM_TEST(dm_test_button_linux_code, UTF_SCAN_PDATA | UTF_SCAN_FDT);

/* Test adc-keys driver */
static int dm_test_button_keys_adc(struct unit_test_state *uts)
{
	struct udevice *supply;
	struct udevice *dev;
	int uV;

	ut_assertok(uclass_get_device_by_name(UCLASS_ADC, "adc@0", &dev));

	ut_assertok(regulator_get_by_devname(SANDBOX_BUCK2_DEVNAME, &supply));
	ut_assertok(regulator_set_value(supply, SANDBOX_BUCK2_SET_UV));
	ut_asserteq(SANDBOX_BUCK2_SET_UV, regulator_get_value(supply));
	/* Update ADC plat and get new Vdd value */
	ut_assertok(adc_vdd_value(dev, &uV));
	ut_asserteq(SANDBOX_BUCK2_SET_UV, uV);

	/*
	 * sandbox-adc returns constant value on channel 3, is used by adc-keys:
	 * SANDBOX_ADC_CHANNEL3_DATA * SANDBOX_BUCK2_SET_UV / SANDBOX_ADC_DATA_MASK =
	 * 0x3000 * 3300000 / 0xffff = 618759uV
	 * This means that button3 and button4 are released and button5
	 * is pressed.
	 */
	ut_assertok(button_get_by_label("button3", &dev));
	ut_asserteq(BUTTON_OFF, button_get_state(dev));
	ut_assertok(button_get_by_label("button4", &dev));
	ut_asserteq(BUTTON_OFF, button_get_state(dev));
	ut_assertok(button_get_by_label("button5", &dev));
	ut_asserteq(BUTTON_ON, button_get_state(dev));

	return 0;
}
DM_TEST(dm_test_button_keys_adc, UTF_SCAN_PDATA | UTF_SCAN_FDT);

/* Test of the button uclass using the button_gpio driver */
static int dm_test_button_cmd(struct unit_test_state *uts)
{
	struct udevice *btn1_dev, *btn2_dev, *gpio;
	const char *envstr;

#define BTN1_GPIO 3
#define BTN2_GPIO 4
#define BTN1_PASS_VAR "test_button_cmds_0"
#define BTN2_PASS_VAR "test_button_cmds_1"

	/*
	 * Buttons 1 and 2 are connected to gpio_a gpios 3 and 4 respectively.
	 * set the GPIOs to known values and then check that the appropriate
	 * commands are run when invoking process_button_cmds().
	 */
	ut_assertok(uclass_get_device(UCLASS_BUTTON, 1, &btn1_dev));
	ut_assertok(uclass_get_device(UCLASS_BUTTON, 2, &btn2_dev));
	ut_assertok(uclass_get_device(UCLASS_GPIO, 1, &gpio));

	/*
	 * Map a command to button 1 and check that it process_button_cmds()
	 * runs it if called with button 1 pressed.
	 */
	ut_assertok(env_set("button_cmd_0_name", "button1"));
	ut_assertok(env_set("button_cmd_0", "env set " BTN1_PASS_VAR " PASS"));
	ut_assertok(sandbox_gpio_set_value(gpio, BTN1_GPIO, 1));
	/* Sanity check that the button is actually pressed */
	ut_asserteq(BUTTON_ON, button_get_state(btn1_dev));
	process_button_cmds();
	ut_assertnonnull((envstr = env_get(BTN1_PASS_VAR)));
	ut_asserteq_str(envstr, "PASS");

	/* Clear result */
	ut_assertok(env_set(BTN1_PASS_VAR, NULL));

	/*
	 * Map a command for button 2, press it, check that only the command
	 * for button 1 runs because it comes first and is also pressed.
	 */
	ut_assertok(env_set("button_cmd_1_name", "button2"));
	ut_assertok(env_set("button_cmd_1", "env set " BTN2_PASS_VAR " PASS"));
	ut_assertok(sandbox_gpio_set_value(gpio, BTN2_GPIO, 1));
	ut_asserteq(BUTTON_ON, button_get_state(btn2_dev));
	process_button_cmds();
	/* Check that button 1 triggered again */
	ut_assertnonnull((envstr = env_get(BTN1_PASS_VAR)));
	ut_asserteq_str(envstr, "PASS");
	/* And button 2 didn't */
	ut_assertnull(env_get(BTN2_PASS_VAR));

	/* Clear result */
	ut_assertok(env_set(BTN1_PASS_VAR, NULL));

	/*
	 * Release button 1 and check that the command for button 2 is run
	 */
	ut_assertok(sandbox_gpio_set_value(gpio, BTN1_GPIO, 0));
	process_button_cmds();
	ut_assertnull(env_get(BTN1_PASS_VAR));
	/* Check that the command for button 2 ran */
	ut_assertnonnull((envstr = env_get(BTN2_PASS_VAR)));
	ut_asserteq_str(envstr, "PASS");

	/* Clear result */
	ut_assertok(env_set(BTN2_PASS_VAR, NULL));

	/*
	 * Unset "button_cmd_0_name" and check that no commands run even
	 * with both buttons pressed.
	 */
	ut_assertok(env_set("button_cmd_0_name", NULL));
	/* Press button 1 (button 2 is already pressed )*/
	ut_assertok(sandbox_gpio_set_value(gpio, BTN1_GPIO, 1));
	ut_asserteq(BUTTON_ON, button_get_state(btn1_dev));
	process_button_cmds();
	ut_assertnull(env_get(BTN1_PASS_VAR));
	ut_assertnull(env_get(BTN2_PASS_VAR));

	/*
	 * Check that no command is run if the button name is wrong.
	 */
	ut_assertok(env_set("button_cmd_0_name", "invalid_button"));
	process_button_cmds();
	ut_assertnull(env_get(BTN1_PASS_VAR));
	ut_assertnull(env_get(BTN2_PASS_VAR));

#undef BTN1_PASS_VAR
#undef BTN2_PASS_VAR
#undef BTN1_GPIO
#undef BTN2_GPIO

	return 0;
}
DM_TEST(dm_test_button_cmd, UTF_SCAN_PDATA | UTF_SCAN_FDT);
