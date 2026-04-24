// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2021-26 Axiado Corporation (or its affiliates).
 */

#include <dm.h>
#include <generic-phy.h>
#include <regmap.h>
#include <time.h>
#include <dm/device.h>
#include <dm/device_compat.h>

/* Axiado eMMC PHY configuration registers */
#define CAP_REG_IN_S1_LSB		0x00
#define CAP_REG_IN_S1_MSB		0x04
#define PHY_CTRL_1			0x38
#define PHY_CTRL_2			0x3C
#define PHY_CTRL_3			0x40
#define STATUS				0x50

/* PHY Control 1 Register bits */
#define DLL_ENBL			BIT(26)
#define RTRIM_EN			BIT(21)
#define PDB_ENBL			BIT(23)
#define RETB_ENBL			BIT(1)

/* Receiver Enable bits */
#define REN_STRB			BIT(27)
#define REN_CMD				BIT(12)
#define REN_DAT0			BIT(13)
#define REN_DAT1			BIT(14)
#define REN_DAT2			BIT(15)
#define REN_DAT3			BIT(16)
#define REN_DAT4			BIT(17)
#define REN_DAT5			BIT(18)
#define REN_DAT6			BIT(19)
#define REN_DAT7			BIT(20)
#define REN_CMD_EN	(REN_CMD | REN_DAT0 | REN_DAT1 | REN_DAT2 | \
			 REN_DAT3 | REN_DAT4 | REN_DAT5 | \
			 REN_DAT6 | REN_DAT7)

/* Pull-Up Enable bits */
#define PU_CMD				BIT(3)
#define PU_DAT0				BIT(4)
#define PU_DAT1				BIT(5)
#define PU_DAT2				BIT(6)
#define PU_DAT3				BIT(7)
#define PU_DAT4				BIT(8)
#define PU_DAT5				BIT(9)
#define PU_DAT6				BIT(10)
#define PU_DAT7				BIT(11)
#define PU_CMD_EN	(PU_CMD | PU_DAT0 | PU_DAT1 | PU_DAT2 | \
			 PU_DAT3 | PU_DAT4 | PU_DAT5 | \
			 PU_DAT6 | PU_DAT7)

/* PHY Control 2 Register bits */
#define OTAPDLY_EN			BIT(11)

/* PHY Control 3 Register bits */
#define SEL_DLY_RXCLK			BIT(18)
#define SEL_DLY_TXCLK			BIT(19)

/* Status Register bits */
#define CALDONE_MASK			0x40
#define DLL_RDY_MASK			0x1

/* Clock buffer bits */
#define MAX_CLK_BUF0			BIT(20)
#define MAX_CLK_BUF1			BIT(21)
#define MAX_CLK_BUF2			BIT(22)

/* Other constants */
#define CLK_MULTIPLIER			0xC008E
#define LOOP_TIMEOUT			3000
#define TIMEOUT_DELAY			100

struct axiado_emmc_phy {
	struct regmap *regmap;
};

static int axiado_emmc_phy_init(struct phy *phy)
{
	u32 val;
	unsigned long timeout;
	struct axiado_emmc_phy *ax_phy = dev_get_priv(phy->dev);

	regmap_read(ax_phy->regmap, PHY_CTRL_1, &val);
	regmap_update_bits(ax_phy->regmap, PHY_CTRL_1,
			   RETB_ENBL | RTRIM_EN,
			   RETB_ENBL | RTRIM_EN);

	regmap_read(ax_phy->regmap, PHY_CTRL_3, &val);
	regmap_update_bits(ax_phy->regmap, PHY_CTRL_3,
			   PDB_ENBL, PDB_ENBL);

	/* Wait max 3000 ms */
	timeout = get_timer(0);

	while (1) {
		regmap_read(ax_phy->regmap, STATUS, &val);
		if (val & CALDONE_MASK)
			break;

		if (get_timer(timeout) > LOOP_TIMEOUT) {
			dev_err(phy->dev, "eMMC-PHY: CALDONE_MASK bit not cleared.\n");
			return -ETIMEDOUT;
		}
		udelay(TIMEOUT_DELAY);
	}

	regmap_update_bits(ax_phy->regmap, PHY_CTRL_1,
			   REN_CMD_EN | PU_CMD_EN,
			   REN_CMD_EN | PU_CMD_EN);

	regmap_update_bits(ax_phy->regmap, PHY_CTRL_2,
			   REN_STRB, REN_STRB);

	regmap_update_bits(ax_phy->regmap, PHY_CTRL_3,
			   MAX_CLK_BUF0 | MAX_CLK_BUF1 | MAX_CLK_BUF2,
			   MAX_CLK_BUF0 | MAX_CLK_BUF1 | MAX_CLK_BUF2);

	regmap_write(ax_phy->regmap, CAP_REG_IN_S1_MSB, CLK_MULTIPLIER);

	regmap_update_bits(ax_phy->regmap, PHY_CTRL_3,
			   SEL_DLY_RXCLK | SEL_DLY_TXCLK,
			   SEL_DLY_RXCLK | SEL_DLY_TXCLK);

	return 0;
}

