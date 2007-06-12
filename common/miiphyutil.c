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

#if defined(CONFIG_MII) || (CONFIG_COMMANDS & CFG_CMD_MII) || defined(CONFIG_CMD_MII)
#include <asm/types.h>
#include <linux/list.h>
#include <malloc.h>
#include <net.h>

/* local debug macro */
#define MII_DEBUG
#undef MII_DEBUG

#undef debug
#ifdef MII_DEBUG
#define debug(fmt,args...)	printf (fmt ,##args)
#else
#define debug(fmt,args...)
#endif /* MII_DEBUG */

struct mii_dev {
	struct list_head link;
	char *name;
	int (* read)(char *devname, unsigned char addr,
			unsigned char reg, unsigned short *value);
	int (* write)(char *devname, unsigned char addr,
			unsigned char reg, unsigned short value);
};

static struct list_head mii_devs;
static struct mii_dev *current_mii;

/*****************************************************************************
 *
 * Initialize global data. Need to be called before any other miiphy routine.
 */
void miiphy_init()
{
		INIT_LIST_HEAD(&mii_devs);
		current_mii = NULL;
}

/*****************************************************************************
 *
 * Register read and write MII access routines for the device <name>.
 */
void miiphy_register(char *name,
		int (* read)(char *devname, unsigned char addr,
			unsigned char reg, unsigned short *value),
		int (* write)(char *devname, unsigned char addr,
			unsigned char reg, unsigned short value))
{
	struct list_head *entry;
	struct mii_dev *new_dev;
	struct mii_dev *miidev;
	unsigned int name_len;

	/* check if we have unique name */
	list_for_each(entry, &mii_devs) {
		miidev = list_entry(entry, struct mii_dev, link);
		if (strcmp(miidev->name, name) == 0) {
			printf("miiphy_register: non unique device name '%s'\n",
					name);
			return;
		}
	}

	/* allocate memory */
	name_len = strlen(name);
	new_dev = (struct mii_dev *)malloc(sizeof(struct mii_dev) + name_len + 1);

	if(new_dev == NULL) {
		printf("miiphy_register: cannot allocate memory for '%s'\n",
				name);
		return;
	}
	memset(new_dev, 0, sizeof(struct mii_dev) + name_len);

	/* initalize mii_dev struct fields */
	INIT_LIST_HEAD(&new_dev->link);
	new_dev->read = read;
	new_dev->write = write;
	new_dev->name = (char *)(new_dev + 1);
	strncpy(new_dev->name, name, name_len);
	new_dev->name[name_len] = '\0';

	debug("miiphy_register: added '%s', read=0x%08lx, write=0x%08lx\n",
			new_dev->name, new_dev->read, new_dev->write);

	/* add it to the list */
	list_add_tail(&new_dev->link, &mii_devs);

	if (!current_mii)
		current_mii = new_dev;
}

int miiphy_set_current_dev(char *devname)
{
	struct list_head *entry;
	struct mii_dev *dev;

	list_for_each(entry, &mii_devs) {
		dev = list_entry(entry, struct mii_dev, link);

		if (strcmp(devname, dev->name) == 0) {
			current_mii = dev;
			return 0;
		}
	}

	printf("No such device: %s\n", devname);
	return 1;
}

char *miiphy_get_current_dev()
{
	if (current_mii)
		return current_mii->name;

	return NULL;
}

/*****************************************************************************
 *
 * Read to variable <value> from the PHY attached to device <devname>,
 * use PHY address <addr> and register <reg>.
 *
 * Returns:
 *   0 on success
 */
int miiphy_read(char *devname, unsigned char addr, unsigned char reg,
		unsigned short *value)
{
	struct list_head *entry;
	struct mii_dev *dev;
	int found_dev = 0;
	int read_ret = 0;

	if (!devname) {
		printf("NULL device name!\n");
		return 1;
	}

	list_for_each(entry, &mii_devs) {
		dev = list_entry(entry, struct mii_dev, link);

		if (strcmp(devname, dev->name) == 0) {
			found_dev = 1;
			read_ret = dev->read(devname, addr, reg, value);
			break;
		}
	}

	if (found_dev == 0)
		printf("No such device: %s\n", devname);

	return ((found_dev) ? read_ret : 1);
}

/*****************************************************************************
 *
 * Write <value> to the PHY attached to device <devname>,
 * use PHY address <addr> and register <reg>.
 *
 * Returns:
 *   0 on success
 */
int miiphy_write(char *devname, unsigned char addr, unsigned char reg,
		unsigned short value)
{
	struct list_head *entry;
	struct mii_dev *dev;
	int found_dev = 0;
	int write_ret = 0;

	if (!devname) {
		printf("NULL device name!\n");
		return 1;
	}

	list_for_each(entry, &mii_devs) {
		dev = list_entry(entry, struct mii_dev, link);

		if (strcmp(devname, dev->name) == 0) {
			found_dev = 1;
			write_ret = dev->write(devname, addr, reg, value);
			break;
		}
	}

	if (found_dev == 0)
		printf("No such device: %s\n", devname);

	return ((found_dev) ? write_ret : 1);
}

/*****************************************************************************
 *
 * Print out list of registered MII capable devices.
 */
