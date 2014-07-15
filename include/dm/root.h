/*
 * Copyright (c) 2013 Google, Inc
 *
 * (C) Copyright 2012
 * Pavel Herrmann <morpheus.ibis@gmail.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef _DM_ROOT_H_
#define _DM_ROOT_H_

struct device;

/**
 * dm_root() - Return pointer to the top of the driver tree
 *
 * This function returns pointer to the root node of the driver tree,
 *
 * @return pointer to root device, or NULL if not inited yet
 */
struct device *dm_root(void);

/**
 * dm_scan_platdata() - Scan all platform data and bind drivers
 *
 * This scans all available platdata and creates drivers for each
 *
 * @return 0 if OK, -ve on error
 */
int dm_scan_platdata(void);

/**
 * dm_scan_fdt() - Scan the device tree and bind drivers
 *
 * This scans the device tree and creates a driver for each node
 *
 * @blob: Pointer to device tree blob
 * @return 0 if OK, -ve on error
 */
int dm_scan_fdt(const void *blob);

/**
 * dm_init() - Initialize Driver Model structures
 *
 * This function will initialize roots of driver tree and class tree.
 * This needs to be called before anything uses the DM
 *
 * @return 0 if OK, -ve on error
 */
int dm_init(void);

#endif
