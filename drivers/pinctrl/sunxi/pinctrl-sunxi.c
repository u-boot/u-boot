// SPDX-License-Identifier: GPL-2.0

#include <clk.h>
#include <dm.h>
#include <dm/device-internal.h>
#include <dm/lists.h>
#include <dm/pinctrl.h>
#include <errno.h>
#include <malloc.h>

#include <asm/gpio.h>

extern U_BOOT_DRIVER(gpio_sunxi);

struct sunxi_pinctrl_desc {
	u8					first_bank;
	u8					num_banks;
};

struct sunxi_pinctrl_plat {
	struct sunxi_gpio __iomem *base;
};

static const struct pinctrl_ops sunxi_pinctrl_ops = {
	.set_state		= pinctrl_generic_set_state,
};

static int sunxi_pinctrl_bind(struct udevice *dev)
{
	struct sunxi_pinctrl_plat *plat = dev_get_plat(dev);
	struct sunxi_pinctrl_desc *desc;
	struct sunxi_gpio_plat *gpio_plat;
	struct udevice *gpio_dev;
	int i, ret;

	desc = (void *)dev_get_driver_data(dev);
	if (!desc)
		return -EINVAL;
	dev_set_priv(dev, desc);

	plat->base = dev_read_addr_ptr(dev);

	ret = device_bind_driver_to_node(dev, "gpio_sunxi", dev->name,
					 dev_ofnode(dev), &gpio_dev);
	if (ret)
		return ret;

	for (i = 0; i < desc->num_banks; ++i) {
		gpio_plat = malloc(sizeof(*gpio_plat));
		if (!gpio_plat)
			return -ENOMEM;

		gpio_plat->regs = plat->base + i;
		gpio_plat->bank_name[0] = 'P';
		gpio_plat->bank_name[1] = 'A' + desc->first_bank + i;
		gpio_plat->bank_name[2] = '\0';

		ret = device_bind(gpio_dev, DM_DRIVER_REF(gpio_sunxi),
				  gpio_plat->bank_name, gpio_plat,
				  ofnode_null(), NULL);
		if (ret)
			return ret;
	}

	return 0;
}

static int sunxi_pinctrl_probe(struct udevice *dev)
{
	struct clk *apb_clk;

	apb_clk = devm_clk_get(dev, "apb");
	if (!IS_ERR(apb_clk))
		clk_enable(apb_clk);

	return 0;
}

static const struct sunxi_pinctrl_desc __maybe_unused suniv_f1c100s_pinctrl_desc = {
	.first_bank	= SUNXI_GPIO_A,
	.num_banks	= 6,
};

static const struct sunxi_pinctrl_desc __maybe_unused sun4i_a10_pinctrl_desc = {
	.first_bank	= SUNXI_GPIO_A,
	.num_banks	= 9,
};

static const struct sunxi_pinctrl_desc __maybe_unused sun5i_a13_pinctrl_desc = {
	.first_bank	= SUNXI_GPIO_A,
	.num_banks	= 7,
};

static const struct sunxi_pinctrl_desc __maybe_unused sun6i_a31_pinctrl_desc = {
	.first_bank	= SUNXI_GPIO_A,
	.num_banks	= 8,
};

static const struct sunxi_pinctrl_desc __maybe_unused sun6i_a31_r_pinctrl_desc = {
	.first_bank	= SUNXI_GPIO_L,
	.num_banks	= 2,
};

static const struct sunxi_pinctrl_desc __maybe_unused sun7i_a20_pinctrl_desc = {
	.first_bank	= SUNXI_GPIO_A,
	.num_banks	= 9,
};

static const struct sunxi_pinctrl_desc __maybe_unused sun8i_a23_pinctrl_desc = {
	.first_bank	= SUNXI_GPIO_A,
	.num_banks	= 8,
};

static const struct sunxi_pinctrl_desc __maybe_unused sun8i_a23_r_pinctrl_desc = {
	.first_bank	= SUNXI_GPIO_L,
	.num_banks	= 1,
};

static const struct sunxi_pinctrl_desc __maybe_unused sun8i_a33_pinctrl_desc = {
	.first_bank	= SUNXI_GPIO_A,
	.num_banks	= 8,
};

static const struct sunxi_pinctrl_desc __maybe_unused sun8i_a83t_pinctrl_desc = {
	.first_bank	= SUNXI_GPIO_A,
	.num_banks	= 8,
};

static const struct sunxi_pinctrl_desc __maybe_unused sun8i_a83t_r_pinctrl_desc = {
	.first_bank	= SUNXI_GPIO_L,
	.num_banks	= 1,
};

