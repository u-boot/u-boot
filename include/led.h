/*
 * Copyright (c) 2015 Google, Inc
 * Written by Simon Glass <sjg@chromium.org>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __LED_H
#define __LED_H

/**
 * struct led_uc_plat - Platform data the uclass stores about each device
 *
 * @label:	LED label
 */
struct led_uc_plat {
	const char *label;
};

enum led_state_t {
	LEDST_OFF = 0,
	LEDST_ON = 1,
	LEDST_TOGGLE,

	LEDST_COUNT,
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
};

#define led_get_ops(dev)	((struct led_ops *)(dev)->driver->ops)

/**
 * led_get_by_label() - Find an LED device by label
 *
 * @label:	LED label to look up
 * @devp:	Returns the associated device, if found
 * @return 0 if found, -ENODEV if not found, other -ve on error
 */
int led_get_by_label(const char *label, struct udevice **devp);

/**
 * led_set_state() - set the state of an LED
 *
 * @dev:	LED device to change
 * @state:	LED state to set
 * @return 0 if OK, -ve on error
 */
int led_set_state(struct udevice *dev, enum led_state_t state);

/**
 * led_get_state() - get the state of an LED
 *
 * @dev:	LED device to change
 * @return LED state led_state_t, or -ve on error
 */
enum led_state_t led_get_state(struct udevice *dev);

#endif
