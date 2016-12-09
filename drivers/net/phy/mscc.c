/*
 * Microsemi PHY drivers
 *
 * SPDX-License-Identifier: The MIT License (MIT)
 *
 * Copyright (c) 2016 Microsemi Corporation
 *
 * Author: John Haechten
 *
 */

#include <miiphy.h>
#include <bitfield.h>

/* Microsemi PHY ID's */
#define PHY_ID_VSC8530                  0x00070560
#define PHY_ID_VSC8531                  0x00070570
#define PHY_ID_VSC8540                  0x00070760
#define PHY_ID_VSC8541                  0x00070770

/* Microsemi VSC85xx PHY Register Pages */
#define MSCC_EXT_PAGE_ACCESS            31     /* Page Access Register */
#define MSCC_PHY_PAGE_STD		0x0000 /* Standard registers */
#define MSCC_PHY_PAGE_EXT1		0x0001 /* Extended registers - page 1 */
#define MSCC_PHY_PAGE_EXT2		0x0002 /* Extended registers - page 2 */
#define MSCC_PHY_PAGE_EXT3		0x0003 /* Extended registers - page 3 */
#define MSCC_PHY_PAGE_EXT4		0x0004 /* Extended registers - page 4 */
#define MSCC_PHY_PAGE_GPIO		0x0010 /* GPIO registers */
#define MSCC_PHY_PAGE_TEST		0x2A30 /* TEST Page registers */
#define MSCC_PHY_PAGE_TR		0x52B5 /* Token Ring Page registers */

/* Std Page Register 28 - PHY AUX Control/Status */
#define MIIM_AUX_CNTRL_STAT_REG		28
#define MIIM_AUX_CNTRL_STAT_ACTIPHY_TO	(0x0004)
#define MIIM_AUX_CNTRL_STAT_F_DUPLEX	(0x0020)
#define MIIM_AUX_CNTRL_STAT_SPEED_MASK	(0x0018)
#define MIIM_AUX_CNTRL_STAT_SPEED_POS	(3)
#define MIIM_AUX_CNTRL_STAT_SPEED_10M	(0x0)
#define MIIM_AUX_CNTRL_STAT_SPEED_100M	(0x1)
#define MIIM_AUX_CNTRL_STAT_SPEED_1000M	(0x2)

/* Std Page Register 23 - Extended PHY CTRL_1 */
#define MSCC_PHY_EXT_PHY_CNTL_1_REG	23
#define MAC_IF_SELECTION_MASK		(0x1800)
#define MAC_IF_SELECTION_GMII		(0)
#define MAC_IF_SELECTION_RMII		(1)
#define MAC_IF_SELECTION_RGMII		(2)
#define MAC_IF_SELECTION_POS		(11)
#define MAC_IF_SELECTION_WIDTH		(2)

/* Extended Page 2 Register 20E2 */
#define MSCC_PHY_RGMII_CNTL_REG		20
#define VSC_FAST_LINK_FAIL2_ENA_MASK	(0x8000)
#define RX_CLK_OUT_MASK			(0x0800)
#define RX_CLK_OUT_POS			(11)
#define RX_CLK_OUT_WIDTH		(1)
#define RX_CLK_OUT_NORMAL		(0)
#define RX_CLK_OUT_DISABLE		(1)
#define RGMII_RX_CLK_DELAY_POS		(4)
#define RGMII_RX_CLK_DELAY_WIDTH	(3)
#define RGMII_RX_CLK_DELAY_MASK		(0x0070)
#define RGMII_TX_CLK_DELAY_POS		(0)
#define RGMII_TX_CLK_DELAY_WIDTH	(3)
#define RGMII_TX_CLK_DELAY_MASK		(0x0007)

/* Extended Page 2 Register 27E2 */
#define MSCC_PHY_WOL_MAC_CONTROL	27
#define EDGE_RATE_CNTL_POS		(5)
#define EDGE_RATE_CNTL_WIDTH		(3)
#define EDGE_RATE_CNTL_MASK		(0x00E0)
#define RMII_CLK_OUT_ENABLE_POS		(4)
#define RMII_CLK_OUT_ENABLE_WIDTH	(1)
#define RMII_CLK_OUT_ENABLE_MASK	(0x10)

/* Token Ring Page 0x52B5 Registers */
#define MSCC_PHY_REG_TR_ADDR_16		16
#define MSCC_PHY_REG_TR_DATA_17		17
#define MSCC_PHY_REG_TR_DATA_18		18

