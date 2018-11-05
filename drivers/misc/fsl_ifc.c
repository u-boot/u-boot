// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2010-2011 Freescale Semiconductor, Inc.
 * Author: Dipen Dudhat <dipen.dudhat@freescale.com>
 */

#include <common.h>
#include <fsl_ifc.h>

struct ifc_regs ifc_cfg_default_boot[CONFIG_SYS_FSL_IFC_BANK_COUNT] = {
	{
		"cs0",
#if defined(CONFIG_SYS_CSPR0) && defined(CONFIG_SYS_CSOR0)
		CONFIG_SYS_CSPR0,
#ifdef CONFIG_SYS_CSPR0_EXT
		CONFIG_SYS_CSPR0_EXT,
#else
		0,
#endif
#ifdef CONFIG_SYS_AMASK0
		CONFIG_SYS_AMASK0,
#else
		0,
#endif
		CONFIG_SYS_CSOR0,
		{
			CONFIG_SYS_CS0_FTIM0,
			CONFIG_SYS_CS0_FTIM1,
			CONFIG_SYS_CS0_FTIM2,
			CONFIG_SYS_CS0_FTIM3,
		},
#ifdef CONFIG_SYS_CSOR0_EXT
		CONFIG_SYS_CSOR0_EXT,
#else
		0,
#endif
#ifdef CONFIG_SYS_CSPR0_FINAL
		CONFIG_SYS_CSPR0_FINAL,
#else
		0,
#endif
#ifdef CONFIG_SYS_AMASK0_FINAL
		CONFIG_SYS_AMASK0_FINAL,
#else
		0,
#endif
#endif
	},

#if CONFIG_SYS_FSL_IFC_BANK_COUNT >= 2
	{
		"cs1",
#if defined(CONFIG_SYS_CSPR1) && defined(CONFIG_SYS_CSOR1)
		CONFIG_SYS_CSPR1,
#ifdef CONFIG_SYS_CSPR1_EXT
		CONFIG_SYS_CSPR1_EXT,
#else
		0,
#endif
#ifdef CONFIG_SYS_AMASK1
		CONFIG_SYS_AMASK1,
#else
		0,
#endif
		CONFIG_SYS_CSOR1,
		{
			CONFIG_SYS_CS1_FTIM0,
			CONFIG_SYS_CS1_FTIM1,
			CONFIG_SYS_CS1_FTIM2,
			CONFIG_SYS_CS1_FTIM3,
		},
#ifdef CONFIG_SYS_CSOR1_EXT
		CONFIG_SYS_CSOR1_EXT,
#else
		0,
#endif
#ifdef CONFIG_SYS_CSPR1_FINAL
		CONFIG_SYS_CSPR1_FINAL,
#else
		0,
#endif
#ifdef CONFIG_SYS_AMASK1_FINAL
		CONFIG_SYS_AMASK1_FINAL,
#else
		0,
#endif
#endif
	},
#endif

#if CONFIG_SYS_FSL_IFC_BANK_COUNT >= 3
	{
		"cs2",
#if defined(CONFIG_SYS_CSPR2) && defined(CONFIG_SYS_CSOR2)
		CONFIG_SYS_CSPR2,
#ifdef CONFIG_SYS_CSPR2_EXT
		CONFIG_SYS_CSPR2_EXT,
#else
		0,
#endif
#ifdef CONFIG_SYS_AMASK2
		CONFIG_SYS_AMASK2,
#else
		0,
#endif
		CONFIG_SYS_CSOR2,
		{
			CONFIG_SYS_CS2_FTIM0,
			CONFIG_SYS_CS2_FTIM1,
			CONFIG_SYS_CS2_FTIM2,
			CONFIG_SYS_CS2_FTIM3,
		},
#ifdef CONFIG_SYS_CSOR2_EXT
		CONFIG_SYS_CSOR2_EXT,
#else
		0,
#endif
#ifdef CONFIG_SYS_CSPR2_FINAL
		CONFIG_SYS_CSPR2_FINAL,
#else
		0,
#endif
#ifdef CONFIG_SYS_AMASK2_FINAL
		CONFIG_SYS_AMASK2_FINAL,
#else
		0,
#endif
#endif
	},
#endif

#if CONFIG_SYS_FSL_IFC_BANK_COUNT >= 4
	{
		"cs3",
#if defined(CONFIG_SYS_CSPR3) && defined(CONFIG_SYS_CSOR3)
		CONFIG_SYS_CSPR3,
#ifdef CONFIG_SYS_CSPR3_EXT
		CONFIG_SYS_CSPR3_EXT,
#else
		0,
#endif
#ifdef CONFIG_SYS_AMASK3
		CONFIG_SYS_AMASK3,
#else
		0,
#endif
		CONFIG_SYS_CSOR3,
		{
			CONFIG_SYS_CS3_FTIM0,
			CONFIG_SYS_CS3_FTIM1,
			CONFIG_SYS_CS3_FTIM2,
			CONFIG_SYS_CS3_FTIM3,
		},
#ifdef CONFIG_SYS_CSOR3_EXT
		CONFIG_SYS_CSOR3_EXT,
#else
		0,
#endif
#ifdef CONFIG_SYS_CSPR3_FINAL
		CONFIG_SYS_CSPR3_FINAL,
#else
		0,
#endif
#ifdef CONFIG_SYS_AMASK3_FINAL
		CONFIG_SYS_AMASK3_FINAL,
#else
		0,
#endif
#endif
	},
#endif

#if CONFIG_SYS_FSL_IFC_BANK_COUNT >= 5
	{
		"cs4",
#if defined(CONFIG_SYS_CSPR4) && defined(CONFIG_SYS_CSOR4)
		CONFIG_SYS_CSPR4,
#ifdef CONFIG_SYS_CSPR4_EXT
		CONFIG_SYS_CSPR4_EXT,
#else
		0,
#endif
#ifdef CONFIG_SYS_AMASK4
		CONFIG_SYS_AMASK4,
#else
		0,
#endif
		CONFIG_SYS_CSOR4,
		{
			CONFIG_SYS_CS4_FTIM0,
			CONFIG_SYS_CS4_FTIM1,
			CONFIG_SYS_CS4_FTIM2,
			CONFIG_SYS_CS4_FTIM3,
		},
#ifdef CONFIG_SYS_CSOR4_EXT
		CONFIG_SYS_CSOR4_EXT,
#else
		0,
#endif
#ifdef CONFIG_SYS_CSPR4_FINAL
		CONFIG_SYS_CSPR4_FINAL,
#else
		0,
#endif
#ifdef CONFIG_SYS_AMASK4_FINAL
		CONFIG_SYS_AMASK4_FINAL,
#else
		0,
#endif
#endif
	},
#endif

#if CONFIG_SYS_FSL_IFC_BANK_COUNT >= 6
	{
		"cs5",
#if defined(CONFIG_SYS_CSPR5) && defined(CONFIG_SYS_CSOR5)
		CONFIG_SYS_CSPR5,
#ifdef CONFIG_SYS_CSPR5_EXT
		CONFIG_SYS_CSPR5_EXT,
#else
		0,
#endif
#ifdef CONFIG_SYS_AMASK5
		CONFIG_SYS_AMASK5,
#else
		0,
#endif
		CONFIG_SYS_CSOR5,
		{
			CONFIG_SYS_CS5_FTIM0,
			CONFIG_SYS_CS5_FTIM1,
			CONFIG_SYS_CS5_FTIM2,
			CONFIG_SYS_CS5_FTIM3,
		},
#ifdef CONFIG_SYS_CSOR5_EXT
		CONFIG_SYS_CSOR5_EXT,
#else
		0,
#endif
#ifdef CONFIG_SYS_CSPR5_FINAL
		CONFIG_SYS_CSPR5_FINAL,
#else
		0,
#endif
#ifdef CONFIG_SYS_AMASK5_FINAL
		CONFIG_SYS_AMASK5_FINAL,
#else
		0,
#endif
#endif
	},
#endif

#if CONFIG_SYS_FSL_IFC_BANK_COUNT >= 7
	{
		"cs6",
#if defined(CONFIG_SYS_CSPR6) && defined(CONFIG_SYS_CSOR6)
		CONFIG_SYS_CSPR6,
#ifdef CONFIG_SYS_CSPR6_EXT
		CONFIG_SYS_CSPR6_EXT,
#else
		0,
#endif
#ifdef CONFIG_SYS_AMASK6
		CONFIG_SYS_AMASK6,
#else
		0,
#endif
		CONFIG_SYS_CSOR6,
		{
			CONFIG_SYS_CS6_FTIM0,
			CONFIG_SYS_CS6_FTIM1,
			CONFIG_SYS_CS6_FTIM2,
			CONFIG_SYS_CS6_FTIM3,
		},
#ifdef CONFIG_SYS_CSOR6_EXT
		CONFIG_SYS_CSOR6_EXT,
#else
		0,
#endif
#ifdef CONFIG_SYS_CSPR6_FINAL
		CONFIG_SYS_CSPR6_FINAL,
#else
		0,
#endif
#ifdef CONFIG_SYS_AMASK6_FINAL
		CONFIG_SYS_AMASK6_FINAL,
#else
		0,
#endif
#endif
	},
#endif

#if CONFIG_SYS_FSL_IFC_BANK_COUNT >= 8
	{
		"cs7",
#if defined(CONFIG_SYS_CSPR7) && defined(CONFIG_SYS_CSOR7)
		CONFIG_SYS_CSPR7,
#ifdef CONFIG_SYS_CSPR7_EXT
		CONFIG_SYS_CSPR7_EXT,
#else
		0,
#endif
#ifdef CONFIG_SYS_AMASK7
		CONFIG_SYS_AMASK7,
#else
		0,
#endif
		CONFIG_SYS_CSOR7,
#ifdef CONFIG_SYS_CSOR7_EXT
		CONFIG_SYS_CSOR7_EXT,
#else
		0,
#endif
		{
			CONFIG_SYS_CS7_FTIM0,
			CONFIG_SYS_CS7_FTIM1,
			CONFIG_SYS_CS7_FTIM2,
			CONFIG_SYS_CS7_FTIM3,
		},
#ifdef CONFIG_SYS_CSPR7_FINAL
		CONFIG_SYS_CSPR7_FINAL,
#else
		0,
#endif
#ifdef CONFIG_SYS_AMASK7_FINAL
		CONFIG_SYS_AMASK7_FINAL,
#else
		0,
#endif
#endif
	},
#endif
};

