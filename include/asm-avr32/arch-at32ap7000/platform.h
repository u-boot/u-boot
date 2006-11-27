/*
 * Copyright (C) 2005-2006 Atmel Corporation
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */
#ifndef _ASM_AVR32_ARCH_PM_H
#define _ASM_AVR32_ARCH_PM_H

#include <config.h>

enum clock_domain_id {
	CLOCK_CPU,
	CLOCK_HSB,
	CLOCK_PBA,
	CLOCK_PBB,
	NR_CLOCK_DOMAINS,
};

enum resource_type {
	RESOURCE_GPIO,
	RESOURCE_CLOCK,
};

enum gpio_func {
	GPIO_FUNC_GPIO,
	GPIO_FUNC_A,
	GPIO_FUNC_B,
};

enum device_id {
	DEVICE_HEBI,
	DEVICE_PBA_BRIDGE,
	DEVICE_PBB_BRIDGE,
	DEVICE_HRAMC,
	/* GPIO controllers must be kept together */
	DEVICE_PIOA,
	DEVICE_PIOB,
	DEVICE_PIOC,
	DEVICE_PIOD,
	DEVICE_PIOE,
	DEVICE_SM,
	DEVICE_INTC,
	DEVICE_HMATRIX,
#if defined(CFG_HPDC)
	DEVICE_HPDC,
#endif
#if defined(CFG_MACB0)
	DEVICE_MACB0,
#endif
#if defined(CFG_MACB1)
	DEVICE_MACB1,
#endif
#if defined(CFG_LCDC)
	DEVICE_LCDC,
#endif
#if defined(CFG_USART0)
	DEVICE_USART0,
#endif
#if defined(CFG_USART1)
	DEVICE_USART1,
#endif
#if defined(CFG_USART2)
	DEVICE_USART2,
#endif
#if defined(CFG_USART3)
	DEVICE_USART3,
#endif
#if defined(CFG_MMCI)
	DEVICE_MMCI,
#endif
#if defined(CFG_DMAC)
	DEVICE_DMAC,
#endif
	NR_DEVICES,
	NO_DEVICE = -1,
};

struct resource {
	enum resource_type type;
	union {
		struct {
			unsigned long base;
		} iomem;
		struct {
			unsigned char nr_pins;
			enum device_id gpio_dev;
			enum gpio_func func;
			unsigned short start;
		} gpio;
		struct {
			enum clock_domain_id id;
			unsigned char index;
		} clock;
	} u;
};

struct device {
	void *regs;
	unsigned int nr_resources;
	const struct resource *resource;
};

struct clock_domain {
	unsigned short reg;
	enum clock_domain_id id;
	enum device_id bridge;
};

extern const struct device chip_device[NR_DEVICES];
extern const struct clock_domain chip_clock[NR_CLOCK_DOMAINS];

/**
 * Set up PIO, clock management and I/O memory for a device.
 */
const struct device *get_device(enum device_id devid);
void put_device(const struct device *dev);

int gpio_set_func(enum device_id gpio_devid, unsigned int start,
		  unsigned int nr_pins, enum gpio_func func);
void gpio_free(enum device_id gpio_devid, unsigned int start,
	       unsigned int nr_pins);

void pm_init(void);
int pm_enable_clock(enum clock_domain_id id, unsigned int index);
void pm_disable_clock(enum clock_domain_id id, unsigned int index);
unsigned long pm_get_clock_freq(enum clock_domain_id domain);

void cpu_enable_sdram(void);

#endif /* _ASM_AVR32_ARCH_PM_H */