static int axiado_emmc_phy_power_on(struct phy *phy)
{
	struct axiado_emmc_phy *ax_phy = dev_get_priv(phy->dev);
	u32 val;
	unsigned long timeout;

	regmap_update_bits(ax_phy->regmap, PHY_CTRL_1,
			   RETB_ENBL, RETB_ENBL);

	regmap_update_bits(ax_phy->regmap, PHY_CTRL_3,
			   PDB_ENBL, PDB_ENBL);

	regmap_update_bits(ax_phy->regmap, PHY_CTRL_2,
			   OTAPDLY_EN | (0xF << 7),
			   OTAPDLY_EN | ((0x2) << 7));

	/* Read back to ensure write is completed */
	regmap_read(ax_phy->regmap, PHY_CTRL_2, &val);

	regmap_update_bits(ax_phy->regmap, PHY_CTRL_1,
			   DLL_ENBL | (0xF << 22),
			   DLL_ENBL | ((0x8) << 22));

	regmap_write(ax_phy->regmap, STATUS, 0x00);

	regmap_update_bits(ax_phy->regmap, PHY_CTRL_3,
			   (0x1 << 25), (0x1 << 25));

	/* Wait max 3000 ms */
	timeout = get_timer(0);

	while (1) {
		regmap_read(ax_phy->regmap, STATUS, &val);
		if (val & DLL_RDY_MASK)
			break;

		if (get_timer(timeout) > LOOP_TIMEOUT) {
			dev_err(phy->dev, "eMMC-PHY: DLL_RDY_MASK bit not cleared.\n");
			return -ETIMEDOUT;
		}
		udelay(TIMEOUT_DELAY);
	}
	return 0;
}

static int axiado_emmc_phy_power_off(struct phy *phy)
{
	struct axiado_emmc_phy *ax_phy = dev_get_priv(phy->dev);

	/* Disable DLL */
	regmap_update_bits(ax_phy->regmap, PHY_CTRL_1,
			   DLL_ENBL, 0);

	return 0;
}

static int axiado_emmc_phy_exit(struct phy *phy)
{
	struct axiado_emmc_phy *ax_phy = dev_get_priv(phy->dev);

	/* Disable all PHY features */
	regmap_update_bits(ax_phy->regmap, PHY_CTRL_1,
			   DLL_ENBL | RETB_ENBL | RTRIM_EN, 0);
	regmap_update_bits(ax_phy->regmap, PHY_CTRL_3,
			   PDB_ENBL, 0);

	return 0;
}

static const struct phy_ops axiado_emmc_phy_ops = {
	.init = axiado_emmc_phy_init,
	.power_on = axiado_emmc_phy_power_on,
	.power_off = axiado_emmc_phy_power_off,
	.exit = axiado_emmc_phy_exit,
};

static int axiado_emmc_phy_probe(struct udevice *dev)
{
	struct axiado_emmc_phy *ax_phy = dev_get_priv(dev);
	int ret;

	ret = regmap_init_mem(dev_ofnode(dev), &ax_phy->regmap);
	if (ret) {
		dev_err(dev, "Failed to init regmap: %d\n", ret);
		return ret;
	}

	return 0;
}

static const struct udevice_id axiado_emmc_phy_ids[] = {
	{ .compatible = "axiado,ax3000-emmc-phy" },
	{ }
};

U_BOOT_DRIVER(axiado_emmc_phy) = {
	.name = "axiado_emmc_phy",
	.id = UCLASS_PHY,
	.of_match = axiado_emmc_phy_ids,
	.ops = &axiado_emmc_phy_ops,
	.probe = axiado_emmc_phy_probe,
	.priv_auto = sizeof(struct axiado_emmc_phy),
};
