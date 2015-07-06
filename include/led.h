/*
 * Copyright (c) 2015 Google, Inc
 * Written by Simon Glass <sjg@chromium.org>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __LED_H
#define __LED_H

/**
 * struct led_uclass_plat - Platform data the uclass stores about each device
 *
 * @label:	LED label
 */
struct led_uclass_plat {
	const char *label;
};

struct led_ops {
	/**
	 * set_on() - set the state of an LED
	 *
	 * @dev:	LED device to change
	 * @on:		1 to turn the LED on, 0 to turn it off
	 * @return 0 if OK, -ve on error
	 */
	int (*set_on)(struct udevice *dev, int on);
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
 * led_set_on() - set the state of an LED
 *
 * @dev:	LED device to change
 * @on:		1 to turn the LED on, 0 to turn it off
 * @return 0 if OK, -ve on error
 */
int led_set_on(struct udevice *dev, int on);

#endif
