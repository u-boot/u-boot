/*
 * Copyright Altera Corporation (C) 2014-2015
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */
#include <common.h>
#include <div64.h>
#include <watchdog.h>
#include <asm/arch/fpga_manager.h>
#include <asm/arch/sdram.h>
#include <asm/arch/system_manager.h>
#include <asm/io.h>

/*
 * FIXME: This path is temporary until the SDRAM driver gets
 *        a proper thorough cleanup.
 */
#include "../../../board/altera/socfpga/qts/sdram_config.h"

DECLARE_GLOBAL_DATA_PTR;

struct sdram_prot_rule {
	u64	sdram_start;	/* SDRAM start address */
	u64	sdram_end;	/* SDRAM end address */
	u32	rule;		/* SDRAM protection rule number: 0-19 */
	int	valid;		/* Rule valid or not? 1 - valid, 0 not*/

	u32	security;
	u32	portmask;
	u32	result;
	u32	lo_prot_id;
	u32	hi_prot_id;
};

static struct socfpga_system_manager *sysmgr_regs =
	(struct socfpga_system_manager *)SOCFPGA_SYSMGR_ADDRESS;
static struct socfpga_sdr_ctrl *sdr_ctrl =
	(struct socfpga_sdr_ctrl *)SDR_CTRLGRP_ADDRESS;

/**
 * get_errata_rows() - Up the number of DRAM rows to cover entire address space
 *
 * SDRAM Failure happens when accessing non-existent memory. Artificially
 * increase the number of rows so that the memory controller thinks it has
 * 4GB of RAM. This function returns such amount of rows.
 */
static int get_errata_rows(void)
{
	/* Define constant for 4G memory - used for SDRAM errata workaround */
#define MEMSIZE_4G	(4ULL * 1024ULL * 1024ULL * 1024ULL)
	const unsigned long long memsize = MEMSIZE_4G;
	const unsigned int cs = CONFIG_HPS_SDR_CTRLCFG_DRAMADDRW_CSBITS;
	const unsigned int rows = CONFIG_HPS_SDR_CTRLCFG_DRAMADDRW_ROWBITS;
	const unsigned int banks = CONFIG_HPS_SDR_CTRLCFG_DRAMADDRW_BANKBITS;
	const unsigned int cols = CONFIG_HPS_SDR_CTRLCFG_DRAMADDRW_COLBITS;
	const unsigned int width = 8;

	unsigned long long newrows;
	int bits, inewrowslog2;

	debug("workaround rows - memsize %lld\n", memsize);
	debug("workaround rows - cs        %d\n", cs);
	debug("workaround rows - width     %d\n", width);
	debug("workaround rows - rows      %d\n", rows);
	debug("workaround rows - banks     %d\n", banks);
	debug("workaround rows - cols      %d\n", cols);

	newrows = lldiv(memsize, cs * (width / 8));
	debug("rows workaround - term1 %lld\n", newrows);

	newrows = lldiv(newrows, (1 << banks) * (1 << cols));
	debug("rows workaround - term2 %lld\n", newrows);

	/*
	 * Compute the hamming weight - same as number of bits set.
	 * Need to see if result is ordinal power of 2 before
	 * attempting log2 of result.
	 */
	bits = generic_hweight32(newrows);

	debug("rows workaround - bits %d\n", bits);

	if (bits != 1) {
		printf("SDRAM workaround failed, bits set %d\n", bits);
		return rows;
	}

	if (newrows > UINT_MAX) {
		printf("SDRAM workaround rangecheck failed, %lld\n", newrows);
		return rows;
	}

	inewrowslog2 = __ilog2(newrows);

	debug("rows workaround - ilog2 %d, %lld\n", inewrowslog2, newrows);

	if (inewrowslog2 == -1) {
		printf("SDRAM workaround failed, newrows %lld\n", newrows);
		return rows;
	}

	return inewrowslog2;
}

/* SDRAM protection rules vary from 0-19, a total of 20 rules. */
static void sdram_set_rule(struct sdram_prot_rule *prule)
{
	uint32_t lo_addr_bits;
	uint32_t hi_addr_bits;
	int ruleno = prule->rule;

	/* Select the rule */
	writel(ruleno, &sdr_ctrl->prot_rule_rdwr);

	/* Obtain the address bits */
	lo_addr_bits = (uint32_t)(((prule->sdram_start) >> 20ULL) & 0xFFF);
	hi_addr_bits = (uint32_t)((((prule->sdram_end-1) >> 20ULL)) & 0xFFF);

	debug("sdram set rule start %x, %lld\n", lo_addr_bits,
	      prule->sdram_start);
	debug("sdram set rule end   %x, %lld\n", hi_addr_bits,
	      prule->sdram_end);

	/* Set rule addresses */
	writel(lo_addr_bits | (hi_addr_bits << 12), &sdr_ctrl->prot_rule_addr);

	/* Set rule protection ids */
	writel(prule->lo_prot_id | (prule->hi_prot_id << 12),
	       &sdr_ctrl->prot_rule_id);

	/* Set the rule data */
	writel(prule->security | (prule->valid << 2) |
	       (prule->portmask << 3) | (prule->result << 13),
	       &sdr_ctrl->prot_rule_data);

	/* write the rule */
	writel(ruleno | (1L << 5), &sdr_ctrl->prot_rule_rdwr);

	/* Set rule number to 0 by default */
	writel(0, &sdr_ctrl->prot_rule_rdwr);
}

