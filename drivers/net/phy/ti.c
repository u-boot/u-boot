/*
 * TI PHY drivers
 *
 * SPDX-License-Identifier:	GPL-2.0
 *
 */
#include <common.h>
#include <phy.h>

/* TI DP83867 */
#define DP83867_DEVADDR		0x1f

#define MII_DP83867_PHYCTRL	0x10
#define MII_DP83867_MICR	0x12
#define DP83867_CTRL		0x1f

/* Extended Registers */
#define DP83867_RGMIICTL	0x0032
#define DP83867_RGMIIDCTL	0x0086

#define DP83867_SW_RESET	BIT(15)
#define DP83867_SW_RESTART	BIT(14)

/* MICR Interrupt bits */
#define MII_DP83867_MICR_AN_ERR_INT_EN		BIT(15)
#define MII_DP83867_MICR_SPEED_CHNG_INT_EN	BIT(14)
#define MII_DP83867_MICR_DUP_MODE_CHNG_INT_EN	BIT(13)
#define MII_DP83867_MICR_PAGE_RXD_INT_EN	BIT(12)
#define MII_DP83867_MICR_AUTONEG_COMP_INT_EN	BIT(11)
#define MII_DP83867_MICR_LINK_STS_CHNG_INT_EN	BIT(10)
#define MII_DP83867_MICR_FALSE_CARRIER_INT_EN	BIT(8)
#define MII_DP83867_MICR_SLEEP_MODE_CHNG_INT_EN	BIT(4)
#define MII_DP83867_MICR_WOL_INT_EN		BIT(3)
#define MII_DP83867_MICR_XGMII_ERR_INT_EN	BIT(2)
#define MII_DP83867_MICR_POL_CHNG_INT_EN	BIT(1)
#define MII_DP83867_MICR_JABBER_INT_EN		BIT(0)

/* RGMIICTL bits */
#define DP83867_RGMII_TX_CLK_DELAY_EN		BIT(1)
#define DP83867_RGMII_RX_CLK_DELAY_EN		BIT(0)

/* PHY CTRL bits */
#define DP83867_PHYCR_FIFO_DEPTH_SHIFT		14
#define DP83867_MDI_CROSSOVER		5
#define DP83867_MDI_CROSSOVER_AUTO	2

/* RGMIIDCTL bits */
#define DP83867_RGMII_TX_CLK_DELAY_SHIFT	4

#define MII_MMD_CTRL	0x0d /* MMD Access Control Register */
#define MII_MMD_DATA	0x0e /* MMD Access Data Register */

/* MMD Access Control register fields */
#define MII_MMD_CTRL_DEVAD_MASK	0x1f /* Mask MMD DEVAD*/
#define MII_MMD_CTRL_ADDR	0x0000 /* Address */
#define MII_MMD_CTRL_NOINCR	0x4000 /* no post increment */
#define MII_MMD_CTRL_INCR_RDWT	0x8000 /* post increment on reads & writes */
#define MII_MMD_CTRL_INCR_ON_WT	0xC000 /* post increment on writes only */

/**
 * phy_read_mmd_indirect - reads data from the MMD registers
 * @phydev: The PHY device bus
 * @prtad: MMD Address
 * @devad: MMD DEVAD
 * @addr: PHY address on the MII bus
 *
 * Description: it reads data from the MMD registers (clause 22 to access to
 * clause 45) of the specified phy address.
 * To read these registers we have:
 * 1) Write reg 13 // DEVAD
 * 2) Write reg 14 // MMD Address
 * 3) Write reg 13 // MMD Data Command for MMD DEVAD
 * 3) Read  reg 14 // Read MMD data
 */
int phy_read_mmd_indirect(struct phy_device *phydev, int prtad,
			  int devad, int addr)
{
	int value = -1;

	/* Write the desired MMD Devad */
	phy_write(phydev, addr, MII_MMD_CTRL, devad);

	/* Write the desired MMD register address */
	phy_write(phydev, addr, MII_MMD_DATA, prtad);

	/* Select the Function : DATA with no post increment */
	phy_write(phydev, addr, MII_MMD_CTRL, (devad | MII_MMD_CTRL_NOINCR));

	/* Read the content of the MMD's selected register */
	value = phy_read(phydev, addr, MII_MMD_DATA);
	return value;
}

