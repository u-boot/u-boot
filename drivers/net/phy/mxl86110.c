// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2026 Altera Corporation <www.altera.com>
 *
 * PHY driver for MaxLinear MXL86110 / MXL86111. Ported from Linux
 * drivers/net/phy/mxl-86110.c
 *
 */

#include <log.h>
#include <phy.h>
#include <linux/bitfield.h>
#include <linux/bitops.h>
#include <linux/errno.h>

/* PHY IDs */
#define PHY_ID_MXL86110				0xc1335580
#define PHY_ID_MXL86111				0xc1335588
#define PHY_ID_MXL_MASK				0xfffffff0

/* Extended register access window (same style as Motorcomm YT* PHYs) */
#define MXL86110_EXTD_REG_ADDR_OFFSET		0x1E
#define MXL86110_EXTD_REG_ADDR_DATA		0x1F

/* SyncE Configuration Register - COM_EXT_SYNCE_CFG */
#define MXL86110_EXT_SYNCE_CFG_REG		0xA012
#define MXL86110_EXT_SYNCE_CFG_CLK_FRE_SEL	BIT(4)
#define MXL86110_EXT_SYNCE_CFG_EN_SYNC_E	BIT(6)
#define MXL86110_EXT_SYNCE_CFG_CLK_SRC_SEL_MASK	GENMASK(3, 1)
#define MXL86110_EXT_SYNCE_CFG_CLK_SRC_SEL_25M	4

/* Chip Configuration Register - COM_EXT_CHIP_CFG */
#define MXL86110_EXT_CHIP_CFG_REG		0xA001
#define MXL86110_EXT_CHIP_CFG_RXDLY_ENABLE	BIT(8)

/* RGMII Config1 */
#define MXL86110_EXT_RGMII_CFG1_REG		0xA003
/* delay can be adjusted in steps of about 150ps */
#define MXL86110_EXT_RGMII_CFG1_RX_NO_DELAY	(0x0 << 10)
/* Closest value to 2000 ps */
#define MXL86110_EXT_RGMII_CFG1_RX_DELAY_1950PS	(0xD << 10)
#define MXL86110_EXT_RGMII_CFG1_RX_DELAY_MASK	GENMASK(13, 10)
#define MXL86110_EXT_RGMII_CFG1_TX_1G_DELAY_1950PS	(0xD << 0)
#define MXL86110_EXT_RGMII_CFG1_TX_1G_DELAY_MASK	GENMASK(3, 0)
#define MXL86110_EXT_RGMII_CFG1_TX_10MB_100MB_DELAY_1950PS	(0xD << 4)
#define MXL86110_EXT_RGMII_CFG1_TX_10MB_100MB_DELAY_MASK	GENMASK(7, 4)
#define MXL86110_EXT_RGMII_CFG1_FULL_MASK \
	(MXL86110_EXT_RGMII_CFG1_RX_DELAY_MASK | \
	 MXL86110_EXT_RGMII_CFG1_TX_1G_DELAY_MASK | \
	 MXL86110_EXT_RGMII_CFG1_TX_10MB_100MB_DELAY_MASK)

/* RGMII MDIO / broadcast */
#define MXL86110_EXT_RGMII_MDIO_CFG		0xA005
#define MXL86110_RGMII_MDIO_CFG_EPA0_MASK	BIT(6)

/* LED (optional activity blink) */
#define MXL86110_MAX_LEDS			3
#define MXL86110_LED0_CFG_REG			0xA00C
#define MXL86110_LEDX_CFG_BLINK			BIT(13)

/* PHY specific status (used by several Alaska-class PHYs) */
#define MXL86110_PHY_STAT_REG			0x11
#define MXL86110_PHY_STAT_SPEED_MASK		GENMASK(15, 14)
#define MXL86110_PHY_STAT_SPEED_10M		0x0
#define MXL86110_PHY_STAT_SPEED_100M		0x1
#define MXL86110_PHY_STAT_SPEED_1000M		0x2
#define MXL86110_PHY_STAT_DPX			BIT(13)
#define MXL86110_PHY_STAT_LINK			BIT(10)

