/*
 * Copyright 2013 Freescale Semiconductor, Inc.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <asm/processor.h>
#include <asm/mmu.h>
#include <asm/cache.h>
#include <asm/immap_85xx.h>
#include <asm/io.h>
#include <miiphy.h>
#include <libfdt.h>
#include <fdt_support.h>
#include <fsl_mdio.h>
#include <tsec.h>
#include <mmc.h>
#include <netdev.h>
#include <pci.h>
#include <asm/fsl_ifc.h>
#include <asm/fsl_pci.h>

#include "cpld.h"

DECLARE_GLOBAL_DATA_PTR;

int checkboard(void)
{
	struct cpu_type *cpu = gd->arch.cpu;
	struct cpld_data *cpld_data = (void *)(CONFIG_SYS_CPLD_BASE);

	printf("Board: %sPCIe, ", cpu->name);
	printf("CPLD Ver: 0x%02x\n", in_8(&cpld_data->cpldver));

	return 0;
}

int board_early_init_f(void)
{
	struct fsl_ifc *ifc = (void *)CONFIG_SYS_IFC_ADDR;

	/* Clock configuration to access CPLD using IFC(GPCM) */
	setbits_be32(&ifc->ifc_gcr, 1 << IFC_GCR_TBCTL_TRN_TIME_SHIFT);

	return 0;
}

int board_early_init_r(void)
{
	const unsigned long flashbase = CONFIG_SYS_FLASH_BASE;
	const u8 flash_esel = find_tlb_idx((void *)flashbase, 1);

	/*
	 * Remap Boot flash region to caching-inhibited
	 * so that flash can be erased properly.
	 */

	/* Flush d-cache and invalidate i-cache of any FLASH data */
	flush_dcache();
	invalidate_icache();

	/* invalidate existing TLB entry for flash */
	disable_tlb(flash_esel);

	set_tlb(1, flashbase, CONFIG_SYS_FLASH_BASE_PHYS,
			MAS3_SW|MAS3_SR, MAS2_I|MAS2_G,
			0, flash_esel, BOOKE_PAGESZ_64M, 1);

	return 0;
}

#ifdef CONFIG_PCI
void pci_init_board(void)
{
	fsl_pcie_init_board(0);
}
#endif /* ifdef CONFIG_PCI */

#ifdef CONFIG_TSEC_ENET
int board_eth_init(bd_t *bis)
{
	struct fsl_pq_mdio_info mdio_info;
	struct tsec_info_struct tsec_info[2];
	int num = 0;

#ifdef CONFIG_TSEC1
	SET_STD_TSEC_INFO(tsec_info[num], 1);
	num++;
#endif
#ifdef CONFIG_TSEC2
	SET_STD_TSEC_INFO(tsec_info[num], 2);
	num++;
#endif
	if (!num) {
		printf("No TSECs initialized\n");
		return 0;
	}

	/* Register 1G MDIO bus */
	mdio_info.regs = (struct tsec_mii_mng *)CONFIG_SYS_MDIO_BASE_ADDR;
	mdio_info.name = DEFAULT_MII_NAME;

	fsl_pq_mdio_init(bis, &mdio_info);

	tsec_eth_init(bis, tsec_info, num);

	return pci_eth_init(bis);
}
#endif

#if defined(CONFIG_OF_BOARD_SETUP)
void fdt_del_sec(void *blob, int offset)
{
	int nodeoff = 0;

	while ((nodeoff = fdt_node_offset_by_compat_reg(blob, "fsl,sec-v6.0",
			CONFIG_SYS_CCSRBAR_PHYS + CONFIG_SYS_FSL_SEC_OFFSET
			+ offset * 0x20000)) >= 0) {
		fdt_del_node(blob, nodeoff);
		offset++;
	}
}

void ft_board_setup(void *blob, bd_t *bd)
{
	phys_addr_t base;
	phys_size_t size;
	struct cpu_type *cpu;

	cpu = gd->arch.cpu;

	ft_cpu_setup(blob, bd);

	base = getenv_bootm_low();
	size = getenv_bootm_size();

#if defined(CONFIG_PCI)
	FT_FSL_PCI_SETUP;
#endif

	fdt_fixup_memory(blob, (u64)base, (u64)size);
	if (cpu->soc_ver == SVR_C291)
		fdt_del_sec(blob, 1);
	else if (cpu->soc_ver == SVR_C292)
		fdt_del_sec(blob, 2);
}
#endif
