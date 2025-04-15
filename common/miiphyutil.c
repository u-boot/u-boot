// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2001
 * Gerald Van Baren, Custom IDEAS, vanbaren@cideas.com.
 */

/*
 * This provides a bit-banged interface to the ethernet MII management
 * channel.
 */

#include <dm.h>
#include <log.h>
#include <miiphy.h>
#include <phy.h>
#include <linux/delay.h>

#include <asm/types.h>
#include <linux/list.h>
#include <malloc.h>
#include <net.h>

/* local debug macro */
#undef MII_DEBUG

#undef debug
#ifdef MII_DEBUG
#define debug(fmt, args...)	printf(fmt, ##args)
#else
#define debug(fmt, args...)
#endif /* MII_DEBUG */

static LIST_HEAD(mii_devs);
static struct mii_dev *current_mii;

/*
 * Lookup the mii_dev struct by the registered device name.
 */
struct mii_dev *miiphy_get_dev_by_name(const char *devname)
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

	return NULL;
}

struct mii_dev *mdio_alloc(void)
{
	struct mii_dev *bus;

	bus = malloc(sizeof(*bus));
	if (!bus)
		return bus;

	memset(bus, 0, sizeof(*bus));

	/* initialize mii_dev struct fields */
	INIT_LIST_HEAD(&bus->link);

	return bus;
}

void mdio_free(struct mii_dev *bus)
{
	free(bus);
}

int mdio_register(struct mii_dev *bus)
{
	if (!bus || !bus->read || !bus->write)
		return -1;

	/* check if we have unique name */
	if (miiphy_get_dev_by_name(bus->name)) {
		printf("mdio_register: non unique device name '%s'\n",
			bus->name);
		return -1;
	}

	/* add it to the list */
	list_add_tail(&bus->link, &mii_devs);

	if (!current_mii)
		current_mii = bus;

	return 0;
}

int mdio_register_seq(struct mii_dev *bus, int seq)
{
	int ret;

	/* Setup a unique name for each mdio bus */
	ret = snprintf(bus->name, MDIO_NAME_LEN, "eth%d", seq);
	if (ret < 0)
		return ret;

	return mdio_register(bus);
}

int mdio_unregister(struct mii_dev *bus)
{
	if (!bus)
		return 0;

	/* delete it from the list */
	list_del(&bus->link);

	if (current_mii == bus)
		current_mii = NULL;

	return 0;
}

void mdio_list_devices(void)
{
	struct list_head *entry;

	list_for_each(entry, &mii_devs) {
		int i;
		struct mii_dev *bus = list_entry(entry, struct mii_dev, link);

		printf("%s:\n", bus->name);

		for (i = 0; i < PHY_MAX_ADDR; i++) {
			struct phy_device *phydev = bus->phymap[i];

			if (phydev) {
				printf("%x - %s", i, phydev->drv->name);

				if (phydev->dev)
					printf(" <--> %s\n", phydev->dev->name);
				else
					printf("\n");
			}
		}
	}
}

int miiphy_set_current_dev(const char *devname)
{
	struct mii_dev *dev;

	dev = miiphy_get_dev_by_name(devname);
	if (dev) {
		current_mii = dev;
		return 0;
	}

	printf("No such device: %s\n", devname);

	return 1;
}

struct mii_dev *mdio_get_current_dev(void)
{
	return current_mii;
}

struct list_head *mdio_get_list_head(void)
{
	return &mii_devs;
}

struct phy_device *mdio_phydev_for_ethname(const char *ethname)
{
	struct list_head *entry;
	struct mii_dev *bus;

	list_for_each(entry, &mii_devs) {
		int i;
		bus = list_entry(entry, struct mii_dev, link);

		for (i = 0; i < PHY_MAX_ADDR; i++) {
			if (!bus->phymap[i] || !bus->phymap[i]->dev)
				continue;

			if (strcmp(bus->phymap[i]->dev->name, ethname) == 0)
				return bus->phymap[i];
		}
	}