__weak void ifc_cfg_boot_info(struct ifc_regs_info *regs_info)
{
	regs_info->regs = ifc_cfg_default_boot;
	regs_info->cs_size = CONFIG_SYS_FSL_IFC_BANK_COUNT;
}

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
	int i, j;
	struct ifc_regs *regs;
	struct ifc_regs_info regs_info = {0};

	ifc_cfg_boot_info(&regs_info);
	regs = regs_info.regs;

	for (i = 0 ; i < regs_info.cs_size; i++) {
		if (regs[i].pr && (regs[i].pr & CSPR_V)) {
			/* skip setting cspr/csor_ext in below condition */
			if (!(CONFIG_IS_ENABLED(A003399_NOR_WORKAROUND) &&
			      i == 0 &&
			      ((regs[0].pr & CSPR_MSEL) == CSPR_MSEL_NOR))) {
				if (regs[i].pr_ext)
					set_ifc_cspr_ext(i, regs[i].pr_ext);
				if (regs[i].or_ext)
					set_ifc_csor_ext(i, regs[i].or_ext);
			}

			for (j = 0; j < ARRAY_SIZE(regs->ftim); j++)
				set_ifc_ftim(i, j, regs[i].ftim[j]);

			set_ifc_csor(i, regs[i].or);
			set_ifc_amask(i, regs[i].amask);
			set_ifc_cspr(i, regs[i].pr);
		}
	}
}

void init_final_memctl_regs(void)
{
	int i;
	struct ifc_regs *regs;
	struct ifc_regs_info regs_info;

	ifc_cfg_boot_info(&regs_info);
	regs = regs_info.regs;

	for (i = 0 ; i < regs_info.cs_size && i < ARRAY_SIZE(regs->ftim); i++) {
		if (!(regs[i].pr_final & CSPR_V))
			continue;
		if (regs[i].pr_final)
			set_ifc_cspr(i, regs[i].pr_final);
		if (regs[i].amask_final)
			set_ifc_amask(i, (i == 1) ? regs[i].amask_final :
								regs[i].amask);
	}
}