/* Token Ring - Read Value in */
#define MSCC_PHY_TR_16_READ		(0xA000)
/* Token Ring - Write Value out */
#define MSCC_PHY_TR_16_WRITE		(0x8000)

/* Token Ring Registers */
#define MSCC_PHY_TR_LINKDETCTRL_POS	(3)
#define MSCC_PHY_TR_LINKDETCTRL_WIDTH	(2)
#define MSCC_PHY_TR_LINKDETCTRL_VAL	(3)
#define MSCC_PHY_TR_LINKDETCTRL_MASK	(0x0018)
#define MSCC_PHY_TR_LINKDETCTRL_ADDR	(0x07F8)

#define MSCC_PHY_TR_VGATHRESH100_POS	(0)
#define MSCC_PHY_TR_VGATHRESH100_WIDTH	(7)
#define MSCC_PHY_TR_VGATHRESH100_VAL	(0x0018)
#define MSCC_PHY_TR_VGATHRESH100_MASK	(0x007f)
#define MSCC_PHY_TR_VGATHRESH100_ADDR	(0x0FA4)

#define MSCC_PHY_TR_VGAGAIN10_U_POS	(0)
#define MSCC_PHY_TR_VGAGAIN10_U_WIDTH	(1)
#define MSCC_PHY_TR_VGAGAIN10_U_MASK	(0x0001)
#define MSCC_PHY_TR_VGAGAIN10_U_VAL	(0)

#define MSCC_PHY_TR_VGAGAIN10_L_POS	(12)
#define MSCC_PHY_TR_VGAGAIN10_L_WIDTH	(4)
#define MSCC_PHY_TR_VGAGAIN10_L_MASK	(0xf000)
#define MSCC_PHY_TR_VGAGAIN10_L_VAL	(0x0001)
#define MSCC_PHY_TR_VGAGAIN10_ADDR	(0x0F92)

/* General Timeout Values */
#define MSCC_PHY_RESET_TIMEOUT		(100)
#define MSCC_PHY_MICRO_TIMEOUT		(500)

/* RGMII/GMII Clock Delay (Skew) Options */ enum vsc_phy_rgmii_skew {
	VSC_PHY_RGMII_DELAY_200_PS,
	VSC_PHY_RGMII_DELAY_800_PS,
	VSC_PHY_RGMII_DELAY_1100_PS,
	VSC_PHY_RGMII_DELAY_1700_PS,
	VSC_PHY_RGMII_DELAY_2000_PS,
	VSC_PHY_RGMII_DELAY_2300_PS,
	VSC_PHY_RGMII_DELAY_2600_PS,
	VSC_PHY_RGMII_DELAY_3400_PS,
};

/* MAC i/f Clock Edge Rage Control (Slew), See Reg27E2  */ enum
vsc_phy_clk_slew {
	VSC_PHY_CLK_SLEW_RATE_0,
	VSC_PHY_CLK_SLEW_RATE_1,
	VSC_PHY_CLK_SLEW_RATE_2,
	VSC_PHY_CLK_SLEW_RATE_3,
	VSC_PHY_CLK_SLEW_RATE_4,
	VSC_PHY_CLK_SLEW_RATE_5,
	VSC_PHY_CLK_SLEW_RATE_6,
	VSC_PHY_CLK_SLEW_RATE_7,
};