/*
 * Digital RGMII delay codes: field value i selects ~150 ps * i, so the table
 * maps a picosecond request from the device tree onto the nearest code.
 */
static const int mxl86110_rgmii_delays[] = {
	0, 150, 300, 450, 600, 750, 900, 1050,
	1200, 1350, 1500, 1650, 1800, 1950, 2100, 2250,
};

static int mxl86110_write_extended_reg(struct phy_device *phydev, u16 regnum,
				       u16 val)
{
	int ret;

	ret = phy_write(phydev, MDIO_DEVAD_NONE, MXL86110_EXTD_REG_ADDR_OFFSET,
			regnum);
	if (ret < 0)
		return ret;

	return phy_write(phydev, MDIO_DEVAD_NONE, MXL86110_EXTD_REG_ADDR_DATA,
			 val);
}

static int mxl86110_read_extended_reg(struct phy_device *phydev, u16 regnum)
{
	int ret;

	ret = phy_write(phydev, MDIO_DEVAD_NONE, MXL86110_EXTD_REG_ADDR_OFFSET,
			regnum);
	if (ret < 0)
		return ret;

	return phy_read(phydev, MDIO_DEVAD_NONE, MXL86110_EXTD_REG_ADDR_DATA);
}

static int mxl86110_modify_extended_reg(struct phy_device *phydev, u16 regnum,
					u16 mask, u16 set)
{
	int val;

	val = mxl86110_read_extended_reg(phydev, regnum);
	if (val < 0)
		return val;

	val = (val & ~mask) | set;

	return mxl86110_write_extended_reg(phydev, regnum, val);
}

static int mxl86110_synce_clk_cfg(struct phy_device *phydev)
{
	u16 mask, val;

	/*
	 * Default SyncE / clk output per Linux driver datasheet defaults:
	 * enable SyncE and select 25 MHz clock source.
	 */
	val = MXL86110_EXT_SYNCE_CFG_EN_SYNC_E |
	      FIELD_PREP(MXL86110_EXT_SYNCE_CFG_CLK_SRC_SEL_MASK,
			 MXL86110_EXT_SYNCE_CFG_CLK_SRC_SEL_25M);
	mask = MXL86110_EXT_SYNCE_CFG_EN_SYNC_E |
	       MXL86110_EXT_SYNCE_CFG_CLK_SRC_SEL_MASK |
	       MXL86110_EXT_SYNCE_CFG_CLK_FRE_SEL;

	return mxl86110_modify_extended_reg(phydev, MXL86110_EXT_SYNCE_CFG_REG,
					    mask, val);
}

static int mxl86110_broadcast_cfg(struct phy_device *phydev)
{
	/* Disable MDIO broadcast (clear EPA0) */
	return mxl86110_modify_extended_reg(phydev, MXL86110_EXT_RGMII_MDIO_CFG,
					    MXL86110_RGMII_MDIO_CFG_EPA0_MASK,
					    0);
}

static int mxl86110_enable_led_activity_blink(struct phy_device *phydev)
{
	int i, ret = 0;

	for (i = 0; i < MXL86110_MAX_LEDS; i++) {
		ret = mxl86110_modify_extended_reg(phydev,
						   MXL86110_LED0_CFG_REG + i,
						   0, MXL86110_LEDX_CFG_BLINK);
		if (ret < 0)
			break;
	}

	return ret;
}

/**
 * mxl86110_get_internal_delay() - map DT *-internal-delay-ps to delay index
 * @phydev: PHY device
 * @is_rx: true for rx-internal-delay-ps, false for tx-internal-delay-ps
 *
 * U-Boot stand-in for Linux phy_get_internal_delay(). Returns the index into
 * mxl86110_rgmii_delays, or a negative errno if the property is absent/invalid.
 */
