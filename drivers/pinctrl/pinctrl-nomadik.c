// SPDX-License-Identifier: GPL-2.0+
/* Copyright (C) 2026 Linus Walleij <linusw@kernel.org> */

#include <dm.h>
#include <dm/pinctrl.h>
#include <dt-bindings/pinctrl/nomadik.h>
#include <vsprintf.h>
#include <asm/io.h>

#define NMK_GPIO_BANKS		9
#define NMK_GPIO_PER_BANK	32

struct nmk_gpio_regs {
	u32 dat;
	u32 dats;
	u32 datc;
	u32 pdis;
	u32 dir;
	u32 dirs;
	u32 dirc;
	u32 slpm;
	u32 afsla;
	u32 afslb;
	u32 lowemi;
};

struct nmk_pinctrl_priv {
	struct nmk_gpio_regs *bank[NMK_GPIO_BANKS];
};

enum nmk_alt {
	NMK_ALT_GPIO,
	NMK_ALT_A,
	NMK_ALT_B,
	NMK_ALT_C,
};

static int nmk_pinctrl_parse_pin(const char *name, unsigned int *pin)
{
	char *end;

	if (strncmp(name, "GPIO", 4))
		return -EINVAL;

	*pin = dectoul(name + 4, &end);
	if (end == name + 4 || *end != '_' ||
	    *pin >= NMK_GPIO_BANKS * NMK_GPIO_PER_BANK)
		return -EINVAL;

	return 0;
}

static struct nmk_gpio_regs *
nmk_pinctrl_bank(struct udevice *dev, unsigned int pin)
{
	struct nmk_pinctrl_priv *priv = dev_get_priv(dev);

	return priv->bank[pin / NMK_GPIO_PER_BANK];
}

static void nmk_pinctrl_set_mux(struct udevice *dev, unsigned int pin,
				unsigned int alt)
{
	struct nmk_gpio_regs *regs = nmk_pinctrl_bank(dev, pin);
	u32 mask = BIT(pin % NMK_GPIO_PER_BANK);
	u32 val;

	val = readl(&regs->afsla);
	if (alt & NMK_ALT_A)
		val |= mask;
	else
		val &= ~mask;
	writel(val, &regs->afsla);

	val = readl(&regs->afslb);
	if (alt & NMK_ALT_B)
		val |= mask;
	else
		val &= ~mask;
	writel(val, &regs->afslb);
}

static int nmk_pinctrl_set_config(struct udevice *dev, ofnode node,
				  unsigned int pin)
{
	struct nmk_gpio_regs *regs = nmk_pinctrl_bank(dev, pin);
	u32 mask = BIT(pin % NMK_GPIO_PER_BANK);
	u32 val;

	if (!ofnode_read_u32(node, "ste,input", &val)) {
		writel(mask, &regs->dirc);
		if (val == INPUT_NOPULL) {
			setbits_le32(&regs->pdis, mask);
		} else {
			clrbits_le32(&regs->pdis, mask);
			if (val == INPUT_PULLUP)
				writel(mask, &regs->dats);
			else if (val == INPUT_PULLDOWN)
				writel(mask, &regs->datc);
			else
				return -EINVAL;
		}
	}

	if (!ofnode_read_u32(node, "ste,output", &val)) {
		if (val == OUTPUT_HIGH)
			writel(mask, &regs->dats);
		else if (val == OUTPUT_LOW)
			writel(mask, &regs->datc);
		else
			return -EINVAL;
		writel(mask, &regs->dirs);
	}

	if (!ofnode_read_u32(node, "ste,lowemi", &val)) {
		if (val)
			setbits_le32(&regs->lowemi, mask);
		else
			clrbits_le32(&regs->lowemi, mask);
	}

	return 0;
}

static int nmk_pinctrl_get_alt(ofnode node, unsigned int *alt)
{
	const char *group;

	if (ofnode_read_string_index(node, "groups", 0, &group))
		return -EINVAL;

	if (strstr(group, "_a_"))
		*alt = NMK_ALT_A;
	else if (strstr(group, "_b_"))
		*alt = NMK_ALT_B;
	else if (strstr(group, "_c_"))
		*alt = NMK_ALT_C;
	else
		return -EINVAL;

	return 0;
}

static int nmk_pinctrl_set_state(struct udevice *dev, struct udevice *config)
{
	unsigned int alt = NMK_ALT_GPIO;
	ofnode node;
	int count;
	int ret;
	int i;

	dev_for_each_subnode(node, config) {
		if (ofnode_read_string(node, "function")) {
			ret = nmk_pinctrl_get_alt(node, &alt);
			if (ret)
				return ret;
		}
	}

	dev_for_each_subnode(node, config) {
		ofnode cfg;

		count = ofnode_read_string_count(node, "pins");
		if (count < 0)
			continue;

		cfg = ofnode_parse_phandle(node, "ste,config", 0);
		if (!ofnode_valid(cfg))
			cfg = node;

		for (i = 0; i < count; i++) {
			const char *name;
			unsigned int pin;

			ret = ofnode_read_string_index(node, "pins", i, &name);
			if (ret)
				return ret;
			ret = nmk_pinctrl_parse_pin(name, &pin);
			if (ret)
				return ret;
			ret = nmk_pinctrl_set_config(dev, cfg, pin);
			if (ret)
				return ret;
			nmk_pinctrl_set_mux(dev, pin, alt);
		}
	}

	return 0;
}

static int nmk_pinctrl_probe(struct udevice *dev)
{
	struct nmk_pinctrl_priv *priv = dev_get_priv(dev);
	struct ofnode_phandle_args args;
	fdt_addr_t addr;
	int ret;
	int i;

	for (i = 0; i < NMK_GPIO_BANKS; i++) {
		ret = dev_read_phandle_with_args(dev, "nomadik-gpio-chips",
						 NULL, 0, i, &args);
		if (ret)
			return ret;

		addr = ofnode_get_addr(args.node);
		if (addr == FDT_ADDR_T_NONE)
			return -EINVAL;
		priv->bank[i] = (struct nmk_gpio_regs *)addr;
	}

	return 0;
}

static const struct pinctrl_ops nmk_pinctrl_ops = {
	.set_state = nmk_pinctrl_set_state,
};

static const struct udevice_id nmk_pinctrl_ids[] = {
	{ .compatible = "stericsson,db8500-pinctrl" },
	{ }
};

U_BOOT_DRIVER(pinctrl_nomadik) = {
	.name = "pinctrl_nomadik",
	.id = UCLASS_PINCTRL,
	.of_match = nmk_pinctrl_ids,
	.probe = nmk_pinctrl_probe,
	.priv_auto = sizeof(struct nmk_pinctrl_priv),
	.ops = &nmk_pinctrl_ops,
};
