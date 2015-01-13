/*
 * Copyright (c) 2014, NVIDIA CORPORATION.  All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0
 */

#define pr_fmt(fmt) "tegra-xusb-padctl: " fmt

#include <common.h>
#include <errno.h>
#include <fdtdec.h>
#include <malloc.h>

#include <asm/io.h>

#include <asm/arch/clock.h>
#include <asm/arch-tegra/xusb-padctl.h>

#include <dt-bindings/pinctrl/pinctrl-tegra-xusb.h>

#define XUSB_PADCTL_ELPG_PROGRAM 0x01c
#define XUSB_PADCTL_ELPG_PROGRAM_AUX_MUX_LP0_VCORE_DOWN (1 << 26)
#define XUSB_PADCTL_ELPG_PROGRAM_AUX_MUX_LP0_CLAMP_EN_EARLY (1 << 25)
#define XUSB_PADCTL_ELPG_PROGRAM_AUX_MUX_LP0_CLAMP_EN (1 << 24)

#define XUSB_PADCTL_IOPHY_PLL_P0_CTL1 0x040
#define XUSB_PADCTL_IOPHY_PLL_P0_CTL1_PLL0_LOCKDET (1 << 19)
#define XUSB_PADCTL_IOPHY_PLL_P0_CTL1_REFCLK_SEL_MASK (0xf << 12)
#define XUSB_PADCTL_IOPHY_PLL_P0_CTL1_PLL_RST (1 << 1)

#define XUSB_PADCTL_IOPHY_PLL_P0_CTL2 0x044
#define XUSB_PADCTL_IOPHY_PLL_P0_CTL2_REFCLKBUF_EN (1 << 6)
#define XUSB_PADCTL_IOPHY_PLL_P0_CTL2_TXCLKREF_EN (1 << 5)
#define XUSB_PADCTL_IOPHY_PLL_P0_CTL2_TXCLKREF_SEL (1 << 4)

#define XUSB_PADCTL_IOPHY_PLL_S0_CTL1 0x138
#define XUSB_PADCTL_IOPHY_PLL_S0_CTL1_PLL1_LOCKDET (1 << 27)
#define XUSB_PADCTL_IOPHY_PLL_S0_CTL1_PLL1_MODE (1 << 24)
#define XUSB_PADCTL_IOPHY_PLL_S0_CTL1_PLL_PWR_OVRD (1 << 3)
#define XUSB_PADCTL_IOPHY_PLL_S0_CTL1_PLL_RST (1 << 1)
#define XUSB_PADCTL_IOPHY_PLL_S0_CTL1_PLL_IDDQ (1 << 0)

#define XUSB_PADCTL_IOPHY_MISC_PAD_S0_CTL1 0x148
#define XUSB_PADCTL_IOPHY_MISC_PAD_S0_CTL1_IDDQ_OVRD (1 << 1)
#define XUSB_PADCTL_IOPHY_MISC_PAD_S0_CTL1_IDDQ (1 << 0)

enum tegra124_function {
	TEGRA124_FUNC_SNPS,
	TEGRA124_FUNC_XUSB,
	TEGRA124_FUNC_UART,
	TEGRA124_FUNC_PCIE,
	TEGRA124_FUNC_USB3,
	TEGRA124_FUNC_SATA,
	TEGRA124_FUNC_RSVD,
};

static const char *const tegra124_functions[] = {
	"snps",
	"xusb",
	"uart",
	"pcie",
	"usb3",
	"sata",
	"rsvd",
};

static const unsigned int tegra124_otg_functions[] = {
	TEGRA124_FUNC_SNPS,
	TEGRA124_FUNC_XUSB,
	TEGRA124_FUNC_UART,
	TEGRA124_FUNC_RSVD,
};

static const unsigned int tegra124_usb_functions[] = {
	TEGRA124_FUNC_SNPS,
	TEGRA124_FUNC_XUSB,
};

static const unsigned int tegra124_pci_functions[] = {
	TEGRA124_FUNC_PCIE,
	TEGRA124_FUNC_USB3,
	TEGRA124_FUNC_SATA,
	TEGRA124_FUNC_RSVD,
};

struct tegra_xusb_padctl_lane {
	const char *name;

	unsigned int offset;
	unsigned int shift;
	unsigned int mask;
	unsigned int iddq;

	const unsigned int *funcs;
	unsigned int num_funcs;
};

