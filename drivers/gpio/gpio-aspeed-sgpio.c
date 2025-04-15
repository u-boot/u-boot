// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) ASPEED Technology Inc.
 * Billy Tsai <billy_tsai@aspeedtech.com>
 */
#include <asm/io.h>
#include <asm/gpio.h>

#include <config.h>
#include <clk.h>
#include <dm.h>
#include <dm/device_compat.h>
#include <asm/io.h>
#include <linux/bug.h>
#include <linux/sizes.h>
#include <linux/bitfield.h>
#include <linux/bitops.h>

#define ASPEED_SGPIO_CTRL 0x54

#define ASPEED_SGPIO_CLK_DIV_MASK GENMASK(31, 16)
#define ASPEED_SGPIO_ENABLE BIT(0)
#define ASPEED_SGPIO_PINS_SHIFT 6

struct aspeed_sgpio_priv {
	void *base;
	struct clk pclk;
	const struct aspeed_sgpio_pdata *pdata;
};

struct aspeed_sgpio_pdata {
	const u32 pin_mask;
	const struct aspeed_sgpio_llops *llops;
};

struct aspeed_sgpio_bank {
	u16 val_regs;
	u16 rdata_reg;
	u16 tolerance_regs;
	const char names[4][3];
};

/*
 * Note: The "value" register returns the input value when the GPIO is
 *	 configured as an input.
 *
 *	 The "rdata" register returns the output value when the GPIO is
 *	 configured as an output.
 */
static const struct aspeed_sgpio_bank aspeed_sgpio_banks[] = {
	{
		.val_regs = 0x0000,
		.rdata_reg = 0x0070,
		.tolerance_regs = 0x0018,
		.names = { "A", "B", "C", "D" },
	},
	{
		.val_regs = 0x001C,
		.rdata_reg = 0x0074,
		.tolerance_regs = 0x0034,
		.names = { "E", "F", "G", "H" },
	},
	{
		.val_regs = 0x0038,
		.rdata_reg = 0x0078,
		.tolerance_regs = 0x0050,
		.names = { "I", "J", "K", "L" },
	},
	{
		.val_regs = 0x0090,
		.rdata_reg = 0x007C,
		.tolerance_regs = 0x00A8,
		.names = { "M", "N", "O", "P" },
	},
};

enum aspeed_sgpio_reg {
	reg_val,
	reg_rdata,
	reg_tolerance,
};

struct aspeed_sgpio_llops {
	void (*reg_bit_set)(struct aspeed_sgpio_priv *gpio, unsigned int offset,
			    const enum aspeed_sgpio_reg reg, bool val);
	bool (*reg_bit_get)(struct aspeed_sgpio_priv *gpio, unsigned int offset,
			    const enum aspeed_sgpio_reg reg);
};

#define GPIO_VAL_VALUE 0x00

static void __iomem *bank_reg(struct aspeed_sgpio_priv *gpio,
			      const struct aspeed_sgpio_bank *bank,
			      const enum aspeed_sgpio_reg reg)
{
	switch (reg) {
	case reg_val:
		return gpio->base + bank->val_regs + GPIO_VAL_VALUE;
	case reg_rdata:
		return gpio->base + bank->rdata_reg;
	case reg_tolerance:
		return gpio->base + bank->tolerance_regs;
	default:
		/* acturally if code runs to here, it's an error case */
		BUG();
	}
}

#define GPIO_BANK(x) ((x) >> 6)
#define GPIO_OFFSET(x) ((x) & GENMASK(5, 0))
#define GPIO_BIT(x) BIT(GPIO_OFFSET(x) >> 1)

static const struct aspeed_sgpio_bank *to_bank(unsigned int offset)
{
	unsigned int bank;

	bank = GPIO_BANK(offset);

	WARN_ON(bank >= ARRAY_SIZE(aspeed_sgpio_banks));
	return &aspeed_sgpio_banks[bank];
}

static bool aspeed_sgpio_is_input(unsigned int offset)
{
	return !(offset % 2);
}

static int aspeed_sgpio_get_value(struct udevice *dev, unsigned int offset)
{
	struct aspeed_sgpio_priv *gpio = dev_get_priv(dev);
	enum aspeed_sgpio_reg reg;

	reg = aspeed_sgpio_is_input(offset) ? reg_val : reg_rdata;

	return gpio->pdata->llops->reg_bit_get(gpio, offset, reg);
}

static int aspeed_sgpio_set_value(struct udevice *dev, unsigned int offset,
				  int value)
{
	struct aspeed_sgpio_priv *gpio = dev_get_priv(dev);

	if (aspeed_sgpio_is_input(offset))
		return -EINVAL;

	gpio->pdata->llops->reg_bit_set(gpio, offset, reg_val, value);

	return 0;
}

static int aspeed_sgpio_direction_input(struct udevice *dev,
					unsigned int offset)
{
	return aspeed_sgpio_is_input(offset) ? 0 : -EINVAL;
}

static int aspeed_sgpio_set_flags(struct udevice *dev, unsigned int offset, ulong flags)
{
	int ret = -EOPNOTSUPP;

	if (flags & GPIOD_IS_OUT) {
		bool value = flags & GPIOD_IS_OUT_ACTIVE;

		ret = aspeed_sgpio_set_value(dev, offset, value);
	} else if (flags & GPIOD_IS_IN) {
		ret = aspeed_sgpio_direction_input(dev, offset);
	}
	return ret;
}

static int aspeed_sgpio_get_function(struct udevice *dev, unsigned int offset)
{
	return aspeed_sgpio_is_input(offset) ? GPIOF_INPUT : GPIOF_OUTPUT;
}

