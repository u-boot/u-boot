/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2015 Stephen Warren
 */

#ifndef _BUS_ADDR_H
#define _BUS_ADDR_H

#ifdef CONFIG_PHYS_TO_BUS
unsigned long phys_to_bus(unsigned long phys);
unsigned long bus_to_phys(unsigned long bus);
#else
static inline unsigned long phys_to_bus(unsigned long phys)
{
	return phys;
}

static inline unsigned long bus_to_phys(unsigned long bus)
{
	return bus;
}
#endif

#if CONFIG_IS_ENABLED(DM_DMA)
#include <dm/device.h>

static inline dma_addr_t dev_phys_to_bus(struct udevice *dev, phys_addr_t phys)
{
	/* If the PA is in the remap range, apply remap. */
	if (phys >= dev->dma_cpu && phys < dev->dma_cpu + dev->dma_size)
		phys -= dev->dma_cpu - dev->dma_bus;
	return phys;
}

static inline phys_addr_t dev_bus_to_phys(struct udevice *dev, dma_addr_t bus)
{
	/* If the DA is in the remap range, apply remap. */
	if (bus >= dev->dma_bus && bus < dev->dma_bus + dev->dma_size)
		bus += dev->dma_cpu - dev->dma_bus;
	return bus;
}
#else
#define dev_phys_to_bus(_, _addr)	_addr
#define dev_bus_to_phys(_, _addr)	_addr
#endif

#endif