#define TEGRA124_LANE(_name, _offset, _shift, _mask, _iddq, _funcs)	\
	{								\
		.name = _name,						\
		.offset = _offset,					\
		.shift = _shift,					\
		.mask = _mask,						\
		.iddq = _iddq,						\
		.num_funcs = ARRAY_SIZE(tegra124_##_funcs##_functions),	\
		.funcs = tegra124_##_funcs##_functions,			\
	}

static const struct tegra_xusb_padctl_lane tegra124_lanes[] = {
	TEGRA124_LANE("otg-0",  0x004,  0, 0x3, 0, otg),
	TEGRA124_LANE("otg-1",  0x004,  2, 0x3, 0, otg),
	TEGRA124_LANE("otg-2",  0x004,  4, 0x3, 0, otg),
	TEGRA124_LANE("ulpi-0", 0x004, 12, 0x1, 0, usb),
	TEGRA124_LANE("hsic-0", 0x004, 14, 0x1, 0, usb),
	TEGRA124_LANE("hsic-1", 0x004, 15, 0x1, 0, usb),
	TEGRA124_LANE("pcie-0", 0x134, 16, 0x3, 1, pci),
	TEGRA124_LANE("pcie-1", 0x134, 18, 0x3, 2, pci),
	TEGRA124_LANE("pcie-2", 0x134, 20, 0x3, 3, pci),
	TEGRA124_LANE("pcie-3", 0x134, 22, 0x3, 4, pci),
	TEGRA124_LANE("pcie-4", 0x134, 24, 0x3, 5, pci),
	TEGRA124_LANE("sata-0", 0x134, 26, 0x3, 6, pci),
};

struct tegra_xusb_phy_ops {
	int (*prepare)(struct tegra_xusb_phy *phy);
	int (*enable)(struct tegra_xusb_phy *phy);
	int (*disable)(struct tegra_xusb_phy *phy);
	int (*unprepare)(struct tegra_xusb_phy *phy);
};

struct tegra_xusb_phy {
	const struct tegra_xusb_phy_ops *ops;

	struct tegra_xusb_padctl *padctl;
};

struct tegra_xusb_padctl_pin {
	const struct tegra_xusb_padctl_lane *lane;

	unsigned int func;
	int iddq;
};

#define MAX_GROUPS 3
#define MAX_PINS 6

struct tegra_xusb_padctl_group {
	const char *name;

	const char *pins[MAX_PINS];
	unsigned int num_pins;

	const char *func;
	int iddq;
};

struct tegra_xusb_padctl_config {
	const char *name;

	struct tegra_xusb_padctl_group groups[MAX_GROUPS];
	unsigned int num_groups;
};

struct tegra_xusb_padctl {
	struct fdt_resource regs;

	unsigned int enable;

	struct tegra_xusb_phy phys[2];

	const struct tegra_xusb_padctl_lane *lanes;
	unsigned int num_lanes;

	const char *const *functions;
	unsigned int num_functions;

	struct tegra_xusb_padctl_config config;
};

static inline u32 padctl_readl(struct tegra_xusb_padctl *padctl,
			       unsigned long offset)
{
	return readl(padctl->regs.start + offset);
}

static inline void padctl_writel(struct tegra_xusb_padctl *padctl,
				 u32 value, unsigned long offset)
{
	writel(value, padctl->regs.start + offset);
}

static int tegra_xusb_padctl_enable(struct tegra_xusb_padctl *padctl)
{
	u32 value;

	if (padctl->enable++ > 0)
		return 0;

	value = padctl_readl(padctl, XUSB_PADCTL_ELPG_PROGRAM);
	value &= ~XUSB_PADCTL_ELPG_PROGRAM_AUX_MUX_LP0_CLAMP_EN;
	padctl_writel(padctl, value, XUSB_PADCTL_ELPG_PROGRAM);

	udelay(100);

	value = padctl_readl(padctl, XUSB_PADCTL_ELPG_PROGRAM);
	value &= ~XUSB_PADCTL_ELPG_PROGRAM_AUX_MUX_LP0_CLAMP_EN_EARLY;
	padctl_writel(padctl, value, XUSB_PADCTL_ELPG_PROGRAM);

	udelay(100);

	value = padctl_readl(padctl, XUSB_PADCTL_ELPG_PROGRAM);
	value &= ~XUSB_PADCTL_ELPG_PROGRAM_AUX_MUX_LP0_VCORE_DOWN;
	padctl_writel(padctl, value, XUSB_PADCTL_ELPG_PROGRAM);

	return 0;
}

static int tegra_xusb_padctl_disable(struct tegra_xusb_padctl *padctl)
{
	u32 value;

	if (padctl->enable == 0) {
		error("tegra-xusb-padctl: unbalanced enable/disable");
		return 0;
	}

	if (--padctl->enable > 0)
		return 0;

	value = padctl_readl(padctl, XUSB_PADCTL_ELPG_PROGRAM);
	value |= XUSB_PADCTL_ELPG_PROGRAM_AUX_MUX_LP0_VCORE_DOWN;
	padctl_writel(padctl, value, XUSB_PADCTL_ELPG_PROGRAM);

	udelay(100);

	value = padctl_readl(padctl, XUSB_PADCTL_ELPG_PROGRAM);
	value |= XUSB_PADCTL_ELPG_PROGRAM_AUX_MUX_LP0_CLAMP_EN_EARLY;
	padctl_writel(padctl, value, XUSB_PADCTL_ELPG_PROGRAM);

	udelay(100);

	value = padctl_readl(padctl, XUSB_PADCTL_ELPG_PROGRAM);
	value |= XUSB_PADCTL_ELPG_PROGRAM_AUX_MUX_LP0_CLAMP_EN;
	padctl_writel(padctl, value, XUSB_PADCTL_ELPG_PROGRAM);

	return 0;
}

static int phy_prepare(struct tegra_xusb_phy *phy)
{
	return tegra_xusb_padctl_enable(phy->padctl);
}

static int phy_unprepare(struct tegra_xusb_phy *phy)
{
	return tegra_xusb_padctl_disable(phy->padctl);
}

static int pcie_phy_enable(struct tegra_xusb_phy *phy)
{
	struct tegra_xusb_padctl *padctl = phy->padctl;
	int err = -ETIMEDOUT;
	unsigned long start;
	u32 value;

	value = padctl_readl(padctl, XUSB_PADCTL_IOPHY_PLL_P0_CTL1);
	value &= ~XUSB_PADCTL_IOPHY_PLL_P0_CTL1_REFCLK_SEL_MASK;
	padctl_writel(padctl, value, XUSB_PADCTL_IOPHY_PLL_P0_CTL1);

	value = padctl_readl(padctl, XUSB_PADCTL_IOPHY_PLL_P0_CTL2);
	value |= XUSB_PADCTL_IOPHY_PLL_P0_CTL2_REFCLKBUF_EN |
		 XUSB_PADCTL_IOPHY_PLL_P0_CTL2_TXCLKREF_EN |
		 XUSB_PADCTL_IOPHY_PLL_P0_CTL2_TXCLKREF_SEL;
	padctl_writel(padctl, value, XUSB_PADCTL_IOPHY_PLL_P0_CTL2);

	value = padctl_readl(padctl, XUSB_PADCTL_IOPHY_PLL_P0_CTL1);
	value |= XUSB_PADCTL_IOPHY_PLL_P0_CTL1_PLL_RST;
	padctl_writel(padctl, value, XUSB_PADCTL_IOPHY_PLL_P0_CTL1);

	start = get_timer(0);

	while (get_timer(start) < 50) {
		value = padctl_readl(padctl, XUSB_PADCTL_IOPHY_PLL_P0_CTL1);
		if (value & XUSB_PADCTL_IOPHY_PLL_P0_CTL1_PLL0_LOCKDET) {
			err = 0;
			break;
		}
	}

	return err;
}

static int pcie_phy_disable(struct tegra_xusb_phy *phy)
{
	struct tegra_xusb_padctl *padctl = phy->padctl;
	u32 value;

	value = padctl_readl(padctl, XUSB_PADCTL_IOPHY_PLL_P0_CTL1);
	value &= ~XUSB_PADCTL_IOPHY_PLL_P0_CTL1_PLL_RST;
	padctl_writel(padctl, value, XUSB_PADCTL_IOPHY_PLL_P0_CTL1);

	return 0;
}

static int sata_phy_enable(struct tegra_xusb_phy *phy)
{
	struct tegra_xusb_padctl *padctl = phy->padctl;
	int err = -ETIMEDOUT;
	unsigned long start;
	u32 value;

	value = padctl_readl(padctl, XUSB_PADCTL_IOPHY_MISC_PAD_S0_CTL1);
	value &= ~XUSB_PADCTL_IOPHY_MISC_PAD_S0_CTL1_IDDQ_OVRD;
	value &= ~XUSB_PADCTL_IOPHY_MISC_PAD_S0_CTL1_IDDQ;
	padctl_writel(padctl, value, XUSB_PADCTL_IOPHY_MISC_PAD_S0_CTL1);

	value = padctl_readl(padctl, XUSB_PADCTL_IOPHY_PLL_S0_CTL1);
	value &= ~XUSB_PADCTL_IOPHY_PLL_S0_CTL1_PLL_PWR_OVRD;
	value &= ~XUSB_PADCTL_IOPHY_PLL_S0_CTL1_PLL_IDDQ;
	padctl_writel(padctl, value, XUSB_PADCTL_IOPHY_PLL_S0_CTL1);

	value = padctl_readl(padctl, XUSB_PADCTL_IOPHY_PLL_S0_CTL1);
	value |= XUSB_PADCTL_IOPHY_PLL_S0_CTL1_PLL1_MODE;
	padctl_writel(padctl, value, XUSB_PADCTL_IOPHY_PLL_S0_CTL1);

	value = padctl_readl(padctl, XUSB_PADCTL_IOPHY_PLL_S0_CTL1);
	value |= XUSB_PADCTL_IOPHY_PLL_S0_CTL1_PLL_RST;
	padctl_writel(padctl, value, XUSB_PADCTL_IOPHY_PLL_S0_CTL1);

	start = get_timer(0);

	while (get_timer(start) < 50) {
		value = padctl_readl(padctl, XUSB_PADCTL_IOPHY_PLL_S0_CTL1);
		if (value & XUSB_PADCTL_IOPHY_PLL_S0_CTL1_PLL1_LOCKDET) {
			err = 0;
			break;
		}
	}

	return err;
}

static int sata_phy_disable(struct tegra_xusb_phy *phy)
{
	struct tegra_xusb_padctl *padctl = phy->padctl;
	u32 value;

	value = padctl_readl(padctl, XUSB_PADCTL_IOPHY_PLL_S0_CTL1);
	value &= ~XUSB_PADCTL_IOPHY_PLL_S0_CTL1_PLL_RST;
	padctl_writel(padctl, value, XUSB_PADCTL_IOPHY_PLL_S0_CTL1);

	value = padctl_readl(padctl, XUSB_PADCTL_IOPHY_PLL_S0_CTL1);
	value &= ~XUSB_PADCTL_IOPHY_PLL_S0_CTL1_PLL1_MODE;
	padctl_writel(padctl, value, XUSB_PADCTL_IOPHY_PLL_S0_CTL1);

	value = padctl_readl(padctl, XUSB_PADCTL_IOPHY_PLL_S0_CTL1);
	value |= XUSB_PADCTL_IOPHY_PLL_S0_CTL1_PLL_PWR_OVRD;
	value |= XUSB_PADCTL_IOPHY_PLL_S0_CTL1_PLL_IDDQ;
	padctl_writel(padctl, value, XUSB_PADCTL_IOPHY_PLL_S0_CTL1);

	value = padctl_readl(padctl, XUSB_PADCTL_IOPHY_MISC_PAD_S0_CTL1);
	value |= ~XUSB_PADCTL_IOPHY_MISC_PAD_S0_CTL1_IDDQ_OVRD;
	value |= ~XUSB_PADCTL_IOPHY_MISC_PAD_S0_CTL1_IDDQ;
	padctl_writel(padctl, value, XUSB_PADCTL_IOPHY_MISC_PAD_S0_CTL1);

	return 0;
}

static const struct tegra_xusb_phy_ops pcie_phy_ops = {
	.prepare = phy_prepare,
	.enable = pcie_phy_enable,
	.disable = pcie_phy_disable,
	.unprepare = phy_unprepare,
};

static const struct tegra_xusb_phy_ops sata_phy_ops = {
	.prepare = phy_prepare,
	.enable = sata_phy_enable,
	.disable = sata_phy_disable,
	.unprepare = phy_unprepare,
};

static struct tegra_xusb_padctl *padctl = &(struct tegra_xusb_padctl) {
	.phys = {
		[0] = {
			.ops = &pcie_phy_ops,
		},
		[1] = {
			.ops = &sata_phy_ops,
		},
	},
};

static const struct tegra_xusb_padctl_lane *
tegra_xusb_padctl_find_lane(struct tegra_xusb_padctl *padctl, const char *name)
{
	unsigned int i;

	for (i = 0; i < padctl->num_lanes; i++)
		if (strcmp(name, padctl->lanes[i].name) == 0)
			return &padctl->lanes[i];

	return NULL;
}

static int
tegra_xusb_padctl_group_parse_dt(struct tegra_xusb_padctl *padctl,
				 struct tegra_xusb_padctl_group *group,
				 const void *fdt, int node)
{
	unsigned int i;
	int len, err;

	group->name = fdt_get_name(fdt, node, &len);

	len = fdt_count_strings(fdt, node, "nvidia,lanes");
	if (len < 0) {
		error("tegra-xusb-padctl: failed to parse \"nvidia,lanes\" property");
		return -EINVAL;
	}

	group->num_pins = len;

	for (i = 0; i < group->num_pins; i++) {
		err = fdt_get_string_index(fdt, node, "nvidia,lanes", i,
					   &group->pins[i]);
		if (err < 0) {
			error("tegra-xusb-padctl: failed to read string from \"nvidia,lanes\" property");
			return -EINVAL;
		}
	}

	group->num_pins = len;

	err = fdt_get_string(fdt, node, "nvidia,function", &group->func);
	if (err < 0) {
		error("tegra-xusb-padctl: failed to parse \"nvidia,func\" property");
		return -EINVAL;
	}

	group->iddq = fdtdec_get_int(fdt, node, "nvidia,iddq", -1);

	return 0;
}

static int tegra_xusb_padctl_find_function(struct tegra_xusb_padctl *padctl,
					   const char *name)
{
	unsigned int i;

	for (i = 0; i < padctl->num_functions; i++)
		if (strcmp(name, padctl->functions[i]) == 0)
			return i;

	return -ENOENT;
}

static int
tegra_xusb_padctl_lane_find_function(struct tegra_xusb_padctl *padctl,
				     const struct tegra_xusb_padctl_lane *lane,
				     const char *name)
{
	unsigned int i;
	int func;

	func = tegra_xusb_padctl_find_function(padctl, name);
	if (func < 0)
		return func;

	for (i = 0; i < lane->num_funcs; i++)
		if (lane->funcs[i] == func)
			return i;

	return -ENOENT;
}

static int
tegra_xusb_padctl_group_apply(struct tegra_xusb_padctl *padctl,
			      const struct tegra_xusb_padctl_group *group)
{
	unsigned int i;

	for (i = 0; i < group->num_pins; i++) {
		const struct tegra_xusb_padctl_lane *lane;
		unsigned int func;
		u32 value;

		lane = tegra_xusb_padctl_find_lane(padctl, group->pins[i]);
		if (!lane) {
			error("tegra-xusb-padctl: no lane for pin %s",
			      group->pins[i]);
			continue;
		}

		func = tegra_xusb_padctl_lane_find_function(padctl, lane,
							    group->func);
		if (func < 0) {
			error("tegra-xusb-padctl: function %s invalid for lane %s: %d",
			      group->func, lane->name, func);
			continue;
		}

		value = padctl_readl(padctl, lane->offset);

		/* set pin function */
		value &= ~(lane->mask << lane->shift);
		value |= func << lane->shift;

		/*
		 * Set IDDQ if supported on the lane and specified in the
		 * configuration.
		 */
		if (lane->iddq > 0 && group->iddq >= 0) {
			if (group->iddq != 0)
				value &= ~(1 << lane->iddq);
			else
				value |= 1 << lane->iddq;
		}

		padctl_writel(padctl, value, lane->offset);
	}

	return 0;
}

static int
tegra_xusb_padctl_config_apply(struct tegra_xusb_padctl *padctl,
			       struct tegra_xusb_padctl_config *config)
{
	unsigned int i;

	for (i = 0; i < config->num_groups; i++) {
		const struct tegra_xusb_padctl_group *group;
		int err;

		group = &config->groups[i];

		err = tegra_xusb_padctl_group_apply(padctl, group);
		if (err < 0) {
			error("tegra-xusb-padctl: failed to apply group %s: %d",
			      group->name, err);
			continue;
		}
	}

	return 0;
}

static int
tegra_xusb_padctl_config_parse_dt(struct tegra_xusb_padctl *padctl,
				  struct tegra_xusb_padctl_config *config,
				  const void *fdt, int node)
{
	int subnode;

	config->name = fdt_get_name(fdt, node, NULL);

	fdt_for_each_subnode(fdt, subnode, node) {
		struct tegra_xusb_padctl_group *group;
		int err;

		group = &config->groups[config->num_groups];

		err = tegra_xusb_padctl_group_parse_dt(padctl, group, fdt,
						       subnode);
		if (err < 0) {
			error("tegra-xusb-padctl: failed to parse group %s",
			      group->name);
			return err;
		}

		config->num_groups++;
	}

	return 0;
}

static int tegra_xusb_padctl_parse_dt(struct tegra_xusb_padctl *padctl,
				      const void *fdt, int node)
{
	int subnode, err;

	err = fdt_get_resource(fdt, node, "reg", 0, &padctl->regs);
	if (err < 0) {
		error("tegra-xusb-padctl: registers not found");
		return err;
	}

	fdt_for_each_subnode(fdt, subnode, node) {
		struct tegra_xusb_padctl_config *config = &padctl->config;

		err = tegra_xusb_padctl_config_parse_dt(padctl, config, fdt,
							subnode);
		if (err < 0) {
			error("tegra-xusb-padctl: failed to parse entry %s: %d",
			      config->name, err);
			continue;
		}
	}

	return 0;
}

static int process_nodes(const void *fdt, int nodes[], unsigned int count)
{
	unsigned int i;

	for (i = 0; i < count; i++) {
		enum fdt_compat_id id;
		int err;

		if (!fdtdec_get_is_enabled(fdt, nodes[i]))
			continue;

		id = fdtdec_lookup(fdt, nodes[i]);
		switch (id) {
		case COMPAT_NVIDIA_TEGRA124_XUSB_PADCTL:
			break;

		default:
			error("tegra-xusb-padctl: unsupported compatible: %s",
			      fdtdec_get_compatible(id));
			continue;
		}

		padctl->num_lanes = ARRAY_SIZE(tegra124_lanes);
		padctl->lanes = tegra124_lanes;

		padctl->num_functions = ARRAY_SIZE(tegra124_functions);
		padctl->functions = tegra124_functions;

		err = tegra_xusb_padctl_parse_dt(padctl, fdt, nodes[i]);
		if (err < 0) {
			error("tegra-xusb-padctl: failed to parse DT: %d",
			      err);
			continue;
		}

		/* deassert XUSB padctl reset */
		reset_set_enable(PERIPH_ID_XUSB_PADCTL, 0);

		err = tegra_xusb_padctl_config_apply(padctl, &padctl->config);
		if (err < 0) {
			error("tegra-xusb-padctl: failed to apply pinmux: %d",
			      err);
			continue;
		}

		/* only a single instance is supported */
		break;
	}

	return 0;
}

struct tegra_xusb_phy *tegra_xusb_phy_get(unsigned int type)
{
	struct tegra_xusb_phy *phy = NULL;

	switch (type) {
	case TEGRA_XUSB_PADCTL_PCIE:
		phy = &padctl->phys[0];
		phy->padctl = padctl;
		break;

	case TEGRA_XUSB_PADCTL_SATA:
		phy = &padctl->phys[1];
		phy->padctl = padctl;
		break;
	}

	return phy;
}

int tegra_xusb_phy_prepare(struct tegra_xusb_phy *phy)
{
	if (phy && phy->ops && phy->ops->prepare)
		return phy->ops->prepare(phy);

	return phy ? -ENOSYS : -EINVAL;
}

int tegra_xusb_phy_enable(struct tegra_xusb_phy *phy)
{
	if (phy && phy->ops && phy->ops->enable)
		return phy->ops->enable(phy);

	return phy ? -ENOSYS : -EINVAL;
}

int tegra_xusb_phy_disable(struct tegra_xusb_phy *phy)
{
	if (phy && phy->ops && phy->ops->disable)
		return phy->ops->disable(phy);

	return phy ? -ENOSYS : -EINVAL;
}

int tegra_xusb_phy_unprepare(struct tegra_xusb_phy *phy)
{
	if (phy && phy->ops && phy->ops->unprepare)
		return phy->ops->unprepare(phy);

	return phy ? -ENOSYS : -EINVAL;
}

void tegra_xusb_padctl_init(const void *fdt)
{
	int count, nodes[1];

	count = fdtdec_find_aliases_for_id(fdt, "padctl",
					   COMPAT_NVIDIA_TEGRA124_XUSB_PADCTL,
					   nodes, ARRAY_SIZE(nodes));
	if (process_nodes(fdt, nodes, count))
		return;
}
