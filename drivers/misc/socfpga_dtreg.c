// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2024 Intel Corporation <www.intel.com>
 */

#include <dm.h>
#include <errno.h>
#include <asm/io.h>
#include <linux/sizes.h>

#define NUMBER_OF_ELEMENTS 3

static int socfpga_dtreg_probe(struct udevice *dev)
{
	const fdt32_t *list;
	fdt_addr_t offset, base;
	fdt_val_t val, read_val, mask, set_mask;
	int size, i;
	u32 blk_sz, reg;
	ofnode node;
	const char *name = NULL;

	debug("%s(dev=%p)\n", __func__, dev);

	if (!dev_has_ofnode(dev))
		return 0;

	dev_for_each_subnode(node, dev) {
		name = ofnode_get_name(node);
		if (!name)
			return -EINVAL;

		if (ofnode_read_u32_index(node, "reg", 1, &blk_sz))
			return -EINVAL;

		base = ofnode_get_addr(node);
		if (base == FDT_ADDR_T_NONE)
			return -EINVAL;

		debug("%s(node_offset 0x%lx node_name %s ", __func__,
		      node.of_offset, name);
		debug("node addr 0x%llx blk sz 0x%x)\n", base, blk_sz);

		list = ofnode_read_prop(node, "intel,offset-settings", &size);
		if (!list)
			return -EINVAL;

		debug("%s(intel,offset-settings property size=%x)\n", __func__,
		      size);
		size /= sizeof(*list) * NUMBER_OF_ELEMENTS;

		/*
		 * First element: offset
		 * Second element: val
		 * Third element: mask
		 */
		for (i = 0; i < size; i++) {
			offset = fdt32_to_cpu(*list++);
			val = fdt32_to_cpu(*list++);

			/* Reads the masking bit value from the list */
			mask = fdt32_to_cpu(*list++);

			/*
			 * Reads out the offsets, value and masking bits
			 * Ex: <0x00000000 0x00000230 0xffffffff>
			 */
			debug("%s(intel,offset-settings 0x%llx : 0x%llx : 0x%llx)\n",
			      __func__, offset, val, mask);

			if (blk_sz < offset + SZ_4) {
				printf("%s: Overflow as offset 0x%llx or reg",
				       __func__, offset);
				printf(" write is more than block size 0x%x\n",
				       blk_sz);
				return -EINVAL;
			}

			reg = base + offset;

			if (mask != 0) {
				if (mask == 0xffffffff) {
					writel(val, (uintptr_t)reg);
				} else {
					/* Mask the value with the masking bits */
					set_mask = val & mask;

					/* Clears and sets specific bits in the register */
					clrsetbits_le32((uintptr_t)reg, mask, set_mask);
				}
			}

			read_val = readl((uintptr_t)reg);

			/* Reads out the register, masked value and the read value */
			debug("%s(reg 0x%x = wr : 0x%llx  rd : 0x%llx)\n",
			      __func__, reg, set_mask, read_val);
		}
	}

	return 0;
};

static const struct udevice_id socfpga_dtreg_ids[] = {
	{.compatible = "intel,socfpga-dtreg"},
	{ }
};

U_BOOT_DRIVER(socfpga_dtreg) = {
	.name		= "socfpga-dtreg",
	.id		= UCLASS_NOP,
	.of_match	= socfpga_dtreg_ids,
	.probe		= socfpga_dtreg_probe,
};