static void sdram_get_rule(struct sdram_prot_rule *prule)
{
	uint32_t addr;
	uint32_t id;
	uint32_t data;
	int ruleno = prule->rule;

	/* Read the rule */
	writel(ruleno, &sdr_ctrl->prot_rule_rdwr);
	writel(ruleno | (1L << 6), &sdr_ctrl->prot_rule_rdwr);

	/* Get the addresses */
	addr = readl(&sdr_ctrl->prot_rule_addr);
	prule->sdram_start = (addr & 0xFFF) << 20;
	prule->sdram_end = ((addr >> 12) & 0xFFF) << 20;

	/* Get the configured protection IDs */
	id = readl(&sdr_ctrl->prot_rule_id);
	prule->lo_prot_id = id & 0xFFF;
	prule->hi_prot_id = (id >> 12) & 0xFFF;

	/* Get protection data */
	data = readl(&sdr_ctrl->prot_rule_data);

	prule->security = data & 0x3;
	prule->valid = (data >> 2) & 0x1;
	prule->portmask = (data >> 3) & 0x3FF;
	prule->result = (data >> 13) & 0x1;
}

static void sdram_set_protection_config(uint64_t sdram_start, uint64_t sdram_end)
{
	struct sdram_prot_rule rule;
	int rules;

	/* Start with accepting all SDRAM transaction */
	writel(0x0, &sdr_ctrl->protport_default);

	/* Clear all protection rules for warm boot case */
	memset(&rule, 0, sizeof(struct sdram_prot_rule));

	for (rules = 0; rules < 20; rules++) {
		rule.rule = rules;
		sdram_set_rule(&rule);
	}

	/* new rule: accept SDRAM */
	rule.sdram_start = sdram_start;
	rule.sdram_end = sdram_end;
	rule.lo_prot_id = 0x0;
	rule.hi_prot_id = 0xFFF;
	rule.portmask = 0x3FF;
	rule.security = 0x3;
	rule.result = 0;
	rule.valid = 1;
	rule.rule = 0;

	/* set new rule */
	sdram_set_rule(&rule);

	/* default rule: reject everything */
	writel(0x3ff, &sdr_ctrl->protport_default);
}

static void sdram_dump_protection_config(void)
{
	struct sdram_prot_rule rule;
	int rules;

	debug("SDRAM Prot rule, default %x\n",
	      readl(&sdr_ctrl->protport_default));

	for (rules = 0; rules < 20; rules++) {
		sdram_get_rule(&rule);
		debug("Rule %d, rules ...\n", rules);
		debug("    sdram start %llx\n", rule.sdram_start);
		debug("    sdram end   %llx\n", rule.sdram_end);
		debug("    low prot id %d, hi prot id %d\n",
		      rule.lo_prot_id,
		      rule.hi_prot_id);
		debug("    portmask %x\n", rule.portmask);
		debug("    security %d\n", rule.security);
		debug("    result %d\n", rule.result);
		debug("    valid %d\n", rule.valid);
	}
}

/* Function to write to register and verify the write */
static unsigned sdram_write_verify(unsigned int *addr, unsigned reg_value)
{
#ifndef SDRAM_MMR_SKIP_VERIFY
	unsigned reg_value1;
#endif
	debug("   Write - Address ");
	debug("0x%08x Data 0x%08x\n", (u32)addr, reg_value);
	/* Write to register */
	writel(reg_value, addr);
#ifndef SDRAM_MMR_SKIP_VERIFY
	debug("   Read and verify...");
	/* Read back the wrote value */
	reg_value1 = readl(addr);
	/* Indicate failure if value not matched */
	if (reg_value1 != reg_value) {
		debug("FAIL - Address 0x%08x Expected 0x%08x Data 0x%08x\n",
		      (u32)addr, reg_value, reg_value1);
		return 1;
	}
	debug("correct!\n");
#endif	/* SDRAM_MMR_SKIP_VERIFY */
	return 0;
}

