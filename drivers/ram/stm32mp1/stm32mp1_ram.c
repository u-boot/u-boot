// SPDX-License-Identifier: GPL-2.0+ OR BSD-3-Clause
/*
 * Copyright (C) 2018, STMicroelectronics - All Rights Reserved
 */

#define LOG_CATEGORY UCLASS_RAM

#include <common.h>
#include <clk.h>
#include <dm.h>
#include <init.h>
#include <log.h>
#include <ram.h>
#include <regmap.h>
#include <syscon.h>
#include <asm/io.h>
#include <dm/device_compat.h>
#include "stm32mp1_ddr.h"

static const char *const clkname[] = {
	"ddrc1",
	"ddrc2",
	"ddrcapb",
	"ddrphycapb",
	"ddrphyc" /* LAST clock => used for get_rate() */
};

int stm32mp1_ddr_clk_enable(struct ddr_info *priv, uint32_t mem_speed)
{
	unsigned long ddrphy_clk;
	unsigned long ddr_clk;
	struct clk clk;
	int ret;
	unsigned int idx;

	for (idx = 0; idx < ARRAY_SIZE(clkname); idx++) {
		ret = clk_get_by_name(priv->dev, clkname[idx], &clk);

		if (!ret)
			ret = clk_enable(&clk);

		if (ret) {
			log_err("error for %s : %d\n", clkname[idx], ret);
			return ret;
		}
	}

	priv->clk = clk;
	ddrphy_clk = clk_get_rate(&priv->clk);

	log_debug("DDR: mem_speed (%d kHz), RCC %d kHz\n",
		  mem_speed, (u32)(ddrphy_clk / 1000));
	/* max 10% frequency delta */
	ddr_clk = abs(ddrphy_clk - mem_speed * 1000);
	if (ddr_clk > (mem_speed * 100)) {
		log_err("DDR expected freq %d kHz, current is %d kHz\n",
			mem_speed, (u32)(ddrphy_clk / 1000));
		return -EINVAL;
	}

	return 0;
}

__weak int board_stm32mp1_ddr_config_name_match(struct udevice *dev,
						const char *name)
{
	return 0;	/* Always match */
}

static ofnode stm32mp1_ddr_get_ofnode(struct udevice *dev)
{
	const char *name;
	ofnode node;

	dev_for_each_subnode(node, dev) {
		name = ofnode_get_property(node, "compatible", NULL);

		if (!board_stm32mp1_ddr_config_name_match(dev, name))
			return node;
	}

	return dev_ofnode(dev);
}

