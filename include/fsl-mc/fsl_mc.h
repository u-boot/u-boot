/*
 * Copyright (C) 2014 Freescale Semiconductor
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __FSL_MC_H__
#define __FSL_MC_H__

#include <common.h>

#define MC_CCSR_BASE_ADDR \
	((struct mc_ccsr_registers __iomem *)0x8340000)

#define BIT(x)			(1 << (x))
#define GCR1_P1_STOP		BIT(31)
#define GCR1_P2_STOP		BIT(30)
#define GCR1_P1_DE_RST		BIT(23)
#define GCR1_P2_DE_RST		BIT(22)
#define GCR1_M1_DE_RST		BIT(15)
#define GCR1_M2_DE_RST		BIT(14)
#define GCR1_M_ALL_DE_RST	(GCR1_M1_DE_RST | GCR1_M2_DE_RST)
#define GSR_FS_MASK		0x3fffffff
#define MCFAPR_PL_MASK		(0x1 << 18)
#define MCFAPR_BMT_MASK		(0x1 << 17)
#define MCFAPR_BYPASS_ICID_MASK	\
	(MCFAPR_PL_MASK | MCFAPR_BMT_MASK)

#define SOC_MC_PORTALS_BASE_ADDR    ((void __iomem *)0x00080C000000)
#define SOC_MC_PORTAL_STRIDE	    0x10000

#define SOC_MC_PORTAL_ADDR(_portal_id) \
	((void __iomem *)((uintptr_t)SOC_MC_PORTALS_BASE_ADDR + \
	 (_portal_id) * SOC_MC_PORTAL_STRIDE))

struct mc_ccsr_registers {
	u32 reg_gcr1;
	u32 reserved1;
	u32 reg_gsr;
	u32 reserved2;
	u32 reg_sicbalr;
	u32 reg_sicbahr;
	u32 reg_sicapr;
	u32 reserved3;
	u32 reg_mcfbalr;
	u32 reg_mcfbahr;
	u32 reg_mcfapr;
	u32 reserved4[0x2f1];
	u32 reg_psr;
	u32 reserved5;
	u32 reg_brr[2];
	u32 reserved6[0x80];
	u32 reg_error[];
};

int get_mc_boot_status(void);
unsigned long mc_get_dram_block_size(void);
int fsl_mc_ldpaa_init(bd_t *bis);
void fsl_mc_ldpaa_exit(bd_t *bis);
#endif
