// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2010
 * Dirk Eibach,  Guntermann & Drunck GmbH, dirk.eibach@gdsys.cc
 */

#include <common.h>
#include <miiphy.h>

#include <asm/io.h>

struct io_bb_pinset {
	int mdio;
	int mdc;
};

static int io_bb_mii_init(struct bb_miiphy_bus *bus)
{
	return 0;
}

static int io_bb_mdio_active(struct bb_miiphy_bus *bus)
{
	struct io_bb_pinset *pins = bus->priv;

	out_be32((void *)GPIO0_TCR,
		in_be32((void *)GPIO0_TCR) | pins->mdio);

	return 0;
}

static int io_bb_mdio_tristate(struct bb_miiphy_bus *bus)
{
	struct io_bb_pinset *pins = bus->priv;

	out_be32((void *)GPIO0_TCR,
		in_be32((void *)GPIO0_TCR) & ~pins->mdio);

	return 0;
}

static int io_bb_set_mdio(struct bb_miiphy_bus *bus, int v)
{
	struct io_bb_pinset *pins = bus->priv;

	if (v)
		out_be32((void *)GPIO0_OR,
			in_be32((void *)GPIO0_OR) | pins->mdio);
	else
		out_be32((void *)GPIO0_OR,
			in_be32((void *)GPIO0_OR) & ~pins->mdio);

	return 0;
}

static int io_bb_get_mdio(struct bb_miiphy_bus *bus, int *v)
{
	struct io_bb_pinset *pins = bus->priv;

	*v = ((in_be32((void *)GPIO0_IR) & pins->mdio) != 0);

	return 0;
}

static int io_bb_set_mdc(struct bb_miiphy_bus *bus, int v)
{
	struct io_bb_pinset *pins = bus->priv;

	if (v)
		out_be32((void *)GPIO0_OR,
			in_be32((void *)GPIO0_OR) | pins->mdc);
	else
		out_be32((void *)GPIO0_OR,
			in_be32((void *)GPIO0_OR) & ~pins->mdc);

	return 0;
}

static int io_bb_delay(struct bb_miiphy_bus *bus)
{
	udelay(1);

	return 0;
}

struct io_bb_pinset io_bb_pinsets[] = {
	{
		.mdio = CONFIG_SYS_MDIO_PIN,
		.mdc = CONFIG_SYS_MDC_PIN,
	},
#ifdef CONFIG_SYS_GBIT_MII1_BUSNAME
	{
		.mdio = CONFIG_SYS_MDIO1_PIN,
		.mdc = CONFIG_SYS_MDC1_PIN,
	},
#endif
};

struct bb_miiphy_bus bb_miiphy_buses[] = {
	{
		.name = CONFIG_SYS_GBIT_MII_BUSNAME,
		.init = io_bb_mii_init,
		.mdio_active = io_bb_mdio_active,
		.mdio_tristate = io_bb_mdio_tristate,
		.set_mdio = io_bb_set_mdio,
		.get_mdio = io_bb_get_mdio,
		.set_mdc = io_bb_set_mdc,
		.delay = io_bb_delay,
		.priv = &io_bb_pinsets[0],
	},
#ifdef CONFIG_SYS_GBIT_MII1_BUSNAME
	{
		.name = CONFIG_SYS_GBIT_MII1_BUSNAME,
		.init = io_bb_mii_init,
		.mdio_active = io_bb_mdio_active,
		.mdio_tristate = io_bb_mdio_tristate,
		.set_mdio = io_bb_set_mdio,
		.get_mdio = io_bb_get_mdio,
		.set_mdc = io_bb_set_mdc,
		.delay = io_bb_delay,
		.priv = &io_bb_pinsets[1],
	},
#endif
};

int bb_miiphy_buses_num = sizeof(bb_miiphy_buses) /
			  sizeof(bb_miiphy_buses[0]);