static int mscc_vsc8531_vsc8541_init_scripts(struct phy_device *phydev)
{
	u16	reg_val;

	/* Set to Access Token Ring Registers */
	phy_write(phydev, MDIO_DEVAD_NONE,
		  MSCC_EXT_PAGE_ACCESS, MSCC_PHY_PAGE_TR);

	/* Update LinkDetectCtrl default to optimized values */
	/* Determined during Silicon Validation Testing */
	phy_write(phydev, MDIO_DEVAD_NONE, MSCC_PHY_REG_TR_ADDR_16,
		  (MSCC_PHY_TR_LINKDETCTRL_ADDR | MSCC_PHY_TR_16_READ));
	reg_val = phy_read(phydev, MDIO_DEVAD_NONE, MSCC_PHY_REG_TR_DATA_17);
	reg_val = bitfield_replace(reg_val, MSCC_PHY_TR_LINKDETCTRL_POS,
				   MSCC_PHY_TR_LINKDETCTRL_WIDTH,
				   MSCC_PHY_TR_LINKDETCTRL_VAL);

	phy_write(phydev, MDIO_DEVAD_NONE, MSCC_PHY_REG_TR_DATA_17, reg_val);
	phy_write(phydev, MDIO_DEVAD_NONE, MSCC_PHY_REG_TR_ADDR_16,
		  (MSCC_PHY_TR_LINKDETCTRL_ADDR | MSCC_PHY_TR_16_WRITE));

	/* Update VgaThresh100 defaults to optimized values */
	/* Determined during Silicon Validation Testing */
	phy_write(phydev, MDIO_DEVAD_NONE, MSCC_PHY_REG_TR_ADDR_16,
		  (MSCC_PHY_TR_VGATHRESH100_ADDR | MSCC_PHY_TR_16_READ));

	reg_val = phy_read(phydev, MDIO_DEVAD_NONE, MSCC_PHY_REG_TR_DATA_18);
	reg_val = bitfield_replace(reg_val, MSCC_PHY_TR_VGATHRESH100_POS,
				   MSCC_PHY_TR_VGATHRESH100_WIDTH,
				   MSCC_PHY_TR_VGATHRESH100_VAL);

	phy_write(phydev, MDIO_DEVAD_NONE, MSCC_PHY_REG_TR_DATA_18, reg_val);
	phy_write(phydev, MDIO_DEVAD_NONE, MSCC_PHY_REG_TR_ADDR_16,
		  (MSCC_PHY_TR_VGATHRESH100_ADDR | MSCC_PHY_TR_16_WRITE));

	/* Update VgaGain10 defaults to optimized values */
	/* Determined during Silicon Validation Testing */
	phy_write(phydev, MDIO_DEVAD_NONE, MSCC_PHY_REG_TR_ADDR_16,
		  (MSCC_PHY_TR_VGAGAIN10_ADDR | MSCC_PHY_TR_16_READ));

	reg_val = phy_read(phydev, MDIO_DEVAD_NONE, MSCC_PHY_REG_TR_DATA_18);
	reg_val = bitfield_replace(reg_val, MSCC_PHY_TR_VGAGAIN10_U_POS,
				   MSCC_PHY_TR_VGAGAIN10_U_WIDTH,
				   MSCC_PHY_TR_VGAGAIN10_U_VAL);

	phy_write(phydev, MDIO_DEVAD_NONE, MSCC_PHY_REG_TR_DATA_18, reg_val);
	reg_val = phy_read(phydev, MDIO_DEVAD_NONE, MSCC_PHY_REG_TR_DATA_17);
	reg_val = bitfield_replace(reg_val, MSCC_PHY_TR_VGAGAIN10_L_POS,
				   MSCC_PHY_TR_VGAGAIN10_L_WIDTH,
				   MSCC_PHY_TR_VGAGAIN10_L_VAL);

	phy_write(phydev, MDIO_DEVAD_NONE, MSCC_PHY_REG_TR_DATA_17, reg_val);
	phy_write(phydev, MDIO_DEVAD_NONE, MSCC_PHY_REG_TR_ADDR_16,
		  (MSCC_PHY_TR_VGAGAIN10_ADDR | MSCC_PHY_TR_16_WRITE));

	/* Set back to Access Standard Page Registers */
	phy_write(phydev, MDIO_DEVAD_NONE, MSCC_EXT_PAGE_ACCESS,
		  MSCC_PHY_PAGE_STD);

	return 0;
}

static int mscc_parse_status(struct phy_device *phydev)
{
	u16 speed;
	u16 mii_reg;

	mii_reg = phy_read(phydev, MDIO_DEVAD_NONE, MIIM_AUX_CNTRL_STAT_REG);

	if (mii_reg & MIIM_AUX_CNTRL_STAT_F_DUPLEX)
		phydev->duplex = DUPLEX_FULL;
	else
		phydev->duplex = DUPLEX_HALF;

	speed = mii_reg & MIIM_AUX_CNTRL_STAT_SPEED_MASK;
	speed = speed >> MIIM_AUX_CNTRL_STAT_SPEED_POS;

	switch (speed) {
	case MIIM_AUX_CNTRL_STAT_SPEED_1000M:
		phydev->speed = SPEED_1000;
		break;
	case MIIM_AUX_CNTRL_STAT_SPEED_100M:
		phydev->speed = SPEED_100;
		break;
	case MIIM_AUX_CNTRL_STAT_SPEED_10M:
		phydev->speed = SPEED_10;
		break;
	default:
		phydev->speed = SPEED_10;
		break;
	}

	return 0;
}

