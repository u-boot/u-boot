/*
 * Copyright 2015-2016 Freescale Semiconductor, Inc.
 * Copyright 2017 NXP
 *
 * SPDX-License-Identifier:GPL-2.0+
 */

#include <common.h>
#include <dm.h>
#include <asm/io.h>
#include <netdev.h>
#include <fm_eth.h>
#include <fsl_mdio.h>
#include <malloc.h>
#include <asm/types.h>
#include <fsl_dtsec.h>
#include <asm/arch/soc.h>
#include <asm/arch-fsl-layerscape/config.h>
#include <asm/arch-fsl-layerscape/immap_lsch2.h>
#include <asm/arch/fsl_serdes.h>
#include <net/pfe_eth/pfe_eth.h>
#include <dm/platform_data/pfe_dm_eth.h>
#include <i2c.h>

#define DEFAULT_PFE_MDIO_NAME "PFE_MDIO"

static inline void ls1012ardb_reset_phy(void)
{
	/* Through reset IO expander reset both RGMII and SGMII PHYs */
	i2c_reg_write(I2C_MUX_IO2_ADDR, 6, __PHY_MASK);
	i2c_reg_write(I2C_MUX_IO2_ADDR, 2, __PHY_ETH2_MASK);
	mdelay(10);
	i2c_reg_write(I2C_MUX_IO2_ADDR, 2, __PHY_ETH1_MASK);
	mdelay(10);
	i2c_reg_write(I2C_MUX_IO2_ADDR, 2, 0xFF);
	mdelay(50);
}

int pfe_eth_board_init(struct udevice *dev)
{
	static int init_done;
	struct mii_dev *bus;
	struct pfe_mdio_info mac_mdio_info;
	struct pfe_eth_dev *priv = dev_get_priv(dev);

	if (!init_done) {
		ls1012ardb_reset_phy();
		mac_mdio_info.reg_base = (void *)EMAC1_BASE_ADDR;
		mac_mdio_info.name = DEFAULT_PFE_MDIO_NAME;

		bus = pfe_mdio_init(&mac_mdio_info);
		if (!bus) {
			printf("Failed to register mdio\n");
			return -1;
		}
		init_done = 1;
	}

	pfe_set_mdio(priv->gemac_port,
		     miiphy_get_dev_by_name(DEFAULT_PFE_MDIO_NAME));

	if (!priv->gemac_port) {
		/* MAC1 */
		pfe_set_phy_address_mode(priv->gemac_port, EMAC1_PHY_ADDR,
					 PHY_INTERFACE_MODE_SGMII);
	} else {
		/* MAC2 */
		pfe_set_phy_address_mode(priv->gemac_port, EMAC2_PHY_ADDR,
					 PHY_INTERFACE_MODE_RGMII_TXID);
	}
	return 0;
}

static struct pfe_eth_pdata pfe_pdata0 = {
	.pfe_eth_pdata_mac = {
		.iobase = (phys_addr_t)EMAC1_BASE_ADDR,
		.phy_interface = 0,
	},

	.pfe_ddr_addr = {
		.ddr_pfe_baseaddr = (void *)CONFIG_DDR_PFE_BASEADDR,
		.ddr_pfe_phys_baseaddr = CONFIG_DDR_PFE_PHYS_BASEADDR,
	},
};

static struct pfe_eth_pdata pfe_pdata1 = {
	.pfe_eth_pdata_mac = {
		.iobase = (phys_addr_t)EMAC2_BASE_ADDR,
		.phy_interface = 1,
	},

	.pfe_ddr_addr = {
		.ddr_pfe_baseaddr = (void *)CONFIG_DDR_PFE_BASEADDR,
		.ddr_pfe_phys_baseaddr = CONFIG_DDR_PFE_PHYS_BASEADDR,
	},
};

U_BOOT_DEVICE(ls1012a_pfe0) = {
	.name = "pfe_eth",
	.platdata = &pfe_pdata0,
};

U_BOOT_DEVICE(ls1012a_pfe1) = {
	.name = "pfe_eth",
	.platdata = &pfe_pdata1,
};
