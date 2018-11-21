/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (c) 2017 Google, Inc
 *
 * (C) Copyright 2012
 * Pavel Herrmann <morpheus.ibis@gmail.com>
 * Marek Vasut <marex@denx.de>
 */

#ifndef _DM_FDTADDR_H
#define _DM_FDTADDR_H

#include <fdtdec.h>

struct udevice;

/**
 * devfdt_get_addr() - Get the reg property of a device
 *
 * @dev: Pointer to a device
 *
 * @return addr
 */
fdt_addr_t devfdt_get_addr(struct udevice *dev);

/**
 * devfdt_get_addr_ptr() - Return pointer to the address of the reg property
 *                      of a device
 *
 * @dev: Pointer to a device
 *
 * @return Pointer to addr, or NULL if there is no such property
 */
void *devfdt_get_addr_ptr(struct udevice *dev);

/**
 * devfdt_remap_addr() - Return pointer to the memory-mapped I/O address
 *                           of the reg property of a device
 *
 * @dev: Pointer to a device
 *
 * @return Pointer to addr, or NULL if there is no such property
 */
void *devfdt_remap_addr(struct udevice *dev);

/**
 * devfdt_remap_addr_index() - Return indexed pointer to the memory-mapped
 *                                 I/O address of the reg property of a device
 * @index: the 'reg' property can hold a list of <addr, size> pairs
 *	   and @index is used to select which one is required
 *
 * @dev: Pointer to a device
 *
 * @return Pointer to addr, or NULL if there is no such property
 */
void *devfdt_remap_addr_index(struct udevice *dev, int index);

/**
 * devfdt_map_physmem() - Read device address from reg property of the
 *                     device node and map the address into CPU address
 *                     space.
 *
 * @dev: Pointer to device
 * @size: size of the memory to map
 *
 * @return  mapped address, or NULL if the device does not have reg
 *          property.
 */
void *devfdt_map_physmem(struct udevice *dev, unsigned long size);

/**
 * devfdt_get_addr_index() - Get the indexed reg property of a device
 *
 * @dev: Pointer to a device
 * @index: the 'reg' property can hold a list of <addr, size> pairs
 *	   and @index is used to select which one is required
 *
 * @return addr
 */
fdt_addr_t devfdt_get_addr_index(struct udevice *dev, int index);

/**
 * devfdt_get_addr_size_index() - Get the indexed reg property of a device
 *
 * Returns the address and size specified in the 'reg' property of a device.
 *
 * @dev: Pointer to a device
 * @index: the 'reg' property can hold a list of <addr, size> pairs
 *	   and @index is used to select which one is required
 * @size: Pointer to size varible - this function returns the size
 *        specified in the 'reg' property here
 *
 * @return addr
 */
fdt_addr_t devfdt_get_addr_size_index(struct udevice *dev, int index,
				   fdt_size_t *size);

/**
 * devfdt_get_addr_name() - Get the reg property of a device, indexed by name
 *
 * @dev: Pointer to a device
 * @name: the 'reg' property can hold a list of <addr, size> pairs, with the
 *	  'reg-names' property providing named-based identification. @index
 *	  indicates the value to search for in 'reg-names'.
 *
 * @return addr
 */
fdt_addr_t devfdt_get_addr_name(struct udevice *dev, const char *name);

/**
 * dm_set_translation_offset() - Set translation offset
 * @offs: Translation offset
 *
 * Some platforms need a special address translation. Those
 * platforms (e.g. mvebu in SPL) can configure a translation
 * offset in the DM by calling this function. It will be
 * added to all addresses returned in devfdt_get_addr().
 */
void dm_set_translation_offset(fdt_addr_t offs);

/**
 * dm_get_translation_offset() - Get translation offset
 *
 * This function returns the translation offset that can
 * be configured by calling dm_set_translation_offset().
 *
 * @return translation offset for the device address (0 as default).
 */
fdt_addr_t dm_get_translation_offset(void);

#endif
