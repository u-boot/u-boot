/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2018
 * Quentin Schulz, Bootlin, quentin.schulz@bootlin.com
 *
 * Structure for use with U_BOOT_DEVICE for pl022 SPI devices or to use
 * in ofdata_to_platdata.
 */

#ifndef __PL022_SPI_H__
#define __PL022_SPI_H__

#if !CONFIG_IS_ENABLED(OF_PLATDATA)
#include <clk.h>
#endif
#include <fdtdec.h>

struct pl022_spi_pdata {
	fdt_addr_t addr;
	fdt_size_t size;
#if !CONFIG_IS_ENABLED(OF_PLATDATA)
	struct clk clk;
#else
	unsigned int freq;
#endif
};

#endif
