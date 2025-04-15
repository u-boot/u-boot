// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2016-2018 Intel Corporation <www.intel.com>
 * Copyright (C) 2025 Altera Corporation <www.altera.com>
 *
 */

#include <errno.h>
#include <hang.h>
#include <asm/global_data.h>
#include <asm/io.h>
#include <asm/secure.h>
#include <asm/arch/reset_manager.h>
#include <asm/arch/smc_api.h>
#include <asm/arch/system_manager.h>
#include <asm/arch/timer.h>
#include <dt-bindings/reset/altr,rst-mgr-s10.h>
#include <exports.h>
#include <linux/iopoll.h>
#include <linux/intel-smc.h>
#include <wait_bit.h>

DECLARE_GLOBAL_DATA_PTR;

#define TIMEOUT_300MS     300

/* F2S manager registers */
#define F2SDRAM_SIDEBAND_FLAGINSTATUS0	0x14
#define F2SDRAM_SIDEBAND_FLAGOUTSET0	0x50
#define F2SDRAM_SIDEBAND_FLAGOUTCLR0	0x54

/* Assert or de-assert SoCFPGA reset manager reset. */
void socfpga_per_reset(u32 reset, int set)
{
	unsigned long reg;

	if (RSTMGR_BANK(reset) == 0)
		reg = RSTMGR_SOC64_MPUMODRST;
	else if (RSTMGR_BANK(reset) == 1)
		reg = RSTMGR_SOC64_PER0MODRST;
	else if (RSTMGR_BANK(reset) == 2)
		reg = RSTMGR_SOC64_PER1MODRST;
	else if (RSTMGR_BANK(reset) == 3)
		reg = RSTMGR_SOC64_BRGMODRST;
	else	/* Invalid reset register, do nothing */
		return;

	if (set)
		setbits_le32(socfpga_get_rstmgr_addr() + reg,
			     1 << RSTMGR_RESET(reset));
	else
		clrbits_le32(socfpga_get_rstmgr_addr() + reg,
			     1 << RSTMGR_RESET(reset));
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
		      socfpga_get_rstmgr_addr() + RSTMGR_SOC64_PER0MODRST);
	writel(~l4wd0, socfpga_get_rstmgr_addr() + RSTMGR_SOC64_PER0MODRST);
	writel(0xffffffff, socfpga_get_rstmgr_addr() + RSTMGR_SOC64_PER1MODRST);
}

