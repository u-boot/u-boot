/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (c) 2013 Google, Inc
 *
 * (C) Copyright 2012
 * Pavel Herrmann <morpheus.ibis@gmail.com>
 * Marek Vasut <marex@denx.de>
 */

#ifndef _DM_PLATDATA_H
#define _DM_PLATDATA_H

#include <linker_lists.h>

/**
 * struct driver_info - Information required to instantiate a device
 *
 * NOTE: Avoid using this except in extreme circumstances, where device tree
 * is not feasible (e.g. serial driver in SPL where <8KB of SRAM is
 * available). U-Boot's driver model uses device tree for configuration.
 *
 * @name:	Driver name
 * @platdata:	Driver-specific platform data
 * @platdata_size: Size of platform data structure
 * @parent_idx:	Index of the parent driver_info structure
 */
struct driver_info {
	const char *name;
	const void *platdata;
#if CONFIG_IS_ENABLED(OF_PLATDATA)
	unsigned short platdata_size;
	short parent_idx;
#endif
};

#if CONFIG_IS_ENABLED(OF_PLATDATA)
#define driver_info_parent_id(driver_info)	driver_info->parent_idx
#else
#define driver_info_parent_id(driver_info)	(-1)
#endif

/**
 * driver_rt - runtime information set up by U-Boot
 *
 * There is one of these for every driver_info in the linker list, indexed by
 * the driver_info idx value.
 *
 * @dev: Device created from this idx
 */
struct driver_rt {
	struct udevice *dev;
};

/**
 * NOTE: Avoid using these except in extreme circumstances, where device tree
 * is not feasible (e.g. serial driver in SPL where <8KB of SRAM is
 * available). U-Boot's driver model uses device tree for configuration.
 *
 * When of-platdata is in use, U_BOOT_DEVICE() cannot be used outside of the
 * dt-platdata.c file created by dtoc
 */
#if CONFIG_IS_ENABLED(OF_PLATDATA) && !defined(DT_PLATDATA_C)
#define U_BOOT_DEVICE(__name)	_Static_assert(false, \
	"Cannot use U_BOOT_DEVICE with of-platdata. Please use devicetree instead")
#else
#define U_BOOT_DEVICE(__name)						\
	ll_entry_declare(struct driver_info, __name, driver_info)
#endif

/* Declare a list of devices. The argument is a driver_info[] array */
#define U_BOOT_DEVICES(__name)						\
	ll_entry_declare_list(struct driver_info, __name, driver_info)

/**
 * Get a pointer to a given device info given its name
 *
 * With the declaration U_BOOT_DEVICE(name), DM_GET_DEVICE(name) will return a
 * pointer to the struct driver_info created by that declaration.
 *
 * if OF_PLATDATA is enabled, from this it is possible to use the @dev member of
 * struct driver_info to find the device pointer itself.
 *
 * TODO(sjg@chromium.org): U_BOOT_DEVICE() tells U-Boot to create a device, so
 * the naming seems sensible, but DM_GET_DEVICE() is a bit of misnomer, since it
 * finds the driver_info record, not the device.
 *
 * @__name: Driver name (C identifier, not a string. E.g. gpio7_at_ff7e0000)
 * @return struct driver_info * to the driver that created the device
 */
#define DM_GET_DEVICE(__name)						\
	ll_entry_get(struct driver_info, __name, driver_info)

/**
 * dm_populate_phandle_data() - Populates phandle data in platda
 *
 * This populates phandle data with an U_BOOT_DEVICE entry get by
 * DM_GET_DEVICE. The implementation of this function will be done
 * by dtoc when parsing dtb.
 */
void dm_populate_phandle_data(void);
#endif
