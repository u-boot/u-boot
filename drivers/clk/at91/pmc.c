// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2016 Atmel Corporation
 *               Wenyou.Yang <wenyou.yang@atmel.com>
 */

#include <common.h>
#include <clk-uclass.h>
#include <dm.h>
#include <log.h>
#include <dm/lists.h>
#include <dm/util.h>
#include "pmc.h"

DECLARE_GLOBAL_DATA_PTR;

static const struct udevice_id at91_pmc_match[] = {
	{ .compatible = "atmel,at91rm9200-pmc" },
	{ .compatible = "atmel,at91sam9260-pmc" },
	{ .compatible = "atmel,at91sam9g45-pmc" },
	{ .compatible = "atmel,at91sam9n12-pmc" },
	{ .compatible = "atmel,at91sam9x5-pmc" },
	{ .compatible = "atmel,sama5d3-pmc" },
	{ .compatible = "atmel,sama5d2-pmc" },
	{}
};

U_BOOT_DRIVER(atmel_at91rm9200_pmc) = {
	.name = "atmel_at91rm9200_pmc",
	.id = UCLASS_SIMPLE_BUS,
	.of_match = at91_pmc_match,
};

U_BOOT_DRIVER_ALIAS(atmel_at91rm9200_pmc, atmel_at91sam9260_pmc)

/*---------------------------------------------------------*/

int at91_pmc_core_probe(struct udevice *dev)
{
	struct pmc_platdata *plat = dev_get_platdata(dev);

	dev = dev_get_parent(dev);

	plat->reg_base = dev_read_addr_ptr(dev);

	return 0;
}

/**
 * at91_clk_sub_device_bind() - for the at91 clock driver
 * Recursively bind its children as clk devices.
 *
 * @return: 0 on success, or negative error code on failure
 */
int at91_clk_sub_device_bind(struct udevice *dev, const char *drv_name)
{
	const void *fdt = gd->fdt_blob;
	int offset = dev_of_offset(dev);
	bool pre_reloc_only = !(gd->flags & GD_FLG_RELOC);
	const char *name;
	int ret;

	for (offset = fdt_first_subnode(fdt, offset);
	     offset > 0;
	     offset = fdt_next_subnode(fdt, offset)) {
		if (pre_reloc_only &&
		    !ofnode_pre_reloc(offset_to_ofnode(offset)))
			continue;
		/*
		 * If this node has "compatible" property, this is not
		 * a clock sub-node, but a normal device. skip.
		 */
		fdt_get_property(fdt, offset, "compatible", &ret);
		if (ret >= 0)
			continue;

		if (ret != -FDT_ERR_NOTFOUND)
			return ret;

		name = fdt_get_name(fdt, offset, NULL);
		if (!name)
			return -EINVAL;
		ret = device_bind_driver_to_node(dev, drv_name, name,
					offset_to_ofnode(offset), NULL);
		if (ret)
			return ret;
	}

	return 0;
}

int at91_clk_of_xlate(struct clk *clk, struct ofnode_phandle_args *args)
{
	int periph;

	if (args->args_count) {
		debug("Invalid args_count: %d\n", args->args_count);
		return -EINVAL;
	}

	periph = fdtdec_get_uint(gd->fdt_blob, dev_of_offset(clk->dev), "reg",
				 -1);
	if (periph < 0)
		return -EINVAL;

	clk->id = periph;

	return 0;
}

int at91_clk_probe(struct udevice *dev)
{
	struct udevice *dev_periph_container, *dev_pmc;
	struct pmc_platdata *plat = dev_get_platdata(dev);

	dev_periph_container = dev_get_parent(dev);
	dev_pmc = dev_get_parent(dev_periph_container);

	plat->reg_base = dev_read_addr_ptr(dev_pmc);

	return 0;
}

/**
 * pmc_read() - read content at address base + off into val
 *
 * @base: base address
 * @off: offset to read from
 * @val: where the content of base + off is stored
 *
 * @return: void
 */
void pmc_read(void __iomem *base, unsigned int off, unsigned int *val)
{
	*val = readl(base + off);
}

/**
 * pmc_write() - write content of val at address base + off
 *
 * @base: base address
 * @off: offset to write to
 * @val: content to be written at base + off
 *
 * @return: void
 */
void pmc_write(void __iomem *base, unsigned int off, unsigned int val)
{
	writel(val, base + off);
}

/**
 * pmc_update_bits() - update a set of bits at address base + off
 *
 * @base: base address
 * @off: offset to be updated
 * @mask: mask of bits to be updated
 * @bits: the new value to be updated
 *
 * @return: void
 */
void pmc_update_bits(void __iomem *base, unsigned int off,
		     unsigned int mask, unsigned int bits)
{
	unsigned int tmp;

	tmp = readl(base + off);
	tmp &= ~mask;
	writel(tmp | (bits & mask), base + off);
}

/**
 * at91_clk_mux_val_to_index() - get parent index in mux table
 *
 * @table: clock mux table
 * @num_parents: clock number of parents
 * @val: clock id who's mux index should be retrieved
 *
 * @return: clock index in mux table or a negative error number in case of
 *		failure
 */
int at91_clk_mux_val_to_index(const u32 *table, u32 num_parents, u32 val)
{
	int i;

	if (!table || !num_parents)
		return -EINVAL;

	for (i = 0; i < num_parents; i++) {
		if (table[i] == val)
			return i;
	}

	return -EINVAL;
}

/**
 * at91_clk_mux_index_to_val() - get parent ID corresponding to an entry in
 *	clock's mux table
 *
 * @table: clock's mux table
 * @num_parents: clock's number of parents
 * @index: index in mux table which clock's ID should be retrieved
 *
 * @return: clock ID or a negative error number in case of failure
 */
int at91_clk_mux_index_to_val(const u32 *table, u32 num_parents, u32 index)
{
	if (!table || !num_parents || index < 0 || index > num_parents)
		return -EINVAL;

	return table[index];
}
