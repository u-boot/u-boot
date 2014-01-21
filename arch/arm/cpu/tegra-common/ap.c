/*
* (C) Copyright 2010-2011
* NVIDIA Corporation <www.nvidia.com>
*
 * SPDX-License-Identifier:	GPL-2.0+
*/

/* Tegra AP (Application Processor) code */

#include <common.h>
#include <asm/io.h>
#include <asm/arch/gp_padctrl.h>
#include <asm/arch-tegra/ap.h>
#include <asm/arch-tegra/clock.h>
#include <asm/arch-tegra/fuse.h>
#include <asm/arch-tegra/pmc.h>
#include <asm/arch-tegra/scu.h>
#include <asm/arch-tegra/tegra.h>
#include <asm/arch-tegra/warmboot.h>

int tegra_get_chip(void)
{
	int rev;
	struct apb_misc_gp_ctlr *gp =
		(struct apb_misc_gp_ctlr *)NV_PA_APB_MISC_GP_BASE;

	/*
	 * This is undocumented, Chip ID is bits 15:8 of the register
	 * APB_MISC + 0x804, and has value 0x20 for Tegra20, 0x30 for
	 * Tegra30, and 0x35 for T114.
	 */
	rev = (readl(&gp->hidrev) & HIDREV_CHIPID_MASK) >> HIDREV_CHIPID_SHIFT;
	debug("%s: CHIPID is 0x%02X\n", __func__, rev);

	return rev;
}

int tegra_get_sku_info(void)
{
	int sku_id;
	struct fuse_regs *fuse = (struct fuse_regs *)NV_PA_FUSE_BASE;

	sku_id = readl(&fuse->sku_info) & 0xff;
	debug("%s: SKU info byte is 0x%02X\n", __func__, sku_id);

	return sku_id;
}

int tegra_get_chip_sku(void)
{
	uint sku_id, chip_id;

	chip_id = tegra_get_chip();
	sku_id = tegra_get_sku_info();

	switch (chip_id) {
	case CHIPID_TEGRA20:
		switch (sku_id) {
		case SKU_ID_T20_7:
		case SKU_ID_T20:
			return TEGRA_SOC_T20;
		case SKU_ID_T25SE:
		case SKU_ID_AP25:
		case SKU_ID_T25:
		case SKU_ID_AP25E:
		case SKU_ID_T25E:
			return TEGRA_SOC_T25;
		}
		break;
	case CHIPID_TEGRA30:
		switch (sku_id) {
		case SKU_ID_T33:
		case SKU_ID_T30:
		case SKU_ID_TM30MQS_P_A3:
			return TEGRA_SOC_T30;
		}
		break;
	case CHIPID_TEGRA114:
		switch (sku_id) {
		case SKU_ID_T114_ENG:
		case SKU_ID_T114_1:
			return TEGRA_SOC_T114;
		}
		break;
	}
	/* unknown chip/sku id */
	printf("%s: ERROR: UNKNOWN CHIP/SKU ID COMBO (0x%02X/0x%02X)\n",
		__func__, chip_id, sku_id);
	return TEGRA_SOC_UNKNOWN;
}

static void enable_scu(void)
{
	struct scu_ctlr *scu = (struct scu_ctlr *)NV_PA_ARM_PERIPHBASE;
	u32 reg;

	/* Only enable the SCU on T20/T25 */
	if (tegra_get_chip() != CHIPID_TEGRA20)
		return;

	/* If SCU already setup/enabled, return */
	if (readl(&scu->scu_ctrl) & SCU_CTRL_ENABLE)
		return;

	/* Invalidate all ways for all processors */
	writel(0xFFFF, &scu->scu_inv_all);

	/* Enable SCU - bit 0 */
	reg = readl(&scu->scu_ctrl);
	reg |= SCU_CTRL_ENABLE;
	writel(reg, &scu->scu_ctrl);
}

static u32 get_odmdata(void)
{
	/*
	 * ODMDATA is stored in the BCT in IRAM by the BootROM.
	 * The BCT start and size are stored in the BIT in IRAM.
	 * Read the data @ bct_start + (bct_size - 12). This works
	 * on T20 and T30 BCTs, which are locked down. If this changes
	 * in new chips (T114, etc.), we can revisit this algorithm.
	 */

	u32 bct_start, odmdata;

	bct_start = readl(NV_PA_BASE_SRAM + NVBOOTINFOTABLE_BCTPTR);
	odmdata = readl(bct_start + BCT_ODMDATA_OFFSET);

	return odmdata;
}

static void init_pmc_scratch(void)
{
	struct pmc_ctlr *const pmc = (struct pmc_ctlr *)NV_PA_PMC_BASE;
	u32 odmdata;
	int i;

	/* SCRATCH0 is initialized by the boot ROM and shouldn't be cleared */
	for (i = 0; i < 23; i++)
		writel(0, &pmc->pmc_scratch1+i);

	/* ODMDATA is for kernel use to determine RAM size, LP config, etc. */
	odmdata = get_odmdata();
	writel(odmdata, &pmc->pmc_scratch20);
}

void s_init(void)
{
	/* Init PMC scratch memory */
	init_pmc_scratch();

	enable_scu();

	/* init the cache */
	config_cache();
}
