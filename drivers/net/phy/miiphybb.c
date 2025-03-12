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

/*****************************************************************************
 *
 * Utility to send the preamble, address, and register (common to read
 * and write).
 */
static void miiphy_pre(struct mii_dev *miidev, const struct bb_miiphy_bus_ops *ops,
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

	ops->mdio_active(miidev);
	ops->set_mdio(miidev, 1);
	for (j = 0; j < 32; j++) {
		ops->set_mdc(miidev, 0);
		ops->delay(miidev);
		ops->set_mdc(miidev, 1);
		ops->delay(miidev);
	}

	/* send the start bit (01) and the read opcode (10) or write (10) */
	ops->set_mdc(miidev, 0);
	ops->set_mdio(miidev, 0);
	ops->delay(miidev);
	ops->set_mdc(miidev, 1);
	ops->delay(miidev);
	ops->set_mdc(miidev, 0);
	ops->set_mdio(miidev, 1);
	ops->delay(miidev);
	ops->set_mdc(miidev, 1);
	ops->delay(miidev);
	ops->set_mdc(miidev, 0);
	ops->set_mdio(miidev, read);
	ops->delay(miidev);
	ops->set_mdc(miidev, 1);
	ops->delay(miidev);
	ops->set_mdc(miidev, 0);
	ops->set_mdio(miidev, !read);
	ops->delay(miidev);
	ops->set_mdc(miidev, 1);
	ops->delay(miidev);

	/* send the PHY address */
	for (j = 0; j < 5; j++) {
		ops->set_mdc(miidev, 0);
		if ((addr & 0x10) == 0) {
			ops->set_mdio(miidev, 0);
		} else {
			ops->set_mdio(miidev, 1);
		}
		ops->delay(miidev);
		ops->set_mdc(miidev, 1);
		ops->delay(miidev);
		addr <<= 1;
	}

	/* send the register address */
	for (j = 0; j < 5; j++) {
		ops->set_mdc(miidev, 0);
		if ((reg & 0x10) == 0) {
			ops->set_mdio(miidev, 0);
		} else {
			ops->set_mdio(miidev, 1);
		}
		ops->delay(miidev);
		ops->set_mdc(miidev, 1);
		ops->delay(miidev);
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

	miiphy_pre(miidev, ops, 1, addr, reg);

	/* tri-state our MDIO I/O pin so we can read */
	ops->set_mdc(miidev, 0);
	ops->mdio_tristate(miidev);
	ops->delay(miidev);
	ops->set_mdc(miidev, 1);
	ops->delay(miidev);

	/* check the turnaround bit: the PHY should be driving it to zero */
	ops->get_mdio(miidev, &v);
	if (v != 0) {
		/* puts ("PHY didn't drive TA low\n"); */
		for (j = 0; j < 32; j++) {
			ops->set_mdc(miidev, 0);
			ops->delay(miidev);
			ops->set_mdc(miidev, 1);
			ops->delay(miidev);
		}
		/* There is no PHY, return */
		return -1;
	}

	ops->set_mdc(miidev, 0);
	ops->delay(miidev);

	/* read 16 bits of register data, MSB first */
	rdreg = 0;
	for (j = 0; j < 16; j++) {
		ops->set_mdc(miidev, 1);
		ops->delay(miidev);
		rdreg <<= 1;
		ops->get_mdio(miidev, &v);
		rdreg |= (v & 0x1);
		ops->set_mdc(miidev, 0);
		ops->delay(miidev);
	}

	ops->set_mdc(miidev, 1);
	ops->delay(miidev);
	ops->set_mdc(miidev, 0);
	ops->delay(miidev);
	ops->set_mdc(miidev, 1);
	ops->delay(miidev);

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
	int j;			/* counter */

	miiphy_pre(miidev, ops, 0, addr, reg);

	/* send the turnaround (10) */
	ops->set_mdc(miidev, 0);
	ops->set_mdio(miidev, 1);
	ops->delay(miidev);
	ops->set_mdc(miidev, 1);
	ops->delay(miidev);
	ops->set_mdc(miidev, 0);
	ops->set_mdio(miidev, 0);
	ops->delay(miidev);
	ops->set_mdc(miidev, 1);
	ops->delay(miidev);

	/* write 16 bits of register data, MSB first */
	for (j = 0; j < 16; j++) {
		ops->set_mdc(miidev, 0);
		if ((value & 0x00008000) == 0) {
			ops->set_mdio(miidev, 0);
		} else {
			ops->set_mdio(miidev, 1);
		}
		ops->delay(miidev);
		ops->set_mdc(miidev, 1);
		ops->delay(miidev);
		value <<= 1;
	}

	/*
	 * Tri-state the MDIO line.
	 */
	ops->mdio_tristate(miidev);
	ops->set_mdc(miidev, 0);
	ops->delay(miidev);
	ops->set_mdc(miidev, 1);
	ops->delay(miidev);

	return 0;
}
