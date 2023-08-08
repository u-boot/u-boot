/*
 * Copyright (C) 2018 Marvell International Ltd.
 *
 * SPDX-License-Identifier:	GPL-2.0
 * https://spdx.org/licenses
 */

/* #define DEBUG */
#include <common.h>
#include <asm/io.h>
#include <errno.h>
#include <fdtdec.h>
#include <mvebu/mvebu_chip_sar.h>

#include <sar-uclass.h>

/* SAR CP1 registers */
#define SAR1_RST_PCIE0_CLOCK_CONFIG_CP1_OFFSET	(0)
#define SAR1_RST_PCIE0_CLOCK_CONFIG_CP1_MASK	\
	(0x1 << SAR1_RST_PCIE0_CLOCK_CONFIG_CP1_OFFSET)
#define SAR1_RST_PCIE1_CLOCK_CONFIG_CP1_OFFSET	(1)
#define SAR1_RST_PCIE1_CLOCK_CONFIG_CP1_MASK	\
	(0x1 << SAR1_RST_PCIE1_CLOCK_CONFIG_CP1_OFFSET)

/* SAR CP0 registers */
#define SAR1_RST_PCIE0_CLOCK_CONFIG_CP0_OFFSET	(2)
#define SAR1_RST_PCIE0_CLOCK_CONFIG_CP0_MASK	\
	(0x1 << SAR1_RST_PCIE0_CLOCK_CONFIG_CP0_OFFSET)
#define SAR1_RST_PCIE1_CLOCK_CONFIG_CP0_OFFSET	(3)
#define SAR1_RST_PCIE1_CLOCK_CONFIG_CP0_MASK	\
	(0x1 << SAR1_RST_PCIE1_CLOCK_CONFIG_CP0_OFFSET)

#define SAR1_RST_BOOT_MODE_AP_CP0_OFFSET	(4)
#define SAR1_RST_BOOT_MODE_AP_CP0_MASK		\
	(0x3f << SAR1_RST_BOOT_MODE_AP_CP0_OFFSET)

struct sar_info {
	char *name;
	u32 offset;
	u32 mask;
};

struct sar_info cp110_sar_1[] = {
	{"PCIE0 clock config   ", SAR1_RST_PCIE0_CLOCK_CONFIG_CP1_OFFSET,
				  SAR1_RST_PCIE0_CLOCK_CONFIG_CP1_MASK},
	{"PCIE1 clock config   ", SAR1_RST_PCIE1_CLOCK_CONFIG_CP1_OFFSET,
				  SAR1_RST_PCIE1_CLOCK_CONFIG_CP1_MASK},
	{"",			-1,			-1},
};

struct sar_info cp110_sar_0[] = {
	{"PCIE0 clock config   ", SAR1_RST_PCIE0_CLOCK_CONFIG_CP0_OFFSET,
				  SAR1_RST_PCIE0_CLOCK_CONFIG_CP0_MASK},
	{"PCIE1 clock config   ", SAR1_RST_PCIE1_CLOCK_CONFIG_CP0_OFFSET,
				  SAR1_RST_PCIE1_CLOCK_CONFIG_CP0_MASK},
	{"Reset Boot Mode     ", SAR1_RST_BOOT_MODE_AP_CP0_OFFSET,
				 SAR1_RST_BOOT_MODE_AP_CP0_MASK },
	{"",			-1,			-1},
};

struct bootsrc_idx_info {
	int start;
	int end;
	enum mvebu_bootsrc_type src;
	int index;
};

static struct bootsrc_idx_info bootsrc_list[] = {
	{0x0,	0x5,	BOOTSRC_NOR,		0},
	{0xA,	0x25,	BOOTSRC_NAND,		0},
	{0x28,	0x28,	BOOTSRC_AP_SD_EMMC,	0},
	{0x29,	0x29,	BOOTSRC_SD_EMMC,	0},
	{0x2A,	0x2A,	BOOTSRC_AP_SD_EMMC,	0},
	{0x2B,	0x2B,	BOOTSRC_SD_EMMC,	0},
	{0x30,	0x30,	BOOTSRC_AP_SPI,		0},
	{0x31,	0x31,	BOOTSRC_AP_SPI,		0}, /* BootRom disabled */
	{0x32,	0x33,	BOOTSRC_SPI,		1},
	{0x34,	0x35,	BOOTSRC_SPI,		0},
	{0x36,	0x37,	BOOTSRC_SPI,		1}, /* BootRom disabled */
	{-1,	-1,	-1}
};

int cp110_sar_bootsrc_get(struct udevice *dev, enum mvebu_sar_opts sar_opt,
			  struct sar_val *val)
{
	u32 reg, mode;
	int i;

	struct dm_sar_pdata *priv = dev_get_priv(dev);

	reg = readl(priv->sar_base);
	mode = (reg & SAR1_RST_BOOT_MODE_AP_CP0_MASK) >>
		SAR1_RST_BOOT_MODE_AP_CP0_OFFSET;