/**
 * phy_write_mmd_indirect - writes data to the MMD registers
 * @phydev: The PHY device
 * @prtad: MMD Address
 * @devad: MMD DEVAD
 * @addr: PHY address on the MII bus
 * @data: data to write in the MMD register
 *
 * Description: Write data from the MMD registers of the specified
 * phy address.
 * To write these registers we have:
 * 1) Write reg 13 // DEVAD
 * 2) Write reg 14 // MMD Address
 * 3) Write reg 13 // MMD Data Command for MMD DEVAD
 * 3) Write reg 14 // Write MMD data
 */
void phy_write_mmd_indirect(struct phy_device *phydev, int prtad,
			    int devad, int addr, u32 data)
{
	/* Write the desired MMD Devad */
	phy_write(phydev, addr, MII_MMD_CTRL, devad);

	/* Write the desired MMD register address */
	phy_write(phydev, addr, MII_MMD_DATA, prtad);

	/* Select the Function : DATA with no post increment */
	phy_write(phydev, addr, MII_MMD_CTRL, (devad | MII_MMD_CTRL_NOINCR));

	/* Write the data into MMD's selected register */
	phy_write(phydev, addr, MII_MMD_DATA, data);
}

/**
 * phy_interface_is_rgmii - Convenience function for testing if a PHY interface
 * is RGMII (all variants)
 * @phydev: the phy_device struct
 */
static inline bool phy_interface_is_rgmii(struct phy_device *phydev)
{
	return phydev->interface >= PHY_INTERFACE_MODE_RGMII &&
		phydev->interface <= PHY_INTERFACE_MODE_RGMII_TXID;
}

/* User setting - can be taken from DTS */
#define RX_ID_DELAY	8
#define TX_ID_DELAY	0xa
#define FIFO_DEPTH	1

static int dp83867_config(struct phy_device *phydev)
{
	unsigned int val, delay;
	int ret;

	/* Restart the PHY.  */
	val = phy_read(phydev, MDIO_DEVAD_NONE, DP83867_CTRL);
	phy_write(phydev, MDIO_DEVAD_NONE, DP83867_CTRL,
		  val | DP83867_SW_RESTART);

	if (phy_interface_is_rgmii(phydev)) {
		ret = phy_write(phydev, MDIO_DEVAD_NONE, MII_DP83867_PHYCTRL,
			(DP83867_MDI_CROSSOVER_AUTO << DP83867_MDI_CROSSOVER) |
			(FIFO_DEPTH << DP83867_PHYCR_FIFO_DEPTH_SHIFT));
		if (ret)
			return ret;
	}

	if ((phydev->interface >= PHY_INTERFACE_MODE_RGMII_ID) &&
	    (phydev->interface <= PHY_INTERFACE_MODE_RGMII_RXID)) {
		val = phy_read_mmd_indirect(phydev, DP83867_RGMIICTL,
					    DP83867_DEVADDR, phydev->addr);

		if (phydev->interface == PHY_INTERFACE_MODE_RGMII_ID)
			val |= (DP83867_RGMII_TX_CLK_DELAY_EN |
				DP83867_RGMII_RX_CLK_DELAY_EN);

		if (phydev->interface == PHY_INTERFACE_MODE_RGMII_TXID)
			val |= DP83867_RGMII_TX_CLK_DELAY_EN;

		if (phydev->interface == PHY_INTERFACE_MODE_RGMII_RXID)
			val |= DP83867_RGMII_RX_CLK_DELAY_EN;

		phy_write_mmd_indirect(phydev, DP83867_RGMIICTL,
				       DP83867_DEVADDR, phydev->addr, val);

		delay = (RX_ID_DELAY |
			 (TX_ID_DELAY << DP83867_RGMII_TX_CLK_DELAY_SHIFT));

		phy_write_mmd_indirect(phydev, DP83867_RGMIIDCTL,
				       DP83867_DEVADDR, phydev->addr, delay);
	}

	genphy_config_aneg(phydev);
	return 0;
}

static struct phy_driver DP83867_driver = {
	.name = "TI DP83867",
	.uid = 0x2000a231,
	.mask = 0xfffffff0,
	.features = PHY_GBIT_FEATURES,
	.config = &dp83867_config,
	.startup = &genphy_startup,
	.shutdown = &genphy_shutdown,
};

int phy_ti_init(void)
{
	phy_register(&DP83867_driver);
	return 0;
}