static const struct sunxi_pinctrl_desc __maybe_unused sun8i_h3_pinctrl_desc = {
	.first_bank	= SUNXI_GPIO_A,
	.num_banks	= 7,
};

static const struct sunxi_pinctrl_desc __maybe_unused sun8i_h3_r_pinctrl_desc = {
	.first_bank	= SUNXI_GPIO_L,
	.num_banks	= 1,
};

static const struct sunxi_pinctrl_desc __maybe_unused sun8i_v3s_pinctrl_desc = {
	.first_bank	= SUNXI_GPIO_A,
	.num_banks	= 7,
};

static const struct sunxi_pinctrl_desc __maybe_unused sun9i_a80_pinctrl_desc = {
	.first_bank	= SUNXI_GPIO_A,
	.num_banks	= 8,
};

static const struct sunxi_pinctrl_desc __maybe_unused sun9i_a80_r_pinctrl_desc = {
	.first_bank	= SUNXI_GPIO_L,
	.num_banks	= 3,
};

static const struct sunxi_pinctrl_desc __maybe_unused sun50i_a64_pinctrl_desc = {
	.first_bank	= SUNXI_GPIO_A,
	.num_banks	= 8,
};

static const struct sunxi_pinctrl_desc __maybe_unused sun50i_a64_r_pinctrl_desc = {
	.first_bank	= SUNXI_GPIO_L,
	.num_banks	= 1,
};

static const struct sunxi_pinctrl_desc __maybe_unused sun50i_h5_pinctrl_desc = {
	.first_bank	= SUNXI_GPIO_A,
	.num_banks	= 7,
};

static const struct sunxi_pinctrl_desc __maybe_unused sun50i_h6_pinctrl_desc = {
	.first_bank	= SUNXI_GPIO_A,
	.num_banks	= 8,
};

static const struct sunxi_pinctrl_desc __maybe_unused sun50i_h6_r_pinctrl_desc = {
	.first_bank	= SUNXI_GPIO_L,
	.num_banks	= 2,
};

static const struct sunxi_pinctrl_desc __maybe_unused sun50i_h616_pinctrl_desc = {
	.first_bank	= SUNXI_GPIO_A,
	.num_banks	= 9,
};

static const struct sunxi_pinctrl_desc __maybe_unused sun50i_h616_r_pinctrl_desc = {
	.first_bank	= SUNXI_GPIO_L,
	.num_banks	= 1,
};