void miiphy_listdev(void)
{
	struct list_head *entry;
	struct mii_dev *dev;

	puts("MII devices: ");
	list_for_each(entry, &mii_devs) {
		dev = list_entry(entry, struct mii_dev, link);
		printf("'%s' ", dev->name);
	}
	puts("\n");

	if (current_mii)
		printf("Current device: '%s'\n", current_mii->name);
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
int miiphy_info (char *devname,
		 unsigned char addr,
		 unsigned int *oui,
		 unsigned char *model, unsigned char *rev)
{
	unsigned int reg = 0;
	unsigned short tmp;

	if (miiphy_read (devname, addr, PHY_PHYIDR2, &tmp) != 0) {
#ifdef DEBUG
		puts ("PHY ID register 2 read failed\n");
#endif
		return (-1);
	}
	reg = tmp;

#ifdef DEBUG
	printf ("PHY_PHYIDR2 @ 0x%x = 0x%04x\n", addr, reg);
#endif
	if (reg == 0xFFFF) {
		/* No physical device present at this address */
		return (-1);
	}

	if (miiphy_read (devname, addr, PHY_PHYIDR1, &tmp) != 0) {
#ifdef DEBUG
		puts ("PHY ID register 1 read failed\n");
#endif
		return (-1);
	}
	reg |= tmp << 16;
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
int miiphy_reset (char *devname, unsigned char addr)
{
	unsigned short reg;
	int loop_cnt;

	if (miiphy_read (devname, addr, PHY_BMCR, &reg) != 0) {
#ifdef DEBUG
		printf ("PHY status read failed\n");
#endif
		return (-1);
	}
	if (miiphy_write (devname, addr, PHY_BMCR, reg | 0x8000) != 0) {
#ifdef DEBUG
		puts ("PHY reset failed\n");
#endif
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
	loop_cnt = 0;
	reg = 0x8000;
	while (((reg & 0x8000) != 0) && (loop_cnt++ < 1000000)) {
		if (miiphy_read (devname, addr, PHY_BMCR, &reg) != 0) {
#     ifdef DEBUG
			puts ("PHY status read failed\n");
#     endif
			return (-1);
		}
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
 * Determine the ethernet speed (10/100).
 */
int miiphy_speed (char *devname, unsigned char addr)
{
	unsigned short reg;

#if defined(CONFIG_PHY_GIGE)
	if (miiphy_read (devname, addr, PHY_1000BTSR, &reg)) {
		printf ("PHY 1000BT Status read failed\n");
	} else {
		if (reg != 0xFFFF) {
			if ((reg & (PHY_1000BTSR_1000FD | PHY_1000BTSR_1000HD)) !=0) {
				return (_1000BASET);
			}
		}
	}
#endif /* CONFIG_PHY_GIGE */

	/* Check Basic Management Control Register first. */
	if (miiphy_read (devname, addr, PHY_BMCR, &reg)) {
		puts ("PHY speed read failed, assuming 10bT\n");
		return (_10BASET);
	}
	/* Check if auto-negotiation is on. */
	if ((reg & PHY_BMCR_AUTON) != 0) {
		/* Get auto-negotiation results. */
		if (miiphy_read (devname, addr, PHY_ANLPAR, &reg)) {
			puts ("PHY AN speed read failed, assuming 10bT\n");
			return (_10BASET);
		}
		if ((reg & PHY_ANLPAR_100) != 0) {
			return (_100BASET);
		} else {
			return (_10BASET);
		}
	}
	/* Get speed from basic control settings. */
	else if (reg & PHY_BMCR_100MB) {
		return (_100BASET);
	} else {
		return (_10BASET);
	}

}


/*****************************************************************************
 *
 * Determine full/half duplex.
 */
int miiphy_duplex (char *devname, unsigned char addr)
{
	unsigned short reg;

#if defined(CONFIG_PHY_GIGE)
	if (miiphy_read (devname, addr, PHY_1000BTSR, &reg)) {
		printf ("PHY 1000BT Status read failed\n");
	} else {
		if ( (reg != 0xFFFF) &&
		     (reg & (PHY_1000BTSR_1000FD | PHY_1000BTSR_1000HD)) ) {
			if ((reg & PHY_1000BTSR_1000FD) !=0) {
				return (FULL);
			} else {
				return (HALF);
			}
		}
	}
#endif /* CONFIG_PHY_GIGE */

	/* Check Basic Management Control Register first. */
	if (miiphy_read (devname, addr, PHY_BMCR, &reg)) {
		puts ("PHY duplex read failed, assuming half duplex\n");
		return (HALF);
	}
	/* Check if auto-negotiation is on. */
	if ((reg & PHY_BMCR_AUTON) != 0) {
		/* Get auto-negotiation results. */
		if (miiphy_read (devname, addr, PHY_ANLPAR, &reg)) {
			puts ("PHY AN duplex read failed, assuming half duplex\n");
			return (HALF);
		}

		if ((reg & (PHY_ANLPAR_10FD | PHY_ANLPAR_TXFD)) != 0) {
			return (FULL);
		} else {
			return (HALF);
		}
	}
	/* Get speed from basic control settings. */
	else if (reg & PHY_BMCR_DPLX) {
		return (FULL);
	} else {
		return (HALF);
	}

}

#ifdef CFG_FAULT_ECHO_LINK_DOWN
/*****************************************************************************
 *
 * Determine link status
 */
int miiphy_link (char *devname, unsigned char addr)
{
	unsigned short reg;

	/* dummy read; needed to latch some phys */
	(void)miiphy_read(devname, addr, PHY_BMSR, &reg);
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

#endif /* CONFIG_MII || (CONFIG_COMMANDS & CFG_CMD_MII) */