	val->raw_sar_val = mode;

	i = 0;
	while (bootsrc_list[i].start != -1) {
		if ((mode >= bootsrc_list[i].start) &&
		    (mode <= bootsrc_list[i].end)) {
			val->bootsrc.type = bootsrc_list[i].src;
			val->bootsrc.index = bootsrc_list[i].index;
			break;
		}
		i++;
	}

	if (bootsrc_list[i].start == -1) {
		pr_err("Bad CP110 sample at reset mode (%d).\n", mode);
		return -EINVAL;
	}
	return 0;
}

int cp110_sar_value_get(struct udevice *dev, enum mvebu_sar_opts sar_opt,
			struct sar_val *val)
{
	u32 reg, mode;
	struct dm_sar_pdata *priv = dev_get_priv(dev);

	reg = readl(priv->sar_base);

	switch (sar_opt) {
	case SAR_BOOT_SRC:
		return cp110_sar_bootsrc_get(dev, sar_opt, val);
	case SAR_CP0_PCIE0_CLK:
		mode = (reg & SAR1_RST_PCIE0_CLOCK_CONFIG_CP0_MASK) >>
			SAR1_RST_PCIE0_CLOCK_CONFIG_CP0_OFFSET;
		val->raw_sar_val = mode;
		val->clk_direction = mode;
		break;
	case SAR_CP0_PCIE1_CLK:
		mode = (reg & SAR1_RST_PCIE1_CLOCK_CONFIG_CP0_MASK) >>
			SAR1_RST_PCIE1_CLOCK_CONFIG_CP0_OFFSET;
		val->raw_sar_val = mode;
		val->clk_direction = mode;
		break;
	case SAR_CP1_PCIE0_CLK:
		mode = (reg & SAR1_RST_PCIE0_CLOCK_CONFIG_CP1_MASK) >>
			SAR1_RST_PCIE0_CLOCK_CONFIG_CP1_OFFSET;
		val->raw_sar_val = mode;
		val->clk_direction = mode;
		break;
	case SAR_CP1_PCIE1_CLK:
		mode = (reg & SAR1_RST_PCIE1_CLOCK_CONFIG_CP1_MASK) >>
			SAR1_RST_PCIE1_CLOCK_CONFIG_CP1_OFFSET;
		val->raw_sar_val = mode;
		val->clk_direction = mode;
		break;
	default:
		pr_err("AP806-SAR: Unsupported SAR option %d.\n", sar_opt);
		return -EINVAL;
	}
	return 0;
}

static int cp110_sar_dump(struct udevice *dev)
{
	u32 reg, val;
	struct sar_info *sar;
	struct dm_sar_pdata *priv = dev_get_priv(dev);

	reg = readl(priv->sar_base);
	printf("\nCP110 SAR register 0 [0x%08x]:\n", reg);
	printf("----------------------------------\n");

	if (!strcmp(priv->sar_name, "cp0-sar"))
		sar = cp110_sar_0;
	else
		sar = cp110_sar_1;

	while (sar->offset != -1) {
		val = (reg & sar->mask) >> sar->offset;
		printf("%s  0x%x\n", sar->name, val);
		sar++;
	}
	return 0;
}

static int cp110_sar_register(struct udevice *dev, u32 sar_list[], int size)
{
	int ret, i;

	for (i = 0; i < size; i++) {
		ret = mvebu_sar_id_register(dev, sar_list[i]);
		if (ret) {
			pr_err("Failed to register SAR %d, for %s.\n",
			       sar_list[i], dev->name);
			return ret;
		}
	}

	return 0;
}

int cp110_sar_init(struct udevice *dev)
{
	int ret = 0;
	struct dm_sar_pdata *priv = dev_get_priv(dev);

	u32 cp0_sar_list[] = {
		SAR_CP0_PCIE0_CLK,
		SAR_CP0_PCIE1_CLK,
		SAR_BOOT_SRC
	};

	u32 cp1_sar_list[] = {
		SAR_CP1_PCIE0_CLK,
		SAR_CP1_PCIE1_CLK,
	};

	if (!strcmp(priv->sar_name, "cp0-sar"))
		ret = cp110_sar_register(dev, cp0_sar_list,
					 ARRAY_SIZE(cp0_sar_list));
	else if (!strcmp(priv->sar_name, "cp1-sar"))
		ret = cp110_sar_register(dev, cp1_sar_list,
					 ARRAY_SIZE(cp1_sar_list));

	if (ret)
		return -EINVAL;

	return 0;
}

static const struct sar_ops cp110_sar_ops = {
	.sar_init_func = cp110_sar_init,
	.sar_value_get_func = cp110_sar_value_get,
	.sar_dump_func = cp110_sar_dump,
};

U_BOOT_DRIVER(cp110_sar) = {
	.name = "cp110_sar",
	.id = UCLASS_SAR,
	.priv_auto_alloc_size = sizeof(struct dm_sar_pdata),
	.ops = &cp110_sar_ops,
};
