/*
 * ICPlus PHY drivers
 *
 * SPDX-License-Identifier:	GPL-2.0+
 *
 * Copyright (c) 2007 Freescale Semiconductor, Inc.
 */
#include <phy.h>

/* IP101A/G - IP1001 */
#define IP10XX_SPEC_CTRL_STATUS         16      /* Spec. Control Register */
#define IP1001_SPEC_CTRL_STATUS_2       20      /* IP1001 Spec. Control Reg 2 */
#define IP1001_PHASE_SEL_MASK           3       /* IP1001 RX/TXPHASE_SEL */
#define IP1001_APS_ON                   11      /* IP1001 APS Mode  bit */
#define IP101A_G_APS_ON                 2       /* IP101A/G APS Mode bit */
#define IP101A_G_IRQ_CONF_STATUS        0x11    /* Conf Info IRQ & Status Reg */
#define IP101A_G_IRQ_PIN_USED           (1<<15) /* INTR pin used */
#define IP101A_G_IRQ_DEFAULT            IP101A_G_IRQ_PIN_USED

static int ip1001_config(struct phy_device *phydev)
{
	int c;

	/* Enable Auto Power Saving mode */
	c = phy_read(phydev, MDIO_DEVAD_NONE, IP1001_SPEC_CTRL_STATUS_2);
	if (c < 0)
		return c;
	c |= IP1001_APS_ON;
	c = phy_write(phydev, MDIO_DEVAD_NONE, IP1001_SPEC_CTRL_STATUS_2, c);
	if (c < 0)
		return c;

	/* INTR pin used: speed/link/duplex will cause an interrupt */
	c = phy_write(phydev, MDIO_DEVAD_NONE, IP101A_G_IRQ_CONF_STATUS,
		      IP101A_G_IRQ_DEFAULT);
	if (c < 0)
		return c;

	if (phydev->interface == PHY_INTERFACE_MODE_RGMII) {
		/*
		 * Additional delay (2ns) used to adjust RX clock phase
		 * at RGMII interface
		 */
		c = phy_read(phydev, MDIO_DEVAD_NONE, IP10XX_SPEC_CTRL_STATUS);
		if (c < 0)
			return c;

		c |= IP1001_PHASE_SEL_MASK;
		c = phy_write(phydev, MDIO_DEVAD_NONE, IP10XX_SPEC_CTRL_STATUS,
			      c);
		if (c < 0)
			return c;
	}

	return 0;
}

static int ip1001_startup(struct phy_device *phydev)
{
	genphy_update_link(phydev);
	genphy_parse_link(phydev);

	return 0;
}
static struct phy_driver IP1001_driver = {
	.name = "ICPlus IP1001",
	.uid = 0x02430d90,
	.mask = 0x0ffffff0,
	.features = PHY_GBIT_FEATURES,
	.config = &ip1001_config,
	.startup = &ip1001_startup,
	.shutdown = &genphy_shutdown,
};

int phy_icplus_init(void)
{
	phy_register(&IP1001_driver);

	return 0;
}
