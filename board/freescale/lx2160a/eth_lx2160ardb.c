// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2018, 2020 NXP
 *
 */

#include <common.h>
#include <netdev.h>
#include <exports.h>
#include <fsl-mc/fsl_mc.h>

DECLARE_GLOBAL_DATA_PTR;

int board_eth_init(struct bd_info *bis)
{
#ifdef CONFIG_PHY_AQUANTIA
	/*
	 * Export functions to be used by AQ firmware
	 * upload application
	 */
	gd->jt->strcpy = strcpy;
	gd->jt->mdelay = mdelay;
	gd->jt->mdio_get_current_dev = mdio_get_current_dev;
	gd->jt->phy_find_by_mask = phy_find_by_mask;
	gd->jt->mdio_phydev_for_ethname = mdio_phydev_for_ethname;
	gd->jt->miiphy_set_current_dev = miiphy_set_current_dev;
#endif
	return pci_eth_init(bis);
}

#if defined(CONFIG_RESET_PHY_R)
void reset_phy(void)
{
#if defined(CONFIG_FSL_MC_ENET)
	mc_env_boot();
#endif
}
#endif /* CONFIG_RESET_PHY_R */

int fdt_fixup_board_phy(void *fdt)
{
	int mdio_offset;
	int ret;
	struct mii_dev *dev;

	ret = 0;

	dev = miiphy_get_dev_by_name(DEFAULT_WRIOP_MDIO2_NAME);
	if (!get_inphi_phy_id(dev, INPHI_PHY_ADDR1, MDIO_MMD_VEND1)) {
		mdio_offset = fdt_path_offset(fdt, "/soc/mdio@0x8B97000");

		if (mdio_offset < 0)
			mdio_offset = fdt_path_offset(fdt, "/mdio@0x8B97000");

		if (mdio_offset < 0) {
			printf("mdio@0x8B9700 node not found in dts\n");
			return mdio_offset;
		}

		ret = fdt_setprop_string(fdt, mdio_offset, "status",
					 "disabled");
		if (ret) {
			printf("Could not set disable mdio@0x8B97000 %s\n",
			       fdt_strerror(ret));
			return ret;
		}
	}

	return ret;
}