static void set_sdr_ctrlcfg(void)
{
	u32 addrorder;
	u32 ctrl_cfg =
		(CONFIG_HPS_SDR_CTRLCFG_CTRLCFG_MEMTYPE <<
			SDR_CTRLGRP_CTRLCFG_MEMTYPE_LSB)	|
		(CONFIG_HPS_SDR_CTRLCFG_CTRLCFG_MEMBL <<
			SDR_CTRLGRP_CTRLCFG_MEMBL_LSB)		|
		(CONFIG_HPS_SDR_CTRLCFG_CTRLCFG_ECCEN <<
			SDR_CTRLGRP_CTRLCFG_ECCEN_LSB)		|
		(CONFIG_HPS_SDR_CTRLCFG_CTRLCFG_ECCCORREN <<
			SDR_CTRLGRP_CTRLCFG_ECCCORREN_LSB)	|
		(CONFIG_HPS_SDR_CTRLCFG_CTRLCFG_REORDEREN <<
			SDR_CTRLGRP_CTRLCFG_REORDEREN_LSB)	|
		(CONFIG_HPS_SDR_CTRLCFG_CTRLCFG_STARVELIMIT <<
			SDR_CTRLGRP_CTRLCFG_STARVELIMIT_LSB)	|
		(CONFIG_HPS_SDR_CTRLCFG_CTRLCFG_DQSTRKEN <<
			SDR_CTRLGRP_CTRLCFG_DQSTRKEN_LSB)	|
		(CONFIG_HPS_SDR_CTRLCFG_CTRLCFG_NODMPINS <<
			SDR_CTRLGRP_CTRLCFG_NODMPINS_LSB);

	debug("\nConfiguring CTRLCFG\n");

	/*
	 * SDRAM Failure When Accessing Non-Existent Memory
	 * Set the addrorder field of the SDRAM control register
	 * based on the CSBITs setting.
	 */
	switch (CONFIG_HPS_SDR_CTRLCFG_DRAMADDRW_CSBITS) {
	case 1:
		addrorder = 0; /* chip, row, bank, column */
		if (CONFIG_HPS_SDR_CTRLCFG_CTRLCFG_ADDRORDER != 0)
			debug("INFO: Changing address order to 0 (chip, row, bank, column)\n");
		break;
	case 2:
		addrorder = 2; /* row, chip, bank, column */
		if (CONFIG_HPS_SDR_CTRLCFG_CTRLCFG_ADDRORDER != 2)
			debug("INFO: Changing address order to 2 (row, chip, bank, column)\n");
		break;
	default:
		addrorder = CONFIG_HPS_SDR_CTRLCFG_CTRLCFG_ADDRORDER;
		break;
	}

	ctrl_cfg |= addrorder << SDR_CTRLGRP_CTRLCFG_ADDRORDER_LSB;

	writel(ctrl_cfg, &sdr_ctrl->ctrl_cfg);
}

static void set_sdr_dram_timing(void)
{
	const u32 dram_timing1 =
		(CONFIG_HPS_SDR_CTRLCFG_DRAMTIMING1_TCWL <<
			SDR_CTRLGRP_DRAMTIMING1_TCWL_LSB)	|
		(CONFIG_HPS_SDR_CTRLCFG_DRAMTIMING1_AL <<
			SDR_CTRLGRP_DRAMTIMING1_TAL_LSB)	|
		(CONFIG_HPS_SDR_CTRLCFG_DRAMTIMING1_TCL <<
			SDR_CTRLGRP_DRAMTIMING1_TCL_LSB)	|
		(CONFIG_HPS_SDR_CTRLCFG_DRAMTIMING1_TRRD <<
			SDR_CTRLGRP_DRAMTIMING1_TRRD_LSB)	|
		(CONFIG_HPS_SDR_CTRLCFG_DRAMTIMING1_TFAW <<
			SDR_CTRLGRP_DRAMTIMING1_TFAW_LSB)	|
		(CONFIG_HPS_SDR_CTRLCFG_DRAMTIMING1_TRFC <<
			SDR_CTRLGRP_DRAMTIMING1_TRFC_LSB);

	const u32 dram_timing2 =
		(CONFIG_HPS_SDR_CTRLCFG_DRAMTIMING2_IF_TREFI <<
			SDR_CTRLGRP_DRAMTIMING2_TREFI_LSB)	|
		(CONFIG_HPS_SDR_CTRLCFG_DRAMTIMING2_IF_TRCD <<
			SDR_CTRLGRP_DRAMTIMING2_TRCD_LSB)	|
		(CONFIG_HPS_SDR_CTRLCFG_DRAMTIMING2_IF_TRP <<
			SDR_CTRLGRP_DRAMTIMING2_TRP_LSB)	|
		(CONFIG_HPS_SDR_CTRLCFG_DRAMTIMING2_IF_TWR <<
			SDR_CTRLGRP_DRAMTIMING2_TWR_LSB)	|
		(CONFIG_HPS_SDR_CTRLCFG_DRAMTIMING2_IF_TWTR <<
			SDR_CTRLGRP_DRAMTIMING2_TWTR_LSB);

	const u32 dram_timing3 =
		(CONFIG_HPS_SDR_CTRLCFG_DRAMTIMING3_TRTP <<
			SDR_CTRLGRP_DRAMTIMING3_TRTP_LSB)	|
		(CONFIG_HPS_SDR_CTRLCFG_DRAMTIMING3_TRAS <<
			SDR_CTRLGRP_DRAMTIMING3_TRAS_LSB)	|
		(CONFIG_HPS_SDR_CTRLCFG_DRAMTIMING3_TRC <<
			SDR_CTRLGRP_DRAMTIMING3_TRC_LSB)	|
		(CONFIG_HPS_SDR_CTRLCFG_DRAMTIMING3_TMRD <<
			SDR_CTRLGRP_DRAMTIMING3_TMRD_LSB)	|
		(CONFIG_HPS_SDR_CTRLCFG_DRAMTIMING3_TCCD <<
			SDR_CTRLGRP_DRAMTIMING3_TCCD_LSB);

	const u32 dram_timing4 =
		(CONFIG_HPS_SDR_CTRLCFG_DRAMTIMING4_SELFRFSHEXIT <<
			SDR_CTRLGRP_DRAMTIMING4_SELFRFSHEXIT_LSB)	|
		(CONFIG_HPS_SDR_CTRLCFG_DRAMTIMING4_PWRDOWNEXIT <<
			SDR_CTRLGRP_DRAMTIMING4_PWRDOWNEXIT_LSB);

	const u32 lowpwr_timing =
		(CONFIG_HPS_SDR_CTRLCFG_LOWPWRTIMING_AUTOPDCYCLES <<
			SDR_CTRLGRP_LOWPWRTIMING_AUTOPDCYCLES_LSB)	|
		(CONFIG_HPS_SDR_CTRLCFG_LOWPWRTIMING_CLKDISABLECYCLES <<
			SDR_CTRLGRP_LOWPWRTIMING_CLKDISABLECYCLES_LSB);

	debug("Configuring DRAMTIMING1\n");
	writel(dram_timing1, &sdr_ctrl->dram_timing1);

	debug("Configuring DRAMTIMING2\n");
	writel(dram_timing2, &sdr_ctrl->dram_timing2);

	debug("Configuring DRAMTIMING3\n");
	writel(dram_timing3, &sdr_ctrl->dram_timing3);

	debug("Configuring DRAMTIMING4\n");
	writel(dram_timing4, &sdr_ctrl->dram_timing4);

	debug("Configuring LOWPWRTIMING\n");
	writel(lowpwr_timing, &sdr_ctrl->lowpwr_timing);
}

