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

void __weak board_detail(void)
{
	/* Please define board_detail() for your PPC platform */
}

void arch_print_bdinfo(void)
{
	struct bd_info *bd = gd->bd;

#if defined(CONFIG_SYS_INIT_RAM_ADDR)
	bdinfo_print_num("sramstart", (ulong)bd->bi_sramstart);
	bdinfo_print_num("sramsize", (ulong)bd->bi_sramsize);
#endif
	bdinfo_print_mhz("busfreq", bd->bi_busfreq);
#if defined(CONFIG_MPC8xx) || defined(CONFIG_E500)
	bdinfo_print_num("immr_base", bd->bi_immr_base);
#endif
	bdinfo_print_num("bootflags", bd->bi_bootflags);
	bdinfo_print_mhz("intfreq", bd->bi_intfreq);
#ifdef CONFIG_ENABLE_36BIT_PHYS
	if (IS_ENABLED(CONFIG_PHYS_64BIT))
		puts("addressing  = 36-bit\n");
	else
		puts("addressing  = 32-bit\n");
#endif
	board_detail();
#if defined(CONFIG_CPM2)
	bdinfo_print_mhz("cpmfreq", bd->bi_cpmfreq);
	bdinfo_print_mhz("vco", bd->bi_vco);
	bdinfo_print_mhz("sccfreq", bd->bi_sccfreq);
	bdinfo_print_mhz("brgfreq", bd->bi_brgfreq);
#endif
}