static int mscc_startup(struct phy_device *phydev)
{
	int retval;

	retval = genphy_update_link(phydev);

	if (retval)
		return retval;

	return mscc_parse_status(phydev);
}

static int mscc_phy_soft_reset(struct phy_device *phydev)
{
	int     retval = 0;
	u16     timeout = MSCC_PHY_RESET_TIMEOUT;
	u16     reg_val = 0;

	phy_write(phydev, MDIO_DEVAD_NONE, MSCC_EXT_PAGE_ACCESS,
		  MSCC_PHY_PAGE_STD);

	reg_val = phy_read(phydev, MDIO_DEVAD_NONE, MII_BMCR);
	phy_write(phydev, MDIO_DEVAD_NONE, MII_BMCR, (reg_val | BMCR_RESET));

	reg_val = phy_read(phydev, MDIO_DEVAD_NONE, MII_BMCR);

	while ((reg_val & BMCR_RESET) && (timeout > 0)) {
		reg_val = phy_read(phydev, MDIO_DEVAD_NONE, MII_BMCR);
		timeout--;
		udelay(1000);   /* 1 ms */
	}

	if (timeout == 0) {
		printf("MSCC PHY Soft_Reset Error: mac i/f = 0x%x\n",
		       phydev->interface);
		retval = -ETIME;
	}

	return retval;
}

static int vsc8531_vsc8541_mac_config(struct phy_device *phydev)
{
	u16	reg_val = 0;
	u16	mac_if = 0;
	u16	rx_clk_out = 0;

	/* For VSC8530/31 the only MAC modes are RMII/RGMII. */
	/* For VSC8540/41 the only MAC modes are (G)MII and RMII/RGMII. */
	/* Setup MAC Configuration */
	switch (phydev->interface) {
	case PHY_INTERFACE_MODE_MII:
	case PHY_INTERFACE_MODE_GMII:
		/* Set Reg23.12:11=0 */
		mac_if = MAC_IF_SELECTION_GMII;
		/* Set Reg20E2.11=1 */
		rx_clk_out = RX_CLK_OUT_DISABLE;
		break;

	case PHY_INTERFACE_MODE_RMII:
		/* Set Reg23.12:11=1 */
		mac_if = MAC_IF_SELECTION_RMII;
		/* Set Reg20E2.11=0 */
		rx_clk_out = RX_CLK_OUT_NORMAL;
		break;

	case PHY_INTERFACE_MODE_RGMII:
		/* Set Reg23.12:11=2 */
		mac_if = MAC_IF_SELECTION_RGMII;
		/* Set Reg20E2.11=0 */
		rx_clk_out = RX_CLK_OUT_NORMAL;
		break;

	default:
		printf("MSCC PHY - INVALID MAC i/f Config: mac i/f = 0x%x\n",
		       phydev->interface);
		return -EINVAL;
	}

	phy_write(phydev, MDIO_DEVAD_NONE, MSCC_EXT_PAGE_ACCESS,
		  MSCC_PHY_PAGE_STD);

	reg_val = phy_read(phydev, MDIO_DEVAD_NONE,
			   MSCC_PHY_EXT_PHY_CNTL_1_REG);
	/* Set MAC i/f bits Reg23.12:11 */
	reg_val = bitfield_replace(reg_val, MAC_IF_SELECTION_POS,
				   MAC_IF_SELECTION_WIDTH, mac_if);
	/* Update Reg23.12:11 */
	phy_write(phydev, MDIO_DEVAD_NONE,
		  MSCC_PHY_EXT_PHY_CNTL_1_REG, reg_val);
	/* Setup ExtPg_2 Register Access */
	phy_write(phydev, MDIO_DEVAD_NONE,
		  MSCC_EXT_PAGE_ACCESS, MSCC_PHY_PAGE_EXT2);
	/* Read Reg20E2 */
	reg_val = phy_read(phydev, MDIO_DEVAD_NONE,
			   MSCC_PHY_RGMII_CNTL_REG);
	reg_val = bitfield_replace(reg_val, RX_CLK_OUT_POS,
				   RX_CLK_OUT_WIDTH, rx_clk_out);
	/* Update Reg20E2.11 */
	phy_write(phydev, MDIO_DEVAD_NONE,
		  MSCC_PHY_RGMII_CNTL_REG, reg_val);
	/* Before leaving - Change back to Std Page Register Access */
	phy_write(phydev, MDIO_DEVAD_NONE, MSCC_EXT_PAGE_ACCESS,
		  MSCC_PHY_PAGE_STD);

	return 0;
}

