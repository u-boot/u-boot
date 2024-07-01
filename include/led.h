/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (c) 2015 Google, Inc
 * Written by Simon Glass <sjg@chromium.org>
 */

#ifndef __LED_H
#define __LED_H

/**
 * DOC: Overview
 *
 * Generic LED API provided when a supported compatible is defined in DeviceTree.
 *
 * To enable support for LEDs, enable the `CONFIG_LED` Kconfig option.
 *
 * The most common implementation is for GPIO-connected LEDs. If using GPIO-connected LEDs,
 * enable the `LED_GPIO` Kconfig option.
 *
 * `LED_BLINK` support requires LED driver support and is therefore optional. If LED blink
 * functionality is needed, enable the `LED_BLINK` Kconfig option.
 *
 * GPIO-connected LEDs can simulate blinking via software using the Cyclic API. To utilize this,
 * enable `CONFIG_LED_GPIO_SW_BLINK`. This will also select `CONFIG_LED_BLINK` and `CONFIG_CYCLIC`.
 *
 * Boot and Activity LEDs are also supported. These LEDs can signal various system operations
 * during runtime, such as boot initialization, file transfers, and flash write/erase operations.
 *
 * To enable a Boot LED, enable `CONFIG_LED_BOOT_ENABLE` and define `CONFIG_LED_BOOT_LABEL`. This
 * will enable the specified LED to blink and turn ON when the bootloader initializes correctly.
 *
 * To enable an Activity LED, enable `CONFIG_LED_ACTIVITY_ENABLE` and define
 * `CONFIG_LED_ACTIVITY_LABEL`.
 * This will enable the specified LED to blink and turn ON during file transfers or flash
 * write/erase operations.
 *
 * Both Boot and Activity LEDs provide a simple API to turn the LED ON or OFF:
 * `led_boot_on()`, `led_boot_off()`, `led_activity_on()`, and `led_activity_off()`.
 *
 * Both configurations can optionally define a `_PERIOD` option if `CONFIG_LED_BLINK` is enabled
 * for LED blink operations, which is usually used by the Activity LED.
 *
 * When `CONFIG_LED_BLINK` is enabled, additional APIs are exposed: `led_boot_blink()` and
 * `led_activity_blink()`. Note that if `CONFIG_LED_BLINK` is disabled, these APIs will behave
 * like the `led_boot_on()` and `led_activity_on()` APIs, respectively.
 */

struct udevice;

enum led_state_t {
	LEDST_OFF = 0,
	LEDST_ON = 1,
	LEDST_TOGGLE,
#ifdef CONFIG_LED_BLINK
	LEDST_BLINK,
#endif

	LEDST_COUNT,
};

/**
 * struct led_uc_plat - Platform data the uclass stores about each device
 *
 * @label:	LED label
 * @default_state:	LED default state
 */
struct led_uc_plat {
	const char *label;
	enum led_state_t default_state;
};

/**
 * struct led_uc_priv - Private data the uclass stores about each device
 *
 * @period_ms:	Flash period in milliseconds
 */
struct led_uc_priv {
	int period_ms;
};

struct led_ops {
	/**
	 * set_state() - set the state of an LED
	 *
	 * @dev:	LED device to change
	 * @state:	LED state to set
	 * @return 0 if OK, -ve on error
	 */
	int (*set_state)(struct udevice *dev, enum led_state_t state);

	/**
	 * led_get_state() - get the state of an LED
	 *
	 * @dev:	LED device to change
	 * @return LED state led_state_t, or -ve on error
	 */
	enum led_state_t (*get_state)(struct udevice *dev);

#ifdef CONFIG_LED_BLINK
	/**
	 * led_set_period() - set the blink period of an LED
	 *
	 * Thie records the period if supported, or returns -ENOSYS if not.
	 * To start the LED blinking, use set_state().
	 *
	 * @dev:	LED device to change
	 * @period_ms:	LED blink period in milliseconds
	 * @return 0 if OK, -ve on error
	 */
	int (*set_period)(struct udevice *dev, int period_ms);
#endif
};

#define led_get_ops(dev)	((struct led_ops *)(dev)->driver->ops)

