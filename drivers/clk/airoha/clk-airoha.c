// SPDX-License-Identifier: GPL-2.0-only
/*
 * Based on the Linux clk-en7523.c but majorly reworked
 * for U-Boot that doesn't require CCF subsystem.
 *
 * Major modification, support for set_rate, realtime
 * get_rate and split for reset part to a different driver.
 *
 * Author: Lorenzo Bianconi <lorenzo@kernel.org> (original driver)
 *	   Christian Marangi <ansuelsmth@gmail.com>
 */

#include <clk-uclass.h>
#include <dm.h>
#include <dm/devres.h>
#include <dm/device_compat.h>
#include <dm/lists.h>
#include <regmap.h>
#include <syscon.h>

#include <dt-bindings/clock/en7523-clk.h>

#define REG_GSW_CLK_DIV_SEL		0x1b4
#define REG_EMI_CLK_DIV_SEL		0x1b8
#define REG_BUS_CLK_DIV_SEL		0x1bc
#define REG_SPI_CLK_DIV_SEL		0x1c4
#define REG_SPI_CLK_FREQ_SEL		0x1c8
#define REG_NPU_CLK_DIV_SEL		0x1fc

#define REG_NP_SCU_PCIC			0x88
#define REG_NP_SCU_SSTR			0x9c
#define REG_PCIE_XSI0_SEL_MASK		GENMASK(14, 13)
#define REG_PCIE_XSI1_SEL_MASK		GENMASK(12, 11)
#define REG_CRYPTO_CLKSRC2		0x20c

#define EN7581_MAX_CLKS			9

struct airoha_clk_desc {
	int id;
	const char *name;
	u32 base_reg;
	u8 base_bits;
	u8 base_shift;
	union {
		const unsigned int *base_values;
		unsigned int base_value;
	};
	size_t n_base_values;

	u16 div_reg;
	u8 div_bits;
	u8 div_shift;
	u16 div_val0;
	u8 div_step;
	u8 div_offset;
};

struct airoha_clk_priv {
	struct regmap *chip_scu_map;
	struct airoha_clk_soc_data *data;
};

struct airoha_clk_soc_data {
	u32 num_clocks;
	const struct airoha_clk_desc *descs;
};

static const u32 gsw_base[] = { 400000000, 500000000 };
static const u32 slic_base[] = { 100000000, 3125000 };

static const u32 emi7581_base[] = { 540000000, 480000000, 400000000, 300000000 };
static const u32 bus7581_base[] = { 600000000, 540000000 };
static const u32 npu7581_base[] = { 800000000, 750000000, 720000000, 600000000 };
static const u32 crypto_base[] = { 540000000, 480000000 };
static const u32 emmc7581_base[] = { 200000000, 150000000 };

static const struct airoha_clk_desc en7581_base_clks[EN7581_MAX_CLKS] = {
	[EN7523_CLK_GSW] = {
		.id = EN7523_CLK_GSW,
		.name = "gsw",

		.base_reg = REG_GSW_CLK_DIV_SEL,
		.base_bits = 1,
		.base_shift = 8,
		.base_values = gsw_base,
		.n_base_values = ARRAY_SIZE(gsw_base),

		.div_bits = 3,
		.div_shift = 0,
		.div_step = 1,
		.div_offset = 1,
	},
	[EN7523_CLK_EMI] = {
		.id = EN7523_CLK_EMI,
		.name = "emi",

		.base_reg = REG_EMI_CLK_DIV_SEL,
		.base_bits = 2,
		.base_shift = 8,
		.base_values = emi7581_base,
		.n_base_values = ARRAY_SIZE(emi7581_base),

		.div_bits = 3,
		.div_shift = 0,
		.div_step = 1,
		.div_offset = 1,
	},
	[EN7523_CLK_BUS] = {
		.id = EN7523_CLK_BUS,
		.name = "bus",

		.base_reg = REG_BUS_CLK_DIV_SEL,
		.base_bits = 1,
		.base_shift = 8,
		.base_values = bus7581_base,
		.n_base_values = ARRAY_SIZE(bus7581_base),

		.div_bits = 3,
		.div_shift = 0,
		.div_step = 1,
		.div_offset = 1,
	},
	[EN7523_CLK_SLIC] = {
		.id = EN7523_CLK_SLIC,
		.name = "slic",

		.base_reg = REG_SPI_CLK_FREQ_SEL,
		.base_bits = 1,
		.base_shift = 0,
		.base_values = slic_base,
		.n_base_values = ARRAY_SIZE(slic_base),

		.div_reg = REG_SPI_CLK_DIV_SEL,
		.div_bits = 5,
		.div_shift = 24,
		.div_val0 = 20,
		.div_step = 2,
	},
	[EN7523_CLK_SPI] = {
		.id = EN7523_CLK_SPI,
		.name = "spi",

		.base_reg = REG_SPI_CLK_DIV_SEL,

		.base_value = 400000000,

		.div_bits = 5,
		.div_shift = 8,
		.div_val0 = 40,
		.div_step = 2,
	},
	[EN7523_CLK_NPU] = {
		.id = EN7523_CLK_NPU,
		.name = "npu",

		.base_reg = REG_NPU_CLK_DIV_SEL,
		.base_bits = 2,
		.base_shift = 8,
		.base_values = npu7581_base,
		.n_base_values = ARRAY_SIZE(npu7581_base),

		.div_bits = 3,
		.div_shift = 0,
		.div_step = 1,
		.div_offset = 1,
	},
	[EN7523_CLK_CRYPTO] = {
		.id = EN7523_CLK_CRYPTO,
		.name = "crypto",

		.base_reg = REG_CRYPTO_CLKSRC2,
		.base_bits = 1,
		.base_shift = 0,
		.base_values = crypto_base,
		.n_base_values = ARRAY_SIZE(crypto_base),
	},
	[EN7581_CLK_EMMC] = {
		.id = EN7581_CLK_EMMC,
		.name = "emmc",

		.base_reg = REG_CRYPTO_CLKSRC2,
		.base_bits = 1,
		.base_shift = 12,
		.base_values = emmc7581_base,
		.n_base_values = ARRAY_SIZE(emmc7581_base),
	}
};

