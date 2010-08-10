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

#include <asm/types.h>
#include <linux/list.h>
#include <malloc.h>
#include <net.h>

/* local debug macro */
#undef MII_DEBUG

#undef debug
#ifdef MII_DEBUG
#define debug(fmt,args...)	printf (fmt ,##args)
#else
#define debug(fmt,args...)
#endif /* MII_DEBUG */

struct mii_dev {
	struct list_head link;
	const char *name;
	int (*read) (const char *devname, unsigned char addr,
		     unsigned char reg, unsigned short *value);
	int (*write) (const char *devname, unsigned char addr,
		      unsigned char reg, unsigned short value);
};

static struct list_head mii_devs;
static struct mii_dev *current_mii;

/*
 * Lookup the mii_dev struct by the registered device name.
 */
static struct mii_dev *miiphy_get_dev_by_name(const char *devname, int quiet)
{
	struct list_head *entry;
	struct mii_dev *dev;

	if (!devname) {
		printf("NULL device name!\n");
		return NULL;
	}

	list_for_each(entry, &mii_devs) {
		dev = list_entry(entry, struct mii_dev, link);
		if (strcmp(dev->name, devname) == 0)
			return dev;
	}

	if (!quiet)
		printf("No such device: %s\n", devname);
	return NULL;
}

/*****************************************************************************
 *
 * Initialize global data. Need to be called before any other miiphy routine.
 */
void miiphy_init(void)
{
	INIT_LIST_HEAD (&mii_devs);
	current_mii = NULL;
}

/*****************************************************************************
 *
 * Register read and write MII access routines for the device <name>.
 */
void miiphy_register(const char *name,
		      int (*read) (const char *devname, unsigned char addr,
				   unsigned char reg, unsigned short *value),
		      int (*write) (const char *devname, unsigned char addr,
				    unsigned char reg, unsigned short value))
{
	struct mii_dev *new_dev;
	unsigned int name_len;
	char *new_name;

	/* check if we have unique name */
	new_dev = miiphy_get_dev_by_name(name, 1);
	if (new_dev) {
		printf("miiphy_register: non unique device name '%s'\n", name);
		return;
	}

	/* allocate memory */
	name_len = strlen (name);
	new_dev =
	    (struct mii_dev *)malloc (sizeof (struct mii_dev) + name_len + 1);

	if (new_dev == NULL) {
		printf ("miiphy_register: cannot allocate memory for '%s'\n",
			name);
		return;
	}
	memset (new_dev, 0, sizeof (struct mii_dev) + name_len);

	/* initalize mii_dev struct fields */
	INIT_LIST_HEAD (&new_dev->link);
	new_dev->read = read;
	new_dev->write = write;
	new_dev->name = new_name = (char *)(new_dev + 1);
	strncpy (new_name, name, name_len);
	new_name[name_len] = '\0';

	debug ("miiphy_register: added '%s', read=0x%08lx, write=0x%08lx\n",
	       new_dev->name, new_dev->read, new_dev->write);

	/* add it to the list */
	list_add_tail (&new_dev->link, &mii_devs);

	if (!current_mii)
		current_mii = new_dev;
}

int miiphy_set_current_dev(const char *devname)
{
	struct mii_dev *dev;

	dev = miiphy_get_dev_by_name(devname, 0);
	if (dev) {
		current_mii = dev;
		return 0;
	}

	return 1;
}

const char *miiphy_get_current_dev(void)
{
	if (current_mii)
		return current_mii->name;

	return NULL;
}

static struct mii_dev *miiphy_get_active_dev(const char *devname)
{
	/* If the current mii is the one we want, return it */
	if (current_mii)
		if (strcmp(current_mii->name, devname) == 0)
			return current_mii;

	/* Otherwise, set the active one to the one we want */
	if (miiphy_set_current_dev(devname))
		return NULL;
	else
		return current_mii;
}

/*****************************************************************************
 *
 * Read to variable <value> from the PHY attached to device <devname>,
 * use PHY address <addr> and register <reg>.
 *
 * Returns:
 *   0 on success
 */
int miiphy_read(const char *devname, unsigned char addr, unsigned char reg,
		 unsigned short *value)
{
	struct mii_dev *dev;

	dev = miiphy_get_active_dev(devname);
	if (dev)
		return dev->read(devname, addr, reg, value);

	return 1;
}

