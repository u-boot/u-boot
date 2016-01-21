/*
 * Copyright (c) 2015 Google, Inc
 * Written by Simon Glass <sjg@chromium.org>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef _CLK_H_
#define _CLK_H_

#include <errno.h>
#include <linux/types.h>

struct udevice;

int soc_clk_dump(void);

struct clk_ops {
	/**
	 * get_rate() - Get current clock rate
	 *
	 * @dev:	Device to check (UCLASS_CLK)
	 * @return clock rate in Hz, or -ve error code
	 */
	ulong (*get_rate)(struct udevice *dev);

	/**
	 * set_rate() - Set current clock rate
	 *
	 * @dev:	Device to adjust
	 * @rate:	New clock rate in Hz
	 * @return new rate, or -ve error code
	 */
	ulong (*set_rate)(struct udevice *dev, ulong rate);

	/**
	 * enable() - Enable the clock for a peripheral
	 *
	 * @dev:	clock provider
	 * @periph:	Peripheral ID to enable
	 * @return zero on success, or -ve error code
	 */
	int (*enable)(struct udevice *dev, int periph);

	/**
	 * get_periph_rate() - Get clock rate for a peripheral
	 *
	 * @dev:	Device to check (UCLASS_CLK)
	 * @periph:	Peripheral ID to check
	 * @return clock rate in Hz, or -ve error code
	 */
	ulong (*get_periph_rate)(struct udevice *dev, int periph);

	/**
	 * set_periph_rate() - Set current clock rate for a peripheral
	 *
	 * @dev:	Device to update (UCLASS_CLK)
	 * @periph:	Peripheral ID to update
	 * @return new clock rate in Hz, or -ve error code
	 */
	ulong (*set_periph_rate)(struct udevice *dev, int periph, ulong rate);
};

#define clk_get_ops(dev)	((struct clk_ops *)(dev)->driver->ops)

/**
 * clk_get_rate() - Get current clock rate
 *
 * @dev:	Device to check (UCLASS_CLK)
 * @return clock rate in Hz, or -ve error code
 */
ulong clk_get_rate(struct udevice *dev);

/**
 * clk_set_rate() - Set current clock rate
 *
 * @dev:	Device to adjust
 * @rate:	New clock rate in Hz
 * @return new rate, or -ve error code
 */
ulong clk_set_rate(struct udevice *dev, ulong rate);

/**
 * clk_enable() - Enable the clock for a peripheral
 *
 * @dev:	clock provider
 * @periph:	Peripheral ID to enable
 * @return zero on success, or -ve error code
 */
int clk_enable(struct udevice *dev, int periph);

/**
 * clk_get_periph_rate() - Get current clock rate for a peripheral
 *
 * @dev:	Device to check (UCLASS_CLK)
 * @return clock rate in Hz, -ve error code
 */
ulong clk_get_periph_rate(struct udevice *dev, int periph);

/**
 * clk_set_periph_rate() - Set current clock rate for a peripheral
 *
 * @dev:	Device to update (UCLASS_CLK)
 * @periph:	Peripheral ID to update
 * @return new clock rate in Hz, or -ve error code
 */
ulong clk_set_periph_rate(struct udevice *dev, int periph, ulong rate);

#if CONFIG_IS_ENABLED(OF_CONTROL)
/**
 * clk_get_by_index() - look up a clock referenced by a device
 *
 * Parse a device's 'clocks' list, returning information on the indexed clock,
 * ensuring that it is activated.
 *
 * @dev:	Device containing the clock reference
 * @index:	Clock index to return (0 = first)
 * @clk_devp:	Returns clock device
 * @return:	Peripheral ID for the device to control. This is the first
 *		argument after the clock node phandle. If there is no arguemnt,
 *		returns 0. Return -ve error code on any error
 */
int clk_get_by_index(struct udevice *dev, int index, struct udevice **clk_devp);
#else
static inline int clk_get_by_index(struct udevice *dev, int index,
				   struct udevice **clk_devp)
{
	return -ENOSYS;
}
#endif

#endif /* _CLK_H_ */
