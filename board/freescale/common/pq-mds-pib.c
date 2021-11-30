/*
 * Copyright (C) 2007 Freescale Semiconductor, Inc.
 *
 * Tony Li <tony.li@freescale.com>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 */

#include <common.h>
#include <i2c.h>
#include <asm/io.h>

#include "pq-mds-pib.h"

int pib_init(void)
{
	u8 val8;
	u8 orig_i2c_bus;

	/* Switch temporarily to I2C bus #2 */
	orig_i2c_bus = i2c_get_bus_num();
	i2c_set_bus_num(1);

	val8 = 0;
#if defined(CONFIG_PCI) && !defined(CONFIG_PCISLAVE)
	/* Assign PIB PMC slot to desired PCI bus */
	i2c_write(0x23, 0x6, 1, &val8, 1);
	i2c_write(0x23, 0x7, 1, &val8, 1);
	val8 = 0xff;
	i2c_write(0x23, 0x2, 1, &val8, 1);
	i2c_write(0x23, 0x3, 1, &val8, 1);

	val8 = 0;
	i2c_write(0x26, 0x6, 1, &val8, 1);
	val8 = 0x34;
	i2c_write(0x26, 0x7, 1, &val8, 1);
	val8 = 0xf3;		/* PMC1, PMC2, PMC3 slot to PCI bus */
	i2c_write(0x26, 0x2, 1, &val8, 1);
	val8 = 0xff;
	i2c_write(0x26, 0x3, 1, &val8, 1);

	val8 = 0;
	i2c_write(0x27, 0x6, 1, &val8, 1);
	i2c_write(0x27, 0x7, 1, &val8, 1);
	val8 = 0xff;
	i2c_write(0x27, 0x2, 1, &val8, 1);
	val8 = 0xef;
	i2c_write(0x27, 0x3, 1, &val8, 1);

	eieio();

	printf("PCI 32bit bus on PMC1 & PMC2 &PMC3\n");
#endif

	/* Reset to original I2C bus */
	i2c_set_bus_num(orig_i2c_bus);
	return 0;
}
