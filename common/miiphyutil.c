/*
 * (C) Copyright 2001
 * Gerald Van Baren, Custom IDEAS, vanbaren@cideas.com.
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

/*
 * This provides a bit-banged interface to the ethernet MII management
 * channel.
 */

#include <common.h>
#include <miiphy.h>

#if defined(CONFIG_MII) || (CONFIG_COMMANDS & CFG_CMD_MII)

/*****************************************************************************
 *
 * Read the OUI, manufacture's model number, and revision number.
 *
 * OUI:     22 bits (unsigned int)
 * Model:    6 bits (unsigned char)
 * Revision: 4 bits (unsigned char)
 *
 * Returns:
 *   0 on success
 */
int miiphy_info (unsigned char addr,
		 unsigned int *oui,
		 unsigned char *model, unsigned char *rev)
{
	unsigned int reg = 0;

	/*
	 * Trick: we are reading two 16 registers into a 32 bit variable
	 * so we do a 16 read into the high order bits of the variable (big
	 * endian, you know), shift it down 16 bits, and the read the rest.
	 */
	if (miiphy_read (addr, PHY_PHYIDR2, (unsigned short *) &reg) != 0) {
#ifdef DEBUG
		printf ("PHY ID register 2 read failed\n");
#endif
		return (-1);
	}
	reg >>= 16;

#ifdef DEBUG
	printf ("PHY_PHYIDR2 @ 0x%x = 0x%04x\n", addr, reg);
#endif
	if (reg == 0xFFFF) {
		/* No physical device present at this address */
		return (-1);
	}

	if (miiphy_read (addr, PHY_PHYIDR1, (unsigned short *) &reg) != 0) {
#ifdef DEBUG
		printf ("PHY ID register 1 read failed\n");
#endif
		return (-1);
	}
#ifdef DEBUG
	printf ("PHY_PHYIDR[1,2] @ 0x%x = 0x%08x\n", addr, reg);
#endif
	*oui   =                 ( reg >> 10);
	*model = (unsigned char) ((reg >>  4) & 0x0000003F);
	*rev   = (unsigned char) ( reg        & 0x0000000F);
	return (0);
}


/*****************************************************************************
 *
 * Reset the PHY.
 * Returns:
 *   0 on success
 */
int miiphy_reset (unsigned char addr)
{
	unsigned short reg;
	int loop_cnt;

	if (miiphy_write (addr, PHY_BMCR, 0x8000) != 0) {
#ifdef DEBUG
		printf ("PHY reset failed\n");
#endif
		return (-1);
	}

	/*
	 * Poll the control register for the reset bit to go to 0 (it is
	 * auto-clearing).  This should happen within 0.5 seconds per the
	 * IEEE spec.
	 */
	loop_cnt = 0;
	reg = 0x8000;
	while (((reg & 0x8000) != 0) && (loop_cnt++ < 1000000)) {
		if (miiphy_read (addr, PHY_BMCR, &reg) != 0) {
#     ifdef DEBUG
			printf ("PHY status read failed\n");
#     endif
			return (-1);
		}
	}
	if ((reg & 0x8000) == 0) {
		return (0);
	} else {
		printf ("PHY reset timed out\n");
		return (-1);
	}
	return (0);
}


/*****************************************************************************
 *
 * Determine the ethernet speed (10/100).
 */
int miiphy_speed (unsigned char addr)
{
	unsigned short reg;

	if (miiphy_read (addr, PHY_ANLPAR, &reg)) {
		printf ("PHY speed1 read failed, assuming 10bT\n");
		return (_10BASET);
	}

	if ((reg & PHY_ANLPAR_100) != 0) {
		return (_100BASET);
	} else {
		return (_10BASET);
	}
}


/*****************************************************************************
 *
 * Determine full/half duplex.
 */
int miiphy_duplex (unsigned char addr)
{
	unsigned short reg;

	if (miiphy_read (addr, PHY_ANLPAR, &reg)) {
		printf ("PHY duplex read failed, assuming half duplex\n");
		return (HALF);
	}

	if ((reg & (PHY_ANLPAR_10FD | PHY_ANLPAR_TXFD)) != 0) {
		return (FULL);
	} else {
		return (HALF);
	}
}

#endif /* CONFIG_MII || (CONFIG_COMMANDS & CFG_CMD_MII) */
