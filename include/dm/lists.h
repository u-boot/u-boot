/*
 * Copyright (c) 2013 Google, Inc
 *
 * (C) Copyright 2012
 * Pavel Herrmann <morpheus.ibis@gmail.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef _DM_LISTS_H_
#define _DM_LISTS_H_

#include <dm/uclass-id.h>

/**
 * lists_driver_lookup_name() - Return u_boot_driver corresponding to name
 *
 * This function returns a pointer to a driver given its name. This is used
 * for binding a driver given its name and platdata.
 *
 * @name: Name of driver to look up
 * @return pointer to driver, or NULL if not found
 */
struct driver *lists_driver_lookup_name(const char *name);

/**
 * lists_uclass_lookup() - Return uclass_driver based on ID of the class
 * id:		ID of the class
 *
 * This function returns the pointer to uclass_driver, which is the class's
 * base structure based on the ID of the class. Returns NULL on error.
 */
struct uclass_driver *lists_uclass_lookup(enum uclass_id id);

int lists_bind_drivers(struct device *parent);

int lists_bind_fdt(struct device *parent, const void *blob, int offset);

#endif