static void set_sdr_addr_rw(void)
{
	/*
	 * SDRAM Failure When Accessing Non-Existent Memory
	 * Set SDR_CTRLGRP_DRAMADDRW_CSBITS_LSB to
	 * log2(number of chip select bits). Since there's only
	 * 1 or 2 chip selects, log2(1) => 0, and log2(2) => 1,
	 * which is the same as "chip selects" - 1.
	 */
	const int rows = get_errata_rows();
	const u32 dram_addrw =
		(CONFIG_HPS_SDR_CTRLCFG_DRAMADDRW_COLBITS <<
			SDR_CTRLGRP_DRAMADDRW_COLBITS_LSB)	|
		(rows << SDR_CTRLGRP_DRAMADDRW_ROWBITS_LSB)	|
		(CONFIG_HPS_SDR_CTRLCFG_DRAMADDRW_BANKBITS <<
			SDR_CTRLGRP_DRAMADDRW_BANKBITS_LSB)	|
		((CONFIG_HPS_SDR_CTRLCFG_DRAMADDRW_CSBITS - 1) <<
			SDR_CTRLGRP_DRAMADDRW_CSBITS_LSB);
	debug("Configuring DRAMADDRW\n");
	writel(dram_addrw, &sdr_ctrl->dram_addrw);
}

static void set_sdr_static_cfg(void)
{
	debug("Configuring STATICCFG\n");
	clrsetbits_le32(&sdr_ctrl->static_cfg, SDR_CTRLGRP_STATICCFG_MEMBL_MASK,
			CONFIG_HPS_SDR_CTRLCFG_STATICCFG_MEMBL <<
			SDR_CTRLGRP_STATICCFG_MEMBL_LSB);

	clrsetbits_le32(&sdr_ctrl->static_cfg,
			SDR_CTRLGRP_STATICCFG_USEECCASDATA_MASK,
			CONFIG_HPS_SDR_CTRLCFG_STATICCFG_USEECCASDATA <<
			SDR_CTRLGRP_STATICCFG_USEECCASDATA_LSB);
}

static void set_sdr_fifo_cfg(void)
{
	debug("Configuring FIFOCFG\n");
	clrsetbits_le32(&sdr_ctrl->fifo_cfg, SDR_CTRLGRP_FIFOCFG_SYNCMODE_MASK,
			CONFIG_HPS_SDR_CTRLCFG_FIFOCFG_SYNCMODE <<
			SDR_CTRLGRP_FIFOCFG_SYNCMODE_LSB);

	clrsetbits_le32(&sdr_ctrl->fifo_cfg, SDR_CTRLGRP_FIFOCFG_INCSYNC_MASK,
			CONFIG_HPS_SDR_CTRLCFG_FIFOCFG_INCSYNC <<
			SDR_CTRLGRP_FIFOCFG_INCSYNC_LSB);
}