static int vsc8531_config(struct phy_device *phydev)
{
	int  retval = -EINVAL;
	u16  reg_val;
	u16  rmii_clk_out;
	enum vsc_phy_rgmii_skew  rx_clk_skew = VSC_PHY_RGMII_DELAY_1700_PS;
	enum vsc_phy_rgmii_skew  tx_clk_skew = VSC_PHY_RGMII_DELAY_800_PS;
	enum vsc_phy_clk_slew    edge_rate = VSC_PHY_CLK_SLEW_RATE_4;

	/* For VSC8530/31 and VSC8540/41 the init scripts are the same */
	mscc_vsc8531_vsc8541_init_scripts(phydev);

	/* For VSC8530/31 the only MAC modes are RMII/RGMII. */
	switch (phydev->interface) {
	case PHY_INTERFACE_MODE_RMII:
	case PHY_INTERFACE_MODE_RGMII:
		retval = vsc8531_vsc8541_mac_config(phydev);
		if (retval != 0)
			return retval;

		retval = mscc_phy_soft_reset(phydev);
		if (retval != 0)
			return retval;
		break;
	default:
		printf("PHY 8530/31 MAC i/f Config Error: mac i/f = 0x%x\n",
		       phydev->interface);
		return -EINVAL;
	}
	/* Default RMII Clk Output to 0=OFF/1=ON  */
	rmii_clk_out = 0;

	phy_write(phydev, MDIO_DEVAD_NONE, MSCC_EXT_PAGE_ACCESS,
		  MSCC_PHY_PAGE_EXT2);
	reg_val = phy_read(phydev, MDIO_DEVAD_NONE, MSCC_PHY_RGMII_CNTL_REG);

	/* Reg20E2 - Update RGMII RX_Clk Skews. */
	reg_val = bitfield_replace(reg_val, RGMII_RX_CLK_DELAY_POS,
				   RGMII_RX_CLK_DELAY_WIDTH, rx_clk_skew);
	/* Reg20E2 - Update RGMII TX_Clk Skews. */
	reg_val = bitfield_replace(reg_val, RGMII_TX_CLK_DELAY_POS,
				   RGMII_TX_CLK_DELAY_WIDTH, tx_clk_skew);

	phy_write(phydev, MDIO_DEVAD_NONE, MSCC_PHY_RGMII_CNTL_REG, reg_val);

	reg_val = phy_read(phydev, MDIO_DEVAD_NONE, MSCC_PHY_WOL_MAC_CONTROL);
	/* Reg27E2 - Update Clk Slew Rate. */
	reg_val = bitfield_replace(reg_val, EDGE_RATE_CNTL_POS,
				   EDGE_RATE_CNTL_WIDTH, edge_rate);
	/* Reg27E2 - Update RMII Clk Out. */
	reg_val = bitfield_replace(reg_val, RMII_CLK_OUT_ENABLE_POS,
				   RMII_CLK_OUT_ENABLE_WIDTH, rmii_clk_out);
	/* Update Reg27E2 */
	phy_write(phydev, MDIO_DEVAD_NONE, MSCC_PHY_WOL_MAC_CONTROL, reg_val);
	phy_write(phydev, MDIO_DEVAD_NONE, MSCC_EXT_PAGE_ACCESS,
		  MSCC_PHY_PAGE_STD);

	return genphy_config_aneg(phydev);
}

