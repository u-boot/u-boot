/*
 * Copyright (C) 2013 Simon Guinot <simon.guinot@sequanux.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 */

#ifndef _LACIE_CPLD_GPI0_BUS_H
#define _LACIE_CPLD_GPI0_BUS_H

struct cpld_gpio_bus {
	unsigned *addr;
	unsigned num_addr;
	unsigned *data;
	unsigned num_data;
	unsigned enable;
};

void cpld_gpio_bus_write(struct cpld_gpio_bus *cpld_gpio_bus,
			 unsigned addr, unsigned value);

#endif /* _LACIE_CPLD_GPI0_BUS_H */