static int mxl86110_get_internal_delay(struct phy_device *phydev, bool is_rx)
{
	ofnode node = phy_get_ofnode(phydev);
	const char *prop = is_rx ? "rx-internal-delay-ps" :
				   "tx-internal-delay-ps";
	u32 delay;
	int i, ret;

	if (!ofnode_valid(node))
		return -ENODEV;

	ret = ofnode_read_u32(node, prop, &delay);
	if (ret)
		return ret;

	if (delay < mxl86110_rgmii_delays[0] ||
	    delay > mxl86110_rgmii_delays[ARRAY_SIZE(mxl86110_rgmii_delays) - 1]) {
		pr_err("%s: %s %u out of range\n", phydev->drv->name, prop,
		       delay);
		return -EINVAL;
	}

	if (delay == mxl86110_rgmii_delays[0])
		return 0;

	for (i = 1; i < ARRAY_SIZE(mxl86110_rgmii_delays); i++) {
		if (delay == mxl86110_rgmii_delays[i])
			return i;

		if (delay > mxl86110_rgmii_delays[i - 1] &&
		    delay < mxl86110_rgmii_delays[i]) {
			if (delay - mxl86110_rgmii_delays[i - 1] <
			    mxl86110_rgmii_delays[i] - delay)
				return i - 1;
			return i;
		}
	}

	pr_err("%s: error finding internal delay index for %u\n",
	       phydev->drv->name, delay);
	return -EINVAL;
}

/**
 * mxl86110_config_rgmii_delay() - program digital RGMII TX/RX delays
 *
 * phy-mode sets the default digital delays, then optional
 * rx/tx-internal-delay-ps overrides the field codes. When
 * rx-internal-delay-ps is present, clear CHIP_CFG RXDLY_ENABLE so
 * only the programmed digital RX delay is used.
 */
static int mxl86110_config_rgmii_delay(struct phy_device *phydev)
{
	int rx_delay;
	int tx_delay;
	u16 val;
	int ret;

	/* Digital delay codes implied by the phy-mode. */
	switch (phydev->interface) {
	case PHY_INTERFACE_MODE_RGMII:
		val = 0;
		break;
	case PHY_INTERFACE_MODE_RGMII_RXID:
		val = MXL86110_EXT_RGMII_CFG1_RX_DELAY_1950PS;
		break;
	case PHY_INTERFACE_MODE_RGMII_TXID:
		val = MXL86110_EXT_RGMII_CFG1_TX_1G_DELAY_1950PS |
		      MXL86110_EXT_RGMII_CFG1_TX_10MB_100MB_DELAY_1950PS;
		break;
	case PHY_INTERFACE_MODE_RGMII_ID:
		val = MXL86110_EXT_RGMII_CFG1_TX_1G_DELAY_1950PS |
		      MXL86110_EXT_RGMII_CFG1_TX_10MB_100MB_DELAY_1950PS |
		      MXL86110_EXT_RGMII_CFG1_RX_DELAY_1950PS;
		break;
	default:
		return -EINVAL;
	}

	/*
	 * Explicit rx/tx-internal-delay-ps bindings, when present, override the
	 * phy-mode defaults so a board can pin the exact digital delay code.
	 */
	rx_delay = mxl86110_get_internal_delay(phydev, true);
	if (rx_delay >= 0) {
		val &= ~MXL86110_EXT_RGMII_CFG1_RX_DELAY_MASK;
		val |= FIELD_PREP(MXL86110_EXT_RGMII_CFG1_RX_DELAY_MASK,
				  rx_delay);
	}

	tx_delay = mxl86110_get_internal_delay(phydev, false);
	if (tx_delay >= 0) {
		val &= ~(MXL86110_EXT_RGMII_CFG1_TX_1G_DELAY_MASK |
			 MXL86110_EXT_RGMII_CFG1_TX_10MB_100MB_DELAY_MASK);
		val |= FIELD_PREP(MXL86110_EXT_RGMII_CFG1_TX_1G_DELAY_MASK,
				  tx_delay) |
		       FIELD_PREP(MXL86110_EXT_RGMII_CFG1_TX_10MB_100MB_DELAY_MASK,
				  tx_delay);
	}

	ret = mxl86110_modify_extended_reg(phydev, MXL86110_EXT_RGMII_CFG1_REG,
					   MXL86110_EXT_RGMII_CFG1_FULL_MASK,
					   val);
	if (ret < 0)
		return ret;

	/*
	 * RXDLY_ENABLE (CHIP_CFG) adds a fixed ~2 ns to RX_CLK on top of the
	 * digital delay above. When rx-internal-delay-ps is set we drive RX
	 * delay digitally, so clear it; otherwise leave it as strapped.
	 */
	if (rx_delay >= 0)
		return mxl86110_modify_extended_reg(phydev,
						    MXL86110_EXT_CHIP_CFG_REG,
						    MXL86110_EXT_CHIP_CFG_RXDLY_ENABLE,
						    0);

	return 0;
}