	printf("%s is not a known ethernet\n", ethname);
	return NULL;
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
 * This API is deprecated. Use phy_read on a phy_device found via phy_connect
 *
 * Returns:
 *   0 on success
 */
int miiphy_read(const char *devname, unsigned char addr, unsigned char reg,
		 unsigned short *value)
{
	struct mii_dev *bus;
	int ret;

	bus = miiphy_get_active_dev(devname);
	if (!bus)
		return 1;

	ret = bus->read(bus, addr, MDIO_DEVAD_NONE, reg);
	if (ret < 0)
		return 1;

	*value = (unsigned short)ret;
	return 0;
}

/*****************************************************************************
 *
 * Write <value> to the PHY attached to device <devname>,
 * use PHY address <addr> and register <reg>.
 *
 * This API is deprecated. Use phy_write on a phy_device found by phy_connect
 *
 * Returns:
 *   0 on success
 */
int miiphy_write(const char *devname, unsigned char addr, unsigned char reg,
		  unsigned short value)
{
	struct mii_dev *bus;

	bus = miiphy_get_active_dev(devname);
	if (bus)
		return bus->write(bus, addr, MDIO_DEVAD_NONE, reg, value);

	return 1;
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
 * This API is deprecated.
 *
 * Returns:
 *   0 on success
 */
int miiphy_info(const char *devname, unsigned char addr, unsigned int *oui,
		 unsigned char *model, unsigned char *rev)
{
	unsigned int reg = 0;
	unsigned short tmp;

	if (miiphy_read(devname, addr, MII_PHYSID2, &tmp) != 0) {
		debug("PHY ID register 2 read failed\n");
		return -1;
	}
	reg = tmp;

	debug("MII_PHYSID2 @ 0x%x = 0x%04x\n", addr, reg);

	if (reg == 0xFFFF) {
		/* No physical device present at this address */
		return -1;
	}

	if (miiphy_read(devname, addr, MII_PHYSID1, &tmp) != 0) {
		debug("PHY ID register 1 read failed\n");
		return -1;
	}
	reg |= tmp << 16;
	debug("PHY_PHYIDR[1,2] @ 0x%x = 0x%08x\n", addr, reg);

	*oui = (reg >> 10);
	*model = (unsigned char)((reg >> 4) & 0x0000003F);
	*rev = (unsigned char)(reg & 0x0000000F);
	return 0;
}

#ifndef CONFIG_PHYLIB
/*****************************************************************************
 *
 * Reset the PHY.
 *
 * This API is deprecated. Use PHYLIB.
 *
 * Returns:
 *   0 on success
 */
int miiphy_reset(const char *devname, unsigned char addr)
{
	unsigned short reg;
	int timeout = 500;

	if (miiphy_read(devname, addr, MII_BMCR, &reg) != 0) {
		debug("PHY status read failed\n");
		return -1;
	}
	if (miiphy_write(devname, addr, MII_BMCR, reg | BMCR_RESET) != 0) {
		debug("PHY reset failed\n");
		return -1;
	}
#if CONFIG_PHY_RESET_DELAY > 0
	udelay(CONFIG_PHY_RESET_DELAY);	/* Intel LXT971A needs this */
#endif
	/*
	 * Poll the control register for the reset bit to go to 0 (it is
	 * auto-clearing).  This should happen within 0.5 seconds per the
	 * IEEE spec.
	 */
	reg = 0x8000;
	while (((reg & 0x8000) != 0) && timeout--) {
		if (miiphy_read(devname, addr, MII_BMCR, &reg) != 0) {
			debug("PHY status read failed\n");
			return -1;
		}
		udelay(1000);
	}
	if ((reg & 0x8000) == 0) {
		return 0;
	} else {
		puts("PHY reset timed out\n");
		return -1;
	}
	return 0;
}
#endif /* !PHYLIB */

/*****************************************************************************
 *
 * Determine the ethernet speed (10/100/1000).  Return 10 on error.
 */
int miiphy_speed(const char *devname, unsigned char addr)
{
	u16 bmcr, anlpar, adv;

#if defined(CONFIG_PHY_GIGE)
	u16 btsr;

	/*
	 * Check for 1000BASE-X.  If it is supported, then assume that the speed
	 * is 1000.
	 */
	if (miiphy_is_1000base_x(devname, addr))
		return _1000BASET;

	/*
	 * No 1000BASE-X, so assume 1000BASE-T/100BASE-TX/10BASE-T register set.
	 */
	/* Check for 1000BASE-T. */
	if (miiphy_read(devname, addr, MII_STAT1000, &btsr)) {
		printf("PHY 1000BT status");
		goto miiphy_read_failed;
	}
	if (btsr != 0xFFFF &&
			(btsr & (PHY_1000BTSR_1000FD | PHY_1000BTSR_1000HD)))
		return _1000BASET;
#endif /* CONFIG_PHY_GIGE */

	/* Check Basic Management Control Register first. */
	if (miiphy_read(devname, addr, MII_BMCR, &bmcr)) {
		printf("PHY speed");
		goto miiphy_read_failed;
	}
	/* Check if auto-negotiation is on. */
	if (bmcr & BMCR_ANENABLE) {
		/* Get auto-negotiation results. */
		if (miiphy_read(devname, addr, MII_LPA, &anlpar)) {
			printf("PHY AN speed");
			goto miiphy_read_failed;
		}

		if (miiphy_read(devname, addr, MII_ADVERTISE, &adv)) {
			puts("PHY AN adv speed");
			goto miiphy_read_failed;
		}
		return ((anlpar & adv) & LPA_100) ? _100BASET : _10BASET;
	}
	/* Get speed from basic control settings. */
	return (bmcr & BMCR_SPEED100) ? _100BASET : _10BASET;

miiphy_read_failed:
	printf(" read failed, assuming 10BASE-T\n");
	return _10BASET;
}

/*****************************************************************************
 *
 * Determine full/half duplex.  Return half on error.
 */
int miiphy_duplex(const char *devname, unsigned char addr)
{
	u16 bmcr, anlpar, adv;

#if defined(CONFIG_PHY_GIGE)
	u16 btsr;

	/* Check for 1000BASE-X. */
	if (miiphy_is_1000base_x(devname, addr)) {
		/* 1000BASE-X */
		if (miiphy_read(devname, addr, MII_LPA, &anlpar)) {
			printf("1000BASE-X PHY AN duplex");
			goto miiphy_read_failed;
		}
	}
	/*
	 * No 1000BASE-X, so assume 1000BASE-T/100BASE-TX/10BASE-T register set.
	 */
	/* Check for 1000BASE-T. */
	if (miiphy_read(devname, addr, MII_STAT1000, &btsr)) {
		printf("PHY 1000BT status");
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
	if (miiphy_read(devname, addr, MII_BMCR, &bmcr)) {
		puts("PHY duplex");
		goto miiphy_read_failed;
	}
	/* Check if auto-negotiation is on. */
	if (bmcr & BMCR_ANENABLE) {
		/* Get auto-negotiation results. */
		if (miiphy_read(devname, addr, MII_LPA, &anlpar)) {
			puts("PHY AN duplex");
			goto miiphy_read_failed;
		}

		if (miiphy_read(devname, addr, MII_ADVERTISE, &adv)) {
			puts("PHY AN adv duplex");
			goto miiphy_read_failed;
		}
		return ((anlpar & adv) & (LPA_10FULL | LPA_100FULL)) ?
		    FULL : HALF;
	}
	/* Get speed from basic control settings. */
	return (bmcr & BMCR_FULLDPLX) ? FULL : HALF;

miiphy_read_failed:
	printf(" read failed, assuming half duplex\n");
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

	if (miiphy_read(devname, addr, MII_ESTATUS, &exsr)) {
		printf("PHY extended status read failed, assuming no "
			"1000BASE-X\n");
		return 0;
	}
	return 0 != (exsr & (ESTATUS_1000XF | ESTATUS_1000XH));
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
	(void)miiphy_read(devname, addr, MII_BMSR, &reg);
	if (miiphy_read(devname, addr, MII_BMSR, &reg)) {
		puts("MII_BMSR read failed, assuming no link\n");
		return 0;
	}

	/* Determine if a link is active */
	if ((reg & BMSR_LSTATUS) != 0) {
		return 1;
	} else {
		return 0;
	}
}
#endif