static void set_sdr_mp_weight(void)
{
	debug("Configuring MPWEIGHT_MPWEIGHT_0\n");
	clrsetbits_le32(&sdr_ctrl->mp_weight0,
			SDR_CTRLGRP_MPWEIGHT_MPWEIGHT_0_STATICWEIGHT_31_0_MASK,
			CONFIG_HPS_SDR_CTRLCFG_MPWIEIGHT_0_STATICWEIGHT_31_0 <<
			SDR_CTRLGRP_MPWEIGHT_MPWEIGHT_0_STATICWEIGHT_31_0_LSB);

	clrsetbits_le32(&sdr_ctrl->mp_weight1,
			SDR_CTRLGRP_MPWEIGHT_MPWEIGHT_1_STATICWEIGHT_49_32_MASK,
			CONFIG_HPS_SDR_CTRLCFG_MPWIEIGHT_1_STATICWEIGHT_49_32 <<
			SDR_CTRLGRP_MPWEIGHT_MPWEIGHT_1_STATICWEIGHT_49_32_LSB);

	clrsetbits_le32(&sdr_ctrl->mp_weight1,
			SDR_CTRLGRP_MPWEIGHT_MPWEIGHT_1_SUMOFWEIGHTS_13_0_MASK,
			CONFIG_HPS_SDR_CTRLCFG_MPWIEIGHT_1_SUMOFWEIGHT_13_0 <<
			SDR_CTRLGRP_MPWEIGHT_MPWEIGHT_1_SUMOFWEIGHTS_13_0_LSB);

	clrsetbits_le32(&sdr_ctrl->mp_weight2,
			SDR_CTRLGRP_MPWEIGHT_MPWEIGHT_2_SUMOFWEIGHTS_45_14_MASK,
			CONFIG_HPS_SDR_CTRLCFG_MPWIEIGHT_2_SUMOFWEIGHT_45_14 <<
			SDR_CTRLGRP_MPWEIGHT_MPWEIGHT_2_SUMOFWEIGHTS_45_14_LSB);

	clrsetbits_le32(&sdr_ctrl->mp_weight3,
			SDR_CTRLGRP_MPWEIGHT_MPWEIGHT_3_SUMOFWEIGHTS_63_46_MASK,
			CONFIG_HPS_SDR_CTRLCFG_MPWIEIGHT_3_SUMOFWEIGHT_63_46 <<
			SDR_CTRLGRP_MPWEIGHT_MPWEIGHT_3_SUMOFWEIGHTS_63_46_LSB);
}

static void set_sdr_mp_pacing(void)
{
	debug("Configuring MPPACING_MPPACING_0\n");
	clrsetbits_le32(&sdr_ctrl->mp_pacing0,
			SDR_CTRLGRP_MPPACING_MPPACING_0_THRESHOLD1_31_0_MASK,
			CONFIG_HPS_SDR_CTRLCFG_MPPACING_0_THRESHOLD1_31_0 <<
			SDR_CTRLGRP_MPPACING_MPPACING_0_THRESHOLD1_31_0_LSB);

	clrsetbits_le32(&sdr_ctrl->mp_pacing1,
			SDR_CTRLGRP_MPPACING_MPPACING_1_THRESHOLD1_59_32_MASK,
			CONFIG_HPS_SDR_CTRLCFG_MPPACING_1_THRESHOLD1_59_32 <<
			SDR_CTRLGRP_MPPACING_MPPACING_1_THRESHOLD1_59_32_LSB);

	clrsetbits_le32(&sdr_ctrl->mp_pacing1,
			SDR_CTRLGRP_MPPACING_MPPACING_1_THRESHOLD2_3_0_MASK,
			CONFIG_HPS_SDR_CTRLCFG_MPPACING_1_THRESHOLD2_3_0 <<
			SDR_CTRLGRP_MPPACING_MPPACING_1_THRESHOLD2_3_0_LSB);

	clrsetbits_le32(&sdr_ctrl->mp_pacing2,
			SDR_CTRLGRP_MPPACING_MPPACING_2_THRESHOLD2_35_4_MASK,
			CONFIG_HPS_SDR_CTRLCFG_MPPACING_2_THRESHOLD2_35_4 <<
			SDR_CTRLGRP_MPPACING_MPPACING_2_THRESHOLD2_35_4_LSB);

	clrsetbits_le32(&sdr_ctrl->mp_pacing3,
			SDR_CTRLGRP_MPPACING_MPPACING_3_THRESHOLD2_59_36_MASK,
			CONFIG_HPS_SDR_CTRLCFG_MPPACING_3_THRESHOLD2_59_36 <<
			SDR_CTRLGRP_MPPACING_MPPACING_3_THRESHOLD2_59_36_LSB);
}