static int mxl86110_config(struct phy_device *phydev)
{
	int ret;

	ret = mxl86110_synce_clk_cfg(phydev);
	if (ret < 0)
		return ret;

	if (phy_interface_is_rgmii(phydev)) {
		ret = mxl86110_config_rgmii_delay(phydev);
		if (ret < 0)
			return ret;
	}

	ret = mxl86110_enable_led_activity_blink(phydev);
	if (ret < 0)
		return ret;

	ret = mxl86110_broadcast_cfg(phydev);
	if (ret < 0)
		return ret;

	ret = phy_reset(phydev);
	if (ret < 0)
		return ret;

	return genphy_config_aneg(phydev);
}

static int mxl86110_parse_status(struct phy_device *phydev)
{
	int val;
	u16 speed;

	val = phy_read(phydev, MDIO_DEVAD_NONE, MXL86110_PHY_STAT_REG);
	if (val < 0)
		return val;

	if (val & MXL86110_PHY_STAT_LINK)
		phydev->link = 1;
	else
		phydev->link = 0;

	if (val & MXL86110_PHY_STAT_DPX)
		phydev->duplex = DUPLEX_FULL;
	else
		phydev->duplex = DUPLEX_HALF;

	speed = FIELD_GET(MXL86110_PHY_STAT_SPEED_MASK, val);
	switch (speed) {
	case MXL86110_PHY_STAT_SPEED_1000M:
		phydev->speed = SPEED_1000;
		break;
	case MXL86110_PHY_STAT_SPEED_100M:
		phydev->speed = SPEED_100;
		break;
	default:
		phydev->speed = SPEED_10;
		break;
	}

	return 0;
}

static int mxl86110_startup(struct phy_device *phydev)
{
	int ret;

	ret = genphy_update_link(phydev);
	if (ret)
		return ret;

	return mxl86110_parse_status(phydev);
}

U_BOOT_PHY_DRIVER(mxl86110) = {
	.name = "MXL86110 Gigabit Ethernet",
	.uid = PHY_ID_MXL86110,
	.mask = PHY_ID_MXL_MASK,
	.features = PHY_GBIT_FEATURES,
	.config = &mxl86110_config,
	.startup = &mxl86110_startup,
	.shutdown = &genphy_shutdown,
};

U_BOOT_PHY_DRIVER(mxl86111) = {
	.name = "MXL86111 Gigabit Ethernet",
	.uid = PHY_ID_MXL86111,
	.mask = PHY_ID_MXL_MASK,
	.features = PHY_GBIT_FEATURES,
	.config = &mxl86110_config,
	.startup = &mxl86110_startup,
	.shutdown = &genphy_shutdown,
};
