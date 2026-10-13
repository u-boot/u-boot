// SPDX-License-Identifier: GPL-2.0+
/*
 * Maxio MAE0621A PHY driver, ported from the vendor out-of-tree
 * Linux driver (MAXIO_PHY_VER "v1.8.1.1").
 *
 * The PHY comes up in a non-functional state: the clock-mode register
 * must be programmed for crystal operation and a set of analog tuning
 * registers applied before the RGMII interface starts toggling.  The
 * generic C22 driver leaves the RX clock dead, which makes the MAC's
 * EQOS_DMA_MODE_SWR reset never complete.
 */

#include <phy.h>
#include <linux/bitops.h>
#include <linux/delay.h>

#define PHY_ID_MAE0621A			0x7b744411
#define PHY_ID_MAE0621A_Q3C		0x7b744412
#define PHY_ID_MASK				0x7fffffff

#define MAXIO_PAGE_SELECT		0x1f

/* Vendor status register, page 0xa43 */
#define MAXIO_PHYSR				0x1a
#define MAXIO_PHY_LINK			BIT(2)
#define MAXIO_PHY_DUPLEX		BIT(3)
#define MAXIO_PHY_SPEED_MASK	GENMASK(5, 4)
#define MAXIO_PHY_1000M			BIT(5)
#define MAXIO_PHY_100M			BIT(4)

static int maxio_write_paged(struct phy_device *phydev, int page,
			     int regnum, u16 val)
{
	int ret;

	ret = phy_write(phydev, MDIO_DEVAD_NONE, MAXIO_PAGE_SELECT, page);
	if (ret < 0)
		return ret;

	ret = phy_write(phydev, MDIO_DEVAD_NONE, regnum, val);

	phy_write(phydev, MDIO_DEVAD_NONE, MAXIO_PAGE_SELECT, 0);

	return ret;
}

static int maxio_read_paged(struct phy_device *phydev, int page, int regnum)
{
	int ret;

	ret = phy_write(phydev, MDIO_DEVAD_NONE, MAXIO_PAGE_SELECT, page);
	if (ret < 0)
		return ret;

	ret = phy_read(phydev, MDIO_DEVAD_NONE, regnum);

	phy_write(phydev, MDIO_DEVAD_NONE, MAXIO_PAGE_SELECT, 0);

	return ret;
}

static int maxio_adcc_check(struct phy_device *phydev)
{
	int adcvalue, regval, i;

	maxio_write_paged(phydev, 0xd96, 0x2, 0x1fff);
	maxio_write_paged(phydev, 0xd96, 0x2, 0x1000);

	for (i = 0; i < 4; i++) {
		regval = 0xf908 + i * 0x100;
		maxio_write_paged(phydev, 0xd8f, 0xb, regval);
		adcvalue = maxio_read_paged(phydev, 0xd92, 0xb);
		if (!(adcvalue & 0x1ff))
			return -1;
	}

	return 0;
}

static void maxio_self_check(struct phy_device *phydev, int checknum)
{
	int ret, i;

	for (i = 0; i < checknum; i++) {
		ret = maxio_adcc_check(phydev);
		if (!ret)
			break;

		maxio_write_paged(phydev, 0x0, 0x0, 0x1940);
		phy_write(phydev, MDIO_DEVAD_NONE, MAXIO_PAGE_SELECT, 0x0);
		mdelay(10);
		maxio_write_paged(phydev, 0x0, 0x0, 0x1140);
		maxio_write_paged(phydev, 0x0, 0x0, 0x9140);
	}

	maxio_write_paged(phydev, 0xd96, 0x2, 0xfff);
	maxio_write_paged(phydev, 0x0, 0x0, 0x9140);
	phy_write(phydev, MDIO_DEVAD_NONE, MAXIO_PAGE_SELECT, 0x0);
}

static int mae0621a_config(struct phy_device *phydev)
{
	pr_info("Maxio MAE0621A init (crystal clock mode + analog tuning)\n");

	maxio_write_paged(phydev, 0xd92, 0x2, 0x200a);	/* clock mode: crystal */

	maxio_write_paged(phydev, 0xda0, 0x10, 0xc13);
	maxio_write_paged(phydev, 0x0, 0xd, 0x7);
	maxio_write_paged(phydev, 0x0, 0xe, 0x3c);
	maxio_write_paged(phydev, 0x0, 0xd, 0x4007);
	maxio_write_paged(phydev, 0x0, 0xe, 0x0);
	maxio_write_paged(phydev, 0xd96, 0x13, 0x7bc);
	maxio_write_paged(phydev, 0xd8f, 0x8, 0x2500);
	maxio_write_paged(phydev, 0xd90, 0x2, 0x1555);
	maxio_write_paged(phydev, 0xd90, 0x5, 0x2b15);
	maxio_write_paged(phydev, 0xd92, 0x14, 0xa);
	maxio_write_paged(phydev, 0xd91, 0x7, 0x5b00);
	maxio_write_paged(phydev, 0xd8f, 0x0, 0x300);
	maxio_write_paged(phydev, 0xd92, 0xa, 0x8506);
	maxio_write_paged(phydev, 0xd91, 0x6, 0x6870);
	maxio_write_paged(phydev, 0xd91, 0x1, 0x940);
	maxio_write_paged(phydev, 0xda0, 0x13, 0x1303);
	maxio_write_paged(phydev, 0xd97, 0xc, 0x177);
	maxio_write_paged(phydev, 0xd97, 0xb, 0x9a9);
	maxio_write_paged(phydev, 0xa42, 0x12, 0x28);
	maxio_write_paged(phydev, 0x0, 0x4, 0xde1);
	maxio_write_paged(phydev, 0x0, 0x0, 0x9140);

	maxio_self_check(phydev, 50);
	mdelay(100);

	return 0;
}

