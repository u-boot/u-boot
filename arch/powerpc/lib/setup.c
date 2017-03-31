/*
 * Copyright (c) 2017 Google, Inc
 *
 * SPDX-License-Identifier:     GPL-2.0+
 */

#include <common.h>
#include <version.h>

DECLARE_GLOBAL_DATA_PTR;

int setup_board_extra(void)
{
	bd_t *bd = gd->bd;

	strncpy((char *)bd->bi_s_version, "1.2", sizeof(bd->bi_s_version));
	strncpy((char *)bd->bi_r_version, U_BOOT_VERSION,
		sizeof(bd->bi_r_version));

	bd->bi_procfreq = gd->cpu_clk;	/* Processor Speed, In Hz */
	bd->bi_plb_busfreq = gd->bus_clk;
#if defined(CONFIG_405GP) || defined(CONFIG_405EP) || \
		defined(CONFIG_440EP) || defined(CONFIG_440GR) || \
		defined(CONFIG_440EPX) || defined(CONFIG_440GRX)
	bd->bi_pci_busfreq = get_PCI_freq();
	bd->bi_opbfreq = get_OPB_freq();
#elif defined(CONFIG_XILINX_405)
	bd->bi_pci_busfreq = get_PCI_freq();
#endif

	return 0;
}
