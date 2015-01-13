/*
 * Copyright (c) 2013 Google, Inc
 *
 * (C) Copyright 2012
 * Pavel Herrmann <morpheus.ibis@gmail.com>
 * Marek Vasut <marex@denx.de>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef _DM_PLATDATA_H
#define _DM_PLATDATA_H

#include <linker_lists.h>

/**
 * struct driver_info - Information required to instantiate a device
 *
 * @name:	Driver name
 * @platdata:	Driver-specific platform data
 */
struct driver_info {
	const char *name;
	const void *platdata;
};

#define U_BOOT_DEVICE(__name)						\
	ll_entry_declare(struct driver_info, __name, driver_info)

/* Declare a list of devices. The argument is a driver_info[] array */
#define U_BOOT_DEVICES(__name)						\
	ll_entry_declare_list(struct driver_info, __name, driver_info)

#endif