static void socfpga_f2s_bridges_reset(int enable, unsigned int mask)
{
	int ret;
	u32 brg_mask;
	u32 flagout_idlereq = 0;
	u32 flagoutset_fdrain = 0;
	u32 flagoutset_en = 0;
	u32 flaginstatus_idleack = 0;
	u32 flaginstatus_respempty = 0;

	if (CONFIG_IS_ENABLED(TARGET_SOCFPGA_STRATIX10)) {
		/* Support fpga2soc and f2sdram */
		brg_mask = mask & (RSTMGR_BRGMODRST_FPGA2SOC_MASK |
				   RSTMGR_BRGMODRST_F2SDRAM0_MASK |
				   RSTMGR_BRGMODRST_F2SDRAM1_MASK |
				   RSTMGR_BRGMODRST_F2SDRAM2_MASK);

		if (brg_mask & RSTMGR_BRGMODRST_F2SDRAM0_MASK) {
			flagout_idlereq |= BIT(0);
			flaginstatus_idleack |= BIT(1);
			flagoutset_fdrain |= BIT(2);
			flagoutset_en |= BIT(1);
			flaginstatus_respempty |= BIT(3);
		}

		if (brg_mask & RSTMGR_BRGMODRST_F2SDRAM1_MASK) {
			flagout_idlereq |= BIT(3);
			flaginstatus_idleack |= BIT(5);
			flagoutset_fdrain |= BIT(5);
			flagoutset_en |= BIT(4);
			flaginstatus_respempty |= BIT(7);
		}

		if (brg_mask & RSTMGR_BRGMODRST_F2SDRAM2_MASK) {
			flagout_idlereq |= BIT(6);
			flaginstatus_idleack |= BIT(9);
			flagoutset_fdrain |= BIT(8);
			flagoutset_en |= BIT(7);
			flaginstatus_respempty |= BIT(11);
		}
	} else {
		/* Support fpga2soc only */
		brg_mask = mask & RSTMGR_BRGMODRST_FPGA2SOC_MASK;
		if (brg_mask & RSTMGR_BRGMODRST_FPGA2SOC_MASK) {
			flagout_idlereq |= BIT(0);
			flaginstatus_idleack |= BIT(1);
			flagoutset_fdrain |= BIT(2);
			flagoutset_en |= BIT(1);
			flaginstatus_respempty |= BIT(3);
		}
	}

	/* mask is not set, return here */
	if (!brg_mask)
		return;

	if (enable) {
		clrbits_le32(socfpga_get_rstmgr_addr() + RSTMGR_SOC64_BRGMODRST,
			     brg_mask);
		setbits_le32(SOCFPGA_F2SDRAM_MGR_ADDRESS +
			     F2SDRAM_SIDEBAND_FLAGOUTCLR0,
			     flagout_idlereq);

		/* Wait for mpfe noc idleack to 0 */
		wait_for_bit_le32((u32 *)(SOCFPGA_F2SDRAM_MGR_ADDRESS +
			     F2SDRAM_SIDEBAND_FLAGINSTATUS0),
			     flaginstatus_idleack, false, TIMEOUT_300MS, false);

		setbits_le32(SOCFPGA_F2SDRAM_MGR_ADDRESS +
			     F2SDRAM_SIDEBAND_FLAGOUTCLR0,
			     flagoutset_fdrain);
		setbits_le32(SOCFPGA_F2SDRAM_MGR_ADDRESS +
			     F2SDRAM_SIDEBAND_FLAGOUTSET0, flagoutset_en);

		udelay(1); /* wait 1us */
	} else {
		if (readl((socfpga_get_rstmgr_addr() +
		    RSTMGR_SOC64_BRGMODRST) & brg_mask)) {
			/* Bridge cannot be reset twice */
			return;
		}

		setbits_le32(socfpga_get_rstmgr_addr() + RSTMGR_SOC64_HDSKEN,
			     RSTMGR_HDSKEN_FPGAHSEN);
		setbits_le32(socfpga_get_rstmgr_addr() + RSTMGR_SOC64_HDSKREQ,
			     RSTMGR_HDSKREQ_FPGAHSREQ);

		/* Wait for FPGA ack the handshake request to 1 */
		wait_for_bit_le32((u32 *)(socfpga_get_rstmgr_addr() +
			     RSTMGR_SOC64_HDSKACK), RSTMGR_HDSKREQ_FPGAHSREQ,
			     true, TIMEOUT_300MS, false);

		setbits_le32(SOCFPGA_F2SDRAM_MGR_ADDRESS +
			     F2SDRAM_SIDEBAND_FLAGOUTCLR0, flagoutset_en);

		udelay(1);

		/* Requests MPFE NoC to idle */
		setbits_le32(SOCFPGA_F2SDRAM_MGR_ADDRESS +
			     F2SDRAM_SIDEBAND_FLAGOUTSET0, flagout_idlereq);

		/*  Force F2S bridge to drain */
		setbits_le32(SOCFPGA_F2SDRAM_MGR_ADDRESS +
			     F2SDRAM_SIDEBAND_FLAGOUTSET0, flagoutset_fdrain);

		/* Wait for respond queue empty status to 1 (resp idle) */
		ret = wait_for_bit_le32((u32 *)(SOCFPGA_F2SDRAM_MGR_ADDRESS +
					   F2SDRAM_SIDEBAND_FLAGINSTATUS0),
					   flaginstatus_respempty, true,
					   TIMEOUT_300MS, false);

		/* Confirm again */
		if (!ret)
			ret = wait_for_bit_le32((u32 *)
					   (SOCFPGA_F2SDRAM_MGR_ADDRESS +
					   F2SDRAM_SIDEBAND_FLAGINSTATUS0),
					   flaginstatus_respempty, true,
					   TIMEOUT_300MS, false);

		setbits_le32(socfpga_get_rstmgr_addr() + RSTMGR_SOC64_BRGMODRST,
			     brg_mask & ~RSTMGR_BRGMODRST_FPGA2SOC_MASK);
		clrbits_le32(socfpga_get_rstmgr_addr() + RSTMGR_SOC64_HDSKREQ,
			     RSTMGR_HDSKREQ_FPGAHSREQ);
		setbits_le32(SOCFPGA_F2SDRAM_MGR_ADDRESS +
			     F2SDRAM_SIDEBAND_FLAGOUTCLR0,
			     flagout_idlereq);
	}
}

