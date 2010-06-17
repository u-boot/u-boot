/*
 * Copyright 2010 Freescale Semiconductor, Inc.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * Version 2 as published by the Free Software Foundation.
 */

#include <common.h>
#include <asm/fsl_lbc.h>

void print_lbc_regs(void)
{
	int i;

	printf("\nLocal Bus Controller Registers\n");
	for (i = 0; i < 8; i++) {
		printf("BR%d\t0x%08X\tOR%d\t0x%08X\n",
		       i, get_lbc_br(i), i, get_lbc_or(i));
	}
}

void init_early_memctl_regs(void)
{
	uint init_br1 = 1;

#ifdef CONFIG_MPC85xx
	/* if cs1 is already set via debugger, leave cs0/cs1 alone */
	if (get_lbc_br(1) & BR_V)
		init_br1 = 0;
#endif

	/*
	 * Map banks 0 (and maybe 1) to the FLASH banks 0 (and 1) at
	 * preliminary addresses - these have to be modified later
	 * when FLASH size has been determined
	 */
#if defined(CONFIG_SYS_OR0_REMAP)
	set_lbc_or(0, CONFIG_SYS_OR0_REMAP);
#endif
#if defined(CONFIG_SYS_OR1_REMAP)
	set_lbc_or(1, CONFIG_SYS_OR1_REMAP);
#endif
	/* now restrict to preliminary range */
	if (init_br1) {
		set_lbc_br(0, CONFIG_SYS_BR0_PRELIM);
		set_lbc_or(0, CONFIG_SYS_OR0_PRELIM);

#if defined(CONFIG_SYS_BR1_PRELIM) && defined(CONFIG_SYS_OR1_PRELIM)
		set_lbc_or(1, CONFIG_SYS_OR1_PRELIM);
		set_lbc_br(1, CONFIG_SYS_BR1_PRELIM);
#endif
	}

#if defined(CONFIG_SYS_BR2_PRELIM) && defined(CONFIG_SYS_OR2_PRELIM)
	set_lbc_or(2, CONFIG_SYS_OR2_PRELIM);
	set_lbc_br(2, CONFIG_SYS_BR2_PRELIM);
#endif

#if defined(CONFIG_SYS_BR3_PRELIM) && defined(CONFIG_SYS_OR3_PRELIM)
	set_lbc_or(3, CONFIG_SYS_OR3_PRELIM);
	set_lbc_br(3, CONFIG_SYS_BR3_PRELIM);
#endif

#if defined(CONFIG_SYS_BR4_PRELIM) && defined(CONFIG_SYS_OR4_PRELIM)
	set_lbc_or(4, CONFIG_SYS_OR4_PRELIM);
	set_lbc_br(4, CONFIG_SYS_BR4_PRELIM);
#endif

#if defined(CONFIG_SYS_BR5_PRELIM) && defined(CONFIG_SYS_OR5_PRELIM)
	set_lbc_or(5, CONFIG_SYS_OR5_PRELIM);
	set_lbc_br(5, CONFIG_SYS_BR5_PRELIM);
#endif

#if defined(CONFIG_SYS_BR6_PRELIM) && defined(CONFIG_SYS_OR6_PRELIM)
	set_lbc_or(6, CONFIG_SYS_OR6_PRELIM);
	set_lbc_br(6, CONFIG_SYS_BR6_PRELIM);
#endif

#if defined(CONFIG_SYS_BR7_PRELIM) && defined(CONFIG_SYS_OR7_PRELIM)
	set_lbc_or(7, CONFIG_SYS_OR7_PRELIM);
	set_lbc_br(7, CONFIG_SYS_BR7_PRELIM);
#endif
}
