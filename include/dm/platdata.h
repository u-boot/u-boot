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
 * @dev:	Device created from this structure data
 */
struct driver_info {
	const char *name;
	const void *platdata;
#if CONFIG_IS_ENABLED(OF_PLATDATA)
	uint platdata_size;
	struct udevice *dev;
#endif
};

/**
 * NOTE: Avoid using these except in extreme circumstances, where device tree
 * is not feasible (e.g. serial driver in SPL where <8KB of SRAM is
 * available). U-Boot's driver model uses device tree for configuration.
 */
#define U_BOOT_DEVICE(__name)						\
	ll_entry_declare(struct driver_info, __name, driver_info)

/* Declare a list of devices. The argument is a driver_info[] array */
#define U_BOOT_DEVICES(__name)						\
	ll_entry_declare_list(struct driver_info, __name, driver_info)

/* Get a pointer to a given driver */
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
