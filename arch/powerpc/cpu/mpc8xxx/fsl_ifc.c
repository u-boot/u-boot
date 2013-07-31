/*
 * Copyright 2010-2011 Freescale Semiconductor, Inc.
 * Author: Dipen Dudhat <dipen.dudhat@freescale.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <asm/fsl_ifc.h>

void print_ifc_regs(void)
{
	int i, j;

	printf("IFC Controller Registers\n");
	for (i = 0; i < CONFIG_SYS_FSL_IFC_BANK_COUNT; i++) {
		printf("CSPR%d:0x%08X\tAMASK%d:0x%08X\tCSOR%d:0x%08X\n",
			i, get_ifc_cspr(i), i, get_ifc_amask(i),
			i, get_ifc_csor(i));
		for (j = 0; j < 4; j++)
			printf("IFC_FTIM%d:0x%08X\n", j, get_ifc_ftim(i, j));
	}
}

void init_early_memctl_regs(void)
{
#if defined(CONFIG_SYS_CSPR0) && defined(CONFIG_SYS_CSOR0)
	set_ifc_ftim(IFC_CS0, IFC_FTIM0, CONFIG_SYS_CS0_FTIM0);
	set_ifc_ftim(IFC_CS0, IFC_FTIM1, CONFIG_SYS_CS0_FTIM1);
	set_ifc_ftim(IFC_CS0, IFC_FTIM2, CONFIG_SYS_CS0_FTIM2);
	set_ifc_ftim(IFC_CS0, IFC_FTIM3, CONFIG_SYS_CS0_FTIM3);

#ifndef CONFIG_A003399_NOR_WORKAROUND
#ifdef CONFIG_SYS_CSPR0_EXT
	set_ifc_cspr_ext(IFC_CS0, CONFIG_SYS_CSPR0_EXT);
#endif
	set_ifc_cspr(IFC_CS0, CONFIG_SYS_CSPR0);
	set_ifc_amask(IFC_CS0, CONFIG_SYS_AMASK0);
	set_ifc_csor(IFC_CS0, CONFIG_SYS_CSOR0);
#endif
#endif

#ifdef CONFIG_SYS_CSPR1_EXT
	set_ifc_cspr_ext(IFC_CS1, CONFIG_SYS_CSPR1_EXT);
#endif
#if defined(CONFIG_SYS_CSPR1) && defined(CONFIG_SYS_CSOR1)
	set_ifc_ftim(IFC_CS1, IFC_FTIM0, CONFIG_SYS_CS1_FTIM0);
	set_ifc_ftim(IFC_CS1, IFC_FTIM1, CONFIG_SYS_CS1_FTIM1);
	set_ifc_ftim(IFC_CS1, IFC_FTIM2, CONFIG_SYS_CS1_FTIM2);
	set_ifc_ftim(IFC_CS1, IFC_FTIM3, CONFIG_SYS_CS1_FTIM3);

	set_ifc_csor(IFC_CS1, CONFIG_SYS_CSOR1);
	set_ifc_amask(IFC_CS1, CONFIG_SYS_AMASK1);
	set_ifc_cspr(IFC_CS1, CONFIG_SYS_CSPR1);
#endif

#ifdef CONFIG_SYS_CSPR2_EXT
	set_ifc_cspr_ext(IFC_CS2, CONFIG_SYS_CSPR2_EXT);
#endif
#if defined(CONFIG_SYS_CSPR2) && defined(CONFIG_SYS_CSOR2)
	set_ifc_ftim(IFC_CS2, IFC_FTIM0, CONFIG_SYS_CS2_FTIM0);
	set_ifc_ftim(IFC_CS2, IFC_FTIM1, CONFIG_SYS_CS2_FTIM1);
	set_ifc_ftim(IFC_CS2, IFC_FTIM2, CONFIG_SYS_CS2_FTIM2);
	set_ifc_ftim(IFC_CS2, IFC_FTIM3, CONFIG_SYS_CS2_FTIM3);

	set_ifc_csor(IFC_CS2, CONFIG_SYS_CSOR2);
	set_ifc_amask(IFC_CS2, CONFIG_SYS_AMASK2);
	set_ifc_cspr(IFC_CS2, CONFIG_SYS_CSPR2);
#endif

#ifdef CONFIG_SYS_CSPR3_EXT
	set_ifc_cspr_ext(IFC_CS3, CONFIG_SYS_CSPR3_EXT);
#endif
#if defined(CONFIG_SYS_CSPR3) && defined(CONFIG_SYS_CSOR3)
	set_ifc_ftim(IFC_CS3, IFC_FTIM0, CONFIG_SYS_CS3_FTIM0);
	set_ifc_ftim(IFC_CS3, IFC_FTIM1, CONFIG_SYS_CS3_FTIM1);
	set_ifc_ftim(IFC_CS3, IFC_FTIM2, CONFIG_SYS_CS3_FTIM2);
	set_ifc_ftim(IFC_CS3, IFC_FTIM3, CONFIG_SYS_CS3_FTIM3);

	set_ifc_cspr(IFC_CS3, CONFIG_SYS_CSPR3);
	set_ifc_amask(IFC_CS3, CONFIG_SYS_AMASK3);
	set_ifc_csor(IFC_CS3, CONFIG_SYS_CSOR3);
#endif

#ifdef CONFIG_SYS_CSPR4_EXT
	set_ifc_cspr_ext(IFC_CS4, CONFIG_SYS_CSPR4_EXT);
#endif
#if defined(CONFIG_SYS_CSPR4) && defined(CONFIG_SYS_CSOR4)
	set_ifc_ftim(IFC_CS4, IFC_FTIM0, CONFIG_SYS_CS4_FTIM0);
	set_ifc_ftim(IFC_CS4, IFC_FTIM1, CONFIG_SYS_CS4_FTIM1);
	set_ifc_ftim(IFC_CS4, IFC_FTIM2, CONFIG_SYS_CS4_FTIM2);
	set_ifc_ftim(IFC_CS4, IFC_FTIM3, CONFIG_SYS_CS4_FTIM3);

	set_ifc_cspr(IFC_CS4, CONFIG_SYS_CSPR4);
	set_ifc_amask(IFC_CS4, CONFIG_SYS_AMASK4);
	set_ifc_csor(IFC_CS4, CONFIG_SYS_CSOR4);
#endif

#ifdef CONFIG_SYS_CSPR5_EXT
	set_ifc_cspr_ext(IFC_CS5, CONFIG_SYS_CSPR5_EXT);
#endif
#if defined(CONFIG_SYS_CSPR5) && defined(CONFIG_SYS_CSOR5)
	set_ifc_ftim(IFC_CS5, IFC_FTIM0, CONFIG_SYS_CS5_FTIM0);
	set_ifc_ftim(IFC_CS5, IFC_FTIM1, CONFIG_SYS_CS5_FTIM1);
	set_ifc_ftim(IFC_CS5, IFC_FTIM2, CONFIG_SYS_CS5_FTIM2);
	set_ifc_ftim(IFC_CS5, IFC_FTIM3, CONFIG_SYS_CS5_FTIM3);

	set_ifc_cspr(IFC_CS5, CONFIG_SYS_CSPR5);
	set_ifc_amask(IFC_CS5, CONFIG_SYS_AMASK5);
	set_ifc_csor(IFC_CS5, CONFIG_SYS_CSOR5);
#endif

#ifdef CONFIG_SYS_CSPR6_EXT
	set_ifc_cspr_ext(IFC_CS6, CONFIG_SYS_CSPR6_EXT);
#endif
#if defined(CONFIG_SYS_CSPR6) && defined(CONFIG_SYS_CSOR6)
	set_ifc_ftim(IFC_CS6, IFC_FTIM0, CONFIG_SYS_CS6_FTIM0);
	set_ifc_ftim(IFC_CS6, IFC_FTIM1, CONFIG_SYS_CS6_FTIM1);
	set_ifc_ftim(IFC_CS6, IFC_FTIM2, CONFIG_SYS_CS6_FTIM2);
	set_ifc_ftim(IFC_CS6, IFC_FTIM3, CONFIG_SYS_CS6_FTIM3);

	set_ifc_cspr(IFC_CS6, CONFIG_SYS_CSPR6);
	set_ifc_amask(IFC_CS6, CONFIG_SYS_AMASK6);
	set_ifc_csor(IFC_CS6, CONFIG_SYS_CSOR6);
#endif

#ifdef CONFIG_SYS_CSPR7_EXT
	set_ifc_cspr_ext(IFC_CS7, CONFIG_SYS_CSPR7_EXT);
#endif
#if defined(CONFIG_SYS_CSPR7) && defined(CONFIG_SYS_CSOR7)
	set_ifc_ftim(IFC_CS7, IFC_FTIM0, CONFIG_SYS_CS7_FTIM0);
	set_ifc_ftim(IFC_CS7, IFC_FTIM1, CONFIG_SYS_CS7_FTIM1);
	set_ifc_ftim(IFC_CS7, IFC_FTIM2, CONFIG_SYS_CS7_FTIM2);
	set_ifc_ftim(IFC_CS7, IFC_FTIM3, CONFIG_SYS_CS7_FTIM3);

	set_ifc_cspr(IFC_CS7, CONFIG_SYS_CSPR7);
	set_ifc_amask(IFC_CS7, CONFIG_SYS_AMASK7);
	set_ifc_csor(IFC_CS7, CONFIG_SYS_CSOR7);
#endif
}
