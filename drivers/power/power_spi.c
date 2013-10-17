/*
 * Copyright (C) 2011 Samsung Electronics
 * Lukasz Majewski <l.majewski@samsung.com>
 *
 * (C) Copyright 2010
 * Stefano Babic, DENX Software Engineering, sbabic@denx.de
 *
 * (C) Copyright 2008-2009 Freescale Semiconductor, Inc.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <linux/types.h>
#include <power/pmic.h>
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

	if (check_reg(p, reg))
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
