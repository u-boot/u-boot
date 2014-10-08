/*
 * Copyright (c) 2011 The Chromium OS Authors.
 * Copyright (c) 2011, NVIDIA Corp. All rights reserved.
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef _ASM_GENERIC_GPIO_H_
#define _ASM_GENERIC_GPIO_H_

/*
 * Generic GPIO API for U-Boot
 *
 * GPIOs are numbered from 0 to GPIO_COUNT-1 which value is defined
 * by the SOC/architecture.
 *
 * Each GPIO can be an input or output. If an input then its value can
 * be read as 0 or 1. If an output then its value can be set to 0 or 1.
 * If you try to write an input then the value is undefined. If you try
 * to read an output, barring something very unusual,  you will get
 * back the value of the output that you previously set.
 *
 * In some cases the operation may fail, for example if the GPIO number
 * is out of range, or the GPIO is not available because its pin is
 * being used by another function. In that case, functions may return
 * an error value of -1.
 */

/**
 * Request a GPIO. This should be called before any of the other functions
 * are used on this GPIO.
 *
 * @param gp	GPIO number
 * @param label	User label for this GPIO
 * @return 0 if ok, -1 on error
 */
int gpio_request(unsigned gpio, const char *label);

/**
 * Stop using the GPIO.  This function should not alter pin configuration.
 *
 * @param gpio	GPIO number
 * @return 0 if ok, -1 on error
 */
int gpio_free(unsigned gpio);

/**
 * Make a GPIO an input.
 *
 * @param gpio	GPIO number
 * @return 0 if ok, -1 on error
 */
int gpio_direction_input(unsigned gpio);

/**
 * Make a GPIO an output, and set its value.
 *
 * @param gpio	GPIO number
 * @param value	GPIO value (0 for low or 1 for high)
 * @return 0 if ok, -1 on error
 */
int gpio_direction_output(unsigned gpio, int value);

/**
 * Get a GPIO's value. This will work whether the GPIO is an input
 * or an output.
 *
 * @param gpio	GPIO number
 * @return 0 if low, 1 if high, -1 on error
 */
int gpio_get_value(unsigned gpio);

/**
 * Set an output GPIO's value. The GPIO must already be an output or
 * this function may have no effect.
 *
 * @param gpio	GPIO number
 * @param value	GPIO value (0 for low or 1 for high)
 * @return 0 if ok, -1 on error
 */
int gpio_set_value(unsigned gpio, int value);

/* State of a GPIO, as reported by get_function() */
enum {
	GPIOF_INPUT = 0,
	GPIOF_OUTPUT,
	GPIOF_UNUSED,		/* Not claimed */
	GPIOF_UNKNOWN,		/* Not known */
	GPIOF_FUNC,		/* Not used as a GPIO */

	GPIOF_COUNT,
};

struct udevice;

/**
 * struct struct dm_gpio_ops - Driver model GPIO operations
 *
 * Refer to functions above for description. These function largely copy
 * the old API.
 *
 * This is trying to be close to Linux GPIO API. Once the U-Boot uses the
 * new DM GPIO API, this should be really easy to flip over to the Linux
 * GPIO API-alike interface.
 *
 * Also it would be useful to standardise additional functions like
 * pullup, slew rate and drive strength.
 *
 * gpio_request)( and gpio_free() are optional - if NULL then they will
 * not be called.
 *
 * Note that @offset is the offset from the base GPIO of the device. So
 * offset 0 is the device's first GPIO and offset o-1 is the last GPIO,
 * where o is the number of GPIO lines controlled by the device. A device
 * is typically used to control a single bank of GPIOs. Within complex
 * SoCs there may be many banks and therefore many devices all referring
 * to the different IO addresses within the SoC.
 *
 * The uclass combines all GPIO devices together to provide a consistent
 * numbering from 0 to n-1, where n is the number of GPIOs in total across
 * all devices. Be careful not to confuse offset with gpio in the parameters.
 */
struct dm_gpio_ops {
	int (*request)(struct udevice *dev, unsigned offset, const char *label);
	int (*free)(struct udevice *dev, unsigned offset);
	int (*direction_input)(struct udevice *dev, unsigned offset);
	int (*direction_output)(struct udevice *dev, unsigned offset,
				int value);
	int (*get_value)(struct udevice *dev, unsigned offset);
	int (*set_value)(struct udevice *dev, unsigned offset, int value);
	/**
	 * get_function() Get the GPIO function
	 *
	 * @dev:     Device to check
	 * @offset:  GPIO offset within that device
	 * @return current function - GPIOF_...
	 */
	int (*get_function)(struct udevice *dev, unsigned offset);
	int (*get_state)(struct udevice *dev, unsigned offset, char *state,
			 int maxlen);
};

/**
 * struct gpio_dev_priv - information about a device used by the uclass
 *
 * The uclass combines all active GPIO devices into a unified numbering
 * scheme. To do this it maintains some private information about each
 * device.
 *
 * To implement driver model support in your GPIO driver, add a probe
 * handler, and set @gpio_count and @bank_name correctly in that handler.
 * This tells the uclass the name of the GPIO bank and the number of GPIOs
 * it contains.
 *
 * @bank_name: Name of the GPIO device (e.g 'a' means GPIOs will be called
 * 'A0', 'A1', etc.
 * @gpio_count: Number of GPIOs in this device
 * @gpio_base: Base GPIO number for this device. For the first active device
 * this will be 0; the numbering for others will follow sequentially so that
 * @gpio_base for device 1 will equal the number of GPIOs in device 0.
 */
struct gpio_dev_priv {
	const char *bank_name;
	unsigned gpio_count;
	unsigned gpio_base;
};

/* Access the GPIO operations for a device */
#define gpio_get_ops(dev)	((struct dm_gpio_ops *)(dev)->driver->ops)

/**
 * gpio_get_bank_info - Return information about a GPIO bank/device
 *
 * This looks up a device and returns both its GPIO base name and the number
 * of GPIOs it controls.
 *
 * @dev: Device to look up
 * @offset_count: Returns number of GPIOs within this bank
 * @return bank name of this device
 */
const char *gpio_get_bank_info(struct udevice *dev, int *offset_count);

/**
 * gpio_lookup_name - Look up a GPIO name and return its details
 *
 * This is used to convert a named GPIO into a device, offset and GPIO
 * number.
 *
 * @name: GPIO name to look up
 * @devp: Returns pointer to device which contains this GPIO
 * @offsetp: Returns the offset number within this device
 * @gpiop: Returns the absolute GPIO number, numbered from 0
 */
int gpio_lookup_name(const char *name, struct udevice **devp,
		     unsigned int *offsetp, unsigned int *gpiop);

int name_to_gpio(const char *name);

#endif	/* _ASM_GENERIC_GPIO_H_ */