static const struct udevice_id sunxi_pinctrl_ids[] = {
#ifdef CONFIG_PINCTRL_SUNIV_F1C100S
	{
		.compatible = "allwinner,suniv-f1c100s-pinctrl",
		.data = (ulong)&suniv_f1c100s_pinctrl_desc,
	},
#endif
#ifdef CONFIG_PINCTRL_SUN4I_A10
	{
		.compatible = "allwinner,sun4i-a10-pinctrl",
		.data = (ulong)&sun4i_a10_pinctrl_desc,
	},
#endif
#ifdef CONFIG_PINCTRL_SUN5I_A13
	{
		.compatible = "allwinner,sun5i-a10s-pinctrl",
		.data = (ulong)&sun5i_a13_pinctrl_desc,
	},
	{
		.compatible = "allwinner,sun5i-a13-pinctrl",
		.data = (ulong)&sun5i_a13_pinctrl_desc,
	},
#endif
#ifdef CONFIG_PINCTRL_SUN6I_A31
	{
		.compatible = "allwinner,sun6i-a31-pinctrl",
		.data = (ulong)&sun6i_a31_pinctrl_desc,
	},
	{
		.compatible = "allwinner,sun6i-a31s-pinctrl",
		.data = (ulong)&sun6i_a31_pinctrl_desc,
	},
#endif
#ifdef CONFIG_PINCTRL_SUN6I_A31_R
	{
		.compatible = "allwinner,sun6i-a31-r-pinctrl",
		.data = (ulong)&sun6i_a31_r_pinctrl_desc,
	},
#endif
#ifdef CONFIG_PINCTRL_SUN7I_A20
	{
		.compatible = "allwinner,sun7i-a20-pinctrl",
		.data = (ulong)&sun7i_a20_pinctrl_desc,
	},
#endif
#ifdef CONFIG_PINCTRL_SUN8I_A23
	{
		.compatible = "allwinner,sun8i-a23-pinctrl",
		.data = (ulong)&sun8i_a23_pinctrl_desc,
	},
#endif
#ifdef CONFIG_PINCTRL_SUN8I_A23_R
	{
		.compatible = "allwinner,sun8i-a23-r-pinctrl",
		.data = (ulong)&sun8i_a23_r_pinctrl_desc,
	},
#endif
#ifdef CONFIG_PINCTRL_SUN8I_A33
	{
		.compatible = "allwinner,sun8i-a33-pinctrl",
		.data = (ulong)&sun8i_a33_pinctrl_desc,
	},
#endif
#ifdef CONFIG_PINCTRL_SUN8I_A83T
	{
		.compatible = "allwinner,sun8i-a83t-pinctrl",
		.data = (ulong)&sun8i_a83t_pinctrl_desc,
	},
#endif
#ifdef CONFIG_PINCTRL_SUN8I_A83T_R
	{
		.compatible = "allwinner,sun8i-a83t-r-pinctrl",
		.data = (ulong)&sun8i_a83t_r_pinctrl_desc,
	},
#endif
#ifdef CONFIG_PINCTRL_SUN8I_H3
	{
		.compatible = "allwinner,sun8i-h3-pinctrl",
		.data = (ulong)&sun8i_h3_pinctrl_desc,
	},
#endif
#ifdef CONFIG_PINCTRL_SUN8I_H3_R
	{
		.compatible = "allwinner,sun8i-h3-r-pinctrl",
		.data = (ulong)&sun8i_h3_r_pinctrl_desc,
	},
#endif
#ifdef CONFIG_PINCTRL_SUN7I_A20
	{
		.compatible = "allwinner,sun8i-r40-pinctrl",
		.data = (ulong)&sun7i_a20_pinctrl_desc,
	},
#endif
#ifdef CONFIG_PINCTRL_SUN8I_V3S
	{
		.compatible = "allwinner,sun8i-v3-pinctrl",
		.data = (ulong)&sun8i_v3s_pinctrl_desc,
	},
	{
		.compatible = "allwinner,sun8i-v3s-pinctrl",
		.data = (ulong)&sun8i_v3s_pinctrl_desc,
	},
#endif
#ifdef CONFIG_PINCTRL_SUN9I_A80
	{
		.compatible = "allwinner,sun9i-a80-pinctrl",
		.data = (ulong)&sun9i_a80_pinctrl_desc,
	},
#endif
#ifdef CONFIG_PINCTRL_SUN9I_A80_R
	{
		.compatible = "allwinner,sun9i-a80-r-pinctrl",
		.data = (ulong)&sun9i_a80_r_pinctrl_desc,
	},
#endif
#ifdef CONFIG_PINCTRL_SUN50I_A64
	{
		.compatible = "allwinner,sun50i-a64-pinctrl",
		.data = (ulong)&sun50i_a64_pinctrl_desc,
	},
#endif
#ifdef CONFIG_PINCTRL_SUN50I_A64_R
	{
		.compatible = "allwinner,sun50i-a64-r-pinctrl",
		.data = (ulong)&sun50i_a64_r_pinctrl_desc,
	},
#endif
#ifdef CONFIG_PINCTRL_SUN50I_H5
	{
		.compatible = "allwinner,sun50i-h5-pinctrl",
		.data = (ulong)&sun50i_h5_pinctrl_desc,
	},
#endif
#ifdef CONFIG_PINCTRL_SUN50I_H6
	{
		.compatible = "allwinner,sun50i-h6-pinctrl",
		.data = (ulong)&sun50i_h6_pinctrl_desc,
	},
#endif
#ifdef CONFIG_PINCTRL_SUN50I_H6_R
	{
		.compatible = "allwinner,sun50i-h6-r-pinctrl",
		.data = (ulong)&sun50i_h6_r_pinctrl_desc,
	},
#endif
#ifdef CONFIG_PINCTRL_SUN50I_H616
	{
		.compatible = "allwinner,sun50i-h616-pinctrl",
		.data = (ulong)&sun50i_h616_pinctrl_desc,
	},
#endif
#ifdef CONFIG_PINCTRL_SUN50I_H616_R
	{
		.compatible = "allwinner,sun50i-h616-r-pinctrl",
		.data = (ulong)&sun50i_h616_r_pinctrl_desc,
	},
#endif
	{}
};

U_BOOT_DRIVER(sunxi_pinctrl) = {
	.name		= "sunxi-pinctrl",
	.id		= UCLASS_PINCTRL,
	.of_match	= sunxi_pinctrl_ids,
	.bind		= sunxi_pinctrl_bind,
	.probe		= sunxi_pinctrl_probe,
	.plat_auto	= sizeof(struct sunxi_pinctrl_plat),
	.ops		= &sunxi_pinctrl_ops,
};
