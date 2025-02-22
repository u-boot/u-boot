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
#include <miiphy.h>
#include <asm/global_data.h>

static inline struct bb_miiphy_bus *bb_miiphy_getbus(const char *devname)
{
	int i;

	/* Search the correct bus */
	for (i = 0; i < bb_miiphy_buses_num; i++) {
		if (!strcmp(bb_miiphy_buses[i].name, devname)) {
			return &bb_miiphy_buses[i];
		}
	}
	return NULL;
}

/*****************************************************************************
 *
 * Utility to send the preamble, address, and register (common to read
 * and write).
 */
static void miiphy_pre(struct bb_miiphy_bus *bus, char read,
		       unsigned char addr, unsigned char reg)
{
	int j;

	/*
	 * Send a 32 bit preamble ('1's) with an extra '1' bit for good measure.
	 * The IEEE spec says this is a PHY optional requirement.  The AMD
	 * 79C874 requires one after power up and one after a MII communications
	 * error.  This means that we are doing more preambles than we need,
	 * but it is safer and will be much more robust.
	 */

	bus->mdio_active(bus);
	bus->set_mdio(bus, 1);
	for (j = 0; j < 32; j++) {
		bus->set_mdc(bus, 0);
		bus->delay(bus);
		bus->set_mdc(bus, 1);
		bus->delay(bus);
	}

	/* send the start bit (01) and the read opcode (10) or write (10) */
	bus->set_mdc(bus, 0);
	bus->set_mdio(bus, 0);
	bus->delay(bus);
	bus->set_mdc(bus, 1);
	bus->delay(bus);
	bus->set_mdc(bus, 0);
	bus->set_mdio(bus, 1);
	bus->delay(bus);
	bus->set_mdc(bus, 1);
	bus->delay(bus);
	bus->set_mdc(bus, 0);
	bus->set_mdio(bus, read);
	bus->delay(bus);
	bus->set_mdc(bus, 1);
	bus->delay(bus);
	bus->set_mdc(bus, 0);
	bus->set_mdio(bus, !read);
	bus->delay(bus);
	bus->set_mdc(bus, 1);
	bus->delay(bus);

	/* send the PHY address */
	for (j = 0; j < 5; j++) {
		bus->set_mdc(bus, 0);
		if ((addr & 0x10) == 0) {
			bus->set_mdio(bus, 0);
		} else {
			bus->set_mdio(bus, 1);
		}
		bus->delay(bus);
		bus->set_mdc(bus, 1);
		bus->delay(bus);
		addr <<= 1;
	}

	/* send the register address */
	for (j = 0; j < 5; j++) {
		bus->set_mdc(bus, 0);
		if ((reg & 0x10) == 0) {
			bus->set_mdio(bus, 0);
		} else {
			bus->set_mdio(bus, 1);
		}
		bus->delay(bus);
		bus->set_mdc(bus, 1);
		bus->delay(bus);
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
int bb_miiphy_read(struct mii_dev *miidev, int addr, int devad, int reg)
{
	unsigned short rdreg; /* register working value */
	int v;
	int j; /* counter */
	struct bb_miiphy_bus *bus;

	bus = bb_miiphy_getbus(miidev->name);
	if (bus == NULL) {
		return -1;
	}

	miiphy_pre (bus, 1, addr, reg);

	/* tri-state our MDIO I/O pin so we can read */
	bus->set_mdc(bus, 0);
	bus->mdio_tristate(bus);
	bus->delay(bus);
	bus->set_mdc(bus, 1);
	bus->delay(bus);

	/* check the turnaround bit: the PHY should be driving it to zero */
	bus->get_mdio(bus, &v);
	if (v != 0) {
		/* puts ("PHY didn't drive TA low\n"); */
		for (j = 0; j < 32; j++) {
			bus->set_mdc(bus, 0);
			bus->delay(bus);
			bus->set_mdc(bus, 1);
			bus->delay(bus);
		}
		/* There is no PHY, return */
		return -1;
	}

	bus->set_mdc(bus, 0);
	bus->delay(bus);

	/* read 16 bits of register data, MSB first */
	rdreg = 0;
	for (j = 0; j < 16; j++) {
		bus->set_mdc(bus, 1);
		bus->delay(bus);
		rdreg <<= 1;
		bus->get_mdio(bus, &v);
		rdreg |= (v & 0x1);
		bus->set_mdc(bus, 0);
		bus->delay(bus);
	}

	bus->set_mdc(bus, 1);
	bus->delay(bus);
	bus->set_mdc(bus, 0);
	bus->delay(bus);
	bus->set_mdc(bus, 1);
	bus->delay(bus);

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
int bb_miiphy_write(struct mii_dev *miidev, int addr, int devad, int reg,
		    u16 value)
{
	struct bb_miiphy_bus *bus;
	int j;			/* counter */

	bus = bb_miiphy_getbus(miidev->name);
	if (bus == NULL) {
		/* Bus not found! */
		return -1;
	}

	miiphy_pre (bus, 0, addr, reg);

	/* send the turnaround (10) */
	bus->set_mdc(bus, 0);
	bus->set_mdio(bus, 1);
	bus->delay(bus);
	bus->set_mdc(bus, 1);
	bus->delay(bus);
	bus->set_mdc(bus, 0);
	bus->set_mdio(bus, 0);
	bus->delay(bus);
	bus->set_mdc(bus, 1);
	bus->delay(bus);

	/* write 16 bits of register data, MSB first */
	for (j = 0; j < 16; j++) {
		bus->set_mdc(bus, 0);
		if ((value & 0x00008000) == 0) {
			bus->set_mdio(bus, 0);
		} else {
			bus->set_mdio(bus, 1);
		}
		bus->delay(bus);
		bus->set_mdc(bus, 1);
		bus->delay(bus);
		value <<= 1;
	}

	/*
	 * Tri-state the MDIO line.
	 */
	bus->mdio_tristate(bus);
	bus->set_mdc(bus, 0);
	bus->delay(bus);
	bus->set_mdc(bus, 1);
	bus->delay(bus);

	return 0;
}