static __maybe_unused int stm32mp1_ddr_setup(struct udevice *dev)
{
	struct ddr_info *priv = dev_get_priv(dev);
	int ret;
	unsigned int idx;
	struct clk axidcg;
	struct stm32mp1_ddr_config config;
	ofnode node = stm32mp1_ddr_get_ofnode(dev);

#define PARAM(x, y, z)							\
	{	.name = x,						\
		.offset = offsetof(struct stm32mp1_ddr_config, y),	\
		.size = sizeof(config.y) / sizeof(u32),			\
		.present = z,						\
	}

#define CTL_PARAM(x) PARAM("st,ctl-"#x, c_##x, NULL)
#define PHY_PARAM(x) PARAM("st,phy-"#x, p_##x, NULL)
#define PHY_PARAM_OPT(x) PARAM("st,phy-"#x, p_##x, &config.p_##x##_present)

	const struct {
		const char *name; /* name in DT */
		const u32 offset; /* offset in config struct */
		const u32 size;   /* size of parameters */
		bool * const present;  /* presence indication for opt */
	} param[] = {
		CTL_PARAM(reg),
		CTL_PARAM(timing),
		CTL_PARAM(map),
		CTL_PARAM(perf),
		PHY_PARAM(reg),
		PHY_PARAM(timing),
		PHY_PARAM_OPT(cal)
	};

	config.info.speed = ofnode_read_u32_default(node, "st,mem-speed", 0);
	config.info.size = ofnode_read_u32_default(node, "st,mem-size", 0);
	config.info.name = ofnode_read_string(node, "st,mem-name");
	if (!config.info.name) {
		dev_dbg(dev, "no st,mem-name\n");
		return -EINVAL;
	}
	printf("RAM: %s\n", config.info.name);

	for (idx = 0; idx < ARRAY_SIZE(param); idx++) {
		ret = ofnode_read_u32_array(node, param[idx].name,
					 (void *)((u32)&config +
						  param[idx].offset),
					 param[idx].size);
		dev_dbg(dev, "%s: %s[0x%x] = %d\n", __func__,
			param[idx].name, param[idx].size, ret);
		if (ret &&
		    (ret != -FDT_ERR_NOTFOUND || !param[idx].present)) {
			dev_err(dev, "Cannot read %s, error=%d\n",
				param[idx].name, ret);
			return -EINVAL;
		}
		if (param[idx].present) {
			/* save presence of optional parameters */
			*param[idx].present = true;
			if (ret == -FDT_ERR_NOTFOUND) {
				*param[idx].present = false;
#ifdef CONFIG_STM32MP1_DDR_INTERACTIVE
				/* reset values if used later */
				memset((void *)((u32)&config +
						param[idx].offset),
					0, param[idx].size * sizeof(u32));
#endif
			}
		}
	}

	ret = clk_get_by_name(dev, "axidcg", &axidcg);
	if (ret) {
		dev_dbg(dev, "%s: Cannot found axidcg\n", __func__);
		return -EINVAL;
	}
	clk_disable(&axidcg); /* disable clock gating during init */

	stm32mp1_ddr_init(priv, &config);

	clk_enable(&axidcg); /* enable clock gating */

	/* check size */
	dev_dbg(dev, "get_ram_size(%x, %x)\n",
		(u32)priv->info.base, (u32)STM32_DDR_SIZE);

	priv->info.size = get_ram_size((long *)priv->info.base,
				       STM32_DDR_SIZE);

	dev_dbg(dev, "info.size: %x\n", (u32)priv->info.size);

	/* check memory access for all memory */
	if (config.info.size != priv->info.size) {
		printf("DDR invalid size : 0x%x, expected 0x%x\n",
		       priv->info.size, config.info.size);
		return -EINVAL;
	}
	return 0;
}

static int stm32mp1_ddr_probe(struct udevice *dev)
{
	struct ddr_info *priv = dev_get_priv(dev);
	struct regmap *map;
	int ret;

	priv->dev = dev;

	ret = regmap_init_mem(dev_ofnode(dev), &map);
	if (ret)
		return log_ret(ret);

	priv->ctl = regmap_get_range(map, 0);
	priv->phy = regmap_get_range(map, 1);

	priv->rcc = STM32_RCC_BASE;

	priv->info.base = STM32_DDR_BASE;

#if !defined(CONFIG_TFABOOT) && \
	(!defined(CONFIG_SPL) || defined(CONFIG_SPL_BUILD))
	priv->info.size = 0;
	ret = stm32mp1_ddr_setup(dev);

	return log_ret(ret);
#else
	ofnode node = stm32mp1_ddr_get_ofnode(dev);
	priv->info.size = ofnode_read_u32_default(node, "st,mem-size", 0);
	return 0;
#endif
}

static int stm32mp1_ddr_get_info(struct udevice *dev, struct ram_info *info)
{
	struct ddr_info *priv = dev_get_priv(dev);

	*info = priv->info;

	return 0;
}

static struct ram_ops stm32mp1_ddr_ops = {
	.get_info = stm32mp1_ddr_get_info,
};

static const struct udevice_id stm32mp1_ddr_ids[] = {
	{ .compatible = "st,stm32mp1-ddr" },
	{ }
};

U_BOOT_DRIVER(ddr_stm32mp1) = {
	.name = "stm32mp1_ddr",
	.id = UCLASS_RAM,
	.of_match = stm32mp1_ddr_ids,
	.ops = &stm32mp1_ddr_ops,
	.probe = stm32mp1_ddr_probe,
	.priv_auto	= sizeof(struct ddr_info),
};