static void set_sdr_mp_threshold(void)
{
	debug("Configuring MPTHRESHOLDRST_MPTHRESHOLDRST_0\n");
	clrsetbits_le32(&sdr_ctrl->mp_threshold0,
			SDR_CTRLGRP_MPTHRESHOLDRST_0_THRESHOLDRSTCYCLES_31_0_MASK,
			CONFIG_HPS_SDR_CTRLCFG_MPTHRESHOLDRST_0_THRESHOLDRSTCYCLES_31_0 <<
			SDR_CTRLGRP_MPTHRESHOLDRST_0_THRESHOLDRSTCYCLES_31_0_LSB);

	clrsetbits_le32(&sdr_ctrl->mp_threshold1,
			SDR_CTRLGRP_MPTHRESHOLDRST_1_THRESHOLDRSTCYCLES_63_32_MASK,
			CONFIG_HPS_SDR_CTRLCFG_MPTHRESHOLDRST_1_THRESHOLDRSTCYCLES_63_32 <<
			SDR_CTRLGRP_MPTHRESHOLDRST_1_THRESHOLDRSTCYCLES_63_32_LSB);

	clrsetbits_le32(&sdr_ctrl->mp_threshold2,
			SDR_CTRLGRP_MPTHRESHOLDRST_2_THRESHOLDRSTCYCLES_79_64_MASK,
			CONFIG_HPS_SDR_CTRLCFG_MPTHRESHOLDRST_2_THRESHOLDRSTCYCLES_79_64 <<
			SDR_CTRLGRP_MPTHRESHOLDRST_2_THRESHOLDRSTCYCLES_79_64_LSB);
}


