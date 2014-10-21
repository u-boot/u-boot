/*
 * Keystone2: DDR3 initialization
 *
 * (C) Copyright 2012-2014
 *     Texas Instruments Incorporated, <www.ti.com>
 *
 * SPDX-License-Identifier:     GPL-2.0+
 */

#include <asm/io.h>
#include <common.h>
#include <asm/arch/ddr3.h>
#include <asm/arch/psc_defs.h>

void ddr3_init_ddrphy(u32 base, struct ddr3_phy_config *phy_cfg)
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

void ddr3_init_ddremif(u32 base, struct ddr3_emif_config *emif_cfg)
{
	__raw_writel(emif_cfg->sdcfg,  base + KS2_DDR3_SDCFG_OFFSET);
	__raw_writel(emif_cfg->sdtim1, base + KS2_DDR3_SDTIM1_OFFSET);
	__raw_writel(emif_cfg->sdtim2, base + KS2_DDR3_SDTIM2_OFFSET);
	__raw_writel(emif_cfg->sdtim3, base + KS2_DDR3_SDTIM3_OFFSET);
	__raw_writel(emif_cfg->sdtim4, base + KS2_DDR3_SDTIM4_OFFSET);
	__raw_writel(emif_cfg->zqcfg,  base + KS2_DDR3_ZQCFG_OFFSET);
	__raw_writel(emif_cfg->sdrfc,  base + KS2_DDR3_SDRFC_OFFSET);
}

void ddr3_reset_ddrphy(void)
{
	u32 tmp;

	/* Assert DDR3A  PHY reset */
	tmp = readl(KS2_DDR3APLLCTL1);
	tmp |= KS2_DDR3_PLLCTRL_PHY_RESET;
	writel(tmp, KS2_DDR3APLLCTL1);

	/* wait 10us to catch the reset */
	udelay(10);

	/* Release DDR3A PHY reset */
	tmp = readl(KS2_DDR3APLLCTL1);
	tmp &= ~KS2_DDR3_PLLCTRL_PHY_RESET;
	__raw_writel(tmp, KS2_DDR3APLLCTL1);
}

#ifdef CONFIG_SOC_K2HK
/**
 * ddr3_reset_workaround - reset workaround in case if leveling error
 * detected for PG 1.0 and 1.1 k2hk SoCs
 */
void ddr3_err_reset_workaround(void)
{
	unsigned int tmp;
	unsigned int tmp_a;
	unsigned int tmp_b;

	/*
	 * Check for PGSR0 error bits of DDR3 PHY.
	 * Check for WLERR, QSGERR, WLAERR,
	 * RDERR, WDERR, REERR, WEERR error to see if they are set or not
	 */
	tmp_a = __raw_readl(KS2_DDR3A_DDRPHYC + KS2_DDRPHY_PGSR0_OFFSET);
	tmp_b = __raw_readl(KS2_DDR3B_DDRPHYC + KS2_DDRPHY_PGSR0_OFFSET);

	if (((tmp_a & 0x0FE00000) != 0) || ((tmp_b & 0x0FE00000) != 0)) {
		printf("DDR Leveling Error Detected!\n");
		printf("DDR3A PGSR0 = 0x%x\n", tmp_a);
		printf("DDR3B PGSR0 = 0x%x\n", tmp_b);

		/*
		 * Write Keys to KICK registers to enable writes to registers
		 * in boot config space
		 */
		__raw_writel(KS2_KICK0_MAGIC, KS2_KICK0);
		__raw_writel(KS2_KICK1_MAGIC, KS2_KICK1);

		/*
		 * Move DDR3A Module out of reset isolation by setting
		 * MDCTL23[12] = 0
		 */
		tmp_a = __raw_readl(KS2_PSC_BASE +
				    PSC_REG_MDCTL(KS2_LPSC_EMIF4F_DDR3A));

		tmp_a = PSC_REG_MDCTL_SET_RESET_ISO(tmp_a, 0);
		__raw_writel(tmp_a, KS2_PSC_BASE +
			     PSC_REG_MDCTL(KS2_LPSC_EMIF4F_DDR3A));

		/*
		 * Move DDR3B Module out of reset isolation by setting
		 * MDCTL24[12] = 0
		 */
		tmp_b = __raw_readl(KS2_PSC_BASE +
				    PSC_REG_MDCTL(KS2_LPSC_EMIF4F_DDR3B));
		tmp_b = PSC_REG_MDCTL_SET_RESET_ISO(tmp_b, 0);
		__raw_writel(tmp_b, KS2_PSC_BASE +
			     PSC_REG_MDCTL(KS2_LPSC_EMIF4F_DDR3B));

		/*
		 * Write 0x5A69 Key to RSTCTRL[15:0] to unlock writes
		 * to RSTCTRL and RSTCFG
		 */
		tmp = __raw_readl(KS2_RSTCTRL);
		tmp &= KS2_RSTCTRL_MASK;
		tmp |= KS2_RSTCTRL_KEY;
		__raw_writel(tmp, KS2_RSTCTRL);

		/*
		 * Set PLL Controller to drive hard reset on SW trigger by
		 * setting RSTCFG[13] = 0
		 */
		tmp = __raw_readl(KS2_RSTCTRL_RSCFG);
		tmp &= ~KS2_RSTYPE_PLL_SOFT;
		__raw_writel(tmp, KS2_RSTCTRL_RSCFG);

		reset_cpu(0);
	}
}
#endif
