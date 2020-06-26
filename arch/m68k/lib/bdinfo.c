// SPDX-License-Identifier: GPL-2.0+
/*
 * PPC-specific information for the 'bd' command
 *
 * (C) Copyright 2003
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 */

#include <common.h>
#include <init.h>

DECLARE_GLOBAL_DATA_PTR;

void arch_print_bdinfo(void)
{
	struct bd_info *bd = gd->bd;

#if defined(CONFIG_SYS_INIT_RAM_ADDR)
	bdinfo_print_num("sramstart", (ulong)bd->bi_sramstart);
	bdinfo_print_num("sramsize", (ulong)bd->bi_sramsize);
#endif
	bdinfo_print_mhz("busfreq", bd->bi_busfreq);
#if defined(CONFIG_SYS_MBAR)
	bdinfo_print_num("mbar", bd->bi_mbar_base);
#endif
	bdinfo_print_mhz("cpufreq", bd->bi_intfreq);
	if (IS_ENABLED(CONFIG_PCI))
		bdinfo_print_mhz("pcifreq", bd->bi_pcifreq);
#ifdef CONFIG_EXTRA_CLOCK
	bdinfo_print_mhz("flbfreq", bd->bi_flbfreq);
	bdinfo_print_mhz("inpfreq", bd->bi_inpfreq);
	bdinfo_print_mhz("vcofreq", bd->bi_vcofreq);
#endif
}