static int mae0621a_q3c_config(struct phy_device *phydev)
{
	maxio_write_paged(phydev, 0xa43, 0x19, 0x823);

	maxio_write_paged(phydev, 0xdab, 0x17, 0xc13);
	maxio_write_paged(phydev, 0xd96, 0x15, 0xc08a);
	maxio_write_paged(phydev, 0xda4, 0x12, 0x7bc);
	maxio_write_paged(phydev, 0xd8f, 0x16, 0x2500);
	maxio_write_paged(phydev, 0xd90, 0x16, 0x1555);
	maxio_write_paged(phydev, 0xd92, 0x11, 0x2b15);
	maxio_write_paged(phydev, 0xd96, 0x16, 0x4010);
	maxio_write_paged(phydev, 0xda5, 0x11, 0x4a12);
	maxio_write_paged(phydev, 0xda5, 0x12, 0x4a12);
	maxio_write_paged(phydev, 0xd99, 0x16, 0xa);
	maxio_write_paged(phydev, 0xd95, 0x13, 0x5b00);
	maxio_write_paged(phydev, 0xd8f, 0x10, 0x300);
	maxio_write_paged(phydev, 0xd98, 0x17, 0x8506);
	maxio_write_paged(phydev, 0xd95, 0x12, 0x6870);
	maxio_write_paged(phydev, 0xd93, 0x15, 0x940);
	maxio_write_paged(phydev, 0xdad, 0x12, 0x1303);
	maxio_write_paged(phydev, 0xda8, 0x11, 0x177);
	maxio_write_paged(phydev, 0xda8, 0x10, 0x9a9);
	maxio_write_paged(phydev, 0xa42, 0x12, 0x28);
	maxio_write_paged(phydev, 0x0, 0x4, 0xde1);
	maxio_write_paged(phydev, 0x0, 0x0, 0x9140);

	phy_write(phydev, MDIO_DEVAD_NONE, MAXIO_PAGE_SELECT, 0);

	return 0;
}

static int mae0621a_startup(struct phy_device *phydev)
{
	int bmsr, physr;

	/*
	 * Read link straight from BMSR (twice: the link bit is latched low
	 * per IEEE 802.3).  Do not wait on the autoneg-complete bit: this
	 * PHY reports link before aneg-complete settles, which the vendor
	 * kernel driver also works around.
	 */
	bmsr = phy_read(phydev, MDIO_DEVAD_NONE, MII_BMSR);
	if (bmsr < 0)
		return bmsr;
	bmsr = phy_read(phydev, MDIO_DEVAD_NONE, MII_BMSR);
	if (bmsr < 0)
		return bmsr;

	phydev->link = !!(bmsr & BMSR_LSTATUS);
	if (!phydev->link)
		return 0;

	physr = maxio_read_paged(phydev, 0xa43, MAXIO_PHYSR);
	if (physr < 0)
		return physr;

	if ((physr & MAXIO_PHY_SPEED_MASK) == MAXIO_PHY_1000M)
		phydev->speed = SPEED_1000;
	else if ((physr & MAXIO_PHY_SPEED_MASK) == MAXIO_PHY_100M)
		phydev->speed = SPEED_100;
	else
		phydev->speed = SPEED_10;

	phydev->duplex = (physr & MAXIO_PHY_DUPLEX) ? DUPLEX_FULL : DUPLEX_HALF;

	return 0;
}

/*
 * This PHY has been observed reporting 0xffff for PHY ID1 on this board
 * (raw, pre-init state) while the vendor kernel reads 0x7b74, so match on
 * the ID2 low bits only; Q2C ends in 0x4411 and Q3C in 0x4412.
 */
#define PHY_ID_MATCH_LOW16		0x0000ffff

U_BOOT_PHY_DRIVER(mae0621a) = {
	.name		= "Maxio MAE0621A-Q2C Gigabit Ethernet",
	.uid		= PHY_ID_MAE0621A,
	.mask		= PHY_ID_MATCH_LOW16,
	.features	= PHY_GBIT_FEATURES,
	.config		= &mae0621a_config,
	.startup	= &mae0621a_startup,
	.shutdown	= &genphy_shutdown,
};

U_BOOT_PHY_DRIVER(mae0621a_q3c) = {
	.name		= "Maxio MAE0621A/B-Q3C(I) Gigabit Ethernet",
	.uid		= PHY_ID_MAE0621A_Q3C,
	.mask		= PHY_ID_MATCH_LOW16,
	.features	= PHY_GBIT_FEATURES,
	.config		= &mae0621a_q3c_config,
	.startup	= &mae0621a_startup,
	.shutdown	= &genphy_shutdown,
};
