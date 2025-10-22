// SPDX-License-Identifier: GPL-2.0+

#include <asm/io.h>
#include <dm.h>
#include <linux/bitfield.h>
#include <linux/iopoll.h>
#include <miiphy.h>

#define MT7531_PHY_IAC			0x701c
#define   MT7531_PHY_ACS_ST		BIT(31)
#define   MT7531_MDIO_REG_ADDR_CL22	GENMASK(29, 25)
#define   MT7531_MDIO_DEV_ADDR		MT7531_MDIO_REG_ADDR_CL22
#define   MT7531_MDIO_PHY_ADDR		GENMASK(24, 20)
#define   MT7531_MDIO_CMD		GENMASK(19, 18)
#define   MT7531_MDIO_CMD_READ_CL45	FIELD_PREP_CONST(MT7531_MDIO_CMD, 0x3)
#define   MT7531_MDIO_CMD_READ_CL22	FIELD_PREP_CONST(MT7531_MDIO_CMD, 0x2)
#define   MT7531_MDIO_CMD_WRITE		FIELD_PREP_CONST(MT7531_MDIO_CMD, 0x1)
#define   MT7531_MDIO_CMD_ADDR		FIELD_PREP_CONST(MT7531_MDIO_CMD, 0x0)
#define   MT7531_MDIO_ST		GENMASK(17, 16)
#define   MT7531_MDIO_ST_CL22		FIELD_PREP_CONST(MT7531_MDIO_ST, 0x1)
#define   MT7531_MDIO_ST_CL45		FIELD_PREP_CONST(MT7531_MDIO_ST, 0x0)
#define   MT7531_MDIO_RW_DATA		GENMASK(15, 0)
#define   MT7531_MDIO_REG_ADDR_CL45	MT7531_MDIO_RW_DATA

#define MT7531_MDIO_TIMEOUT		100000
#define MT7531_MDIO_SLEEP		20

struct mt7531_mdio_priv {
	phys_addr_t switch_regs;
};

static int mt7531_mdio_wait_busy(struct mt7531_mdio_priv *priv)
{
	unsigned int busy;

	return readl_poll_sleep_timeout(priv->switch_regs + MT7531_PHY_IAC,
					busy, (busy & MT7531_PHY_ACS_ST) == 0,
					MT7531_MDIO_SLEEP, MT7531_MDIO_TIMEOUT);
}

static int mt7531_mdio_read(struct mt7531_mdio_priv *priv, int addr, int devad, int reg)
{
	u32 val;

	if (devad != MDIO_DEVAD_NONE) {
		if (mt7531_mdio_wait_busy(priv))
			return -ETIMEDOUT;

		val = MT7531_PHY_ACS_ST |
		      MT7531_MDIO_ST_CL45 | MT7531_MDIO_CMD_ADDR |
		      FIELD_PREP(MT7531_MDIO_PHY_ADDR, addr) |
		      FIELD_PREP(MT7531_MDIO_DEV_ADDR, devad) |
		      FIELD_PREP(MT7531_MDIO_REG_ADDR_CL45, reg);

		writel(val, priv->switch_regs + MT7531_PHY_IAC);
	}

	if (mt7531_mdio_wait_busy(priv))
		return -ETIMEDOUT;

	val = MT7531_PHY_ACS_ST | FIELD_PREP(MT7531_MDIO_PHY_ADDR, addr);
	if (devad != MDIO_DEVAD_NONE)
		val |= MT7531_MDIO_ST_CL45 | MT7531_MDIO_CMD_READ_CL45 |
		       FIELD_PREP(MT7531_MDIO_DEV_ADDR, devad);
	else
		val |= MT7531_MDIO_ST_CL22 | MT7531_MDIO_CMD_READ_CL22 |
		       FIELD_PREP(MT7531_MDIO_REG_ADDR_CL22, reg);

	writel(val, priv->switch_regs + MT7531_PHY_IAC);

	if (mt7531_mdio_wait_busy(priv))
		return -ETIMEDOUT;

	val = readl(priv->switch_regs + MT7531_PHY_IAC);
	return val & MT7531_MDIO_RW_DATA;
}

static int mt7531_mdio_write(struct mt7531_mdio_priv *priv, int addr, int devad,
			     int reg, u16 value)
{
	u32 val;

	if (devad != MDIO_DEVAD_NONE) {
		if (mt7531_mdio_wait_busy(priv))
			return -ETIMEDOUT;

		val = MT7531_PHY_ACS_ST |
		      MT7531_MDIO_ST_CL45 | MT7531_MDIO_CMD_ADDR |
		      FIELD_PREP(MT7531_MDIO_PHY_ADDR, addr) |
		      FIELD_PREP(MT7531_MDIO_DEV_ADDR, devad) |
		      FIELD_PREP(MT7531_MDIO_REG_ADDR_CL45, reg);

		writel(val, priv->switch_regs + MT7531_PHY_IAC);
	}

	if (mt7531_mdio_wait_busy(priv))
		return -ETIMEDOUT;

	val = MT7531_PHY_ACS_ST | FIELD_PREP(MT7531_MDIO_PHY_ADDR, addr) |
	      MT7531_MDIO_CMD_WRITE | FIELD_PREP(MT7531_MDIO_RW_DATA, value);
	if (devad != MDIO_DEVAD_NONE)
		val |= MT7531_MDIO_ST_CL45 |
		       FIELD_PREP(MT7531_MDIO_DEV_ADDR, devad);
	else
		val |= MT7531_MDIO_ST_CL22 |
		       FIELD_PREP(MT7531_MDIO_REG_ADDR_CL22, reg);

	writel(val, priv->switch_regs + MT7531_PHY_IAC);

	if (mt7531_mdio_wait_busy(priv))
		return -ETIMEDOUT;

	return 0;
}

int mt7531_mdio_mmio_read(struct mii_dev *bus, int addr, int devad, int reg)
{
	struct mt7531_mdio_priv *priv = bus->priv;

	return mt7531_mdio_read(priv, addr, devad, reg);
}

int mt7531_mdio_mmio_write(struct mii_dev *bus, int addr, int devad,
			   int reg, u16 value)
{
	struct mt7531_mdio_priv *priv = bus->priv;

	return mt7531_mdio_write(priv, addr, devad, reg, value);
}

static int dm_mt7531_mdio_read(struct udevice *dev, int addr, int devad, int reg)
{
	struct mt7531_mdio_priv *priv = dev_get_priv(dev);

	return mt7531_mdio_read(priv, addr, devad, reg);
}

static int dm_mt7531_mdio_write(struct udevice *dev, int addr, int devad,
				int reg, u16 value)
{
	struct mt7531_mdio_priv *priv = dev_get_priv(dev);

	return mt7531_mdio_write(priv, addr, devad, reg, value);
}

static const struct mdio_ops mt7531_mdio_ops = {
	.read = dm_mt7531_mdio_read,
	.write = dm_mt7531_mdio_write,
};

static int mt7531_mdio_probe(struct udevice *dev)
{
	struct mt7531_mdio_priv *priv = dev_get_priv(dev);

	priv->switch_regs = dev_read_addr(dev);
	if (priv->switch_regs == FDT_ADDR_T_NONE)
		return -EINVAL;

	return 0;
}

U_BOOT_DRIVER(mt7531_mdio) = {
	.name           = "mt7531-mdio-mmio",
	.id             = UCLASS_MDIO,
	.probe          = mt7531_mdio_probe,
	.ops            = &mt7531_mdio_ops,
	.priv_auto	  = sizeof(struct mt7531_mdio_priv),
};
