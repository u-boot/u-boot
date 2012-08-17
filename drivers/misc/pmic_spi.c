/*
 * Copyright (C) 2011 Samsung Electronics
 * Lukasz Majewski <l.majewski@samsung.com>
 *
 * (C) Copyright 2010
 * Stefano Babic, DENX Software Engineering, sbabic@denx.de
 *
 * (C) Copyright 2008-2009 Freescale Semiconductor, Inc.
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <common.h>
#include <linux/types.h>
#include <pmic.h>
#include <spi.h>

static struct spi_slave *slave;

void pmic_spi_free(struct spi_slave *slave)
{
	if (slave)
		spi_free_slave(slave);
}

struct spi_slave *pmic_spi_probe(struct pmic *p)
{
	return spi_setup_slave(p->bus,
		p->hw.spi.cs,
		p->hw.spi.clk,
		p->hw.spi.mode);
}

static u32 pmic_reg(struct pmic *p, u32 reg, u32 *val, u32 write)
{
	u32 pmic_tx, pmic_rx;
	u32 tmp;

	if (!slave) {
		slave = pmic_spi_probe(p);

		if (!slave)
			return -1;
	}

	if (check_reg(reg))
		return -1;

	if (spi_claim_bus(slave))
		return -1;

	pmic_tx = p->hw.spi.prepare_tx(reg, val, write);

	tmp = cpu_to_be32(pmic_tx);

	if (spi_xfer(slave, pmic_spi_bitlen, &tmp, &pmic_rx,
			pmic_spi_flags)) {
		spi_release_bus(slave);
		return -1;
	}

	if (write) {
		pmic_tx = p->hw.spi.prepare_tx(reg, val, 0);
		tmp = cpu_to_be32(pmic_tx);
		if (spi_xfer(slave, pmic_spi_bitlen, &tmp, &pmic_rx,
			pmic_spi_flags)) {
			spi_release_bus(slave);
			return -1;
		}
	}

	spi_release_bus(slave);
	*val = cpu_to_be32(pmic_rx);

	return 0;
}

int pmic_reg_write(struct pmic *p, u32 reg, u32 val)
{
	if (pmic_reg(p, reg, &val, 1))
		return -1;

	return 0;
}

int pmic_reg_read(struct pmic *p, u32 reg, u32 *val)
{
	if (pmic_reg(p, reg, val, 0))
		return -1;

	return 0;
}
