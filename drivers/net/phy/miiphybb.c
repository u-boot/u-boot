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
#include <ioports.h>
#include <ppc_asm.tmpl>

/*****************************************************************************
 *
 * Utility to send the preamble, address, and register (common to read
 * and write).
 */
static void miiphy_pre (char read, unsigned char addr, unsigned char reg)
{
	int j;			/* counter */
#if !(defined(CONFIG_EP8248) || defined(CONFIG_EP82XXM))
	volatile ioport_t *iop = ioport_addr ((immap_t *) CONFIG_SYS_IMMR, MDIO_PORT);
#endif

	/*
	 * Send a 32 bit preamble ('1's) with an extra '1' bit for good measure.
	 * The IEEE spec says this is a PHY optional requirement.  The AMD
	 * 79C874 requires one after power up and one after a MII communications
	 * error.  This means that we are doing more preambles than we need,
	 * but it is safer and will be much more robust.
	 */

	MDIO_ACTIVE;
	MDIO (1);
	for (j = 0; j < 32; j++) {
		MDC (0);
		MIIDELAY;
		MDC (1);
		MIIDELAY;
	}

	/* send the start bit (01) and the read opcode (10) or write (10) */
	MDC (0);
	MDIO (0);
	MIIDELAY;
	MDC (1);
	MIIDELAY;
	MDC (0);
	MDIO (1);
	MIIDELAY;
	MDC (1);
	MIIDELAY;
	MDC (0);
	MDIO (read);
	MIIDELAY;
	MDC (1);
	MIIDELAY;
	MDC (0);
	MDIO (!read);
	MIIDELAY;
	MDC (1);
	MIIDELAY;

	/* send the PHY address */
	for (j = 0; j < 5; j++) {
		MDC (0);
		if ((addr & 0x10) == 0) {
			MDIO (0);
		} else {
			MDIO (1);
		}
		MIIDELAY;
		MDC (1);
		MIIDELAY;
		addr <<= 1;
	}

	/* send the register address */
	for (j = 0; j < 5; j++) {
		MDC (0);
		if ((reg & 0x10) == 0) {
			MDIO (0);
		} else {
			MDIO (1);
		}
		MIIDELAY;
		MDC (1);
		MIIDELAY;
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
int bb_miiphy_read (char *devname, unsigned char addr,
		unsigned char reg, unsigned short *value)
{
	short rdreg;		/* register working value */
	int j;			/* counter */
#if !(defined(CONFIG_EP8248) || defined(CONFIG_EP82XXM))
	volatile ioport_t *iop = ioport_addr ((immap_t *) CONFIG_SYS_IMMR, MDIO_PORT);
#endif

	if (value == NULL) {
		puts("NULL value pointer\n");
		return (-1);
	}

	miiphy_pre (1, addr, reg);

	/* tri-state our MDIO I/O pin so we can read */
	MDC (0);
	MDIO_TRISTATE;
	MIIDELAY;
	MDC (1);
	MIIDELAY;

	/* check the turnaround bit: the PHY should be driving it to zero */
	if (MDIO_READ != 0) {
		/* puts ("PHY didn't drive TA low\n"); */
		for (j = 0; j < 32; j++) {
			MDC (0);
			MIIDELAY;
			MDC (1);
			MIIDELAY;
		}
		/* There is no PHY, set value to 0xFFFF and return */
		*value = 0xFFFF;
		return (-1);
	}

	MDC (0);
	MIIDELAY;

	/* read 16 bits of register data, MSB first */
	rdreg = 0;
	for (j = 0; j < 16; j++) {
		MDC (1);
		MIIDELAY;
		rdreg <<= 1;
		rdreg |= MDIO_READ;
		MDC (0);
		MIIDELAY;
	}

	MDC (1);
	MIIDELAY;
	MDC (0);
	MIIDELAY;
	MDC (1);
	MIIDELAY;

	*value = rdreg;

#ifdef DEBUG
	printf ("miiphy_read(0x%x) @ 0x%x = 0x%04x\n", reg, addr, *value);
#endif

	return 0;
}


/*****************************************************************************
 *
 * Write a MII PHY register.
 *
 * Returns:
 *   0 on success
 */
int bb_miiphy_write (char *devname, unsigned char addr,
		unsigned char reg, unsigned short value)
{
	int j;			/* counter */
#if !(defined(CONFIG_EP8248) || defined(CONFIG_EP82XXM))
	volatile ioport_t *iop = ioport_addr ((immap_t *) CONFIG_SYS_IMMR, MDIO_PORT);
#endif

	miiphy_pre (0, addr, reg);

	/* send the turnaround (10) */
	MDC (0);
	MDIO (1);
	MIIDELAY;
	MDC (1);
	MIIDELAY;
	MDC (0);
	MDIO (0);
	MIIDELAY;
	MDC (1);
	MIIDELAY;

	/* write 16 bits of register data, MSB first */
	for (j = 0; j < 16; j++) {
		MDC (0);
		if ((value & 0x00008000) == 0) {
			MDIO (0);
		} else {
			MDIO (1);
		}
		MIIDELAY;
		MDC (1);
		MIIDELAY;
		value <<= 1;
	}

	/*
	 * Tri-state the MDIO line.
	 */
	MDIO_TRISTATE;
	MDC (0);
	MIIDELAY;
	MDC (1);
	MIIDELAY;

	return 0;
}
