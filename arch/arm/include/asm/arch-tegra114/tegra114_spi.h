/*
 * NVIDIA Tegra SPI controller
 *
 * Copyright 2010-2013 NVIDIA Corporation
 *
 * This software may be used and distributed according to the
 * terms of the GNU Public License, Version 2, incorporated
 * herein by reference.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * Version 2 as published by the Free Software Foundation.
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

#ifndef _TEGRA114_SPI_H_
#define _TEGRA114_SPI_H_

#include <asm/types.h>

int tegra114_spi_init(int *node_list, int count);
int tegra114_spi_cs_is_valid(unsigned int bus, unsigned int cs);
struct spi_slave *tegra114_spi_setup_slave(unsigned int bus, unsigned int cs,
				  unsigned int max_hz, unsigned int mode);
void tegra114_spi_free_slave(struct spi_slave *slave);
int tegra114_spi_claim_bus(struct spi_slave *slave);
void tegra114_spi_cs_activate(struct spi_slave *slave);
void tegra114_spi_cs_deactivate(struct spi_slave *slave);
int tegra114_spi_xfer(struct spi_slave *slave, unsigned int bitlen,
		     const void *data_out, void *data_in, unsigned long flags);

#endif	/* _TEGRA114_SPI_H_ */
