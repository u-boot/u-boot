/*
 * (C) Copyright 2010
 * Dirk Eibach,  Guntermann & Drunck GmbH, eibach@gdsys.de
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

#include <common.h>
#include <miiphy.h>

#include <asm/io.h>

static int io_bb_mii_init(struct bb_miiphy_bus *bus)
{
	return 0;
}

static int io_bb_mdio_active(struct bb_miiphy_bus *bus)
{
	out_be32((void *)GPIO0_TCR,
		in_be32((void *)GPIO0_TCR) | CONFIG_SYS_MDIO_PIN);

	return 0;
}

static int io_bb_mdio_tristate(struct bb_miiphy_bus *bus)
{
	out_be32((void *)GPIO0_TCR,
		in_be32((void *)GPIO0_TCR) & ~CONFIG_SYS_MDIO_PIN);

	return 0;
}

static int io_bb_set_mdio(struct bb_miiphy_bus *bus, int v)
{
	if (v)
		out_be32((void *)GPIO0_OR,
			in_be32((void *)GPIO0_OR) | CONFIG_SYS_MDIO_PIN);
	else
		out_be32((void *)GPIO0_OR,
			in_be32((void *)GPIO0_OR) & ~CONFIG_SYS_MDIO_PIN);

	return 0;
}

static int io_bb_get_mdio(struct bb_miiphy_bus *bus, int *v)
{
	*v = ((in_be32((void *)GPIO0_IR) & CONFIG_SYS_MDIO_PIN) != 0);

	return 0;
}

static int io_bb_set_mdc(struct bb_miiphy_bus *bus, int v)
{
	if (v)
		out_be32((void *)GPIO0_OR,
			in_be32((void *)GPIO0_OR) | CONFIG_SYS_MDC_PIN);
	else
		out_be32((void *)GPIO0_OR,
			in_be32((void *)GPIO0_OR) & ~CONFIG_SYS_MDC_PIN);

	return 0;
}

static int io_bb_delay(struct bb_miiphy_bus *bus)
{
	udelay(1);

	return 0;
}

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
	}
};

int bb_miiphy_buses_num = sizeof(bb_miiphy_buses) /
			  sizeof(bb_miiphy_buses[0]);
