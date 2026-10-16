// SPDX-License-Identifier: (GPL-2.0+ OR MIT)
/*
 * Copyright (c) 2026 Microchip
 */

#include <clk.h>
#include <dm/device_compat.h>
#include <linux/bitops.h>
#include <linux/bitfield.h>
#include <linux/kernel.h>
#include <miiphy.h>
#include <wait_bit.h>

#define P64H_MDIO_REG_PRESCALER 0x20
#define P64H_MDIO_CFG_PRESCALE_MASK GENMASK(7, 0)

#define P64H_MDIO_REG_FRAME_CFG_1 0x24
#define P64H_MDIO_WDATA_MASK GENMASK(15, 0)

#define P64H_MDIO_REG_FRAME_CFG_2 0x28
#define P64H_MDIO_TRIGGER_BIT BIT(31)
#define P64H_MDIO_REG_DEV_ADDR_MASK GENMASK(20, 16)
#define P64H_MDIO_PHY_PRT_ADDR_MASK GENMASK(8, 4)
#define P64H_MDIO_OPERATION_MASK GENMASK(3, 2)
#define P64H_MDIO_START_OF_FRAME_MASK GENMASK(1, 0)

#define P64H_MDIO_OPERATION_WRITE BIT(0)
#define P64H_MDIO_OPERATION_READ BIT(1)

#define P64H_MDIO_REG_FRAME_STATUS 0x2C
#define P64H_MDIO_READOK_BIT BIT(24)
#define P64H_MDIO_RDATA_MASK GENMASK(15, 0)

struct pic64hpsc_mdio_priv {
	void *regs_base;
};

static int pic64hpsc_mdio_wait_trigger(struct udevice *dev)
{
	struct pic64hpsc_mdio_priv *priv = dev_get_priv(dev);
	int ret;

	ret = wait_for_bit_le32(priv->regs_base + P64H_MDIO_REG_FRAME_CFG_2,
				P64H_MDIO_TRIGGER_BIT, false, 250, false);

	if (ret < 0) {
		dev_err(dev, "TRIGGER bit timeout: %x\n",
			in_le32(priv->regs_base + P64H_MDIO_REG_FRAME_CFG_2));
	}

	return ret;
}

static int pic64hpsc_mdio_read(struct udevice *dev, int addr, int devad,
			       int reg)
{
	struct pic64hpsc_mdio_priv *priv = dev_get_priv(dev);
	u32 val;
	int ret;

	if (devad != MDIO_DEVAD_NONE)
		return -EOPNOTSUPP;

	ret = pic64hpsc_mdio_wait_trigger(dev);
	if (ret)
		return ret;

	out_le32(priv->regs_base + P64H_MDIO_REG_FRAME_CFG_2,
		 P64H_MDIO_TRIGGER_BIT |
			 FIELD_PREP(P64H_MDIO_REG_DEV_ADDR_MASK, reg) |
			 FIELD_PREP(P64H_MDIO_PHY_PRT_ADDR_MASK, addr) |
			 FIELD_PREP(P64H_MDIO_OPERATION_MASK,
				    P64H_MDIO_OPERATION_READ) |
			 FIELD_PREP(P64H_MDIO_START_OF_FRAME_MASK, 1));

	ret = pic64hpsc_mdio_wait_trigger(dev);
	if (ret)
		return ret;

	val = in_le32(priv->regs_base + P64H_MDIO_REG_FRAME_STATUS);

	if (!FIELD_GET(P64H_MDIO_READOK_BIT, val))
		return -ENODEV;

	return FIELD_GET(P64H_MDIO_RDATA_MASK, val);
}

static int pic64hpsc_mdio_write(struct udevice *dev, int addr, int devad,
				int reg, u16 val)
{
	struct pic64hpsc_mdio_priv *priv = dev_get_priv(dev);
	int ret;

	if (devad != MDIO_DEVAD_NONE)
		return -EOPNOTSUPP;

	ret = pic64hpsc_mdio_wait_trigger(dev);
	if (ret < 0)
		return ret;

	out_le32(priv->regs_base + P64H_MDIO_REG_FRAME_CFG_1,
		 FIELD_PREP(P64H_MDIO_WDATA_MASK, val));

	out_le32(priv->regs_base + P64H_MDIO_REG_FRAME_CFG_2,
		 P64H_MDIO_TRIGGER_BIT |
			 FIELD_PREP(P64H_MDIO_REG_DEV_ADDR_MASK, reg) |
			 FIELD_PREP(P64H_MDIO_PHY_PRT_ADDR_MASK, addr) |
			 FIELD_PREP(P64H_MDIO_OPERATION_MASK,
				    P64H_MDIO_OPERATION_WRITE) |
			 FIELD_PREP(P64H_MDIO_START_OF_FRAME_MASK, 1));

	return 0;
}

static const struct mdio_ops pic64hpsc_mdio_ops = {
	.read = pic64hpsc_mdio_read,
	.write = pic64hpsc_mdio_write,
};

static int pic64hpsc_mdio_probe(struct udevice *dev)
{
	struct pic64hpsc_mdio_priv *priv = dev_get_priv(dev);
	u32 div, bus_freq;
	ulong clk_rate;
	struct clk clk;
	int ret;

	priv->regs_base = dev_read_addr_ptr(dev);
	if (!priv->regs_base)
		return -ENODEV;

	ret = clk_get_by_index(dev, 0, &clk);
	if (ret) {
		dev_err(dev, "Missing clock\n");
		return ret;
	}

	ret = clk_enable(&clk);
	if (ret) {
		dev_err(dev, "Cannot enable clock\n");
		return ret;
	}

	clk_rate = clk_get_rate(&clk);
	if (!clk_rate) {
		dev_err(dev, "Cannot get clock rate\n");
		return -EINVAL;
	}

	bus_freq = ofnode_read_u32_default(dev_ofnode(dev), "clock-frequency",
					   2500000);

	div = DIV_ROUND_UP(clk_rate, 2 * bus_freq) - 1;
	if (div == 0 || div & ~P64H_MDIO_CFG_PRESCALE_MASK) {
		dev_err(dev, "MDIO clock-frequency out of range\n");
		return -EINVAL;
	}

	dev_dbg(dev, "rate=%lu bus_freq=%u real_bus_freq=%lu div=%u\n",
		clk_rate, bus_freq, clk_rate / (2 * (1 + div)), div);
	out_le32(priv->regs_base + P64H_MDIO_REG_PRESCALER, div);

	return 0;
}

static const struct udevice_id pic64hpsc_mdio_of_ids[] = {
	{ .compatible = "microchip,pic64hpsc-mdio" },
	{ /* sentinel */ }
};

U_BOOT_DRIVER(pic64hpsc_mdio) = {
	.name = "pic64hpsc_mdio",
	.id = UCLASS_MDIO,
	.of_match = pic64hpsc_mdio_of_ids,
	.probe = pic64hpsc_mdio_probe,
	.ops = &pic64hpsc_mdio_ops,
	.priv_auto = sizeof(struct pic64hpsc_mdio_priv),
};
