// SPDX-License-Identifier: GPL-2.0
/*
 * board/renesas/rcar-common/common.c
 *
 * Copyright (C) 2013 Renesas Electronics Corporation
 * Copyright (C) 2013 Nobuhiro Iwamatsu <nobuhiro.iwamatsu.yj@renesas.com>
 * Copyright (C) 2015 Nobuhiro Iwamatsu <iwamatsu@nigauri.org>
 */

#include <common.h>
#include <dm.h>
#include <dm/uclass-internal.h>
#include <asm/arch/rmobile.h>

#ifdef CONFIG_RCAR_GEN3

DECLARE_GLOBAL_DATA_PTR;

/* If the firmware passed a device tree use it for U-Boot DRAM setup. */
extern u64 rcar_atf_boot_args[];

int dram_init(void)
{
	const void *atf_fdt_blob = (const void *)(rcar_atf_boot_args[1]);
	const void *blob;

	/* Check if ATF passed us DTB. If not, fall back to builtin DTB. */
	if (fdt_magic(atf_fdt_blob) == FDT_MAGIC)
		blob = atf_fdt_blob;
	else
		blob = gd->fdt_blob;

	return fdtdec_setup_mem_size_base_fdt(blob);
}

int dram_init_banksize(void)
{
	const void *atf_fdt_blob = (const void *)(rcar_atf_boot_args[1]);
	const void *blob;

	/* Check if ATF passed us DTB. If not, fall back to builtin DTB. */
	if (fdt_magic(atf_fdt_blob) == FDT_MAGIC)
		blob = atf_fdt_blob;
	else
		blob = gd->fdt_blob;

	fdtdec_setup_memory_banksize_fdt(blob);

	return 0;
}

#if CONFIG_IS_ENABLED(OF_BOARD_SETUP) && CONFIG_IS_ENABLED(PCI)
int ft_board_setup(void *blob, bd_t *bd)
{
	struct udevice *dev;
	struct uclass *uc;
	fdt_addr_t regs_addr;
	int i, off, ret;

	ret = uclass_get(UCLASS_PCI, &uc);
	if (ret)
		return ret;

	uclass_foreach_dev(dev, uc) {
		struct pci_controller hose = { 0 };

		for (i = 0; i < CONFIG_NR_DRAM_BANKS; i++) {
			if (hose.region_count == MAX_PCI_REGIONS) {
				printf("maximum number of regions parsed, aborting\n");
				break;
			}

			if (bd->bi_dram[i].size) {
				pci_set_region(&hose.regions[hose.region_count++],
					       bd->bi_dram[i].start,
					       bd->bi_dram[i].start,
					       bd->bi_dram[i].size,
					       PCI_REGION_MEM |
					       PCI_REGION_PREFETCH |
					       PCI_REGION_SYS_MEMORY);
			}
		}

		regs_addr = devfdt_get_addr_index(dev, 0);
		off = fdt_node_offset_by_compat_reg(blob,
				"renesas,pcie-rcar-gen3", regs_addr);
		if (off < 0) {
			printf("Failed to find PCIe node@%llx\n", regs_addr);
			return off;
		}

		fdt_pci_dma_ranges(blob, off, &hose);
	}

	return 0;
}
#endif
#endif