/* Function to initialize SDRAM MMR */
unsigned sdram_mmr_init_full(unsigned int sdr_phy_reg)
{
	unsigned long reg_value;
	unsigned long status = 0;

#if defined(CONFIG_HPS_SDR_CTRLCFG_DRAMADDRW_CSBITS) && \
defined(CONFIG_HPS_SDR_CTRLCFG_DRAMADDRW_ROWBITS) && \
defined(CONFIG_HPS_SDR_CTRLCFG_DRAMADDRW_BANKBITS) && \
defined(CONFIG_HPS_SDR_CTRLCFG_DRAMADDRW_COLBITS) && \
defined(CONFIG_HPS_SDR_CTRLCFG_DRAMADDRW_ROWBITS)

	writel(CONFIG_HPS_SDR_CTRLCFG_DRAMADDRW_ROWBITS,
	       &sysmgr_regs->iswgrp_handoff[4]);
#endif
	set_sdr_ctrlcfg();
	set_sdr_dram_timing();
	set_sdr_addr_rw();

	debug("Configuring DRAMIFWIDTH\n");
	clrsetbits_le32(&sdr_ctrl->dram_if_width,
			SDR_CTRLGRP_DRAMIFWIDTH_IFWIDTH_MASK,
			CONFIG_HPS_SDR_CTRLCFG_DRAMIFWIDTH_IFWIDTH <<
			SDR_CTRLGRP_DRAMIFWIDTH_IFWIDTH_LSB);

	debug("Configuring DRAMDEVWIDTH\n");
	clrsetbits_le32(&sdr_ctrl->dram_dev_width,
			SDR_CTRLGRP_DRAMDEVWIDTH_DEVWIDTH_MASK,
			CONFIG_HPS_SDR_CTRLCFG_DRAMDEVWIDTH_DEVWIDTH <<
			SDR_CTRLGRP_DRAMDEVWIDTH_DEVWIDTH_LSB);

	debug("Configuring LOWPWREQ\n");
	clrsetbits_le32(&sdr_ctrl->lowpwr_eq,
			SDR_CTRLGRP_LOWPWREQ_SELFRFSHMASK_MASK,
			CONFIG_HPS_SDR_CTRLCFG_LOWPWREQ_SELFRFSHMASK <<
			SDR_CTRLGRP_LOWPWREQ_SELFRFSHMASK_LSB);

	debug("Configuring DRAMINTR\n");
	clrsetbits_le32(&sdr_ctrl->dram_intr, SDR_CTRLGRP_DRAMINTR_INTREN_MASK,
			CONFIG_HPS_SDR_CTRLCFG_DRAMINTR_INTREN <<
			SDR_CTRLGRP_DRAMINTR_INTREN_LSB);

	set_sdr_static_cfg();

	debug("Configuring CTRLWIDTH\n");
	clrsetbits_le32(&sdr_ctrl->ctrl_width,
			SDR_CTRLGRP_CTRLWIDTH_CTRLWIDTH_MASK,
			CONFIG_HPS_SDR_CTRLCFG_CTRLWIDTH_CTRLWIDTH <<
			SDR_CTRLGRP_CTRLWIDTH_CTRLWIDTH_LSB);

	debug("Configuring PORTCFG\n");
	clrsetbits_le32(&sdr_ctrl->port_cfg, SDR_CTRLGRP_PORTCFG_AUTOPCHEN_MASK,
			CONFIG_HPS_SDR_CTRLCFG_PORTCFG_AUTOPCHEN <<
			SDR_CTRLGRP_PORTCFG_AUTOPCHEN_LSB);

	set_sdr_fifo_cfg();

	debug("Configuring MPPRIORITY\n");
	clrsetbits_le32(&sdr_ctrl->mp_priority,
			SDR_CTRLGRP_MPPRIORITY_USERPRIORITY_MASK,
			CONFIG_HPS_SDR_CTRLCFG_MPPRIORITY_USERPRIORITY <<
			SDR_CTRLGRP_MPPRIORITY_USERPRIORITY_LSB);

	set_sdr_mp_weight();
	set_sdr_mp_pacing();
	set_sdr_mp_threshold();

	debug("Configuring PHYCTRL_PHYCTRL_0\n");
	setbits_le32(&sdr_ctrl->phy_ctrl0,
		     CONFIG_HPS_SDR_CTRLCFG_PHYCTRL_PHYCTRL_0);

	debug("Configuring CPORTWIDTH\n");
	clrsetbits_le32(&sdr_ctrl->cport_width,
			SDR_CTRLGRP_CPORTWIDTH_CMDPORTWIDTH_MASK,
			CONFIG_HPS_SDR_CTRLCFG_CPORTWIDTH_CPORTWIDTH <<
			SDR_CTRLGRP_CPORTWIDTH_CMDPORTWIDTH_LSB);
	debug("   Write - Address ");
	debug("0x%08x Data 0x%08x\n",
		(unsigned)(&sdr_ctrl->cport_width),
		(unsigned)reg_value);
	reg_value = readl(&sdr_ctrl->cport_width);
	debug("   Read value without verify 0x%08x\n", (unsigned)reg_value);

	debug("Configuring CPORTWMAP\n");
	clrsetbits_le32(&sdr_ctrl->cport_wmap,
			SDR_CTRLGRP_CPORTWMAP_CPORTWFIFOMAP_MASK,
			CONFIG_HPS_SDR_CTRLCFG_CPORTWMAP_CPORTWMAP <<
			SDR_CTRLGRP_CPORTWMAP_CPORTWFIFOMAP_LSB);
	debug("   Write - Address ");
	debug("0x%08x Data 0x%08x\n",
		(unsigned)(&sdr_ctrl->cport_wmap),
		(unsigned)reg_value);
	reg_value = readl(&sdr_ctrl->cport_wmap);
	debug("   Read value without verify 0x%08x\n", (unsigned)reg_value);

	debug("Configuring CPORTRMAP\n");
	clrsetbits_le32(&sdr_ctrl->cport_rmap,
			SDR_CTRLGRP_CPORTRMAP_CPORTRFIFOMAP_MASK,
			CONFIG_HPS_SDR_CTRLCFG_CPORTRMAP_CPORTRMAP <<
			SDR_CTRLGRP_CPORTRMAP_CPORTRFIFOMAP_LSB);
	debug("   Write - Address ");
	debug("0x%08x Data 0x%08x\n",
		(unsigned)(&sdr_ctrl->cport_rmap),
		(unsigned)reg_value);
	reg_value = readl(&sdr_ctrl->cport_rmap);
	debug("   Read value without verify 0x%08x\n", (unsigned)reg_value);

	debug("Configuring RFIFOCMAP\n");
	clrsetbits_le32(&sdr_ctrl->rfifo_cmap,
			SDR_CTRLGRP_RFIFOCMAP_RFIFOCPORTMAP_MASK,
			CONFIG_HPS_SDR_CTRLCFG_RFIFOCMAP_RFIFOCMAP <<
			SDR_CTRLGRP_RFIFOCMAP_RFIFOCPORTMAP_LSB);
	debug("   Write - Address ");
	debug("0x%08x Data 0x%08x\n",
		(unsigned)(&sdr_ctrl->rfifo_cmap),
		(unsigned)reg_value);
	reg_value = readl(&sdr_ctrl->rfifo_cmap);
	debug("   Read value without verify 0x%08x\n", (unsigned)reg_value);

	debug("Configuring WFIFOCMAP\n");
	reg_value = readl(&sdr_ctrl->wfifo_cmap);
	clrsetbits_le32(&sdr_ctrl->wfifo_cmap,
			SDR_CTRLGRP_WFIFOCMAP_WFIFOCPORTMAP_MASK,
			CONFIG_HPS_SDR_CTRLCFG_WFIFOCMAP_WFIFOCMAP <<
			SDR_CTRLGRP_WFIFOCMAP_WFIFOCPORTMAP_LSB);
	debug("   Write - Address ");
	debug("0x%08x Data 0x%08x\n",
		(unsigned)(&sdr_ctrl->wfifo_cmap),
		(unsigned)reg_value);
	reg_value = readl(&sdr_ctrl->wfifo_cmap);
	debug("   Read value without verify 0x%08x\n", (unsigned)reg_value);

	debug("Configuring CPORTRDWR\n");
	clrsetbits_le32(&sdr_ctrl->cport_rdwr,
			SDR_CTRLGRP_CPORTRDWR_CPORTRDWR_MASK,
			CONFIG_HPS_SDR_CTRLCFG_CPORTRDWR_CPORTRDWR <<
			SDR_CTRLGRP_CPORTRDWR_CPORTRDWR_LSB);
	debug("   Write - Address ");
	debug("0x%08x Data 0x%08x\n",
		(unsigned)(&sdr_ctrl->cport_rdwr),
		(unsigned)reg_value);
	reg_value = readl(&sdr_ctrl->cport_rdwr);
	debug("   Read value without verify 0x%08x\n", (unsigned)reg_value);

	debug("Configuring DRAMODT\n");
	clrsetbits_le32(&sdr_ctrl->dram_odt,
			SDR_CTRLGRP_DRAMODT_READ_MASK,
			CONFIG_HPS_SDR_CTRLCFG_DRAMODT_READ <<
			SDR_CTRLGRP_DRAMODT_READ_LSB);

	clrsetbits_le32(&sdr_ctrl->dram_odt,
			SDR_CTRLGRP_DRAMODT_WRITE_MASK,
			CONFIG_HPS_SDR_CTRLCFG_DRAMODT_WRITE <<
			SDR_CTRLGRP_DRAMODT_WRITE_LSB);

	/* saving this value to SYSMGR.ISWGRP.HANDOFF.FPGA2SDR */
	writel(CONFIG_HPS_SDR_CTRLCFG_FPGAPORTRST,
	       &sysmgr_regs->iswgrp_handoff[3]);

	/* only enable if the FPGA is programmed */
	if (fpgamgr_test_fpga_ready()) {
		if (sdram_write_verify(&sdr_ctrl->fpgaport_rst,
		    CONFIG_HPS_SDR_CTRLCFG_FPGAPORTRST) == 1) {
			status = 1;
			return 1;
		}
	}

	/* Restore the SDR PHY Register if valid */
	if (sdr_phy_reg != 0xffffffff)
		writel(sdr_phy_reg, &sdr_ctrl->phy_ctrl0);

/***** Final step - apply configuration changes *****/
	debug("Configuring STATICCFG_\n");
	clrsetbits_le32(&sdr_ctrl->static_cfg, SDR_CTRLGRP_STATICCFG_APPLYCFG_MASK,
			1 << SDR_CTRLGRP_STATICCFG_APPLYCFG_LSB);
	debug("   Write - Address ");
	debug("0x%08x Data 0x%08x\n",
		(unsigned)(&sdr_ctrl->static_cfg),
		(unsigned)reg_value);
	reg_value = readl(&sdr_ctrl->static_cfg);
	debug("   Read value without verify 0x%08x\n", (unsigned)reg_value);

	sdram_set_protection_config(0, sdram_calculate_size());

	sdram_dump_protection_config();

	return status;
}

