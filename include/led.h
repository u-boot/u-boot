/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (c) 2015 Google, Inc
 * Written by Simon Glass <sjg@chromium.org>
 */

#ifndef __LED_H
#define __LED_H

#include <stdbool.h>
#include <cyclic.h>
#include <dm/ofnode.h>

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
 * functionality is needed, enable the `LED_BLINK` Kconfig option. If LED driver doesn't
 * support HW Blink, SW Blink can be used with the Cyclic framework by enabling the
 * CONFIG_LED_SW_BLINK.
 *
 * Boot and Activity LEDs are also supported. These LEDs can signal various system operations
 * during runtime, such as boot initialization, file transfers, and flash write/erase operations.
 *
 * To enable a Boot LED, enable `CONFIG_LED_BOOT` and define in `/options/u-boot` root node the
 * property `boot-led`. This will enable the specified LED to blink and turn ON when
 * the bootloader initializes correctly.
 *
 * To enable an Activity LED, enable `CONFIG_LED_ACTIVITY` and define in `/options/u-boot` root
 * node the property `activity-led`.
 * This will enable the specified LED to blink and turn ON during file transfers or flash
 * write/erase operations.
 *
 * Both Boot and Activity LEDs provide a simple API to turn the LED ON or OFF:
 * `led_boot_on()`, `led_boot_off()`, `led_activity_on()`, and `led_activity_off()`.
 *
 * Both configurations can optionally define a `boot/activity-led-period` property
 * if `CONFIG_LED_BLINK` or `CONFIG_LED_SW_BLINK` is enabled for LED blink operations, which
 * is usually used by the Activity LED. If not defined the default value of 250 (ms) is used.
 *
 * When `CONFIG_LED_BLINK` or `CONFIG_LED_SW_BLINK` is enabled, additional APIs are exposed:
 * `led_boot_blink()` and `led_activity_blink()`. Note that if `CONFIG_LED_BLINK` or
 * `CONFIG_LED_SW_BLINK` is disabled, these APIs will behave like the `led_boot_on()` and
 * `led_activity_on()` APIs, respectively.
 */

struct udevice;

/*
 * value imported from linux:include/linux/uapi/linux/uleds.h
 */
#define LED_MAX_NAME_SIZE      64

enum led_state_t {
	LEDST_OFF = 0,
	LEDST_ON = 1,
	LEDST_TOGGLE,
	LEDST_BLINK,

	LEDST_COUNT,
};

enum led_sw_blink_state_t {
	LED_SW_BLINK_ST_DISABLED,
	LED_SW_BLINK_ST_NOT_READY,
	LED_SW_BLINK_ST_OFF,
	LED_SW_BLINK_ST_ON,
};

struct led_sw_blink {
	enum led_sw_blink_state_t state;
	struct udevice *dev;
	struct cyclic_info cyclic;
	const char cyclic_name[0];
};

/**
 * struct led_uc_plat - Platform data the uclass stores about each device
 *
 * @label:	LED label
 * @default_state:	LED default state
 * @name:	LED name, derived from function, color or function-enumerator
 *		property.
 * @sw_blink:	LED software blink struct
 */
struct led_uc_plat {
	const char *label;
	enum led_state_t default_state;
	char name[LED_MAX_NAME_SIZE];
#ifdef CONFIG_LED_SW_BLINK
	struct led_sw_blink *sw_blink;
#endif
};

/**
 * struct led_uc_priv - Private data the uclass stores about each device
 *
 * @boot_led_label:	Boot LED label
 * @activity_led_label:	Activity LED label
 * @boot_led_dev:	Boot LED dev
 * @activity_led_dev:	Activity LED dev
 * @boot_led_period:	Boot LED blink period
 * @activity_led_period: Activity LED blink period
 */
struct led_uc_priv {
#ifdef CONFIG_LED_BOOT
	const char *boot_led_label;
	int boot_led_period;
#endif
#ifdef CONFIG_LED_ACTIVITY
	const char *activity_led_label;
	int activity_led_period;
#endif
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
 * led_bind_generic() - bind children of parent to given driver
 *
 * @parent:      Top-level LED device
 * @driver_name: Driver for handling individual child nodes
 */
int led_bind_generic(struct udevice *parent, const char *driver_name);

/* Internal functions for software blinking. Do not use them in your code */
int led_sw_set_period(struct udevice *dev, int period_ms);
bool led_sw_is_blinking(struct udevice *dev);
bool led_sw_on_state_change(struct udevice *dev, enum led_state_t state);

#ifdef CONFIG_LED_BOOT

/**
 * led_boot_on() - turn ON the designated LED for booting
 *
 * Return: 0 if OK, -ve on error
 */
int led_boot_on(void);

/**
 * led_boot_off() - turn OFF the designated LED for booting
 *
 * Return: 0 if OK, -ve on error
 */
int led_boot_off(void);

#if defined(CONFIG_LED_BLINK) || defined(CONFIG_LED_SW_BLINK)
/**
 * led_boot_blink() - turn ON the designated LED for booting
 *
 * Return: 0 if OK, -ve on error
 */
int led_boot_blink(void);

#else
/* If LED BLINK is not supported/enabled, fallback to LED ON */
#define led_boot_blink led_boot_on
#endif
#else
static inline int led_boot_on(void)
{
	return -ENOSYS;
}

static inline int led_boot_off(void)
{
	return -ENOSYS;
}

static inline int led_boot_blink(void)
{
	return -ENOSYS;
}
#endif

#ifdef CONFIG_LED_ACTIVITY

/**
 * led_activity_on() - turn ON the designated LED for activity
 *
 * Return: 0 if OK, -ve on error
 */
int led_activity_on(void);

/**
 * led_activity_off() - turn OFF the designated LED for activity
 *
 * Return: 0 if OK, -ve on error
 */
int led_activity_off(void);

#if defined(CONFIG_LED_BLINK) || defined(CONFIG_LED_SW_BLINK)
/**
 * led_activity_blink() - turn ON the designated LED for activity
 *
 * Return: 0 if OK, -ve on error
 */
int led_activity_blink(void);
#else
/* If LED BLINK is not supported/enabled, fallback to LED ON */
#define led_activity_blink led_activity_on
#endif
#else
static inline int led_activity_on(void)
{
	return -ENOSYS;
}

static inline int led_activity_off(void)
{
	return -ENOSYS;
}

static inline int led_activity_blink(void)
{
	return -ENOSYS;
}
#endif

#endif