/*****************************************************************************
 *
 * Write <value> to the PHY attached to device <devname>,
 * use PHY address <addr> and register <reg>.
 *
 * Returns:
 *   0 on success
 */
int miiphy_write(const char *devname, unsigned char addr, unsigned char reg,
		  unsigned short value)
{
	struct mii_dev *dev;

	dev = miiphy_get_active_dev(devname);
	if (dev)
		return dev->write(devname, addr, reg, value);

	return 1;
}

/*****************************************************************************
 *
 * Print out list of registered MII capable devices.
 */
void miiphy_listdev (void)
{
	struct list_head *entry;
	struct mii_dev *dev;

	puts ("MII devices: ");
	list_for_each (entry, &mii_devs) {
		dev = list_entry (entry, struct mii_dev, link);
		printf ("'%s' ", dev->name);
	}
	puts ("\n");

	if (current_mii)
		printf ("Current device: '%s'\n", current_mii->name);
}

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
int miiphy_info(const char *devname, unsigned char addr, unsigned int *oui,
		 unsigned char *model, unsigned char *rev)
{
	unsigned int reg = 0;
	unsigned short tmp;

	if (miiphy_read (devname, addr, PHY_PHYIDR2, &tmp) != 0) {
		debug ("PHY ID register 2 read failed\n");
		return (-1);
	}
	reg = tmp;

	debug ("PHY_PHYIDR2 @ 0x%x = 0x%04x\n", addr, reg);

	if (reg == 0xFFFF) {
		/* No physical device present at this address */
		return (-1);
	}

	if (miiphy_read (devname, addr, PHY_PHYIDR1, &tmp) != 0) {
		debug ("PHY ID register 1 read failed\n");
		return (-1);
	}
	reg |= tmp << 16;
	debug ("PHY_PHYIDR[1,2] @ 0x%x = 0x%08x\n", addr, reg);

	*oui = (reg >> 10);
	*model = (unsigned char)((reg >> 4) & 0x0000003F);
	*rev = (unsigned char)(reg & 0x0000000F);
	return (0);
}

/*****************************************************************************
 *
 * Reset the PHY.
 * Returns:
 *   0 on success
 */
int miiphy_reset(const char *devname, unsigned char addr)
{
	unsigned short reg;
	int timeout = 500;

	if (miiphy_read (devname, addr, PHY_BMCR, &reg) != 0) {
		debug ("PHY status read failed\n");
		return (-1);
	}
	if (miiphy_write (devname, addr, PHY_BMCR, reg | PHY_BMCR_RESET) != 0) {
		debug ("PHY reset failed\n");
		return (-1);
	}
#ifdef CONFIG_PHY_RESET_DELAY
	udelay (CONFIG_PHY_RESET_DELAY);	/* Intel LXT971A needs this */
#endif
	/*
	 * Poll the control register for the reset bit to go to 0 (it is
	 * auto-clearing).  This should happen within 0.5 seconds per the
	 * IEEE spec.
	 */
	reg = 0x8000;
	while (((reg & 0x8000) != 0) && timeout--) {
		if (miiphy_read(devname, addr, PHY_BMCR, &reg) != 0) {
			debug("PHY status read failed\n");
			return -1;
		}
		udelay(1000);
	}
	if ((reg & 0x8000) == 0) {
		return (0);
	} else {
		puts ("PHY reset timed out\n");
		return (-1);
	}
	return (0);
}

/*****************************************************************************
 *
 * Determine the ethernet speed (10/100/1000).  Return 10 on error.
 */
int miiphy_speed(const char *devname, unsigned char addr)
{
	u16 bmcr, anlpar;

#if defined(CONFIG_PHY_GIGE)
	u16 btsr;

	/*
	 * Check for 1000BASE-X.  If it is supported, then assume that the speed
	 * is 1000.
	 */
	if (miiphy_is_1000base_x (devname, addr)) {
		return _1000BASET;
	}
	/*
	 * No 1000BASE-X, so assume 1000BASE-T/100BASE-TX/10BASE-T register set.
	 */
	/* Check for 1000BASE-T. */
	if (miiphy_read (devname, addr, PHY_1000BTSR, &btsr)) {
		printf ("PHY 1000BT status");
		goto miiphy_read_failed;
	}
	if (btsr != 0xFFFF &&
	    (btsr & (PHY_1000BTSR_1000FD | PHY_1000BTSR_1000HD))) {
		return _1000BASET;
	}
#endif /* CONFIG_PHY_GIGE */

	/* Check Basic Management Control Register first. */
	if (miiphy_read (devname, addr, PHY_BMCR, &bmcr)) {
		printf ("PHY speed");
		goto miiphy_read_failed;
	}
	/* Check if auto-negotiation is on. */
	if (bmcr & PHY_BMCR_AUTON) {
		/* Get auto-negotiation results. */
		if (miiphy_read (devname, addr, PHY_ANLPAR, &anlpar)) {
			printf ("PHY AN speed");
			goto miiphy_read_failed;
		}
		return (anlpar & PHY_ANLPAR_100) ? _100BASET : _10BASET;
	}
	/* Get speed from basic control settings. */
	return (bmcr & PHY_BMCR_100MB) ? _100BASET : _10BASET;

miiphy_read_failed:
	printf (" read failed, assuming 10BASE-T\n");
	return _10BASET;
}

