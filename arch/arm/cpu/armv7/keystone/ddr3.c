/*
 * Keystone2: DDR3 initialization
 *
 * (C) Copyright 2012-2014
 *     Texas Instruments Incorporated, <www.ti.com>
 *
 * SPDX-License-Identifier:     GPL-2.0+
 */

#include <asm/arch/hardware.h>
#include <asm/io.h>

void init_ddrphy(u32 base, struct ddr3_phy_config *phy_cfg)
{
	unsigned int tmp;

	while ((__raw_readl(base + KS2_DDRPHY_PGSR0_OFFSET)
		 & 0x00000001) != 0x00000001)
		;

	__raw_writel(phy_cfg->pllcr, base + KS2_DDRPHY_PLLCR_OFFSET);

	tmp = __raw_readl(base + KS2_DDRPHY_PGCR1_OFFSET);
	tmp &= ~(phy_cfg->pgcr1_mask);
	tmp |= phy_cfg->pgcr1_val;
	__raw_writel(tmp, base + KS2_DDRPHY_PGCR1_OFFSET);

	__raw_writel(phy_cfg->ptr0,   base + KS2_DDRPHY_PTR0_OFFSET);
	__raw_writel(phy_cfg->ptr1,   base + KS2_DDRPHY_PTR1_OFFSET);
	__raw_writel(phy_cfg->ptr3,  base + KS2_DDRPHY_PTR3_OFFSET);
	__raw_writel(phy_cfg->ptr4,  base + KS2_DDRPHY_PTR4_OFFSET);

	tmp =  __raw_readl(base + KS2_DDRPHY_DCR_OFFSET);
	tmp &= ~(phy_cfg->dcr_mask);
	tmp |= phy_cfg->dcr_val;
	__raw_writel(tmp, base + KS2_DDRPHY_DCR_OFFSET);

	__raw_writel(phy_cfg->dtpr0, base + KS2_DDRPHY_DTPR0_OFFSET);
	__raw_writel(phy_cfg->dtpr1, base + KS2_DDRPHY_DTPR1_OFFSET);
	__raw_writel(phy_cfg->dtpr2, base + KS2_DDRPHY_DTPR2_OFFSET);
	__raw_writel(phy_cfg->mr0,   base + KS2_DDRPHY_MR0_OFFSET);
	__raw_writel(phy_cfg->mr1,   base + KS2_DDRPHY_MR1_OFFSET);
	__raw_writel(phy_cfg->mr2,   base + KS2_DDRPHY_MR2_OFFSET);
	__raw_writel(phy_cfg->dtcr,  base + KS2_DDRPHY_DTCR_OFFSET);
	__raw_writel(phy_cfg->pgcr2, base + KS2_DDRPHY_PGCR2_OFFSET);

	__raw_writel(phy_cfg->zq0cr1, base + KS2_DDRPHY_ZQ0CR1_OFFSET);
	__raw_writel(phy_cfg->zq1cr1, base + KS2_DDRPHY_ZQ1CR1_OFFSET);
	__raw_writel(phy_cfg->zq2cr1, base + KS2_DDRPHY_ZQ2CR1_OFFSET);

	__raw_writel(phy_cfg->pir_v1, base + KS2_DDRPHY_PIR_OFFSET);
	while ((__raw_readl(base + KS2_DDRPHY_PGSR0_OFFSET) & 0x1) != 0x1)
		;

	__raw_writel(phy_cfg->pir_v2, base + KS2_DDRPHY_PIR_OFFSET);
	while ((__raw_readl(base + KS2_DDRPHY_PGSR0_OFFSET) & 0x1) != 0x1)
		;
}

void init_ddremif(u32 base, struct ddr3_emif_config *emif_cfg)
{
	__raw_writel(emif_cfg->sdcfg,  base + KS2_DDR3_SDCFG_OFFSET);
	__raw_writel(emif_cfg->sdtim1, base + KS2_DDR3_SDTIM1_OFFSET);
	__raw_writel(emif_cfg->sdtim2, base + KS2_DDR3_SDTIM2_OFFSET);
	__raw_writel(emif_cfg->sdtim3, base + KS2_DDR3_SDTIM3_OFFSET);
	__raw_writel(emif_cfg->sdtim4, base + KS2_DDR3_SDTIM4_OFFSET);
	__raw_writel(emif_cfg->zqcfg,  base + KS2_DDR3_ZQCFG_OFFSET);
	__raw_writel(emif_cfg->sdrfc,  base + KS2_DDR3_SDRFC_OFFSET);
}
