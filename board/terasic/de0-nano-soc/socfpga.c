/*
 *  Copyright (C) 2012 Altera Corporation <www.altera.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>

#include <micrel.h>
#include <netdev.h>
#include <phy.h>

DECLARE_GLOBAL_DATA_PTR;

void s_init(void) {}

/*
 * Miscellaneous platform dependent initialisations
 */
int board_init(void)
{
	/* Address of boot parameters for ATAG (if ATAG is used) */
	gd->bd->bi_boot_params = CONFIG_SYS_SDRAM_BASE + 0x100;

	return 0;
}

/*
 * PHY configuration
 */
#ifdef CONFIG_PHY_MICREL_KSZ9031
int board_phy_config(struct phy_device *phydev)
{
	int ret;
	/*
	 * These skew settings for the KSZ9021 ethernet phy is required for ethernet
	 * to work reliably on most flavors of cyclone5 boards.
	 */
	ret = ksz9031_phy_extended_write(phydev, 0x2,
					 MII_KSZ9031_EXT_RGMII_CTRL_SIG_SKEW,
					 MII_KSZ9031_MOD_DATA_NO_POST_INC,
					 0x70);
	if (ret)
		return ret;

	ret = ksz9031_phy_extended_write(phydev, 0x2,
					 MII_KSZ9031_EXT_RGMII_RX_DATA_SKEW,
					 MII_KSZ9031_MOD_DATA_NO_POST_INC,
					 0x7777);
	if (ret)
		return ret;

	ret = ksz9031_phy_extended_write(phydev, 0x2,
					 MII_KSZ9031_EXT_RGMII_TX_DATA_SKEW,
					 MII_KSZ9031_MOD_DATA_NO_POST_INC,
					 0);
	if (ret)
		return ret;

	ret = ksz9031_phy_extended_write(phydev, 0x2,
					 MII_KSZ9031_EXT_RGMII_CLOCK_SKEW,
					 MII_KSZ9031_MOD_DATA_NO_POST_INC,
					 0x03FC);
	if (ret)
		return ret;

	if (phydev->drv->config)
		return phydev->drv->config(phydev);

	return 0;
}
#endif