static u32 airoha_clk_get_base_rate(const struct airoha_clk_desc *desc, u32 val)
{
	if (!desc->base_bits)
		return desc->base_value;

	val >>= desc->base_shift;
	val &= (1 << desc->base_bits) - 1;

	if (val >= desc->n_base_values)
		return 0;

	return desc->base_values[val];
}

static u32 airoha_clk_get_div(const struct airoha_clk_desc *desc, u32 val)
{
	if (!desc->div_bits)
		return 1;

	val >>= desc->div_shift;
	val &= (1 << desc->div_bits) - 1;

	if (!val && desc->div_val0)
		return desc->div_val0;

	return (val + desc->div_offset) * desc->div_step;
}

static int airoha_clk_enable(struct clk *clk)
{
	struct airoha_clk_priv *priv = dev_get_priv(clk->dev);
	struct airoha_clk_soc_data *data = priv->data;
	int id = clk->id;

	if (id > data->num_clocks)
		return -EINVAL;

	return 0;
}

static int airoha_clk_disable(struct clk *clk)
{
	return 0;
}

static ulong airoha_clk_get_rate(struct clk *clk)
{
	struct airoha_clk_priv *priv = dev_get_priv(clk->dev);
	struct airoha_clk_soc_data *data = priv->data;
	const struct airoha_clk_desc *desc;
	struct regmap *map = priv->chip_scu_map;
	int id = clk->id;
	u32 reg, val;
	ulong rate;
	int ret;

	if (id > data->num_clocks) {
		dev_err(clk->dev, "Invalid clk ID %d\n", id);
		return 0;
	}

	desc = &data->descs[id];

	ret = regmap_read(map, desc->base_reg, &val);
	if (ret) {
		dev_err(clk->dev, "Failed to read reg for clock %s\n",
			desc->name);
		return 0;
	}

	rate = airoha_clk_get_base_rate(desc, val);

	reg = desc->div_reg ? desc->div_reg : desc->base_reg;
	ret = regmap_read(map, reg, &val);
	if (ret) {
		dev_err(clk->dev, "Failed to read reg for clock %s\n",
			desc->name);
		return 0;
	}

	rate /= airoha_clk_get_div(desc, val);

	return rate;
}

static int airoha_clk_search_rate(const struct airoha_clk_desc *desc, int div,
				  ulong rate)
{
	int i;

	/* Single base rate */
	if (!desc->base_bits) {
		if (rate != desc->base_value / div)
			goto err;

		return 0;
	}

	/* Check every base rate with provided divisor */
	for (i = 0; i < desc->n_base_values; i++)
		if (rate == desc->base_values[i] / div)
			return i;

err:
	return -EINVAL;
}

