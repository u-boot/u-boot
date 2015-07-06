/*
 * Copyright (c) 2015 Google, Inc
 * Written by Simon Glass <sjg@chromium.org>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __RESET_H
#define __RESET_H

enum reset_t {
	RESET_WARM,	/* Reset CPU, keep GPIOs active */
	RESET_COLD,	/* Reset CPU and GPIOs */
	RESET_POWER,	/* Reset PMIC (remove and restore power) */

	RESET_COUNT,
};

struct reset_ops {
	/**
	 * request() - request a reset of the given type
	 *
	 * Note that this function may return before the reset takes effect.
	 *
	 * @type:	Reset type to request
	 * @return -EINPROGRESS if the reset has been started and
	 *		will complete soon, -EPROTONOSUPPORT if not supported
	 *		by this device, 0 if the reset has already happened
	 *		(in which case this method will not actually return)
	 */
	int (*request)(struct udevice *dev, enum reset_t type);
};

#define reset_get_ops(dev)        ((struct reset_ops *)(dev)->driver->ops)

/**
 * reset_request() - request a reset
 *
 * @type:	Reset type to request
 * @return 0 if OK, -EPROTONOSUPPORT if not supported by this device
 */
int reset_request(struct udevice *dev, enum reset_t type);

/**
 * reset_walk() - cause a reset
 *
 * This works through the available reset devices until it finds one that can
 * perform a reset. If the provided reset type is not available, the next one
 * will be tried.
 *
 * If this function fails to reset, it will display a message and halt
 *
 * @type:	Reset type to request
 * @return -EINPROGRESS if a reset is in progress, -ENOSYS if not available
 */
int reset_walk(enum reset_t type);

/**
 * reset_walk_halt() - try to reset, otherwise halt
 *
 * This calls reset_walk(). If it returns, indicating that reset is not
 * supported, it prints a message and halts.
 */
void reset_walk_halt(enum reset_t type);

/**
 * reset_cpu() - calls reset_walk(RESET_WARM)
 */
void reset_cpu(ulong addr);

#endif
