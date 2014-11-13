/*
 * From coreboot southbridge/intel/bd82x6x/lpc.c
 *
 * Copyright (C) 2008-2009 coresystems GmbH
 *
 * SPDX-License-Identifier:	GPL-2.0
 */

#include <common.h>
#include <errno.h>
#include <fdtdec.h>
#include <pci.h>
#include <asm/pci.h>
#include <asm/arch/pch.h>

int lpc_early_init(const void *blob, int node, pci_dev_t dev)
{
	struct reg_info {
		u32 base;
		u32 size;
	} values[4], *ptr;
	int count;
	int i;

	count = fdtdec_get_int_array_count(blob, node, "gen-dec",
			(u32 *)values, sizeof(values) / sizeof(u32));
	if (count < 0)
		return -EINVAL;

	/* Set COM1/COM2 decode range */
	pci_write_config16(dev, LPC_IO_DEC, 0x0010);

	/* Enable PS/2 Keyboard/Mouse, EC areas and COM1 */
	pci_write_config16(dev, LPC_EN, KBC_LPC_EN | MC_LPC_EN |
			   GAMEL_LPC_EN | COMA_LPC_EN);

	/* Write all registers but use 0 if we run out of data */
	count = count * sizeof(u32) / sizeof(values[0]);
	for (i = 0, ptr = values; i < ARRAY_SIZE(values); i++, ptr++) {
		u32 reg = 0;

		if (i < count)
			reg = ptr->base | PCI_COMMAND_IO | (ptr->size << 16);
		pci_write_config32(dev, LPC_GENX_DEC(i), reg);
	}

	return 0;
}