static void aspeed_g4_reg_bit_set(struct aspeed_sgpio_priv *gpio, unsigned int offset,
				  const enum aspeed_sgpio_reg reg, bool val)
{
	const struct aspeed_sgpio_bank *bank = to_bank(offset);
	void __iomem *addr = bank_reg(gpio, bank, reg);
	u32 temp;

	if (reg == reg_val)
		/* Since this is an output, read the cached value from rdata, then update val. */
		temp = readl(bank_reg(gpio, bank, reg_rdata));
	else
		temp = readl(addr);

	if (val)
		temp |= GPIO_BIT(offset);
	else
		temp &= ~GPIO_BIT(offset);

	writel(temp, addr);
}

static bool aspeed_g4_reg_bit_get(struct aspeed_sgpio_priv *gpio, unsigned int offset,
				  const enum aspeed_sgpio_reg reg)
{
	const struct aspeed_sgpio_bank *bank = to_bank(offset);
	void __iomem *addr = bank_reg(gpio, bank, reg);

	return !!(readl(addr) & GPIO_BIT(offset));
}

static const struct aspeed_sgpio_llops aspeed_g4_llops = {
	.reg_bit_set = aspeed_g4_reg_bit_set,
	.reg_bit_get = aspeed_g4_reg_bit_get,
};

static const struct dm_gpio_ops aspeed_sgpio_ops = {
	.get_value = aspeed_sgpio_get_value,
	.set_value = aspeed_sgpio_set_value,
	.get_function = aspeed_sgpio_get_function,
	.set_flags = aspeed_sgpio_set_flags,
};

static int aspeed_sgpio_probe(struct udevice *dev)
{
	struct gpio_dev_priv *uc_priv = dev_get_uclass_priv(dev);
	struct aspeed_sgpio_priv *priv = dev_get_priv(dev);
	u32 sgpio_freq, sgpio_clk_div, nr_gpios, gpio_cnt_regval, pin_mask;
	ulong apb_freq;
	int ret;

	priv->base = devfdt_get_addr_ptr(dev);
	if (IS_ERR(priv->base))
		return PTR_ERR(priv->base);

	priv->pdata = (const struct aspeed_sgpio_pdata *)dev_get_driver_data(dev);
	if (!priv->pdata)
		return -EINVAL;

	pin_mask = priv->pdata->pin_mask;

	ret = ofnode_read_u32(dev_ofnode(dev), "ngpios", &nr_gpios);
	if (ret < 0) {
		dev_err(dev, "Could not read ngpios property\n");
		return -EINVAL;
	} else if (nr_gpios % 8) {
		dev_err(dev, "Number of GPIOs not multiple of 8: %d\n",
			nr_gpios);
		return -EINVAL;
	}

	ret = ofnode_read_u32(dev_ofnode(dev), "bus-frequency", &sgpio_freq);
	if (ret < 0) {
		dev_err(dev, "Could not read bus-frequency property\n");
		return -EINVAL;
	}

	ret = clk_get_by_index(dev, 0, &priv->pclk);
	if (ret < 0) {
		dev_err(dev, "get clock failed\n");
		return ret;
	}

	apb_freq = clk_get_rate(&priv->pclk);

	/*
	 * From the datasheet,
	 *	SGPIO period = 1/PCLK * 2 * (GPIO254[31:16] + 1)
	 *	period = 2 * (GPIO254[31:16] + 1) / PCLK
	 *	frequency = 1 / (2 * (GPIO254[31:16] + 1) / PCLK)
	 *	frequency = PCLK / (2 * (GPIO254[31:16] + 1))
	 *	frequency * 2 * (GPIO254[31:16] + 1) = PCLK
	 *	GPIO254[31:16] = PCLK / (frequency * 2) - 1
	 */
	if (sgpio_freq == 0)
		return -EINVAL;

	sgpio_clk_div = (apb_freq / (sgpio_freq * 2)) - 1;

	if (sgpio_clk_div > (1 << 16) - 1)
		return -EINVAL;

	gpio_cnt_regval = ((nr_gpios / 8) << ASPEED_SGPIO_PINS_SHIFT) & pin_mask;
	writel(FIELD_PREP(ASPEED_SGPIO_CLK_DIV_MASK, sgpio_clk_div) | gpio_cnt_regval |
	       ASPEED_SGPIO_ENABLE, priv->base + ASPEED_SGPIO_CTRL);

	uc_priv->bank_name = dev->name;
	uc_priv->gpio_count = nr_gpios * 2;

	return 0;
}

static const struct aspeed_sgpio_pdata ast2400_sgpio_pdata = {
	.pin_mask = GENMASK(9, 6),
	.llops = &aspeed_g4_llops,
};

static const struct aspeed_sgpio_pdata ast2600_sgpiom_pdata = {
	.pin_mask = GENMASK(10, 6),
	.llops = &aspeed_g4_llops,
};

static const struct udevice_id aspeed_sgpio_ids[] = {
	{ .compatible = "aspeed,ast2400-sgpio", .data = (ulong)&ast2400_sgpio_pdata, },
	{ .compatible = "aspeed,ast2500-sgpio", .data = (ulong)&ast2400_sgpio_pdata, },
	{ .compatible = "aspeed,ast2600-sgpiom", .data = (ulong)&ast2600_sgpiom_pdata, },
};

U_BOOT_DRIVER(sgpio_aspeed) = {
	.name = "sgpio-aspeed",
	.id = UCLASS_GPIO,
	.of_match = aspeed_sgpio_ids,
	.ops = &aspeed_sgpio_ops,
	.probe = aspeed_sgpio_probe,
	.priv_auto = sizeof(struct aspeed_sgpio_priv),
};