/*
 * To calculate SDRAM device size based on SDRAM controller parameters.
 * Size is specified in bytes.
 *
 * NOTE:
 * This function is compiled and linked into the preloader and
 * Uboot (there may be others). So if this function changes, the Preloader
 * and UBoot must be updated simultaneously.
 */
unsigned long sdram_calculate_size(void)
{
	unsigned long temp;
	unsigned long row, bank, col, cs, width;

	temp = readl(&sdr_ctrl->dram_addrw);
	col = (temp & SDR_CTRLGRP_DRAMADDRW_COLBITS_MASK) >>
		SDR_CTRLGRP_DRAMADDRW_COLBITS_LSB;

	/* SDRAM Failure When Accessing Non-Existent Memory
	 * Use ROWBITS from Quartus/QSys to calculate SDRAM size
	 * since the FB specifies we modify ROWBITs to work around SDRAM
	 * controller issue.
	 *
	 * If the stored handoff value for rows is 0, it probably means
	 * the preloader is older than UBoot. Use the
	 * #define from the SOCEDS Tools per Crucible review
	 * uboot-socfpga-204. Note that this is not a supported
	 * configuration and is not tested. The customer
	 * should be using preloader and uboot built from the
	 * same tag.
	 */
	row = readl(&sysmgr_regs->iswgrp_handoff[4]);
	if (row == 0)
		row = CONFIG_HPS_SDR_CTRLCFG_DRAMADDRW_ROWBITS;
	/* If the stored handoff value for rows is greater than
	 * the field width in the sdr.dramaddrw register then
	 * something is very wrong. Revert to using the the #define
	 * value handed off by the SOCEDS tool chain instead of
	 * using a broken value.
	 */
	if (row > 31)
		row = CONFIG_HPS_SDR_CTRLCFG_DRAMADDRW_ROWBITS;

	bank = (temp & SDR_CTRLGRP_DRAMADDRW_BANKBITS_MASK) >>
		SDR_CTRLGRP_DRAMADDRW_BANKBITS_LSB;

	/* SDRAM Failure When Accessing Non-Existent Memory
	 * Use CSBITs from Quartus/QSys to calculate SDRAM size
	 * since the FB specifies we modify CSBITs to work around SDRAM
	 * controller issue.
	 */
	cs = (temp & SDR_CTRLGRP_DRAMADDRW_CSBITS_MASK) >>
	      SDR_CTRLGRP_DRAMADDRW_CSBITS_LSB;
	cs += 1;

	cs = CONFIG_HPS_SDR_CTRLCFG_DRAMADDRW_CSBITS;

	width = readl(&sdr_ctrl->dram_if_width);
	/* ECC would not be calculated as its not addressible */
	if (width == SDRAM_WIDTH_32BIT_WITH_ECC)
		width = 32;
	if (width == SDRAM_WIDTH_16BIT_WITH_ECC)
		width = 16;

	/* calculate the SDRAM size base on this info */
	temp = 1 << (row + bank + col);
	temp = temp * cs * (width  / 8);

	debug("sdram_calculate_memory returns %ld\n", temp);

	return temp;
}
