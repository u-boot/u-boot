/*
 *
 * (C) Copyright 2014 Freescale Semiconductor, Inc
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef _THERMAL_H_
#define _THERMAL_H_

#include <dm.h>

int thermal_get_temp(struct udevice *dev, int *temp);

/**
 * struct struct dm_thermal_ops - Driver model Thermal operations
 *
 * The uclass interface is implemented by all Thermal devices which use
 * driver model.
 */
struct dm_thermal_ops {
	/**
	 * Get the current temperature
	 *
	 * The device provided is the slave device. It's parent controller
	 * will be used to provide the communication.
	 *
	 * This must be called before doing any transfers with a Thermal slave.
	 * It will enable and initialize any Thermal hardware as necessary,
	 * and make sure that the SCK line is in the correct idle state. It is
	 * not allowed to claim the same bus for several slaves without
	 * releasing the bus in between.
	 *
	 * @dev:	The Thermal device
	 *
	 * Returns: 0 if the bus was claimed successfully, or a negative value
	 * if it wasn't.
	 */
	int (*get_temp)(struct udevice *dev, int *temp);
};

#endif	/* _THERMAL_H_ */