/**
 * led_get_by_label() - Find an LED device by label
 *
 * @label:	LED label to look up
 * @devp:	Returns the associated device, if found
 * Return: 0 if found, -ENODEV if not found, other -ve on error
 */
int led_get_by_label(const char *label, struct udevice **devp);

/**
 * led_set_state() - set the state of an LED
 *
 * @dev:	LED device to change
 * @state:	LED state to set
 * Return: 0 if OK, -ve on error
 */
int led_set_state(struct udevice *dev, enum led_state_t state);

/**
 * led_set_state_by_label - set the state of an LED referenced by Label
 *
 * @label:	LED label
 * @state:	LED state to set
 * Return: 0 if OK, -ve on error
 */
int led_set_state_by_label(const char *label, enum led_state_t state);

/**
 * led_get_state() - get the state of an LED
 *
 * @dev:	LED device to change
 * Return: LED state led_state_t, or -ve on error
 */
enum led_state_t led_get_state(struct udevice *dev);

/**
 * led_set_period() - set the blink period of an LED
 *
 * @dev:	LED device to change
 * @period_ms:	LED blink period in milliseconds
 * Return: 0 if OK, -ve on error
 */
int led_set_period(struct udevice *dev, int period_ms);

/**
 * led_set_period_by_label - set the blink period of an LED referenced by Label
 *
 * @label:	LED label
 * @period_ms:	LED blink period in milliseconds
 * Return: 0 if OK, -ve on error
 */
int led_set_period_by_label(const char *label, int period_ms);

/**
 * led_bind_generic() - bind children of parent to given driver
 *
 * @parent:      Top-level LED device
 * @driver_name: Driver for handling individual child nodes
 */
int led_bind_generic(struct udevice *parent, const char *driver_name);

#ifdef CONFIG_LED_BOOT_ENABLE

#define LED_BOOT_PERIOD CONFIG_SYS_HZ / CONFIG_LED_BOOT_PERIOD

/**
 * led_boot_on() - turn ON the designated LED for booting
 *
 * Return: 0 if OK, -ve on error
 */
static inline int led_boot_on(void)
{
	return led_set_state_by_label(CONFIG_LED_BOOT_LABEL, LEDST_ON);
}

/**
 * led_boot_off() - turn OFF the designated LED for booting
 *
 * Return: 0 if OK, -ve on error
 */
static inline int led_boot_off(void)
{
	return led_set_state_by_label(CONFIG_LED_BOOT_LABEL, LEDST_OFF);
}

#ifdef CONFIG_LED_BLINK
/**
 * led_boot_blink() - turn ON the designated LED for booting
 *
 * Return: 0 if OK, -ve on error
 */
static inline int led_boot_blink(void)
{
	return led_set_period_by_label(CONFIG_LED_BOOT_LABEL, LED_BOOT_PERIOD);
}
#else
/* If LED BLINK is not supported/enabled, fallback to LED ON */
#define led_boot_blink led_boot_on
#endif
#endif

#ifdef CONFIG_LED_ACTIVITY_ENABLE

#define LED_ACTIVITY_PERIOD CONFIG_SYS_HZ / CONFIG_LED_ACTIVITY_PERIOD

/**
 * led_activity_on() - turn ON the designated LED for activity
 *
 * Return: 0 if OK, -ve on error
 */
static inline int led_activity_on(void)
{
	return led_set_state_by_label(CONFIG_LED_ACTIVITY_LABEL, LEDST_ON);
}

/**
 * led_activity_off() - turn OFF the designated LED for activity
 *
 * Return: 0 if OK, -ve on error
 */
static inline int led_activity_off(void)
{
	return led_set_state_by_label(CONFIG_LED_ACTIVITY_LABEL, LEDST_OFF);
}

#ifdef CONFIG_LED_BLINK
/**
 * led_activity_blink() - turn ON the designated LED for activity
 *
 * Return: 0 if OK, -ve on error
 */
static inline int led_activity_blink(void)
{
	return led_set_period_by_label(CONFIG_LED_ACTIVITY_LABEL, LED_BOOT_PERIOD);
}
#else
/* If LED BLINK is not supported/enabled, fallback to LED ON */
#define led_activity_blink led_activity_on
#endif
#endif

#endif
