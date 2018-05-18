// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2016-2018 Intel Corporation <www.intel.com>
 *
 */

#include <common.h>
#include <asm/io.h>
#include <asm/arch/reset_manager.h>
#include <asm/arch/system_manager.h>
#include <dt-bindings/reset/altr,rst-mgr-s10.h>

DECLARE_GLOBAL_DATA_PTR;

static const struct socfpga_reset_manager *reset_manager_base =
		(void *)SOCFPGA_RSTMGR_ADDRESS;
static const struct socfpga_system_manager *system_manager_base =
		(void *)SOCFPGA_SYSMGR_ADDRESS;

/* Assert or de-assert SoCFPGA reset manager reset. */
void socfpga_per_reset(u32 reset, int set)
{
	const void *reg;

	if (RSTMGR_BANK(reset) == 0)
		reg = &reset_manager_base->mpumodrst;
	else if (RSTMGR_BANK(reset) == 1)
		reg = &reset_manager_base->per0modrst;
	else if (RSTMGR_BANK(reset) == 2)
		reg = &reset_manager_base->per1modrst;
	else if (RSTMGR_BANK(reset) == 3)
		reg = &reset_manager_base->brgmodrst;
	else	/* Invalid reset register, do nothing */
		return;

	if (set)
		setbits_le32(reg, 1 << RSTMGR_RESET(reset));
	else
		clrbits_le32(reg, 1 << RSTMGR_RESET(reset));
}

/*
 * Assert reset on every peripheral but L4WD0.
 * Watchdog must be kept intact to prevent glitches
 * and/or hangs.
 */
void socfpga_per_reset_all(void)
{
	const u32 l4wd0 = 1 << RSTMGR_RESET(SOCFPGA_RESET(L4WD0));

	/* disable all except OCP and l4wd0. OCP disable later */
	writel(~(l4wd0 | RSTMGR_PER0MODRST_OCP_MASK),
	       &reset_manager_base->per0modrst);
	writel(~l4wd0, &reset_manager_base->per0modrst);
	writel(0xffffffff, &reset_manager_base->per1modrst);
}

void socfpga_bridges_reset(int enable)
{
	if (enable) {
		/* clear idle request to all bridges */
		setbits_le32(&system_manager_base->noc_idlereq_clr, ~0);

		/* Release bridges from reset state per handoff value */
		clrbits_le32(&reset_manager_base->brgmodrst, ~0);

		/* Poll until all idleack to 0 */
		while (readl(&system_manager_base->noc_idleack))
			;
	} else {
		/* set idle request to all bridges */
		writel(~0, &system_manager_base->noc_idlereq_set);

		/* Enable the NOC timeout */
		writel(1, &system_manager_base->noc_timeout);

		/* Poll until all idleack to 1 */
		while ((readl(&system_manager_base->noc_idleack) ^
			(SYSMGR_NOC_H2F_MSK | SYSMGR_NOC_LWH2F_MSK)))
			;

		/* Poll until all idlestatus to 1 */
		while ((readl(&system_manager_base->noc_idlestatus) ^
			(SYSMGR_NOC_H2F_MSK | SYSMGR_NOC_LWH2F_MSK)))
			;

		/* Put all bridges (except NOR DDR scheduler) into reset */
		setbits_le32(&reset_manager_base->brgmodrst,
			     ~RSTMGR_BRGMODRST_DDRSCH_MASK);

		/* Disable NOC timeout */
		writel(0, &system_manager_base->noc_timeout);
	}
}

/* of_reset_id: emac reset id
 * state: 0 - disable reset, !0 - enable reset
 */
void socfpga_emac_manage_reset(const unsigned int of_reset_id, u32 state)
{
	u32 reset_emac;
	u32 reset_emacocp;

	/* hardcode this now */
	switch (of_reset_id) {
	case EMAC0_RESET:
		reset_emac = SOCFPGA_RESET(EMAC0);
		reset_emacocp = SOCFPGA_RESET(EMAC0_OCP);
		break;
	case EMAC1_RESET:
		reset_emac = SOCFPGA_RESET(EMAC1);
		reset_emacocp = SOCFPGA_RESET(EMAC1_OCP);
		break;
	case EMAC2_RESET:
		reset_emac = SOCFPGA_RESET(EMAC2);
		reset_emacocp = SOCFPGA_RESET(EMAC2_OCP);
		break;
	default:
		printf("GMAC: Invalid reset ID (%i)!\n", of_reset_id);
		hang();
		break;
	}

	/* Reset ECC OCP first */
	socfpga_per_reset(reset_emacocp, state);

	/* Release the EMAC controller from reset */
	socfpga_per_reset(reset_emac, state);
}

/*
 * Release peripherals from reset based on handoff
 */
void reset_deassert_peripherals_handoff(void)
{
	writel(0, &reset_manager_base->per1modrst);
	/* Enable OCP first */
	writel(~RSTMGR_PER0MODRST_OCP_MASK, &reset_manager_base->per0modrst);
	writel(0, &reset_manager_base->per0modrst);
}
