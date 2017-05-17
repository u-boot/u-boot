/*
 * Copyright (C) 2017 Álvaro Fernández Rojas <noltari@gmail.com>
 *
 * Derived from linux/arch/mips/bcm63xx/cpu.c:
 *	Copyright (C) 2008 Maxime Bizon <mbizon@freebox.fr>
 *	Copyright (C) 2009 Florian Fainelli <florian@openwrt.org>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <dm.h>
#include <errno.h>
#include <ram.h>
#include <asm/io.h>

#define SDRAM_CFG_REG		0x0
#define SDRAM_CFG_COL_SHIFT	4
#define SDRAM_CFG_COL_MASK	(0x3 << SDRAM_CFG_COL_SHIFT)
#define SDRAM_CFG_ROW_SHIFT	6
#define SDRAM_CFG_ROW_MASK	(0x3 << SDRAM_CFG_ROW_SHIFT)
#define SDRAM_CFG_32B_SHIFT	10
#define SDRAM_CFG_32B_MASK	(1 << SDRAM_CFG_32B_SHIFT)
#define SDRAM_CFG_BANK_SHIFT	13
#define SDRAM_CFG_BANK_MASK	(1 << SDRAM_CFG_BANK_SHIFT)

#define MEMC_CFG_REG		0x4
#define MEMC_CFG_32B_SHIFT	1
#define MEMC_CFG_32B_MASK	(1 << MEMC_CFG_32B_SHIFT)
#define MEMC_CFG_COL_SHIFT	3
#define MEMC_CFG_COL_MASK	(0x3 << MEMC_CFG_COL_SHIFT)
#define MEMC_CFG_ROW_SHIFT	6
#define MEMC_CFG_ROW_MASK	(0x3 << MEMC_CFG_ROW_SHIFT)

#define DDR_CSEND_REG		0x8

struct bmips_ram_priv;

struct bmips_ram_hw {
	ulong (*get_ram_size)(struct bmips_ram_priv *);
};

struct bmips_ram_priv {
	void __iomem *regs;
	const struct bmips_ram_hw *hw;
};

static ulong bcm6328_get_ram_size(struct bmips_ram_priv *priv)
{
	return readl_be(priv->regs + DDR_CSEND_REG) << 24;
}

static ulong bmips_dram_size(unsigned int cols, unsigned int rows,
			     unsigned int is_32b, unsigned int banks)
{
	rows += 11; /* 0 => 11 address bits ... 2 => 13 address bits */
	cols += 8; /* 0 => 8 address bits ... 2 => 10 address bits */
	is_32b += 1;

	return 1 << (cols + rows + is_32b + banks);
}

static ulong bcm6338_get_ram_size(struct bmips_ram_priv *priv)
{
	unsigned int cols = 0, rows = 0, is_32b = 0, banks = 0;
	u32 val;

	val = readl_be(priv->regs + SDRAM_CFG_REG);
	rows = (val & SDRAM_CFG_ROW_MASK) >> SDRAM_CFG_ROW_SHIFT;
	cols = (val & SDRAM_CFG_COL_MASK) >> SDRAM_CFG_COL_SHIFT;
	is_32b = (val & SDRAM_CFG_32B_MASK) ? 1 : 0;
	banks = (val & SDRAM_CFG_BANK_MASK) ? 2 : 1;

	return bmips_dram_size(cols, rows, is_32b, banks);
}

static ulong bcm6358_get_ram_size(struct bmips_ram_priv *priv)
{
	unsigned int cols = 0, rows = 0, is_32b = 0;
	u32 val;

	val = readl_be(priv->regs + MEMC_CFG_REG);
	rows = (val & MEMC_CFG_ROW_MASK) >> MEMC_CFG_ROW_SHIFT;
	cols = (val & MEMC_CFG_COL_MASK) >> MEMC_CFG_COL_SHIFT;
	is_32b = (val & MEMC_CFG_32B_MASK) ? 0 : 1;

	return bmips_dram_size(cols, rows, is_32b, 2);
}

static int bmips_ram_get_info(struct udevice *dev, struct ram_info *info)
{
	struct bmips_ram_priv *priv = dev_get_priv(dev);
	const struct bmips_ram_hw *hw = priv->hw;

	info->base = 0x80000000;
	info->size = hw->get_ram_size(priv);

	return 0;
}

static const struct ram_ops bmips_ram_ops = {
	.get_info = bmips_ram_get_info,
};

static const struct bmips_ram_hw bmips_ram_bcm6328 = {
	.get_ram_size = bcm6328_get_ram_size,
};

static const struct bmips_ram_hw bmips_ram_bcm6338 = {
	.get_ram_size = bcm6338_get_ram_size,
};

static const struct bmips_ram_hw bmips_ram_bcm6358 = {
	.get_ram_size = bcm6358_get_ram_size,
};

static const struct udevice_id bmips_ram_ids[] = {
	{
		.compatible = "brcm,bcm6328-mc",
		.data = (ulong)&bmips_ram_bcm6328,
	}, {
		.compatible = "brcm,bcm6338-mc",
		.data = (ulong)&bmips_ram_bcm6338,
	}, {
		.compatible = "brcm,bcm6358-mc",
		.data = (ulong)&bmips_ram_bcm6358,
	}, { /* sentinel */ }
};

static int bmips_ram_probe(struct udevice *dev)
{
	struct bmips_ram_priv *priv = dev_get_priv(dev);
	const struct bmips_ram_hw *hw =
		(const struct bmips_ram_hw *)dev_get_driver_data(dev);
	fdt_addr_t addr;
	fdt_size_t size;

	addr = devfdt_get_addr_size_index(dev, 0, &size);
	if (addr == FDT_ADDR_T_NONE)
		return -EINVAL;

	priv->regs = ioremap(addr, size);
	priv->hw = hw;

	return 0;
}

U_BOOT_DRIVER(bmips_ram) = {
	.name = "bmips-mc",
	.id = UCLASS_RAM,
	.of_match = bmips_ram_ids,
	.probe = bmips_ram_probe,
	.priv_auto_alloc_size = sizeof(struct bmips_ram_priv),
	.ops = &bmips_ram_ops,
	.flags = DM_FLAG_PRE_RELOC,
};