/*****************************************************************************
 *
 * Determine full/half duplex.  Return half on error.
 */
int miiphy_duplex(const char *devname, unsigned char addr)
{
	u16 bmcr, anlpar;

#if defined(CONFIG_PHY_GIGE)
	u16 btsr;

	/* Check for 1000BASE-X. */
	if (miiphy_is_1000base_x (devname, addr)) {
		/* 1000BASE-X */
		if (miiphy_read (devname, addr, PHY_ANLPAR, &anlpar)) {
			printf ("1000BASE-X PHY AN duplex");
			goto miiphy_read_failed;
		}
	}
	/*
	 * No 1000BASE-X, so assume 1000BASE-T/100BASE-TX/10BASE-T register set.
	 */
	/* Check for 1000BASE-T. */
	if (miiphy_read (devname, addr, PHY_1000BTSR, &btsr)) {
		printf ("PHY 1000BT status");
		goto miiphy_read_failed;
	}
	if (btsr != 0xFFFF) {
		if (btsr & PHY_1000BTSR_1000FD) {
			return FULL;
		} else if (btsr & PHY_1000BTSR_1000HD) {
			return HALF;
		}
	}
#endif /* CONFIG_PHY_GIGE */

	/* Check Basic Management Control Register first. */
	if (miiphy_read (devname, addr, PHY_BMCR, &bmcr)) {
		puts ("PHY duplex");
		goto miiphy_read_failed;
	}
	/* Check if auto-negotiation is on. */
	if (bmcr & PHY_BMCR_AUTON) {
		/* Get auto-negotiation results. */
		if (miiphy_read (devname, addr, PHY_ANLPAR, &anlpar)) {
			puts ("PHY AN duplex");
			goto miiphy_read_failed;
		}
		return (anlpar & (PHY_ANLPAR_10FD | PHY_ANLPAR_TXFD)) ?
		    FULL : HALF;
	}
	/* Get speed from basic control settings. */
	return (bmcr & PHY_BMCR_DPLX) ? FULL : HALF;

miiphy_read_failed:
	printf (" read failed, assuming half duplex\n");
	return HALF;
}

/*****************************************************************************
 *
 * Return 1 if PHY supports 1000BASE-X, 0 if PHY supports 10BASE-T/100BASE-TX/
 * 1000BASE-T, or on error.
 */
int miiphy_is_1000base_x(const char *devname, unsigned char addr)
{
#if defined(CONFIG_PHY_GIGE)
	u16 exsr;

	if (miiphy_read (devname, addr, PHY_EXSR, &exsr)) {
		printf ("PHY extended status read failed, assuming no "
			"1000BASE-X\n");
		return 0;
	}
	return 0 != (exsr & (PHY_EXSR_1000XF | PHY_EXSR_1000XH));
#else
	return 0;
#endif
}

#ifdef CONFIG_SYS_FAULT_ECHO_LINK_DOWN
/*****************************************************************************
 *
 * Determine link status
 */
int miiphy_link(const char *devname, unsigned char addr)
{
	unsigned short reg;

	/* dummy read; needed to latch some phys */
	(void)miiphy_read (devname, addr, PHY_BMSR, &reg);
	if (miiphy_read (devname, addr, PHY_BMSR, &reg)) {
		puts ("PHY_BMSR read failed, assuming no link\n");
		return (0);
	}

	/* Determine if a link is active */
	if ((reg & PHY_BMSR_LS) != 0) {
		return (1);
	} else {
		return (0);
	}
}
#endif
