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
#include <asm/io.h>
#include <linux/bug.h>
#include <linux/sizes.h>

struct aspeed_gpio_priv {
	void *regs;
};

#define GPIO_G7_IRQ_STS_BASE 0x100
#define GPIO_G7_IRQ_STS_OFFSET(x) (GPIO_G7_IRQ_STS_BASE + (x) * 0x4)
#define GPIO_G7_CTRL_REG_BASE 0x180
#define GPIO_G7_CTRL_REG_OFFSET(x) (GPIO_G7_CTRL_REG_BASE + (x) * 0x4)
#define GPIO_G7_OUT_DATA BIT(0)
#define GPIO_G7_DIR BIT(1)
#define GPIO_G7_IRQ_EN BIT(2)
#define GPIO_G7_IRQ_TYPE0 BIT(3)
#define GPIO_G7_IRQ_TYPE1 BIT(4)
#define GPIO_G7_IRQ_TYPE2 BIT(5)
#define GPIO_G7_RST_TOLERANCE BIT(6)
#define GPIO_G7_DEBOUNCE_SEL GENMASK(8, 7)
#define GPIO_G7_INPUT_MASK BIT(9)
#define GPIO_G7_IRQ_STS BIT(12)
#define GPIO_G7_IN_DATA BIT(13)
/*
 * The configuration of the following registers should be determined
 * outside of the GPIO driver.
 */
#define GPIO_G7_PRIVILEGE_W_REG_BASE 0x810
#define GPIO_G7_PRIVILEGE_W_REG_OFFSET(x) (GPIO_G7_PRIVILEGE_W_REG_BASE + ((x) >> 2) * 0x4)
#define GPIO_G7_PRIVILEGE_R_REG_BASE 0x910
#define GPIO_G7_PRIVILEGE_R_REG_OFFSET(x) (GPIO_G7_PRIVILEGE_R_REG_BASE + ((x) >> 2) * 0x4)
#define GPIO_G7_IRQ_TARGET_REG_BASE 0xA10
#define GPIO_G7_IRQ_TARGET_REG_OFFSET(x) (GPIO_G7_IRQ_TARGET_REG_BASE + ((x) >> 2) * 0x4)
#define GPIO_G7_IRQ_TO_INTC2_18 BIT(0)
#define GPIO_G7_IRQ_TO_INTC2_19 BIT(1)
#define GPIO_G7_IRQ_TO_INTC2_20 BIT(2)
#define GPIO_G7_IRQ_TO_SIO BIT(3)
#define GPIO_G7_IRQ_TARGET_RESET_TOLERANCE BIT(6)
#define GPIO_G7_IRQ_TARGET_W_PROTECT BIT(7)

static int
aspeed_gpio_direction_input(struct udevice *dev, unsigned int offset)
{
	struct aspeed_gpio_priv *priv = dev_get_priv(dev);
	void __iomem *addr = priv->regs + GPIO_G7_CTRL_REG_OFFSET(offset);
	u32 dir = readl(addr);

	dir &= ~GPIO_G7_DIR;
	writel(dir, addr);

	return 0;
}

static int aspeed_gpio_direction_output(struct udevice *dev, unsigned int offset,
					int value)
{
	struct aspeed_gpio_priv *priv = dev_get_priv(dev);
	void __iomem *addr = priv->regs + GPIO_G7_CTRL_REG_OFFSET(offset);
	u32 data = readl(addr);

	if (value)
		data |= GPIO_G7_OUT_DATA;
	else
		data &= ~GPIO_G7_OUT_DATA;
	writel(data, addr);
	data |= GPIO_G7_DIR;
	writel(data, addr);

	return 0;
}

static int aspeed_gpio_get_value(struct udevice *dev, unsigned int offset)
{
	struct aspeed_gpio_priv *priv = dev_get_priv(dev);
	void __iomem *addr = priv->regs + GPIO_G7_CTRL_REG_OFFSET(offset);

	return !!(readl(addr) & GPIO_G7_IN_DATA);
}

static int
aspeed_gpio_set_value(struct udevice *dev, unsigned int offset, int value)
{
	struct aspeed_gpio_priv *priv = dev_get_priv(dev);
	void __iomem *addr = priv->regs + GPIO_G7_CTRL_REG_OFFSET(offset);
	u32 data = readl(addr);

	if (value)
		data |= GPIO_G7_OUT_DATA;
	else
		data &= ~GPIO_G7_OUT_DATA;

	writel(data, addr);

	return 0;
}

static int aspeed_gpio_get_function(struct udevice *dev, unsigned int offset)
{
	struct aspeed_gpio_priv *priv = dev_get_priv(dev);
	void __iomem *addr = priv->regs + GPIO_G7_CTRL_REG_OFFSET(offset);

	if (readl(addr) & GPIO_G7_DIR)
		return GPIOF_OUTPUT;

	return GPIOF_INPUT;
}

static const struct dm_gpio_ops aspeed_gpio_ops = {
	.direction_input	= aspeed_gpio_direction_input,
	.direction_output	= aspeed_gpio_direction_output,
	.get_value		= aspeed_gpio_get_value,
	.set_value		= aspeed_gpio_set_value,
	.get_function		= aspeed_gpio_get_function,
};

static int aspeed_gpio_probe(struct udevice *dev)
{
	struct gpio_dev_priv *uc_priv = dev_get_uclass_priv(dev);
	struct aspeed_gpio_priv *priv = dev_get_priv(dev);

	uc_priv->bank_name = dev->name;
	ofnode_read_u32(dev_ofnode(dev), "ngpios", &uc_priv->gpio_count);
	priv->regs = devfdt_get_addr_ptr(dev);

	return 0;
}

static const struct udevice_id aspeed_gpio_ids[] = {
	{ .compatible = "aspeed,ast2700-gpio",  },
	{ }
};

U_BOOT_DRIVER(gpio_aspeed) = {
	.name   = "gpio-aspeed",
	.id     = UCLASS_GPIO,
	.of_match = aspeed_gpio_ids,
	.ops    = &aspeed_gpio_ops,
	.probe  = aspeed_gpio_probe,
	.priv_auto = sizeof(struct aspeed_gpio_priv),
};
