// SPDX-License-Identifier: GPL-2.0+

#include <common.h>
#include <malloc.h>
#include <errno.h>
#include <fsl_ddr.h>
#include <fdt_support.h>
#include <asm/global_data.h>
#include <linux/libfdt.h>
#include <env_internal.h>
#include <asm/arch-fsl-layerscape/soc.h>
#include <asm/arch-fsl-layerscape/fsl_icid.h>
#include <i2c.h>
#include <asm/arch/soc.h>
#include <fsl_immap.h>
#include <netdev.h>

#include <fdtdec.h>
#include <miiphy.h>

DECLARE_GLOBAL_DATA_PTR;

int board_init(void)
{
	if (CONFIG_IS_ENABLED(FSL_CAAM))
		sec_init();

	return 0;
}

int board_eth_init(struct bd_info *bis)
{
	return pci_eth_init(bis);
}

int checkboard(void)
{
	printf("EL:    %d\n", current_el());
	return 0;
}

void detail_board_ddr_info(void)
{
	puts("\nDDR    ");
	print_size(gd->bd->bi_dram[0].size + gd->bd->bi_dram[1].size, "");
	print_ddr_info(0);
}

int ft_board_setup(void *blob, struct bd_info *bd)
{
	u64 base[CONFIG_NR_DRAM_BANKS];
	u64 size[CONFIG_NR_DRAM_BANKS];
	int nbanks = CONFIG_NR_DRAM_BANKS;
	int node;
	int i;

	ft_cpu_setup(blob, bd);

	/* fixup DT for the two GPP DDR banks */
	for (i = 0; i < nbanks; i++) {
		base[i] = gd->bd->bi_dram[i].start;
		size[i] = gd->bd->bi_dram[i].size;
	}

	fdt_fixup_memory_banks(blob, base, size, nbanks);

	fdt_fixup_icid(blob);

	if (CONFIG_IS_ENABLED(SL28_SPL_LOADS_OPTEE_BL32)) {
		node = fdt_node_offset_by_compatible(blob, -1, "linaro,optee-tz");
		if (node)
			fdt_set_node_status(blob, node, FDT_STATUS_OKAY, 0);
	}

	return 0;
}
