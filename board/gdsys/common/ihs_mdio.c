/*
 * (C) Copyright 2014
 * Dirk Eibach,  Guntermann & Drunck GmbH, eibach@gdsys.de
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>

#include <gdsys_fpga.h>
#include <miiphy.h>

#include "ihs_mdio.h"

static int ihs_mdio_idle(struct mii_dev *bus)
{
	struct ihs_mdio_info *info = bus->priv;
	u16 val;
	unsigned int ctr = 0;

	do {
		FPGA_GET_REG(info->fpga, mdio.control, &val);
		udelay(100);
		if (ctr++ > 10)
			return -1;
	} while (!(val & (1 << 12)));

	return 0;
}

static int ihs_mdio_reset(struct mii_dev *bus)
{
	ihs_mdio_idle(bus);

	return 0;
}

static int ihs_mdio_read(struct mii_dev *bus, int addr, int dev_addr,
			 int regnum)
{
	struct ihs_mdio_info *info = bus->priv;
	u16 val;

	ihs_mdio_idle(bus);

	FPGA_SET_REG(info->fpga, mdio.control,
		     ((addr & 0x1f) << 5) | (regnum & 0x1f) | (2 << 10));

	/* wait for rx data available */
	udelay(100);

	FPGA_GET_REG(info->fpga, mdio.rx_data, &val);

	return val;
}

static int ihs_mdio_write(struct mii_dev *bus, int addr, int dev_addr,
			  int regnum, u16 value)
{
	struct ihs_mdio_info *info = bus->priv;

	ihs_mdio_idle(bus);

	FPGA_SET_REG(info->fpga, mdio.address_data, value);
	FPGA_SET_REG(info->fpga, mdio.control,
		     ((addr & 0x1f) << 5) | (regnum & 0x1f) | (1 << 10));

	return 0;
}

int ihs_mdio_init(struct ihs_mdio_info *info)
{
	struct mii_dev *bus = mdio_alloc();

	if (!bus) {
		printf("Failed to allocate FSL MDIO bus\n");
		return -1;
	}

	bus->read = ihs_mdio_read;
	bus->write = ihs_mdio_write;
	bus->reset = ihs_mdio_reset;
	strcpy(bus->name, info->name);

	bus->priv = info;

	return mdio_register(bus);
}