static int vsc8541_config(struct phy_device *phydev)
{
	int  retval = -EINVAL;
	u16  reg_val;
	u16  rmii_clk_out;
	enum vsc_phy_rgmii_skew  rx_clk_skew = VSC_PHY_RGMII_DELAY_1700_PS;
	enum vsc_phy_rgmii_skew  tx_clk_skew = VSC_PHY_RGMII_DELAY_800_PS;
	enum vsc_phy_clk_slew    edge_rate = VSC_PHY_CLK_SLEW_RATE_4;

	/* For VSC8530/31 and VSC8540/41 the init scripts are the same */
	mscc_vsc8531_vsc8541_init_scripts(phydev);

	/* For VSC8540/41 the only MAC modes are (G)MII and RMII/RGMII. */
	switch (phydev->interface) {
	case PHY_INTERFACE_MODE_MII:
	case PHY_INTERFACE_MODE_GMII:
	case PHY_INTERFACE_MODE_RMII:
	case PHY_INTERFACE_MODE_RGMII:
		retval = vsc8531_vsc8541_mac_config(phydev);
		if (retval != 0)
			return retval;

		retval = mscc_phy_soft_reset(phydev);
		if (retval != 0)
			return retval;
		break;
	default:
		printf("PHY 8541 MAC i/f config Error: mac i/f = 0x%x\n",
		       phydev->interface);
		return -EINVAL;
	}
	/* Default RMII Clk Output to 0=OFF/1=ON  */
	rmii_clk_out = 0;

	phy_write(phydev, MDIO_DEVAD_NONE, MSCC_EXT_PAGE_ACCESS,
		  MSCC_PHY_PAGE_EXT2);
	reg_val = phy_read(phydev, MDIO_DEVAD_NONE, MSCC_PHY_RGMII_CNTL_REG);
	/* Reg20E2 - Update RGMII RX_Clk Skews. */
	reg_val = bitfield_replace(reg_val, RGMII_RX_CLK_DELAY_POS,
				   RGMII_RX_CLK_DELAY_WIDTH, rx_clk_skew);
	/* Reg20E2 - Update RGMII TX_Clk Skews. */
	reg_val = bitfield_replace(reg_val, RGMII_TX_CLK_DELAY_POS,
				   RGMII_TX_CLK_DELAY_WIDTH, tx_clk_skew);
	phy_write(phydev, MDIO_DEVAD_NONE, MSCC_PHY_RGMII_CNTL_REG, reg_val);

	reg_val = phy_read(phydev, MDIO_DEVAD_NONE, MSCC_PHY_WOL_MAC_CONTROL);
	/* Reg27E2 - Update Clk Slew Rate. */
	reg_val = bitfield_replace(reg_val, EDGE_RATE_CNTL_POS,
				   EDGE_RATE_CNTL_WIDTH, edge_rate);
	/* Reg27E2 - Update RMII Clk Out. */
	reg_val = bitfield_replace(reg_val, RMII_CLK_OUT_ENABLE_POS,
				   RMII_CLK_OUT_ENABLE_WIDTH, rmii_clk_out);
	/* Update Reg27E2 */
	phy_write(phydev, MDIO_DEVAD_NONE, MSCC_PHY_WOL_MAC_CONTROL, reg_val);
	phy_write(phydev, MDIO_DEVAD_NONE, MSCC_EXT_PAGE_ACCESS,
		  MSCC_PHY_PAGE_STD);

	return genphy_config_aneg(phydev);
}

static struct phy_driver VSC8530_driver = {
	.name = "Microsemi VSC8530",
	.uid = PHY_ID_VSC8530,
	.mask = 0x000ffff0,
	.features = PHY_BASIC_FEATURES,
	.config = &vsc8531_config,
	.startup = &mscc_startup,
	.shutdown = &genphy_shutdown,
};

static struct phy_driver VSC8531_driver = {
	.name = "Microsemi VSC8531",
	.uid = PHY_ID_VSC8531,
	.mask = 0x000ffff0,
	.features = PHY_GBIT_FEATURES,
	.config = &vsc8531_config,
	.startup = &mscc_startup,
	.shutdown = &genphy_shutdown,
};

static struct phy_driver VSC8540_driver = {
	.name = "Microsemi VSC8540",
	.uid = PHY_ID_VSC8540,
	.mask = 0x000ffff0,
	.features = PHY_BASIC_FEATURES,
	.config = &vsc8541_config,
	.startup = &mscc_startup,
	.shutdown = &genphy_shutdown,
};

static struct phy_driver VSC8541_driver = {
	.name = "Microsemi VSC8541",
	.uid = PHY_ID_VSC8541,
	.mask = 0x000ffff0,
	.features = PHY_GBIT_FEATURES,
	.config = &vsc8541_config,
	.startup = &mscc_startup,
	.shutdown = &genphy_shutdown,
};

int phy_mscc_init(void)
{
	phy_register(&VSC8530_driver);
	phy_register(&VSC8531_driver);
	phy_register(&VSC8540_driver);
	phy_register(&VSC8541_driver);

	return 0;
}