static void socfpga_s2f_bridges_reset(int enable, unsigned int mask)
{
	unsigned int noc_mask = 0;
	unsigned int brg_mask = 0;

	if (mask & RSTMGR_BRGMODRST_SOC2FPGA_MASK) {
		noc_mask = SYSMGR_NOC_H2F_MSK;
		brg_mask = RSTMGR_BRGMODRST_SOC2FPGA_MASK;
	}

	if (mask & RSTMGR_BRGMODRST_LWSOC2FPGA_MASK) {
		noc_mask |= SYSMGR_NOC_LWH2F_MSK;
		brg_mask |= RSTMGR_BRGMODRST_LWSOC2FPGA_MASK;
	}

	/* s2f mask is not set, return here */
	if (!brg_mask)
		return;

	if (enable) {
		/* clear idle request to all bridges */
		setbits_le32(socfpga_get_sysmgr_addr() +
			     SYSMGR_SOC64_NOC_IDLEREQ_CLR, noc_mask);

		/* Release SOC2FPGA bridges from reset state */
		clrbits_le32(socfpga_get_rstmgr_addr() + RSTMGR_SOC64_BRGMODRST,
			     brg_mask);

		/* Wait for all NOC master ack to 0 */
		wait_for_bit_le32((u32 *)(socfpga_get_sysmgr_addr() +
			     SYSMGR_SOC64_NOC_IDLEACK), noc_mask, false,
			     TIMEOUT_300MS, false);
	} else {
		/* set idle request to all bridges */
		setbits_le32(socfpga_get_sysmgr_addr() +
			     SYSMGR_SOC64_NOC_IDLEREQ_SET, noc_mask);

		/* Enable the NOC timeout */
		writel(1, socfpga_get_sysmgr_addr() + SYSMGR_SOC64_NOC_TIMEOUT);

		/* Wait for all NOC master ack to 1 */
		wait_for_bit_le32((u32 *)(socfpga_get_sysmgr_addr() +
			     SYSMGR_SOC64_NOC_IDLEACK), noc_mask, true,
			     TIMEOUT_300MS, false);

		/* Wait for all NOC master idlestatus to 1 */
		wait_for_bit_le32((u32 *)(socfpga_get_sysmgr_addr() +
			     SYSMGR_SOC64_NOC_IDLESTATUS), noc_mask, true,
			     TIMEOUT_300MS, false);

		/* Reset all SOC2FPGA bridges */
		setbits_le32(socfpga_get_rstmgr_addr() + RSTMGR_SOC64_BRGMODRST,
			     brg_mask);

		/* Disable NOC timeout */
		writel(0, socfpga_get_sysmgr_addr() + SYSMGR_SOC64_NOC_TIMEOUT);
	}
}

void socfpga_bridges_reset(int enable, unsigned int mask)
{
	if (!IS_ENABLED(CONFIG_XPL_BUILD) && IS_ENABLED(CONFIG_SPL_ATF)) {
		u64 arg[2];
		int ret;

		/* Set bit-1 to indicate has mask value in arg[1]. */
		arg[0] = (enable & BIT(0)) | BIT(1);
		arg[1] = mask;

		ret = invoke_smc(INTEL_SIP_SMC_HPS_SET_BRIDGES, arg,
				 ARRAY_SIZE(arg), NULL, 0);
		if (ret)
			printf("Failed to %s the HPS bridges, check bridges availability. Status %d.\n",
				enable ? "enable" : "disable", ret);
	} else {
		socfpga_s2f_bridges_reset(enable, mask);
		socfpga_f2s_bridges_reset(enable, mask);
	}
}

/*
 * Return non-zero if the CPU has been warm reset
 */
int cpu_has_been_warmreset(void)
{
	return readl(socfpga_get_rstmgr_addr() + RSTMGR_SOC64_STATUS) &
			RSTMGR_L4WD_MPU_WARMRESET_MASK;
}

void print_reset_info(void)
{
	bool iswd;
	int n;
	u32 stat = cpu_has_been_warmreset();

	printf("Reset state: %s%s", stat ? "Warm " : "Cold",
	       (stat & RSTMGR_STAT_SDMWARMRST) ? "[from SDM] " : "");

	stat &= ~RSTMGR_STAT_SDMWARMRST;
	if (!stat) {
		puts("\n");
		return;
	}

	n = generic_ffs(stat) - 1;
	iswd = (n >= RSTMGR_STAT_L4WD0RST_BITPOS);
	printf("(Triggered by %s %d)\n", iswd ? "Watchdog" : "MPU",
	       iswd ? (n - RSTMGR_STAT_L4WD0RST_BITPOS) :
	       (n - RSTMGR_STAT_MPU0RST_BITPOS));
}
