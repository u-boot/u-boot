// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2009 Industrie Dial Face S.p.A.
 * Luigi 'Comio' Mantellini <luigi.mantellini@idf-hit.com>
 *
 * (C) Copyright 2001
 * Gerald Van Baren, Custom IDEAS, vanbaren@cideas.com.
 */

/*
 * This provides a bit-banged interface to the ethernet MII management
 * channel.
 */

#include <ioports.h>
#include <ppc_asm.tmpl>
#include <malloc.h>
#include <miiphy.h>
#include <asm/global_data.h>

static inline struct bb_miiphy_bus *bb_miiphy_getbus(struct mii_dev *miidev)
{
	return container_of(miidev, struct bb_miiphy_bus, mii);
}

struct bb_miiphy_bus *bb_miiphy_alloc(void)
{
	struct bb_miiphy_bus *bus;

	bus = malloc(sizeof(*bus));
	if (!bus)
		return bus;

	mdio_init(&bus->mii);

	return bus;
}

void bb_miiphy_free(struct bb_miiphy_bus *bus)
{
	free(bus);
}

/*****************************************************************************
 *
 * Utility to send the preamble, address, and register (common to read
 * and write).
 */
static void miiphy_pre(struct bb_miiphy_bus *bus, const struct bb_miiphy_bus_ops *ops,
		       char read, unsigned char addr, unsigned char reg)
{
	int j;

	/*
	 * Send a 32 bit preamble ('1's) with an extra '1' bit for good measure.
	 * The IEEE spec says this is a PHY optional requirement.  The AMD
	 * 79C874 requires one after power up and one after a MII communications
	 * error.  This means that we are doing more preambles than we need,
	 * but it is safer and will be much more robust.
	 */

	ops->mdio_active(bus);
	ops->set_mdio(bus, 1);
	for (j = 0; j < 32; j++) {
		ops->set_mdc(bus, 0);
		ops->delay(bus);
		ops->set_mdc(bus, 1);
		ops->delay(bus);
	}

	/* send the start bit (01) and the read opcode (10) or write (10) */
	ops->set_mdc(bus, 0);
	ops->set_mdio(bus, 0);
	ops->delay(bus);
	ops->set_mdc(bus, 1);
	ops->delay(bus);
	ops->set_mdc(bus, 0);
	ops->set_mdio(bus, 1);
	ops->delay(bus);
	ops->set_mdc(bus, 1);
	ops->delay(bus);
	ops->set_mdc(bus, 0);
	ops->set_mdio(bus, read);
	ops->delay(bus);
	ops->set_mdc(bus, 1);
	ops->delay(bus);
	ops->set_mdc(bus, 0);
	ops->set_mdio(bus, !read);
	ops->delay(bus);
	ops->set_mdc(bus, 1);
	ops->delay(bus);

	/* send the PHY address */
	for (j = 0; j < 5; j++) {
		ops->set_mdc(bus, 0);
		if ((addr & 0x10) == 0) {
			ops->set_mdio(bus, 0);
		} else {
			ops->set_mdio(bus, 1);
		}
		ops->delay(bus);
		ops->set_mdc(bus, 1);
		ops->delay(bus);
		addr <<= 1;
	}

	/* send the register address */
	for (j = 0; j < 5; j++) {
		ops->set_mdc(bus, 0);
		if ((reg & 0x10) == 0) {
			ops->set_mdio(bus, 0);
		} else {
			ops->set_mdio(bus, 1);
		}
		ops->delay(bus);
		ops->set_mdc(bus, 1);
		ops->delay(bus);
		reg <<= 1;
	}
}

/*****************************************************************************
 *
 * Read a MII PHY register.
 *
 * Returns:
 *   0 on success
 */
int bb_miiphy_read(struct mii_dev *miidev, const struct bb_miiphy_bus_ops *ops,
		   int addr, int devad, int reg)
{
	unsigned short rdreg; /* register working value */
	int v;
	int j; /* counter */
	struct bb_miiphy_bus *bus;

	bus = bb_miiphy_getbus(miidev);
	if (bus == NULL) {
		return -1;
	}

	miiphy_pre(bus, ops, 1, addr, reg);

	/* tri-state our MDIO I/O pin so we can read */
	ops->set_mdc(bus, 0);
	ops->mdio_tristate(bus);
	ops->delay(bus);
	ops->set_mdc(bus, 1);
	ops->delay(bus);

	/* check the turnaround bit: the PHY should be driving it to zero */
	ops->get_mdio(bus, &v);
	if (v != 0) {
		/* puts ("PHY didn't drive TA low\n"); */
		for (j = 0; j < 32; j++) {
			ops->set_mdc(bus, 0);
			ops->delay(bus);
			ops->set_mdc(bus, 1);
			ops->delay(bus);
		}
		/* There is no PHY, return */
		return -1;
	}

	ops->set_mdc(bus, 0);
	ops->delay(bus);

	/* read 16 bits of register data, MSB first */
	rdreg = 0;
	for (j = 0; j < 16; j++) {
		ops->set_mdc(bus, 1);
		ops->delay(bus);
		rdreg <<= 1;
		ops->get_mdio(bus, &v);
		rdreg |= (v & 0x1);
		ops->set_mdc(bus, 0);
		ops->delay(bus);
	}

	ops->set_mdc(bus, 1);
	ops->delay(bus);
	ops->set_mdc(bus, 0);
	ops->delay(bus);
	ops->set_mdc(bus, 1);
	ops->delay(bus);

	debug("%s[%s](0x%x) @ 0x%x = 0x%04x\n", __func__, miidev->name, reg, addr, rdreg);

	return rdreg;
}

/*****************************************************************************
 *
 * Write a MII PHY register.
 *
 * Returns:
 *   0 on success
 */
int bb_miiphy_write(struct mii_dev *miidev, const struct bb_miiphy_bus_ops *ops,
		    int addr, int devad, int reg, u16 value)
{
	struct bb_miiphy_bus *bus;
	int j;			/* counter */

	bus = bb_miiphy_getbus(miidev);
	if (bus == NULL) {
		/* Bus not found! */
		return -1;
	}

	miiphy_pre(bus, ops, 0, addr, reg);

	/* send the turnaround (10) */
	ops->set_mdc(bus, 0);
	ops->set_mdio(bus, 1);
	ops->delay(bus);
	ops->set_mdc(bus, 1);
	ops->delay(bus);
	ops->set_mdc(bus, 0);
	ops->set_mdio(bus, 0);
	ops->delay(bus);
	ops->set_mdc(bus, 1);
	ops->delay(bus);

	/* write 16 bits of register data, MSB first */
	for (j = 0; j < 16; j++) {
		ops->set_mdc(bus, 0);
		if ((value & 0x00008000) == 0) {
			ops->set_mdio(bus, 0);
		} else {
			ops->set_mdio(bus, 1);
		}
		ops->delay(bus);
		ops->set_mdc(bus, 1);
		ops->delay(bus);
		value <<= 1;
	}

	/*
	 * Tri-state the MDIO line.
	 */
	ops->mdio_tristate(bus);
	ops->set_mdc(bus, 0);
	ops->delay(bus);
	ops->set_mdc(bus, 1);
	ops->delay(bus);

	return 0;
}
