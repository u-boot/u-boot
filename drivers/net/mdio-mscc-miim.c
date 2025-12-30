// SPDX-License-Identifier: GPL-2.0+

#include <asm/io.h>
#include <dm.h>
#include <time.h>
#include <regmap.h>
#include <miiphy.h>
#include <linux/bitfield.h>

#define MSCC_MIIM_REG_STATUS		0x0
#define		MSCC_MIIM_STATUS_STAT_PENDING	BIT(2)
#define		MSCC_MIIM_STATUS_STAT_BUSY	BIT(3)
#define MSCC_MIIM_REG_CMD		0x8
#define		MSCC_MIIM_CMD_OPR_WRITE		BIT(1)
#define		MSCC_MIIM_CMD_OPR_READ		BIT(2)
#define		MSCC_MIIM_CMD_WRDATA_SHIFT	4
#define		MSCC_MIIM_CMD_REGAD_SHIFT	20
#define		MSCC_MIIM_CMD_PHYAD_SHIFT	25
#define		MSCC_MIIM_CMD_VLD		BIT(31)
#define MSCC_MIIM_REG_DATA		0xC
#define		MSCC_MIIM_DATA_ERROR		(BIT(16) | BIT(17))
#define		MSCC_MIIM_DATA_MASK		GENMASK(15, 0)
#define MSCC_MIIM_REG_CFG		0x10
#define		MSCC_MIIM_CFG_PRESCALE_MASK	GENMASK(7, 0)
/* 01 = Clause 22, 00 = Clause 45 */
#define		MSCC_MIIM_CFG_ST_CFG_MASK	GENMASK(10, 9)
#define		MSCC_MIIM_C22			1
#define		MSCC_MIIM_C45			0

#define MSCC_MDIO_TIMEOUT    10000
#define MSCC_MDIO_SLEEP      50

struct mscc_mdio_priv {
	struct regmap *map;
};

static int mscc_mdio_wait_busy(struct mscc_mdio_priv *priv)
{
	u32 busy;

	return regmap_read_poll_timeout(priv->map, MSCC_MIIM_REG_STATUS, busy,
				       (busy & MSCC_MIIM_STATUS_STAT_BUSY) == 0,
				       MSCC_MDIO_SLEEP,
				       MSCC_MDIO_TIMEOUT);
}

static int mscc_mdio_read(struct udevice *dev, int addr, int devad, int reg)
{
	struct mscc_mdio_priv *priv = dev_get_priv(dev);
	u32 val;
	int ret;

	if (mscc_mdio_wait_busy(priv))
		return -ETIMEDOUT;

	ret = regmap_write(priv->map, MSCC_MIIM_REG_CMD,
			   MSCC_MIIM_CMD_VLD |
			   (addr << MSCC_MIIM_CMD_PHYAD_SHIFT) |
			   (reg << MSCC_MIIM_CMD_REGAD_SHIFT) |
			   MSCC_MIIM_CMD_OPR_READ);
	if (ret)
		return ret;

	if (mscc_mdio_wait_busy(priv))
		return -ETIMEDOUT;

	regmap_read(priv->map, MSCC_MIIM_REG_DATA, &val);
	if (val & MSCC_MIIM_DATA_ERROR)
		return -EIO;

	return FIELD_GET(MSCC_MIIM_DATA_MASK, val);
}

int mscc_mdio_write(struct udevice *dev, int addr, int devad, int reg, u16 val)
{
	struct mscc_mdio_priv *priv = dev_get_priv(dev);
	int ret;

	if (mscc_mdio_wait_busy(priv))
		return -ETIMEDOUT;

	ret = regmap_write(priv->map, MSCC_MIIM_REG_CMD,
			   MSCC_MIIM_CMD_VLD |
			   (addr << MSCC_MIIM_CMD_PHYAD_SHIFT) |
			   (reg << MSCC_MIIM_CMD_REGAD_SHIFT) |
			   (val << MSCC_MIIM_CMD_WRDATA_SHIFT) |
			   MSCC_MIIM_CMD_OPR_WRITE);

	return ret;
}

static const struct mdio_ops mscc_mdio_ops = {
	.read = mscc_mdio_read,
	.write = mscc_mdio_write,
};

static int mscc_mdio_bind(struct udevice *dev)
{
	if (ofnode_valid(dev_ofnode(dev)))
		device_set_name(dev, ofnode_get_name(dev_ofnode(dev)));

	return 0;
}

static int mscc_mdio_probe(struct udevice *dev)
{
	struct mscc_mdio_priv *priv = dev_get_priv(dev);
	int ret;

	ret = regmap_init_mem(dev_ofnode(dev), &priv->map);
	if (ret)
		return -EINVAL;

	/* Enter Clause 22 mode */
	ret = regmap_update_bits(priv->map, MSCC_MIIM_REG_CFG,
				 MSCC_MIIM_CFG_ST_CFG_MASK,
				 FIELD_PREP(MSCC_MIIM_CFG_ST_CFG_MASK,
					    MSCC_MIIM_C22));

	return ret;
}

static const struct udevice_id mscc_mdio_ids[] = {
	{ .compatible = "mscc,ocelot-miim", },
	{ }
};

U_BOOT_DRIVER(mscc_mdio) = {
	.name           = "mscc_mdio",
	.id             = UCLASS_MDIO,
	.of_match       = mscc_mdio_ids,
	.bind           = mscc_mdio_bind,
	.probe          = mscc_mdio_probe,
	.ops            = &mscc_mdio_ops,
	.priv_auto	  = sizeof(struct mscc_mdio_priv),
};