static ulong airoha_clk_set_rate(struct clk *clk, ulong rate)
{
	struct airoha_clk_priv *priv = dev_get_priv(clk->dev);
	struct airoha_clk_soc_data *data = priv->data;
	const struct airoha_clk_desc *desc;
	struct regmap *map = priv->chip_scu_map;
	int div_val, base_val;
	u32 reg, val, mask;
	int id = clk->id;
	int div;
	int ret;

	if (id > data->num_clocks) {
		dev_err(clk->dev, "Invalid clk ID %d\n", id);
		return 0;
	}

	desc = &data->descs[id];

	if (!desc->base_bits && !desc->div_bits) {
		dev_err(clk->dev, "Can't set rate for fixed clock %s\n",
			desc->name);
		return 0;
	}

	if (!desc->div_bits) {
		/* Divisor not supported, just search in base rate */
		div_val = 0;
		base_val = airoha_clk_search_rate(desc, 1, rate);
		if (base_val < 0) {
			dev_err(clk->dev, "Invalid rate for clock %s\n",
				desc->name);
			return 0;
		}
	} else {
		div_val = 0;

		/* Check if div0 satisfy the request */
		if (desc->div_val0) {
			base_val = airoha_clk_search_rate(desc, desc->div_val0,
							  rate);
			if (base_val >= 0) {
				div_val = 0;
				goto apply;
			}

			/* Skip checking first divisor val */
			div_val = 1;
		}

		/* Simulate rate with every divisor supported */
		for (div_val = div_val + desc->div_offset;
		     div_val < BIT(desc->div_bits) - 1; div_val++) {
			div = div_val * desc->div_step;

			base_val = airoha_clk_search_rate(desc, div, rate);
			if (base_val >= 0)
				break;
		}

		if (div_val == BIT(desc->div_bits) - 1) {
			dev_err(clk->dev, "Invalid rate for clock %s\n",
				desc->name);
			return 0;
		}
	}

apply:
	if (desc->div_bits) {
		reg = desc->div_reg ? desc->div_reg : desc->base_reg;

		mask = (BIT(desc->div_bits) - 1) << desc->div_shift;
		val = div_val << desc->div_shift;

		ret = regmap_update_bits(map, reg, mask, val);
		if (ret) {
			dev_err(clk->dev, "Failed to update div reg for clock %s\n",
				desc->name);
			return 0;
		}
	}

	if (desc->base_bits) {
		mask = (BIT(desc->base_bits) - 1) << desc->base_shift;
		val = base_val << desc->base_shift;

		ret = regmap_update_bits(map, desc->base_reg, mask, val);
		if (ret) {
			dev_err(clk->dev, "Failed to update reg for clock %s\n",
				desc->name);
			return 0;
		}
	}

	return rate;
}

const struct clk_ops airoha_clk_ops = {
	.enable = airoha_clk_enable,
	.disable = airoha_clk_disable,
	.get_rate = airoha_clk_get_rate,
	.set_rate = airoha_clk_set_rate,
};

static int airoha_clk_probe(struct udevice *dev)
{
	struct airoha_clk_priv *priv = dev_get_priv(dev);
	ofnode chip_scu_node;

	chip_scu_node = ofnode_by_compatible(ofnode_null(),
					     "airoha,en7581-chip-scu");
	if (!ofnode_valid(chip_scu_node))
		return -EINVAL;

	priv->chip_scu_map = syscon_node_to_regmap(chip_scu_node);
	if (IS_ERR(priv->chip_scu_map))
		return PTR_ERR(priv->chip_scu_map);

	priv->data = (void *)dev_get_driver_data(dev);

	return 0;
}

static int airoha_clk_bind(struct udevice *dev)
{
	struct udevice *rst_dev;
	int ret = 0;

	if (CONFIG_IS_ENABLED(RESET_AIROHA)) {
		ret = device_bind_driver_to_node(dev, "airoha-reset", "reset",
						 dev_ofnode(dev), &rst_dev);
		if (ret)
			debug("Warning: failed to bind reset controller\n");
	}

	return ret;
}

static const struct airoha_clk_soc_data en7581_data = {
	.num_clocks = ARRAY_SIZE(en7581_base_clks),
	.descs = en7581_base_clks,
};

static const struct udevice_id airoha_clk_ids[] = {
	{ .compatible = "airoha,en7581-scu",
	  .data = (ulong)&en7581_data,
	},
	{ }
};

U_BOOT_DRIVER(airoha_clk) = {
	.name = "clk-airoha",
	.id = UCLASS_CLK,
	.of_match = airoha_clk_ids,
	.probe = airoha_clk_probe,
	.bind = airoha_clk_bind,
	.priv_auto = sizeof(struct airoha_clk_priv),
	.ops = &airoha_clk_ops,
};
