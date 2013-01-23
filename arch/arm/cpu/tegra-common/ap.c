/*
* (C) Copyright 2010-2011
* NVIDIA Corporation <www.nvidia.com>
*
* See file CREDITS for list of people who contributed to this
* project.
*
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License as
* published by the Free Software Foundation; either version 2 of
* the License, or (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 59 Temple Place, Suite 330, Boston,
* MA 02111-1307 USA
*/
#include <common.h>
#include <asm/io.h>
#include <asm/arch/gp_padctrl.h>
#include <asm/arch-tegra/ap.h>
#include <asm/arch-tegra/fuse.h>
#include <asm/arch-tegra/pmc.h>
#include <asm/arch-tegra/scu.h>
#include <asm/arch-tegra/warmboot.h>

int tegra_get_chip_type(void)
{
	struct apb_misc_gp_ctlr *gp;
	struct fuse_regs *fuse = (struct fuse_regs *)NV_PA_FUSE_BASE;
	uint tegra_sku_id, rev;

	/*
	 * This is undocumented, Chip ID is bits 15:8 of the register
	 * APB_MISC + 0x804, and has value 0x20 for Tegra20, 0x30 for
	 * Tegra30
	 */
	gp = (struct apb_misc_gp_ctlr *)NV_PA_APB_MISC_GP_BASE;
	rev = (readl(&gp->hidrev) & HIDREV_CHIPID_MASK) >> HIDREV_CHIPID_SHIFT;

	tegra_sku_id = readl(&fuse->sku_info) & 0xff;

	switch (rev) {
	case CHIPID_TEGRA20:
		switch (tegra_sku_id) {
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
	}
	/* unknown sku id */
	return TEGRA_SOC_UNKNOWN;
}

static void enable_scu(void)
{
	struct scu_ctlr *scu = (struct scu_ctlr *)NV_PA_ARM_PERIPHBASE;
	u32 reg;

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

	bct_start = readl(AP20_BASE_PA_SRAM + NVBOOTINFOTABLE_BCTPTR);
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

	/* enable SMP mode and FW for CPU0, by writing to Auxiliary Ctl reg */
	asm volatile(
		"mrc	p15, 0, r0, c1, c0, 1\n"
		"orr	r0, r0, #0x41\n"
		"mcr	p15, 0, r0, c1, c0, 1\n");

	/* FIXME: should have ap20's L2 disabled too? */
}
