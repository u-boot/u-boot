/*
 * Copyright Altera Corporation (C) 2012-2015
 *
 * SPDX-License-Identifier:    BSD-3-Clause
 */

#include <common.h>
#include <asm/io.h>
#include <asm/arch/sdram.h>
#include "sequencer.h"
#include "sequencer_auto.h"
#include "sequencer_auto_ac_init.h"
#include "sequencer_auto_inst_init.h"
#include "sequencer_defines.h"

static void scc_mgr_load_dqs_for_write_group(uint32_t write_group);

static struct socfpga_sdr_rw_load_manager *sdr_rw_load_mgr_regs =
	(struct socfpga_sdr_rw_load_manager *)(SDR_PHYGRP_RWMGRGRP_ADDRESS | 0x800);

static struct socfpga_sdr_rw_load_jump_manager *sdr_rw_load_jump_mgr_regs =
	(struct socfpga_sdr_rw_load_jump_manager *)(SDR_PHYGRP_RWMGRGRP_ADDRESS | 0xC00);

static struct socfpga_sdr_reg_file *sdr_reg_file =
	(struct socfpga_sdr_reg_file *)SDR_PHYGRP_REGFILEGRP_ADDRESS;

static struct socfpga_sdr_scc_mgr *sdr_scc_mgr =
	(struct socfpga_sdr_scc_mgr *)(SDR_PHYGRP_SCCGRP_ADDRESS | 0xe00);

static struct socfpga_phy_mgr_cmd *phy_mgr_cmd =
	(struct socfpga_phy_mgr_cmd *)SDR_PHYGRP_PHYMGRGRP_ADDRESS;

static struct socfpga_phy_mgr_cfg *phy_mgr_cfg =
	(struct socfpga_phy_mgr_cfg *)(SDR_PHYGRP_PHYMGRGRP_ADDRESS | 0x40);

static struct socfpga_data_mgr *data_mgr =
	(struct socfpga_data_mgr *)SDR_PHYGRP_DATAMGRGRP_ADDRESS;

static struct socfpga_sdr_ctrl *sdr_ctrl =
	(struct socfpga_sdr_ctrl *)SDR_CTRLGRP_ADDRESS;

#define DELTA_D		1

/*
 * In order to reduce ROM size, most of the selectable calibration steps are
 * decided at compile time based on the user's calibration mode selection,
 * as captured by the STATIC_CALIB_STEPS selection below.
 *
 * However, to support simulation-time selection of fast simulation mode, where
 * we skip everything except the bare minimum, we need a few of the steps to
 * be dynamic.  In those cases, we either use the DYNAMIC_CALIB_STEPS for the
 * check, which is based on the rtl-supplied value, or we dynamically compute
 * the value to use based on the dynamically-chosen calibration mode
 */

#define DLEVEL 0
#define STATIC_IN_RTL_SIM 0
#define STATIC_SKIP_DELAY_LOOPS 0

#define STATIC_CALIB_STEPS (STATIC_IN_RTL_SIM | CALIB_SKIP_FULL_TEST | \
	STATIC_SKIP_DELAY_LOOPS)

/* calibration steps requested by the rtl */
uint16_t dyn_calib_steps;

/*
 * To make CALIB_SKIP_DELAY_LOOPS a dynamic conditional option
 * instead of static, we use boolean logic to select between
 * non-skip and skip values
 *
 * The mask is set to include all bits when not-skipping, but is
 * zero when skipping
 */

uint16_t skip_delay_mask;	/* mask off bits when skipping/not-skipping */

#define SKIP_DELAY_LOOP_VALUE_OR_ZERO(non_skip_value) \
	((non_skip_value) & skip_delay_mask)

struct gbl_type *gbl;
struct param_type *param;
uint32_t curr_shadow_reg;

static uint32_t rw_mgr_mem_calibrate_write_test(uint32_t rank_bgn,
	uint32_t write_group, uint32_t use_dm,
	uint32_t all_correct, uint32_t *bit_chk, uint32_t all_ranks);

static void set_failing_group_stage(uint32_t group, uint32_t stage,
	uint32_t substage)
{
	/*
	 * Only set the global stage if there was not been any other
	 * failing group
	 */
	if (gbl->error_stage == CAL_STAGE_NIL)	{
		gbl->error_substage = substage;
		gbl->error_stage = stage;
		gbl->error_group = group;
	}
}

static void reg_file_set_group(u16 set_group)
{
	clrsetbits_le32(&sdr_reg_file->cur_stage, 0xffff0000, set_group << 16);
}

static void reg_file_set_stage(u8 set_stage)
{
	clrsetbits_le32(&sdr_reg_file->cur_stage, 0xffff, set_stage & 0xff);
}

static void reg_file_set_sub_stage(u8 set_sub_stage)
{
	set_sub_stage &= 0xff;
	clrsetbits_le32(&sdr_reg_file->cur_stage, 0xff00, set_sub_stage << 8);
}

static void initialize(void)
{
	debug("%s:%d\n", __func__, __LINE__);
	/* USER calibration has control over path to memory */
	/*
	 * In Hard PHY this is a 2-bit control:
	 * 0: AFI Mux Select
	 * 1: DDIO Mux Select
	 */
	writel(0x3, &phy_mgr_cfg->mux_sel);

	/* USER memory clock is not stable we begin initialization  */
	writel(0, &phy_mgr_cfg->reset_mem_stbl);

	/* USER calibration status all set to zero */
	writel(0, &phy_mgr_cfg->cal_status);

	writel(0, &phy_mgr_cfg->cal_debug_info);

	if ((dyn_calib_steps & CALIB_SKIP_ALL) != CALIB_SKIP_ALL) {
		param->read_correct_mask_vg  = ((uint32_t)1 <<
			(RW_MGR_MEM_DQ_PER_READ_DQS /
			RW_MGR_MEM_VIRTUAL_GROUPS_PER_READ_DQS)) - 1;
		param->write_correct_mask_vg = ((uint32_t)1 <<
			(RW_MGR_MEM_DQ_PER_READ_DQS /
			RW_MGR_MEM_VIRTUAL_GROUPS_PER_READ_DQS)) - 1;
		param->read_correct_mask     = ((uint32_t)1 <<
			RW_MGR_MEM_DQ_PER_READ_DQS) - 1;
		param->write_correct_mask    = ((uint32_t)1 <<
			RW_MGR_MEM_DQ_PER_WRITE_DQS) - 1;
		param->dm_correct_mask       = ((uint32_t)1 <<
			(RW_MGR_MEM_DATA_WIDTH / RW_MGR_MEM_DATA_MASK_WIDTH))
			- 1;
	}
}

static void set_rank_and_odt_mask(uint32_t rank, uint32_t odt_mode)
{
	uint32_t odt_mask_0 = 0;
	uint32_t odt_mask_1 = 0;
	uint32_t cs_and_odt_mask;

	if (odt_mode == RW_MGR_ODT_MODE_READ_WRITE) {
		if (RW_MGR_MEM_NUMBER_OF_RANKS == 1) {
			/*
			 * 1 Rank
			 * Read: ODT = 0
			 * Write: ODT = 1
			 */
			odt_mask_0 = 0x0;
			odt_mask_1 = 0x1;
		} else if (RW_MGR_MEM_NUMBER_OF_RANKS == 2) {
			/* 2 Ranks */
			if (RW_MGR_MEM_NUMBER_OF_CS_PER_DIMM == 1) {
				/* - Dual-Slot , Single-Rank
				 * (1 chip-select per DIMM)
				 * OR
				 * - RDIMM, 4 total CS (2 CS per DIMM)
				 * means 2 DIMM
				 * Since MEM_NUMBER_OF_RANKS is 2 they are
				 * both single rank
				 * with 2 CS each (special for RDIMM)
				 * Read: Turn on ODT on the opposite rank
				 * Write: Turn on ODT on all ranks
				 */
				odt_mask_0 = 0x3 & ~(1 << rank);
				odt_mask_1 = 0x3;
			} else {
				/*
				 * USER - Single-Slot , Dual-rank DIMMs
				 * (2 chip-selects per DIMM)
				 * USER Read: Turn on ODT off on all ranks
				 * USER Write: Turn on ODT on active rank
				 */
				odt_mask_0 = 0x0;
				odt_mask_1 = 0x3 & (1 << rank);
			}
		} else {
			/* 4 Ranks
			 * Read:
			 * ----------+-----------------------+
			 *           |                       |
			 *           |         ODT           |
			 * Read From +-----------------------+
			 *   Rank    |  3  |  2  |  1  |  0  |
			 * ----------+-----+-----+-----+-----+
			 *     0     |  0  |  1  |  0  |  0  |
			 *     1     |  1  |  0  |  0  |  0  |
			 *     2     |  0  |  0  |  0  |  1  |
			 *     3     |  0  |  0  |  1  |  0  |
			 * ----------+-----+-----+-----+-----+
			 *
			 * Write:
			 * ----------+-----------------------+
			 *           |                       |
			 *           |         ODT           |
			 * Write To  +-----------------------+
			 *   Rank    |  3  |  2  |  1  |  0  |
			 * ----------+-----+-----+-----+-----+
			 *     0     |  0  |  1  |  0  |  1  |
			 *     1     |  1  |  0  |  1  |  0  |
			 *     2     |  0  |  1  |  0  |  1  |
			 *     3     |  1  |  0  |  1  |  0  |
			 * ----------+-----+-----+-----+-----+
			 */
			switch (rank) {
			case 0:
				odt_mask_0 = 0x4;
				odt_mask_1 = 0x5;
				break;
			case 1:
				odt_mask_0 = 0x8;
				odt_mask_1 = 0xA;
				break;
			case 2:
				odt_mask_0 = 0x1;
				odt_mask_1 = 0x5;
				break;
			case 3:
				odt_mask_0 = 0x2;
				odt_mask_1 = 0xA;
				break;
			}
		}
	} else {
		odt_mask_0 = 0x0;
		odt_mask_1 = 0x0;
	}

	cs_and_odt_mask =
		(0xFF & ~(1 << rank)) |
		((0xFF & odt_mask_0) << 8) |
		((0xFF & odt_mask_1) << 16);
	writel(cs_and_odt_mask, SDR_PHYGRP_RWMGRGRP_ADDRESS |
				RW_MGR_SET_CS_AND_ODT_MASK_OFFSET);
}

static void scc_mgr_initialize(void)
{
	u32 addr = SDR_PHYGRP_SCCGRP_ADDRESS | SCC_MGR_HHP_RFILE_OFFSET;

	/*
	 * Clear register file for HPS
	 * 16 (2^4) is the size of the full register file in the scc mgr:
	 *	RFILE_DEPTH = log2(MEM_DQ_PER_DQS + 1 + MEM_DM_PER_DQS +
	 * MEM_IF_READ_DQS_WIDTH - 1) + 1;
	 */
	uint32_t i;
	for (i = 0; i < 16; i++) {
		debug_cond(DLEVEL == 1, "%s:%d: Clearing SCC RFILE index %u\n",
			   __func__, __LINE__, i);
		writel(0, addr + (i << 2));
	}
}

static void scc_mgr_set_dqs_bus_in_delay(uint32_t read_group,
						uint32_t delay)
{
	u32 addr = SDR_PHYGRP_SCCGRP_ADDRESS | SCC_MGR_DQS_IN_DELAY_OFFSET;

	/* Load the setting in the SCC manager */
	writel(delay, addr + (read_group << 2));
}

static void scc_mgr_set_dqs_io_in_delay(uint32_t write_group,
	uint32_t delay)
{
	u32 addr = SDR_PHYGRP_SCCGRP_ADDRESS | SCC_MGR_IO_IN_DELAY_OFFSET;

	writel(delay, addr + (RW_MGR_MEM_DQ_PER_WRITE_DQS << 2));
}

static void scc_mgr_set_dqs_en_phase(uint32_t read_group, uint32_t phase)
{
	u32 addr = SDR_PHYGRP_SCCGRP_ADDRESS | SCC_MGR_DQS_EN_PHASE_OFFSET;

	/* Load the setting in the SCC manager */
	writel(phase, addr + (read_group << 2));
}

static void scc_mgr_set_dqs_en_phase_all_ranks(uint32_t read_group,
					       uint32_t phase)
{
	uint32_t r;
	uint32_t update_scan_chains;

	for (r = 0; r < RW_MGR_MEM_NUMBER_OF_RANKS;
	     r += NUM_RANKS_PER_SHADOW_REG) {
		/*
		 * USER although the h/w doesn't support different phases per
		 * shadow register, for simplicity our scc manager modeling
		 * keeps different phase settings per shadow reg, and it's
		 * important for us to keep them in sync to match h/w.
		 * for efficiency, the scan chain update should occur only
		 * once to sr0.
		 */
		update_scan_chains = (r == 0) ? 1 : 0;

		scc_mgr_set_dqs_en_phase(read_group, phase);

		if (update_scan_chains) {
			writel(read_group, &sdr_scc_mgr->dqs_ena);
			writel(0, &sdr_scc_mgr->update);
		}
	}
}

static void scc_mgr_set_dqdqs_output_phase(uint32_t write_group,
						  uint32_t phase)
{
	u32 addr = SDR_PHYGRP_SCCGRP_ADDRESS | SCC_MGR_DQDQS_OUT_PHASE_OFFSET;

	/* Load the setting in the SCC manager */
	writel(phase, addr + (write_group << 2));
}

static void scc_mgr_set_dqdqs_output_phase_all_ranks(uint32_t write_group,
						     uint32_t phase)
{
	uint32_t r;
	uint32_t update_scan_chains;

	for (r = 0; r < RW_MGR_MEM_NUMBER_OF_RANKS;
	     r += NUM_RANKS_PER_SHADOW_REG) {
		/*
		 * USER although the h/w doesn't support different phases per
		 * shadow register, for simplicity our scc manager modeling
		 * keeps different phase settings per shadow reg, and it's
		 * important for us to keep them in sync to match h/w.
		 * for efficiency, the scan chain update should occur only
		 * once to sr0.
		 */
		update_scan_chains = (r == 0) ? 1 : 0;

		scc_mgr_set_dqdqs_output_phase(write_group, phase);

		if (update_scan_chains) {
			writel(write_group, &sdr_scc_mgr->dqs_ena);
			writel(0, &sdr_scc_mgr->update);
		}
	}
}

static void scc_mgr_set_dqs_en_delay(uint32_t read_group, uint32_t delay)
{
	uint32_t addr = SDR_PHYGRP_SCCGRP_ADDRESS | SCC_MGR_DQS_EN_DELAY_OFFSET;

	/* Load the setting in the SCC manager */
	writel(delay + IO_DQS_EN_DELAY_OFFSET, addr +
	       (read_group << 2));
}

static void scc_mgr_set_dqs_en_delay_all_ranks(uint32_t read_group,
					       uint32_t delay)
{
	uint32_t r;

	for (r = 0; r < RW_MGR_MEM_NUMBER_OF_RANKS;
		r += NUM_RANKS_PER_SHADOW_REG) {
		scc_mgr_set_dqs_en_delay(read_group, delay);

		writel(read_group, &sdr_scc_mgr->dqs_ena);
		/*
		 * In shadow register mode, the T11 settings are stored in
		 * registers in the core, which are updated by the DQS_ENA
		 * signals. Not issuing the SCC_MGR_UPD command allows us to
		 * save lots of rank switching overhead, by calling
		 * select_shadow_regs_for_update with update_scan_chains
		 * set to 0.
		 */
		writel(0, &sdr_scc_mgr->update);
	}
	/*
	 * In shadow register mode, the T11 settings are stored in
	 * registers in the core, which are updated by the DQS_ENA
	 * signals. Not issuing the SCC_MGR_UPD command allows us to
	 * save lots of rank switching overhead, by calling
	 * select_shadow_regs_for_update with update_scan_chains
	 * set to 0.
	 */
	writel(0, &sdr_scc_mgr->update);
}

static void scc_mgr_set_oct_out1_delay(uint32_t write_group, uint32_t delay)
{
	uint32_t read_group;
	uint32_t addr = SDR_PHYGRP_SCCGRP_ADDRESS | SCC_MGR_OCT_OUT1_DELAY_OFFSET;

	/*
	 * Load the setting in the SCC manager
	 * Although OCT affects only write data, the OCT delay is controlled
	 * by the DQS logic block which is instantiated once per read group.
	 * For protocols where a write group consists of multiple read groups,
	 * the setting must be set multiple times.
	 */
	for (read_group = write_group * RW_MGR_MEM_IF_READ_DQS_WIDTH /
	     RW_MGR_MEM_IF_WRITE_DQS_WIDTH;
	     read_group < (write_group + 1) * RW_MGR_MEM_IF_READ_DQS_WIDTH /
	     RW_MGR_MEM_IF_WRITE_DQS_WIDTH; ++read_group)
		writel(delay, addr + (read_group << 2));
}

static void scc_mgr_set_dq_out1_delay(uint32_t write_group,
				      uint32_t dq_in_group, uint32_t delay)
{
	uint32_t addr = SDR_PHYGRP_SCCGRP_ADDRESS | SCC_MGR_IO_OUT1_DELAY_OFFSET;

	/* Load the setting in the SCC manager */
	writel(delay, addr + (dq_in_group << 2));
}

static void scc_mgr_set_dq_in_delay(uint32_t write_group,
	uint32_t dq_in_group, uint32_t delay)
{
	uint32_t addr = SDR_PHYGRP_SCCGRP_ADDRESS | SCC_MGR_IO_IN_DELAY_OFFSET;

	/* Load the setting in the SCC manager */
	writel(delay, addr + (dq_in_group << 2));
}

static void scc_mgr_set_hhp_extras(void)
{
	/*
	 * Load the fixed setting in the SCC manager
	 * bits: 0:0 = 1'b1   - dqs bypass
	 * bits: 1:1 = 1'b1   - dq bypass
	 * bits: 4:2 = 3'b001   - rfifo_mode
	 * bits: 6:5 = 2'b01  - rfifo clock_select
	 * bits: 7:7 = 1'b0  - separate gating from ungating setting
	 * bits: 8:8 = 1'b0  - separate OE from Output delay setting
	 */
	uint32_t value = (0<<8) | (0<<7) | (1<<5) | (1<<2) | (1<<1) | (1<<0);
	uint32_t addr = SDR_PHYGRP_SCCGRP_ADDRESS | SCC_MGR_HHP_GLOBALS_OFFSET;

	writel(value, addr + SCC_MGR_HHP_EXTRAS_OFFSET);
}

static void scc_mgr_set_dqs_out1_delay(uint32_t write_group,
					      uint32_t delay)
{
	uint32_t addr = SDR_PHYGRP_SCCGRP_ADDRESS | SCC_MGR_IO_OUT1_DELAY_OFFSET;

	/* Load the setting in the SCC manager */
	writel(delay, addr + (RW_MGR_MEM_DQ_PER_WRITE_DQS << 2));
}

static void scc_mgr_set_dm_out1_delay(uint32_t write_group,
					     uint32_t dm, uint32_t delay)
{
	uint32_t addr = SDR_PHYGRP_SCCGRP_ADDRESS | SCC_MGR_IO_OUT1_DELAY_OFFSET;

	/* Load the setting in the SCC manager */
	writel(delay, addr +
		((RW_MGR_MEM_DQ_PER_WRITE_DQS + 1 + dm) << 2));
}

/*
 * USER Zero all DQS config
 * TODO: maybe rename to scc_mgr_zero_dqs_config (or something)
 */
static void scc_mgr_zero_all(void)
{
	uint32_t i, r;

	/*
	 * USER Zero all DQS config settings, across all groups and all
	 * shadow registers
	 */
	for (r = 0; r < RW_MGR_MEM_NUMBER_OF_RANKS; r +=
	     NUM_RANKS_PER_SHADOW_REG) {
		for (i = 0; i < RW_MGR_MEM_IF_READ_DQS_WIDTH; i++) {
			/*
			 * The phases actually don't exist on a per-rank basis,
			 * but there's no harm updating them several times, so
			 * let's keep the code simple.
			 */
			scc_mgr_set_dqs_bus_in_delay(i, IO_DQS_IN_RESERVE);
			scc_mgr_set_dqs_en_phase(i, 0);
			scc_mgr_set_dqs_en_delay(i, 0);
		}

		for (i = 0; i < RW_MGR_MEM_IF_WRITE_DQS_WIDTH; i++) {
			scc_mgr_set_dqdqs_output_phase(i, 0);
			/* av/cv don't have out2 */
			scc_mgr_set_oct_out1_delay(i, IO_DQS_OUT_RESERVE);
		}
	}

	/* multicast to all DQS group enables */
	writel(0xff, &sdr_scc_mgr->dqs_ena);
	writel(0, &sdr_scc_mgr->update);
}

static void scc_set_bypass_mode(uint32_t write_group, uint32_t mode)
{
	/* mode = 0 : Do NOT bypass - Half Rate Mode */
	/* mode = 1 : Bypass - Full Rate Mode */

	/* only need to set once for all groups, pins, dq, dqs, dm */
	if (write_group == 0) {
		debug_cond(DLEVEL == 1, "%s:%d Setting HHP Extras\n", __func__,
			   __LINE__);
		scc_mgr_set_hhp_extras();
		debug_cond(DLEVEL == 1, "%s:%d Done Setting HHP Extras\n",
			  __func__, __LINE__);
	}
	/* multicast to all DQ enables */
	writel(0xff, &sdr_scc_mgr->dq_ena);
	writel(0xff, &sdr_scc_mgr->dm_ena);

	/* update current DQS IO enable */
	writel(0, &sdr_scc_mgr->dqs_io_ena);

	/* update the DQS logic */
	writel(write_group, &sdr_scc_mgr->dqs_ena);

	/* hit update */
	writel(0, &sdr_scc_mgr->update);
}

static void scc_mgr_zero_group(uint32_t write_group, uint32_t test_begin,
			       int32_t out_only)
{
	uint32_t i, r;

	for (r = 0; r < RW_MGR_MEM_NUMBER_OF_RANKS; r +=
		NUM_RANKS_PER_SHADOW_REG) {
		/* Zero all DQ config settings */
		for (i = 0; i < RW_MGR_MEM_DQ_PER_WRITE_DQS; i++) {
			scc_mgr_set_dq_out1_delay(write_group, i, 0);
			if (!out_only)
				scc_mgr_set_dq_in_delay(write_group, i, 0);
		}

		/* multicast to all DQ enables */
		writel(0xff, &sdr_scc_mgr->dq_ena);

		/* Zero all DM config settings */
		for (i = 0; i < RW_MGR_NUM_DM_PER_WRITE_GROUP; i++) {
			scc_mgr_set_dm_out1_delay(write_group, i, 0);
		}

		/* multicast to all DM enables */
		writel(0xff, &sdr_scc_mgr->dm_ena);

		/* zero all DQS io settings */
		if (!out_only)
			scc_mgr_set_dqs_io_in_delay(write_group, 0);
		/* av/cv don't have out2 */
		scc_mgr_set_dqs_out1_delay(write_group, IO_DQS_OUT_RESERVE);
		scc_mgr_set_oct_out1_delay(write_group, IO_DQS_OUT_RESERVE);
		scc_mgr_load_dqs_for_write_group(write_group);

		/* multicast to all DQS IO enables (only 1) */
		writel(0, &sdr_scc_mgr->dqs_io_ena);

		/* hit update to zero everything */
		writel(0, &sdr_scc_mgr->update);
	}
}

/* load up dqs config settings */
static void scc_mgr_load_dqs(uint32_t dqs)
{
	writel(dqs, &sdr_scc_mgr->dqs_ena);
}

static void scc_mgr_load_dqs_for_write_group(uint32_t write_group)
{
	uint32_t read_group;
	uint32_t addr = (u32)&sdr_scc_mgr->dqs_ena;
	/*
	 * Although OCT affects only write data, the OCT delay is controlled
	 * by the DQS logic block which is instantiated once per read group.
	 * For protocols where a write group consists of multiple read groups,
	 * the setting must be scanned multiple times.
	 */
	for (read_group = write_group * RW_MGR_MEM_IF_READ_DQS_WIDTH /
	     RW_MGR_MEM_IF_WRITE_DQS_WIDTH;
	     read_group < (write_group + 1) * RW_MGR_MEM_IF_READ_DQS_WIDTH /
	     RW_MGR_MEM_IF_WRITE_DQS_WIDTH; ++read_group)
		writel(read_group, addr);
}

/* load up dqs io config settings */
static void scc_mgr_load_dqs_io(void)
{
	writel(0, &sdr_scc_mgr->dqs_io_ena);
}

/* load up dq config settings */
static void scc_mgr_load_dq(uint32_t dq_in_group)
{
	writel(dq_in_group, &sdr_scc_mgr->dq_ena);
}

/* load up dm config settings */
static void scc_mgr_load_dm(uint32_t dm)
{
	writel(dm, &sdr_scc_mgr->dm_ena);
}

/*
 * apply and load a particular input delay for the DQ pins in a group
 * group_bgn is the index of the first dq pin (in the write group)
 */
static void scc_mgr_apply_group_dq_in_delay(uint32_t write_group,
					    uint32_t group_bgn, uint32_t delay)
{
	uint32_t i, p;

	for (i = 0, p = group_bgn; i < RW_MGR_MEM_DQ_PER_READ_DQS; i++, p++) {
		scc_mgr_set_dq_in_delay(write_group, p, delay);
		scc_mgr_load_dq(p);
	}
}

/* apply and load a particular output delay for the DQ pins in a group */
static void scc_mgr_apply_group_dq_out1_delay(uint32_t write_group,
					      uint32_t group_bgn,
					      uint32_t delay1)
{
	uint32_t i, p;

	for (i = 0, p = group_bgn; i < RW_MGR_MEM_DQ_PER_WRITE_DQS; i++, p++) {
		scc_mgr_set_dq_out1_delay(write_group, i, delay1);
		scc_mgr_load_dq(i);
	}
}

/* apply and load a particular output delay for the DM pins in a group */
static void scc_mgr_apply_group_dm_out1_delay(uint32_t write_group,
					      uint32_t delay1)
{
	uint32_t i;

	for (i = 0; i < RW_MGR_NUM_DM_PER_WRITE_GROUP; i++) {
		scc_mgr_set_dm_out1_delay(write_group, i, delay1);
		scc_mgr_load_dm(i);
	}
}


/* apply and load delay on both DQS and OCT out1 */
static void scc_mgr_apply_group_dqs_io_and_oct_out1(uint32_t write_group,
						    uint32_t delay)
{
	scc_mgr_set_dqs_out1_delay(write_group, delay);
	scc_mgr_load_dqs_io();

	scc_mgr_set_oct_out1_delay(write_group, delay);
	scc_mgr_load_dqs_for_write_group(write_group);
}

/* apply a delay to the entire output side: DQ, DM, DQS, OCT */
static void scc_mgr_apply_group_all_out_delay_add(uint32_t write_group,
						  uint32_t group_bgn,
						  uint32_t delay)
{
	uint32_t i, p, new_delay;

	/* dq shift */
	for (i = 0, p = group_bgn; i < RW_MGR_MEM_DQ_PER_WRITE_DQS; i++, p++) {
		new_delay = READ_SCC_DQ_OUT2_DELAY;
		new_delay += delay;

		if (new_delay > IO_IO_OUT2_DELAY_MAX) {
			debug_cond(DLEVEL == 1, "%s:%d (%u, %u, %u) DQ[%u,%u]:\
				   %u > %lu => %lu", __func__, __LINE__,
				   write_group, group_bgn, delay, i, p, new_delay,
				   (long unsigned int)IO_IO_OUT2_DELAY_MAX,
				   (long unsigned int)IO_IO_OUT2_DELAY_MAX);
			new_delay = IO_IO_OUT2_DELAY_MAX;
		}

		scc_mgr_load_dq(i);
	}

	/* dm shift */
	for (i = 0; i < RW_MGR_NUM_DM_PER_WRITE_GROUP; i++) {
		new_delay = READ_SCC_DM_IO_OUT2_DELAY;
		new_delay += delay;

		if (new_delay > IO_IO_OUT2_DELAY_MAX) {
			debug_cond(DLEVEL == 1, "%s:%d (%u, %u, %u) DM[%u]:\
				   %u > %lu => %lu\n",  __func__, __LINE__,
				   write_group, group_bgn, delay, i, new_delay,
				   (long unsigned int)IO_IO_OUT2_DELAY_MAX,
				   (long unsigned int)IO_IO_OUT2_DELAY_MAX);
			new_delay = IO_IO_OUT2_DELAY_MAX;
		}

		scc_mgr_load_dm(i);
	}

	/* dqs shift */
	new_delay = READ_SCC_DQS_IO_OUT2_DELAY;
	new_delay += delay;

	if (new_delay > IO_IO_OUT2_DELAY_MAX) {
		debug_cond(DLEVEL == 1, "%s:%d (%u, %u, %u) DQS: %u > %d => %d;"
			   " adding %u to OUT1\n", __func__, __LINE__,
			   write_group, group_bgn, delay, new_delay,
			   IO_IO_OUT2_DELAY_MAX, IO_IO_OUT2_DELAY_MAX,
			   new_delay - IO_IO_OUT2_DELAY_MAX);
		scc_mgr_set_dqs_out1_delay(write_group, new_delay -
					   IO_IO_OUT2_DELAY_MAX);
		new_delay = IO_IO_OUT2_DELAY_MAX;
	}

	scc_mgr_load_dqs_io();

	/* oct shift */
	new_delay = READ_SCC_OCT_OUT2_DELAY;
	new_delay += delay;

	if (new_delay > IO_IO_OUT2_DELAY_MAX) {
		debug_cond(DLEVEL == 1, "%s:%d (%u, %u, %u) DQS: %u > %d => %d;"
			   " adding %u to OUT1\n", __func__, __LINE__,
			   write_group, group_bgn, delay, new_delay,
			   IO_IO_OUT2_DELAY_MAX, IO_IO_OUT2_DELAY_MAX,
			   new_delay - IO_IO_OUT2_DELAY_MAX);
		scc_mgr_set_oct_out1_delay(write_group, new_delay -
					   IO_IO_OUT2_DELAY_MAX);
		new_delay = IO_IO_OUT2_DELAY_MAX;
	}

	scc_mgr_load_dqs_for_write_group(write_group);
}

/*
 * USER apply a delay to the entire output side (DQ, DM, DQS, OCT)
 * and to all ranks
 */
static void scc_mgr_apply_group_all_out_delay_add_all_ranks(
	uint32_t write_group, uint32_t group_bgn, uint32_t delay)
{
	uint32_t r;

	for (r = 0; r < RW_MGR_MEM_NUMBER_OF_RANKS;
		r += NUM_RANKS_PER_SHADOW_REG) {
		scc_mgr_apply_group_all_out_delay_add(write_group,
						      group_bgn, delay);
		writel(0, &sdr_scc_mgr->update);
	}
}

/* optimization used to recover some slots in ddr3 inst_rom */
/* could be applied to other protocols if we wanted to */
static void set_jump_as_return(void)
{
	/*
	 * to save space, we replace return with jump to special shared
	 * RETURN instruction so we set the counter to large value so that
	 * we always jump
	 */
	writel(0xff, &sdr_rw_load_mgr_regs->load_cntr0);
	writel(RW_MGR_RETURN, &sdr_rw_load_jump_mgr_regs->load_jump_add0);
}

/*
 * should always use constants as argument to ensure all computations are
 * performed at compile time
 */
static void delay_for_n_mem_clocks(const uint32_t clocks)
{
	uint32_t afi_clocks;
	uint8_t inner = 0;
	uint8_t outer = 0;
	uint16_t c_loop = 0;

	debug("%s:%d: clocks=%u ... start\n", __func__, __LINE__, clocks);


	afi_clocks = (clocks + AFI_RATE_RATIO-1) / AFI_RATE_RATIO;
	/* scale (rounding up) to get afi clocks */

	/*
	 * Note, we don't bother accounting for being off a little bit
	 * because of a few extra instructions in outer loops
	 * Note, the loops have a test at the end, and do the test before
	 * the decrement, and so always perform the loop
	 * 1 time more than the counter value
	 */
	if (afi_clocks == 0) {
		;
	} else if (afi_clocks <= 0x100) {
		inner = afi_clocks-1;
		outer = 0;
		c_loop = 0;
	} else if (afi_clocks <= 0x10000) {
		inner = 0xff;
		outer = (afi_clocks-1) >> 8;
		c_loop = 0;
	} else {
		inner = 0xff;
		outer = 0xff;
		c_loop = (afi_clocks-1) >> 16;
	}

	/*
	 * rom instructions are structured as follows:
	 *
	 *    IDLE_LOOP2: jnz cntr0, TARGET_A
	 *    IDLE_LOOP1: jnz cntr1, TARGET_B
	 *                return
	 *
	 * so, when doing nested loops, TARGET_A is set to IDLE_LOOP2, and
	 * TARGET_B is set to IDLE_LOOP2 as well
	 *
	 * if we have no outer loop, though, then we can use IDLE_LOOP1 only,
	 * and set TARGET_B to IDLE_LOOP1 and we skip IDLE_LOOP2 entirely
	 *
	 * a little confusing, but it helps save precious space in the inst_rom
	 * and sequencer rom and keeps the delays more accurate and reduces
	 * overhead
	 */
	if (afi_clocks <= 0x100) {
		writel(SKIP_DELAY_LOOP_VALUE_OR_ZERO(inner),
			&sdr_rw_load_mgr_regs->load_cntr1);

		writel(RW_MGR_IDLE_LOOP1,
			&sdr_rw_load_jump_mgr_regs->load_jump_add1);

		writel(RW_MGR_IDLE_LOOP1, SDR_PHYGRP_RWMGRGRP_ADDRESS |
					  RW_MGR_RUN_SINGLE_GROUP_OFFSET);
	} else {
		writel(SKIP_DELAY_LOOP_VALUE_OR_ZERO(inner),
			&sdr_rw_load_mgr_regs->load_cntr0);

		writel(SKIP_DELAY_LOOP_VALUE_OR_ZERO(outer),
			&sdr_rw_load_mgr_regs->load_cntr1);

		writel(RW_MGR_IDLE_LOOP2,
			&sdr_rw_load_jump_mgr_regs->load_jump_add0);

		writel(RW_MGR_IDLE_LOOP2,
			&sdr_rw_load_jump_mgr_regs->load_jump_add1);

		/* hack to get around compiler not being smart enough */
		if (afi_clocks <= 0x10000) {
			/* only need to run once */
			writel(RW_MGR_IDLE_LOOP2, SDR_PHYGRP_RWMGRGRP_ADDRESS |
						  RW_MGR_RUN_SINGLE_GROUP_OFFSET);
		} else {
			do {
				writel(RW_MGR_IDLE_LOOP2,
					SDR_PHYGRP_RWMGRGRP_ADDRESS |
					RW_MGR_RUN_SINGLE_GROUP_OFFSET);
			} while (c_loop-- != 0);
		}
	}
	debug("%s:%d clocks=%u ... end\n", __func__, __LINE__, clocks);
}

static void rw_mgr_mem_initialize(void)
{
	uint32_t r;
	uint32_t grpaddr = SDR_PHYGRP_RWMGRGRP_ADDRESS |
			   RW_MGR_RUN_SINGLE_GROUP_OFFSET;

	debug("%s:%d\n", __func__, __LINE__);

	/* The reset / cke part of initialization is broadcasted to all ranks */
	writel(RW_MGR_RANK_ALL, SDR_PHYGRP_RWMGRGRP_ADDRESS |
				RW_MGR_SET_CS_AND_ODT_MASK_OFFSET);

	/*
	 * Here's how you load register for a loop
	 * Counters are located @ 0x800
	 * Jump address are located @ 0xC00
	 * For both, registers 0 to 3 are selected using bits 3 and 2, like
	 * in 0x800, 0x804, 0x808, 0x80C and 0xC00, 0xC04, 0xC08, 0xC0C
	 * I know this ain't pretty, but Avalon bus throws away the 2 least
	 * significant bits
	 */

	/* start with memory RESET activated */

	/* tINIT = 200us */

	/*
	 * 200us @ 266MHz (3.75 ns) ~ 54000 clock cycles
	 * If a and b are the number of iteration in 2 nested loops
	 * it takes the following number of cycles to complete the operation:
	 * number_of_cycles = ((2 + n) * a + 2) * b
	 * where n is the number of instruction in the inner loop
	 * One possible solution is n = 0 , a = 256 , b = 106 => a = FF,
	 * b = 6A
	 */

	/* Load counters */
	writel(SKIP_DELAY_LOOP_VALUE_OR_ZERO(SEQ_TINIT_CNTR0_VAL),
	       &sdr_rw_load_mgr_regs->load_cntr0);
	writel(SKIP_DELAY_LOOP_VALUE_OR_ZERO(SEQ_TINIT_CNTR1_VAL),
	       &sdr_rw_load_mgr_regs->load_cntr1);
	writel(SKIP_DELAY_LOOP_VALUE_OR_ZERO(SEQ_TINIT_CNTR2_VAL),
	       &sdr_rw_load_mgr_regs->load_cntr2);

	/* Load jump address */
	writel(RW_MGR_INIT_RESET_0_CKE_0,
		&sdr_rw_load_jump_mgr_regs->load_jump_add0);
	writel(RW_MGR_INIT_RESET_0_CKE_0,
		&sdr_rw_load_jump_mgr_regs->load_jump_add1);
	writel(RW_MGR_INIT_RESET_0_CKE_0,
		&sdr_rw_load_jump_mgr_regs->load_jump_add2);

	/* Execute count instruction */
	writel(RW_MGR_INIT_RESET_0_CKE_0, grpaddr);

	/* indicate that memory is stable */
	writel(1, &phy_mgr_cfg->reset_mem_stbl);

	/*
	 * transition the RESET to high
	 * Wait for 500us
	 */

	/*
	 * 500us @ 266MHz (3.75 ns) ~ 134000 clock cycles
	 * If a and b are the number of iteration in 2 nested loops
	 * it takes the following number of cycles to complete the operation
	 * number_of_cycles = ((2 + n) * a + 2) * b
	 * where n is the number of instruction in the inner loop
	 * One possible solution is n = 2 , a = 131 , b = 256 => a = 83,
	 * b = FF
	 */

	/* Load counters */
	writel(SKIP_DELAY_LOOP_VALUE_OR_ZERO(SEQ_TRESET_CNTR0_VAL),
	       &sdr_rw_load_mgr_regs->load_cntr0);
	writel(SKIP_DELAY_LOOP_VALUE_OR_ZERO(SEQ_TRESET_CNTR1_VAL),
	       &sdr_rw_load_mgr_regs->load_cntr1);
	writel(SKIP_DELAY_LOOP_VALUE_OR_ZERO(SEQ_TRESET_CNTR2_VAL),
	       &sdr_rw_load_mgr_regs->load_cntr2);

	/* Load jump address */
	writel(RW_MGR_INIT_RESET_1_CKE_0,
		&sdr_rw_load_jump_mgr_regs->load_jump_add0);
	writel(RW_MGR_INIT_RESET_1_CKE_0,
		&sdr_rw_load_jump_mgr_regs->load_jump_add1);
	writel(RW_MGR_INIT_RESET_1_CKE_0,
		&sdr_rw_load_jump_mgr_regs->load_jump_add2);

	writel(RW_MGR_INIT_RESET_1_CKE_0, grpaddr);

	/* bring up clock enable */

	/* tXRP < 250 ck cycles */
	delay_for_n_mem_clocks(250);

	for (r = 0; r < RW_MGR_MEM_NUMBER_OF_RANKS; r++) {
		if (param->skip_ranks[r]) {
			/* request to skip the rank */
			continue;
		}

		/* set rank */
		set_rank_and_odt_mask(r, RW_MGR_ODT_MODE_OFF);

		/*
		 * USER Use Mirror-ed commands for odd ranks if address
		 * mirrorring is on
		 */
		if ((RW_MGR_MEM_ADDRESS_MIRRORING >> r) & 0x1) {
			set_jump_as_return();
			writel(RW_MGR_MRS2_MIRR, grpaddr);
			delay_for_n_mem_clocks(4);
			set_jump_as_return();
			writel(RW_MGR_MRS3_MIRR, grpaddr);
			delay_for_n_mem_clocks(4);
			set_jump_as_return();
			writel(RW_MGR_MRS1_MIRR, grpaddr);
			delay_for_n_mem_clocks(4);
			set_jump_as_return();
			writel(RW_MGR_MRS0_DLL_RESET_MIRR, grpaddr);
		} else {
			set_jump_as_return();
			writel(RW_MGR_MRS2, grpaddr);
			delay_for_n_mem_clocks(4);
			set_jump_as_return();
			writel(RW_MGR_MRS3, grpaddr);
			delay_for_n_mem_clocks(4);
			set_jump_as_return();
			writel(RW_MGR_MRS1, grpaddr);
			set_jump_as_return();
			writel(RW_MGR_MRS0_DLL_RESET, grpaddr);
		}
		set_jump_as_return();
		writel(RW_MGR_ZQCL, grpaddr);

		/* tZQinit = tDLLK = 512 ck cycles */
		delay_for_n_mem_clocks(512);
	}
}

/*
 * At the end of calibration we have to program the user settings in, and
 * USER  hand off the memory to the user.
 */
static void rw_mgr_mem_handoff(void)
{
	uint32_t r;
	uint32_t grpaddr = SDR_PHYGRP_RWMGRGRP_ADDRESS |
			   RW_MGR_RUN_SINGLE_GROUP_OFFSET;

	debug("%s:%d\n", __func__, __LINE__);
	for (r = 0; r < RW_MGR_MEM_NUMBER_OF_RANKS; r++) {
		if (param->skip_ranks[r])
			/* request to skip the rank */
			continue;
		/* set rank */
		set_rank_and_odt_mask(r, RW_MGR_ODT_MODE_OFF);

		/* precharge all banks ... */
		writel(RW_MGR_PRECHARGE_ALL, grpaddr);

		/* load up MR settings specified by user */

		/*
		 * Use Mirror-ed commands for odd ranks if address
		 * mirrorring is on
		 */
		if ((RW_MGR_MEM_ADDRESS_MIRRORING >> r) & 0x1) {
			set_jump_as_return();
			writel(RW_MGR_MRS2_MIRR, grpaddr);
			delay_for_n_mem_clocks(4);
			set_jump_as_return();
			writel(RW_MGR_MRS3_MIRR, grpaddr);
			delay_for_n_mem_clocks(4);
			set_jump_as_return();
			writel(RW_MGR_MRS1_MIRR, grpaddr);
			delay_for_n_mem_clocks(4);
			set_jump_as_return();
			writel(RW_MGR_MRS0_USER_MIRR, grpaddr);
		} else {
			set_jump_as_return();
			writel(RW_MGR_MRS2, grpaddr);
			delay_for_n_mem_clocks(4);
			set_jump_as_return();
			writel(RW_MGR_MRS3, grpaddr);
			delay_for_n_mem_clocks(4);
			set_jump_as_return();
			writel(RW_MGR_MRS1, grpaddr);
			delay_for_n_mem_clocks(4);
			set_jump_as_return();
			writel(RW_MGR_MRS0_USER, grpaddr);
		}
		/*
		 * USER  need to wait tMOD (12CK or 15ns) time before issuing
		 * other commands, but we will have plenty of NIOS cycles before
		 * actual handoff so its okay.
		 */
	}
}

/*
 * performs a guaranteed read on the patterns we are going to use during a
 * read test to ensure memory works
 */
static uint32_t rw_mgr_mem_calibrate_read_test_patterns(uint32_t rank_bgn,
	uint32_t group, uint32_t num_tries, uint32_t *bit_chk,
	uint32_t all_ranks)
{
	uint32_t r, vg;
	uint32_t correct_mask_vg;
	uint32_t tmp_bit_chk;
	uint32_t rank_end = all_ranks ? RW_MGR_MEM_NUMBER_OF_RANKS :
		(rank_bgn + NUM_RANKS_PER_SHADOW_REG);
	uint32_t addr;
	uint32_t base_rw_mgr;

	*bit_chk = param->read_correct_mask;
	correct_mask_vg = param->read_correct_mask_vg;

	for (r = rank_bgn; r < rank_end; r++) {
		if (param->skip_ranks[r])
			/* request to skip the rank */
			continue;

		/* set rank */
		set_rank_and_odt_mask(r, RW_MGR_ODT_MODE_READ_WRITE);

		/* Load up a constant bursts of read commands */
		writel(0x20, &sdr_rw_load_mgr_regs->load_cntr0);
		writel(RW_MGR_GUARANTEED_READ,
			&sdr_rw_load_jump_mgr_regs->load_jump_add0);

		writel(0x20, &sdr_rw_load_mgr_regs->load_cntr1);
		writel(RW_MGR_GUARANTEED_READ_CONT,
			&sdr_rw_load_jump_mgr_regs->load_jump_add1);

		tmp_bit_chk = 0;
		for (vg = RW_MGR_MEM_VIRTUAL_GROUPS_PER_READ_DQS-1; ; vg--) {
			/* reset the fifos to get pointers to known state */

			writel(0, &phy_mgr_cmd->fifo_reset);
			writel(0, SDR_PHYGRP_RWMGRGRP_ADDRESS |
				  RW_MGR_RESET_READ_DATAPATH_OFFSET);

			tmp_bit_chk = tmp_bit_chk << (RW_MGR_MEM_DQ_PER_READ_DQS
				/ RW_MGR_MEM_VIRTUAL_GROUPS_PER_READ_DQS);

			addr = SDR_PHYGRP_RWMGRGRP_ADDRESS | RW_MGR_RUN_SINGLE_GROUP_OFFSET;
			writel(RW_MGR_GUARANTEED_READ, addr +
			       ((group * RW_MGR_MEM_VIRTUAL_GROUPS_PER_READ_DQS +
				vg) << 2));

			base_rw_mgr = readl(SDR_PHYGRP_RWMGRGRP_ADDRESS);
			tmp_bit_chk = tmp_bit_chk | (correct_mask_vg & (~base_rw_mgr));

			if (vg == 0)
				break;
		}
		*bit_chk &= tmp_bit_chk;
	}

	addr = SDR_PHYGRP_RWMGRGRP_ADDRESS | RW_MGR_RUN_SINGLE_GROUP_OFFSET;
	writel(RW_MGR_CLEAR_DQS_ENABLE, addr + (group << 2));

	set_rank_and_odt_mask(0, RW_MGR_ODT_MODE_OFF);
	debug_cond(DLEVEL == 1, "%s:%d test_load_patterns(%u,ALL) => (%u == %u) =>\
		   %lu\n", __func__, __LINE__, group, *bit_chk, param->read_correct_mask,
		   (long unsigned int)(*bit_chk == param->read_correct_mask));
	return *bit_chk == param->read_correct_mask;
}

static uint32_t rw_mgr_mem_calibrate_read_test_patterns_all_ranks
	(uint32_t group, uint32_t num_tries, uint32_t *bit_chk)
{
	return rw_mgr_mem_calibrate_read_test_patterns(0, group,
		num_tries, bit_chk, 1);
}

/* load up the patterns we are going to use during a read test */
static void rw_mgr_mem_calibrate_read_load_patterns(uint32_t rank_bgn,
	uint32_t all_ranks)
{
	uint32_t r;
	uint32_t rank_end = all_ranks ? RW_MGR_MEM_NUMBER_OF_RANKS :
		(rank_bgn + NUM_RANKS_PER_SHADOW_REG);

	debug("%s:%d\n", __func__, __LINE__);
	for (r = rank_bgn; r < rank_end; r++) {
		if (param->skip_ranks[r])
			/* request to skip the rank */
			continue;

		/* set rank */
		set_rank_and_odt_mask(r, RW_MGR_ODT_MODE_READ_WRITE);

		/* Load up a constant bursts */
		writel(0x20, &sdr_rw_load_mgr_regs->load_cntr0);

		writel(RW_MGR_GUARANTEED_WRITE_WAIT0,
			&sdr_rw_load_jump_mgr_regs->load_jump_add0);

		writel(0x20, &sdr_rw_load_mgr_regs->load_cntr1);

		writel(RW_MGR_GUARANTEED_WRITE_WAIT1,
			&sdr_rw_load_jump_mgr_regs->load_jump_add1);

		writel(0x04, &sdr_rw_load_mgr_regs->load_cntr2);

		writel(RW_MGR_GUARANTEED_WRITE_WAIT2,
			&sdr_rw_load_jump_mgr_regs->load_jump_add2);

		writel(0x04, &sdr_rw_load_mgr_regs->load_cntr3);

		writel(RW_MGR_GUARANTEED_WRITE_WAIT3,
			&sdr_rw_load_jump_mgr_regs->load_jump_add3);

		writel(RW_MGR_GUARANTEED_WRITE, SDR_PHYGRP_RWMGRGRP_ADDRESS |
						RW_MGR_RUN_SINGLE_GROUP_OFFSET);
	}

	set_rank_and_odt_mask(0, RW_MGR_ODT_MODE_OFF);
}

/*
 * try a read and see if it returns correct data back. has dummy reads
 * inserted into the mix used to align dqs enable. has more thorough checks
 * than the regular read test.
 */
static uint32_t rw_mgr_mem_calibrate_read_test(uint32_t rank_bgn, uint32_t group,
	uint32_t num_tries, uint32_t all_correct, uint32_t *bit_chk,
	uint32_t all_groups, uint32_t all_ranks)
{
	uint32_t r, vg;
	uint32_t correct_mask_vg;
	uint32_t tmp_bit_chk;
	uint32_t rank_end = all_ranks ? RW_MGR_MEM_NUMBER_OF_RANKS :
		(rank_bgn + NUM_RANKS_PER_SHADOW_REG);
	uint32_t addr;
	uint32_t base_rw_mgr;

	*bit_chk = param->read_correct_mask;
	correct_mask_vg = param->read_correct_mask_vg;

	uint32_t quick_read_mode = (((STATIC_CALIB_STEPS) &
		CALIB_SKIP_DELAY_SWEEPS) && ENABLE_SUPER_QUICK_CALIBRATION);

	for (r = rank_bgn; r < rank_end; r++) {
		if (param->skip_ranks[r])
			/* request to skip the rank */
			continue;

		/* set rank */
		set_rank_and_odt_mask(r, RW_MGR_ODT_MODE_READ_WRITE);

		writel(0x10, &sdr_rw_load_mgr_regs->load_cntr1);

		writel(RW_MGR_READ_B2B_WAIT1,
			&sdr_rw_load_jump_mgr_regs->load_jump_add1);

		writel(0x10, &sdr_rw_load_mgr_regs->load_cntr2);
		writel(RW_MGR_READ_B2B_WAIT2,
			&sdr_rw_load_jump_mgr_regs->load_jump_add2);

		if (quick_read_mode)
			writel(0x1, &sdr_rw_load_mgr_regs->load_cntr0);
			/* need at least two (1+1) reads to capture failures */
		else if (all_groups)
			writel(0x06, &sdr_rw_load_mgr_regs->load_cntr0);
		else
			writel(0x32, &sdr_rw_load_mgr_regs->load_cntr0);

		writel(RW_MGR_READ_B2B,
			&sdr_rw_load_jump_mgr_regs->load_jump_add0);
		if (all_groups)
			writel(RW_MGR_MEM_IF_READ_DQS_WIDTH *
			       RW_MGR_MEM_VIRTUAL_GROUPS_PER_READ_DQS - 1,
			       &sdr_rw_load_mgr_regs->load_cntr3);
		else
			writel(0x0, &sdr_rw_load_mgr_regs->load_cntr3);

		writel(RW_MGR_READ_B2B,
			&sdr_rw_load_jump_mgr_regs->load_jump_add3);

		tmp_bit_chk = 0;
		for (vg = RW_MGR_MEM_VIRTUAL_GROUPS_PER_READ_DQS-1; ; vg--) {
			/* reset the fifos to get pointers to known state */
			writel(0, &phy_mgr_cmd->fifo_reset);
			writel(0, SDR_PHYGRP_RWMGRGRP_ADDRESS |
				  RW_MGR_RESET_READ_DATAPATH_OFFSET);

			tmp_bit_chk = tmp_bit_chk << (RW_MGR_MEM_DQ_PER_READ_DQS
				/ RW_MGR_MEM_VIRTUAL_GROUPS_PER_READ_DQS);

			if (all_groups)
				addr = SDR_PHYGRP_RWMGRGRP_ADDRESS | RW_MGR_RUN_ALL_GROUPS_OFFSET;
			else
				addr = SDR_PHYGRP_RWMGRGRP_ADDRESS | RW_MGR_RUN_SINGLE_GROUP_OFFSET;

			writel(RW_MGR_READ_B2B, addr +
			       ((group * RW_MGR_MEM_VIRTUAL_GROUPS_PER_READ_DQS +
			       vg) << 2));

			base_rw_mgr = readl(SDR_PHYGRP_RWMGRGRP_ADDRESS);
			tmp_bit_chk = tmp_bit_chk | (correct_mask_vg & ~(base_rw_mgr));

			if (vg == 0)
				break;
		}
		*bit_chk &= tmp_bit_chk;
	}

	addr = SDR_PHYGRP_RWMGRGRP_ADDRESS | RW_MGR_RUN_SINGLE_GROUP_OFFSET;
	writel(RW_MGR_CLEAR_DQS_ENABLE, addr + (group << 2));

	if (all_correct) {
		set_rank_and_odt_mask(0, RW_MGR_ODT_MODE_OFF);
		debug_cond(DLEVEL == 2, "%s:%d read_test(%u,ALL,%u) =>\
			   (%u == %u) => %lu", __func__, __LINE__, group,
			   all_groups, *bit_chk, param->read_correct_mask,
			   (long unsigned int)(*bit_chk ==
			   param->read_correct_mask));
		return *bit_chk == param->read_correct_mask;
	} else	{
		set_rank_and_odt_mask(0, RW_MGR_ODT_MODE_OFF);
		debug_cond(DLEVEL == 2, "%s:%d read_test(%u,ONE,%u) =>\
			   (%u != %lu) => %lu\n", __func__, __LINE__,
			   group, all_groups, *bit_chk, (long unsigned int)0,
			   (long unsigned int)(*bit_chk != 0x00));
		return *bit_chk != 0x00;
	}
}

static uint32_t rw_mgr_mem_calibrate_read_test_all_ranks(uint32_t group,
	uint32_t num_tries, uint32_t all_correct, uint32_t *bit_chk,
	uint32_t all_groups)
{
	return rw_mgr_mem_calibrate_read_test(0, group, num_tries, all_correct,
					      bit_chk, all_groups, 1);
}

static void rw_mgr_incr_vfifo(uint32_t grp, uint32_t *v)
{
	writel(grp, &phy_mgr_cmd->inc_vfifo_hard_phy);
	(*v)++;
}

static void rw_mgr_decr_vfifo(uint32_t grp, uint32_t *v)
{
	uint32_t i;

	for (i = 0; i < VFIFO_SIZE-1; i++)
		rw_mgr_incr_vfifo(grp, v);
}

static int find_vfifo_read(uint32_t grp, uint32_t *bit_chk)
{
	uint32_t  v;
	uint32_t fail_cnt = 0;
	uint32_t test_status;

	for (v = 0; v < VFIFO_SIZE; ) {
		debug_cond(DLEVEL == 2, "%s:%d find_dqs_en_phase: vfifo %u\n",
			   __func__, __LINE__, v);
		test_status = rw_mgr_mem_calibrate_read_test_all_ranks
			(grp, 1, PASS_ONE_BIT, bit_chk, 0);
		if (!test_status) {
			fail_cnt++;

			if (fail_cnt == 2)
				break;
		}

		/* fiddle with FIFO */
		rw_mgr_incr_vfifo(grp, &v);
	}

	if (v >= VFIFO_SIZE) {
		/* no failing read found!! Something must have gone wrong */
		debug_cond(DLEVEL == 2, "%s:%d find_dqs_en_phase: vfifo failed\n",
			   __func__, __LINE__);
		return 0;
	} else {
		return v;
	}
}

static int find_working_phase(uint32_t *grp, uint32_t *bit_chk,
			      uint32_t dtaps_per_ptap, uint32_t *work_bgn,
			      uint32_t *v, uint32_t *d, uint32_t *p,
			      uint32_t *i, uint32_t *max_working_cnt)
{
	uint32_t found_begin = 0;
	uint32_t tmp_delay = 0;
	uint32_t test_status;

	for (*d = 0; *d <= dtaps_per_ptap; (*d)++, tmp_delay +=
		IO_DELAY_PER_DQS_EN_DCHAIN_TAP) {
		*work_bgn = tmp_delay;
		scc_mgr_set_dqs_en_delay_all_ranks(*grp, *d);

		for (*i = 0; *i < VFIFO_SIZE; (*i)++) {
			for (*p = 0; *p <= IO_DQS_EN_PHASE_MAX; (*p)++, *work_bgn +=
				IO_DELAY_PER_OPA_TAP) {
				scc_mgr_set_dqs_en_phase_all_ranks(*grp, *p);

				test_status =
				rw_mgr_mem_calibrate_read_test_all_ranks
				(*grp, 1, PASS_ONE_BIT, bit_chk, 0);

				if (test_status) {
					*max_working_cnt = 1;
					found_begin = 1;
					break;
				}
			}

			if (found_begin)
				break;

			if (*p > IO_DQS_EN_PHASE_MAX)
				/* fiddle with FIFO */
				rw_mgr_incr_vfifo(*grp, v);
		}

		if (found_begin)
			break;
	}

	if (*i >= VFIFO_SIZE) {
		/* cannot find working solution */
		debug_cond(DLEVEL == 2, "%s:%d find_dqs_en_phase: no vfifo/\
			   ptap/dtap\n", __func__, __LINE__);
		return 0;
	} else {
		return 1;
	}
}

static void sdr_backup_phase(uint32_t *grp, uint32_t *bit_chk,
			     uint32_t *work_bgn, uint32_t *v, uint32_t *d,
			     uint32_t *p, uint32_t *max_working_cnt)
{
	uint32_t found_begin = 0;
	uint32_t tmp_delay;

	/* Special case code for backing up a phase */
	if (*p == 0) {
		*p = IO_DQS_EN_PHASE_MAX;
		rw_mgr_decr_vfifo(*grp, v);
	} else {
		(*p)--;
	}
	tmp_delay = *work_bgn - IO_DELAY_PER_OPA_TAP;
	scc_mgr_set_dqs_en_phase_all_ranks(*grp, *p);

	for (*d = 0; *d <= IO_DQS_EN_DELAY_MAX && tmp_delay < *work_bgn;
		(*d)++, tmp_delay += IO_DELAY_PER_DQS_EN_DCHAIN_TAP) {
		scc_mgr_set_dqs_en_delay_all_ranks(*grp, *d);

		if (rw_mgr_mem_calibrate_read_test_all_ranks(*grp, 1,
							     PASS_ONE_BIT,
							     bit_chk, 0)) {
			found_begin = 1;
			*work_bgn = tmp_delay;
			break;
		}
	}

	/* We have found a working dtap before the ptap found above */
	if (found_begin == 1)
		(*max_working_cnt)++;

	/*
	 * Restore VFIFO to old state before we decremented it
	 * (if needed).
	 */
	(*p)++;
	if (*p > IO_DQS_EN_PHASE_MAX) {
		*p = 0;
		rw_mgr_incr_vfifo(*grp, v);
	}

	scc_mgr_set_dqs_en_delay_all_ranks(*grp, 0);
}

static int sdr_nonworking_phase(uint32_t *grp, uint32_t *bit_chk,
			     uint32_t *work_bgn, uint32_t *v, uint32_t *d,
			     uint32_t *p, uint32_t *i, uint32_t *max_working_cnt,
			     uint32_t *work_end)
{
	uint32_t found_end = 0;

	(*p)++;
	*work_end += IO_DELAY_PER_OPA_TAP;
	if (*p > IO_DQS_EN_PHASE_MAX) {
		/* fiddle with FIFO */
		*p = 0;
		rw_mgr_incr_vfifo(*grp, v);
	}

	for (; *i < VFIFO_SIZE + 1; (*i)++) {
		for (; *p <= IO_DQS_EN_PHASE_MAX; (*p)++, *work_end
			+= IO_DELAY_PER_OPA_TAP) {
			scc_mgr_set_dqs_en_phase_all_ranks(*grp, *p);

			if (!rw_mgr_mem_calibrate_read_test_all_ranks
				(*grp, 1, PASS_ONE_BIT, bit_chk, 0)) {
				found_end = 1;
				break;
			} else {
				(*max_working_cnt)++;
			}
		}

		if (found_end)
			break;

		if (*p > IO_DQS_EN_PHASE_MAX) {
			/* fiddle with FIFO */
			rw_mgr_incr_vfifo(*grp, v);
			*p = 0;
		}
	}

	if (*i >= VFIFO_SIZE + 1) {
		/* cannot see edge of failing read */
		debug_cond(DLEVEL == 2, "%s:%d sdr_nonworking_phase: end:\
			   failed\n", __func__, __LINE__);
		return 0;
	} else {
		return 1;
	}
}

static int sdr_find_window_centre(uint32_t *grp, uint32_t *bit_chk,
				  uint32_t *work_bgn, uint32_t *v, uint32_t *d,
				  uint32_t *p, uint32_t *work_mid,
				  uint32_t *work_end)
{
	int i;
	int tmp_delay = 0;

	*work_mid = (*work_bgn + *work_end) / 2;

	debug_cond(DLEVEL == 2, "work_bgn=%d work_end=%d work_mid=%d\n",
		   *work_bgn, *work_end, *work_mid);
	/* Get the middle delay to be less than a VFIFO delay */
	for (*p = 0; *p <= IO_DQS_EN_PHASE_MAX;
		(*p)++, tmp_delay += IO_DELAY_PER_OPA_TAP)
		;
	debug_cond(DLEVEL == 2, "vfifo ptap delay %d\n", tmp_delay);
	while (*work_mid > tmp_delay)
		*work_mid -= tmp_delay;
	debug_cond(DLEVEL == 2, "new work_mid %d\n", *work_mid);

	tmp_delay = 0;
	for (*p = 0; *p <= IO_DQS_EN_PHASE_MAX && tmp_delay < *work_mid;
		(*p)++, tmp_delay += IO_DELAY_PER_OPA_TAP)
		;
	tmp_delay -= IO_DELAY_PER_OPA_TAP;
	debug_cond(DLEVEL == 2, "new p %d, tmp_delay=%d\n", (*p) - 1, tmp_delay);
	for (*d = 0; *d <= IO_DQS_EN_DELAY_MAX && tmp_delay < *work_mid; (*d)++,
		tmp_delay += IO_DELAY_PER_DQS_EN_DCHAIN_TAP)
		;
	debug_cond(DLEVEL == 2, "new d %d, tmp_delay=%d\n", *d, tmp_delay);

	scc_mgr_set_dqs_en_phase_all_ranks(*grp, (*p) - 1);
	scc_mgr_set_dqs_en_delay_all_ranks(*grp, *d);

	/*
	 * push vfifo until we can successfully calibrate. We can do this
	 * because the largest possible margin in 1 VFIFO cycle.
	 */
	for (i = 0; i < VFIFO_SIZE; i++) {
		debug_cond(DLEVEL == 2, "find_dqs_en_phase: center: vfifo=%u\n",
			   *v);
		if (rw_mgr_mem_calibrate_read_test_all_ranks(*grp, 1,
							     PASS_ONE_BIT,
							     bit_chk, 0)) {
			break;
		}

		/* fiddle with FIFO */
		rw_mgr_incr_vfifo(*grp, v);
	}

	if (i >= VFIFO_SIZE) {
		debug_cond(DLEVEL == 2, "%s:%d find_dqs_en_phase: center: \
			   failed\n", __func__, __LINE__);
		return 0;
	} else {
		return 1;
	}
}

/* find a good dqs enable to use */
static uint32_t rw_mgr_mem_calibrate_vfifo_find_dqs_en_phase(uint32_t grp)
{
	uint32_t v, d, p, i;
	uint32_t max_working_cnt;
	uint32_t bit_chk;
	uint32_t dtaps_per_ptap;
	uint32_t work_bgn, work_mid, work_end;
	uint32_t found_passing_read, found_failing_read, initial_failing_dtap;

	debug("%s:%d %u\n", __func__, __LINE__, grp);

	reg_file_set_sub_stage(CAL_SUBSTAGE_VFIFO_CENTER);

	scc_mgr_set_dqs_en_delay_all_ranks(grp, 0);
	scc_mgr_set_dqs_en_phase_all_ranks(grp, 0);

	/* ************************************************************** */
	/* * Step 0 : Determine number of delay taps for each phase tap * */
	dtaps_per_ptap = IO_DELAY_PER_OPA_TAP/IO_DELAY_PER_DQS_EN_DCHAIN_TAP;

	/* ********************************************************* */
	/* * Step 1 : First push vfifo until we get a failing read * */
	v = find_vfifo_read(grp, &bit_chk);

	max_working_cnt = 0;

	/* ******************************************************** */
	/* * step 2: find first working phase, increment in ptaps * */
	work_bgn = 0;
	if (find_working_phase(&grp, &bit_chk, dtaps_per_ptap, &work_bgn, &v, &d,
				&p, &i, &max_working_cnt) == 0)
		return 0;

	work_end = work_bgn;

	/*
	 * If d is 0 then the working window covers a phase tap and
	 * we can follow the old procedure otherwise, we've found the beginning,
	 * and we need to increment the dtaps until we find the end.
	 */
	if (d == 0) {
		/* ********************************************************* */
		/* * step 3a: if we have room, back off by one and
		increment in dtaps * */

		sdr_backup_phase(&grp, &bit_chk, &work_bgn, &v, &d, &p,
				 &max_working_cnt);

		/* ********************************************************* */
		/* * step 4a: go forward from working phase to non working
		phase, increment in ptaps * */
		if (sdr_nonworking_phase(&grp, &bit_chk, &work_bgn, &v, &d, &p,
					 &i, &max_working_cnt, &work_end) == 0)
			return 0;

		/* ********************************************************* */
		/* * step 5a:  back off one from last, increment in dtaps  * */

		/* Special case code for backing up a phase */
		if (p == 0) {
			p = IO_DQS_EN_PHASE_MAX;
			rw_mgr_decr_vfifo(grp, &v);
		} else {
			p = p - 1;
		}

		work_end -= IO_DELAY_PER_OPA_TAP;
		scc_mgr_set_dqs_en_phase_all_ranks(grp, p);

		/* * The actual increment of dtaps is done outside of
		the if/else loop to share code */
		d = 0;

		debug_cond(DLEVEL == 2, "%s:%d find_dqs_en_phase: v/p: \
			   vfifo=%u ptap=%u\n", __func__, __LINE__,
			   v, p);
	} else {
		/* ******************************************************* */
		/* * step 3-5b:  Find the right edge of the window using
		delay taps   * */
		debug_cond(DLEVEL == 2, "%s:%d find_dqs_en_phase:vfifo=%u \
			   ptap=%u dtap=%u bgn=%u\n", __func__, __LINE__,
			   v, p, d, work_bgn);

		work_end = work_bgn;

		/* * The actual increment of dtaps is done outside of the
		if/else loop to share code */

		/* Only here to counterbalance a subtract later on which is
		not needed if this branch of the algorithm is taken */
		max_working_cnt++;
	}

	/* The dtap increment to find the failing edge is done here */
	for (; d <= IO_DQS_EN_DELAY_MAX; d++, work_end +=
		IO_DELAY_PER_DQS_EN_DCHAIN_TAP) {
			debug_cond(DLEVEL == 2, "%s:%d find_dqs_en_phase: \
				   end-2: dtap=%u\n", __func__, __LINE__, d);
			scc_mgr_set_dqs_en_delay_all_ranks(grp, d);

			if (!rw_mgr_mem_calibrate_read_test_all_ranks(grp, 1,
								      PASS_ONE_BIT,
								      &bit_chk, 0)) {
				break;
			}
	}

	/* Go back to working dtap */
	if (d != 0)
		work_end -= IO_DELAY_PER_DQS_EN_DCHAIN_TAP;

	debug_cond(DLEVEL == 2, "%s:%d find_dqs_en_phase: v/p/d: vfifo=%u \
		   ptap=%u dtap=%u end=%u\n", __func__, __LINE__,
		   v, p, d-1, work_end);

	if (work_end < work_bgn) {
		/* nil range */
		debug_cond(DLEVEL == 2, "%s:%d find_dqs_en_phase: end-2: \
			   failed\n", __func__, __LINE__);
		return 0;
	}

	debug_cond(DLEVEL == 2, "%s:%d find_dqs_en_phase: found range [%u,%u]\n",
		   __func__, __LINE__, work_bgn, work_end);

	/* *************************************************************** */
	/*
	 * * We need to calculate the number of dtaps that equal a ptap
	 * * To do that we'll back up a ptap and re-find the edge of the
	 * * window using dtaps
	 */

	debug_cond(DLEVEL == 2, "%s:%d find_dqs_en_phase: calculate dtaps_per_ptap \
		   for tracking\n", __func__, __LINE__);

	/* Special case code for backing up a phase */
	if (p == 0) {
		p = IO_DQS_EN_PHASE_MAX;
		rw_mgr_decr_vfifo(grp, &v);
		debug_cond(DLEVEL == 2, "%s:%d find_dqs_en_phase: backedup \
			   cycle/phase: v=%u p=%u\n", __func__, __LINE__,
			   v, p);
	} else {
		p = p - 1;
		debug_cond(DLEVEL == 2, "%s:%d find_dqs_en_phase: backedup \
			   phase only: v=%u p=%u", __func__, __LINE__,
			   v, p);
	}

	scc_mgr_set_dqs_en_phase_all_ranks(grp, p);

	/*
	 * Increase dtap until we first see a passing read (in case the
	 * window is smaller than a ptap),
	 * and then a failing read to mark the edge of the window again
	 */

	/* Find a passing read */
	debug_cond(DLEVEL == 2, "%s:%d find_dqs_en_phase: find passing read\n",
		   __func__, __LINE__);
	found_passing_read = 0;
	found_failing_read = 0;
	initial_failing_dtap = d;
	for (; d <= IO_DQS_EN_DELAY_MAX; d++) {
		debug_cond(DLEVEL == 2, "%s:%d find_dqs_en_phase: testing \
			   read d=%u\n", __func__, __LINE__, d);
		scc_mgr_set_dqs_en_delay_all_ranks(grp, d);

		if (rw_mgr_mem_calibrate_read_test_all_ranks(grp, 1,
							     PASS_ONE_BIT,
							     &bit_chk, 0)) {
			found_passing_read = 1;
			break;
		}
	}

	if (found_passing_read) {
		/* Find a failing read */
		debug_cond(DLEVEL == 2, "%s:%d find_dqs_en_phase: find failing \
			   read\n", __func__, __LINE__);
		for (d = d + 1; d <= IO_DQS_EN_DELAY_MAX; d++) {
			debug_cond(DLEVEL == 2, "%s:%d find_dqs_en_phase: \
				   testing read d=%u\n", __func__, __LINE__, d);
			scc_mgr_set_dqs_en_delay_all_ranks(grp, d);

			if (!rw_mgr_mem_calibrate_read_test_all_ranks
				(grp, 1, PASS_ONE_BIT, &bit_chk, 0)) {
				found_failing_read = 1;
				break;
			}
		}
	} else {
		debug_cond(DLEVEL == 1, "%s:%d find_dqs_en_phase: failed to \
			   calculate dtaps", __func__, __LINE__);
		debug_cond(DLEVEL == 1, "per ptap. Fall back on static value\n");
	}

	/*
	 * The dynamically calculated dtaps_per_ptap is only valid if we
	 * found a passing/failing read. If we didn't, it means d hit the max
	 * (IO_DQS_EN_DELAY_MAX). Otherwise, dtaps_per_ptap retains its
	 * statically calculated value.
	 */
	if (found_passing_read && found_failing_read)
		dtaps_per_ptap = d - initial_failing_dtap;

	writel(dtaps_per_ptap, &sdr_reg_file->dtaps_per_ptap);
	debug_cond(DLEVEL == 2, "%s:%d find_dqs_en_phase: dtaps_per_ptap=%u \
		   - %u = %u",  __func__, __LINE__, d,
		   initial_failing_dtap, dtaps_per_ptap);

	/* ******************************************** */
	/* * step 6:  Find the centre of the window   * */
	if (sdr_find_window_centre(&grp, &bit_chk, &work_bgn, &v, &d, &p,
				   &work_mid, &work_end) == 0)
		return 0;

	debug_cond(DLEVEL == 2, "%s:%d find_dqs_en_phase: center found: \
		   vfifo=%u ptap=%u dtap=%u\n", __func__, __LINE__,
		   v, p-1, d);
	return 1;
}

/*
 * Try rw_mgr_mem_calibrate_vfifo_find_dqs_en_phase across different
 * dq_in_delay values
 */
static uint32_t
rw_mgr_mem_calibrate_vfifo_find_dqs_en_phase_sweep_dq_in_delay
(uint32_t write_group, uint32_t read_group, uint32_t test_bgn)
{
	uint32_t found;
	uint32_t i;
	uint32_t p;
	uint32_t d;
	uint32_t r;

	const uint32_t delay_step = IO_IO_IN_DELAY_MAX /
		(RW_MGR_MEM_DQ_PER_READ_DQS-1);
		/* we start at zero, so have one less dq to devide among */

	debug("%s:%d (%u,%u,%u)", __func__, __LINE__, write_group, read_group,
	      test_bgn);

	/* try different dq_in_delays since the dq path is shorter than dqs */

	for (r = 0; r < RW_MGR_MEM_NUMBER_OF_RANKS;
	     r += NUM_RANKS_PER_SHADOW_REG) {
		for (i = 0, p = test_bgn, d = 0; i < RW_MGR_MEM_DQ_PER_READ_DQS;
			i++, p++, d += delay_step) {
			debug_cond(DLEVEL == 1, "%s:%d rw_mgr_mem_calibrate_\
				   vfifo_find_dqs_", __func__, __LINE__);
			debug_cond(DLEVEL == 1, "en_phase_sweep_dq_in_delay: g=%u/%u ",
			       write_group, read_group);
			debug_cond(DLEVEL == 1, "r=%u, i=%u p=%u d=%u\n", r, i , p, d);
			scc_mgr_set_dq_in_delay(write_group, p, d);
			scc_mgr_load_dq(p);
		}
		writel(0, &sdr_scc_mgr->update);
	}

	found = rw_mgr_mem_calibrate_vfifo_find_dqs_en_phase(read_group);

	debug_cond(DLEVEL == 1, "%s:%d rw_mgr_mem_calibrate_vfifo_find_dqs_\
		   en_phase_sweep_dq", __func__, __LINE__);
	debug_cond(DLEVEL == 1, "_in_delay: g=%u/%u found=%u; Reseting delay \
		   chain to zero\n", write_group, read_group, found);

	for (r = 0; r < RW_MGR_MEM_NUMBER_OF_RANKS;
	     r += NUM_RANKS_PER_SHADOW_REG) {
		for (i = 0, p = test_bgn; i < RW_MGR_MEM_DQ_PER_READ_DQS;
			i++, p++) {
			scc_mgr_set_dq_in_delay(write_group, p, 0);
			scc_mgr_load_dq(p);
		}
		writel(0, &sdr_scc_mgr->update);
	}

	return found;
}

/* per-bit deskew DQ and center */
static uint32_t rw_mgr_mem_calibrate_vfifo_center(uint32_t rank_bgn,
	uint32_t write_group, uint32_t read_group, uint32_t test_bgn,
	uint32_t use_read_test, uint32_t update_fom)
{
	uint32_t i, p, d, min_index;
	/*
	 * Store these as signed since there are comparisons with
	 * signed numbers.
	 */
	uint32_t bit_chk;
	uint32_t sticky_bit_chk;
	int32_t left_edge[RW_MGR_MEM_DQ_PER_READ_DQS];
	int32_t right_edge[RW_MGR_MEM_DQ_PER_READ_DQS];
	int32_t final_dq[RW_MGR_MEM_DQ_PER_READ_DQS];
	int32_t mid;
	int32_t orig_mid_min, mid_min;
	int32_t new_dqs, start_dqs, start_dqs_en, shift_dq, final_dqs,
		final_dqs_en;
	int32_t dq_margin, dqs_margin;
	uint32_t stop;
	uint32_t temp_dq_in_delay1, temp_dq_in_delay2;
	uint32_t addr;

	debug("%s:%d: %u %u", __func__, __LINE__, read_group, test_bgn);

	addr = SDR_PHYGRP_SCCGRP_ADDRESS | SCC_MGR_DQS_IN_DELAY_OFFSET;
	start_dqs = readl(addr + (read_group << 2));
	if (IO_SHIFT_DQS_EN_WHEN_SHIFT_DQS)
		start_dqs_en = readl(addr + ((read_group << 2)
				     - IO_DQS_EN_DELAY_OFFSET));

	/* set the left and right edge of each bit to an illegal value */
	/* use (IO_IO_IN_DELAY_MAX + 1) as an illegal value */
	sticky_bit_chk = 0;
	for (i = 0; i < RW_MGR_MEM_DQ_PER_READ_DQS; i++) {
		left_edge[i]  = IO_IO_IN_DELAY_MAX + 1;
		right_edge[i] = IO_IO_IN_DELAY_MAX + 1;
	}

	/* Search for the left edge of the window for each bit */
	for (d = 0; d <= IO_IO_IN_DELAY_MAX; d++) {
		scc_mgr_apply_group_dq_in_delay(write_group, test_bgn, d);

		writel(0, &sdr_scc_mgr->update);

		/*
		 * Stop searching when the read test doesn't pass AND when
		 * we've seen a passing read on every bit.
		 */
		if (use_read_test) {
			stop = !rw_mgr_mem_calibrate_read_test(rank_bgn,
				read_group, NUM_READ_PB_TESTS, PASS_ONE_BIT,
				&bit_chk, 0, 0);
		} else {
			rw_mgr_mem_calibrate_write_test(rank_bgn, write_group,
							0, PASS_ONE_BIT,
							&bit_chk, 0);
			bit_chk = bit_chk >> (RW_MGR_MEM_DQ_PER_READ_DQS *
				(read_group - (write_group *
					RW_MGR_MEM_IF_READ_DQS_WIDTH /
					RW_MGR_MEM_IF_WRITE_DQS_WIDTH)));
			stop = (bit_chk == 0);
		}
		sticky_bit_chk = sticky_bit_chk | bit_chk;
		stop = stop && (sticky_bit_chk == param->read_correct_mask);
		debug_cond(DLEVEL == 2, "%s:%d vfifo_center(left): dtap=%u => %u == %u \
			   && %u", __func__, __LINE__, d,
			   sticky_bit_chk,
			param->read_correct_mask, stop);

		if (stop == 1) {
			break;
		} else {
			for (i = 0; i < RW_MGR_MEM_DQ_PER_READ_DQS; i++) {
				if (bit_chk & 1) {
					/* Remember a passing test as the
					left_edge */
					left_edge[i] = d;
				} else {
					/* If a left edge has not been seen yet,
					then a future passing test will mark
					this edge as the right edge */
					if (left_edge[i] ==
						IO_IO_IN_DELAY_MAX + 1) {
						right_edge[i] = -(d + 1);
					}
				}
				bit_chk = bit_chk >> 1;
			}
		}
	}

	/* Reset DQ delay chains to 0 */
	scc_mgr_apply_group_dq_in_delay(write_group, test_bgn, 0);
	sticky_bit_chk = 0;
	for (i = RW_MGR_MEM_DQ_PER_READ_DQS - 1;; i--) {
		debug_cond(DLEVEL == 2, "%s:%d vfifo_center: left_edge[%u]: \
			   %d right_edge[%u]: %d\n", __func__, __LINE__,
			   i, left_edge[i], i, right_edge[i]);

		/*
		 * Check for cases where we haven't found the left edge,
		 * which makes our assignment of the the right edge invalid.
		 * Reset it to the illegal value.
		 */
		if ((left_edge[i] == IO_IO_IN_DELAY_MAX + 1) && (
			right_edge[i] != IO_IO_IN_DELAY_MAX + 1)) {
			right_edge[i] = IO_IO_IN_DELAY_MAX + 1;
			debug_cond(DLEVEL == 2, "%s:%d vfifo_center: reset \
				   right_edge[%u]: %d\n", __func__, __LINE__,
				   i, right_edge[i]);
		}

		/*
		 * Reset sticky bit (except for bits where we have seen
		 * both the left and right edge).
		 */
		sticky_bit_chk = sticky_bit_chk << 1;
		if ((left_edge[i] != IO_IO_IN_DELAY_MAX + 1) &&
		    (right_edge[i] != IO_IO_IN_DELAY_MAX + 1)) {
			sticky_bit_chk = sticky_bit_chk | 1;
		}

		if (i == 0)
			break;
	}

	/* Search for the right edge of the window for each bit */
	for (d = 0; d <= IO_DQS_IN_DELAY_MAX - start_dqs; d++) {
		scc_mgr_set_dqs_bus_in_delay(read_group, d + start_dqs);
		if (IO_SHIFT_DQS_EN_WHEN_SHIFT_DQS) {
			uint32_t delay = d + start_dqs_en;
			if (delay > IO_DQS_EN_DELAY_MAX)
				delay = IO_DQS_EN_DELAY_MAX;
			scc_mgr_set_dqs_en_delay(read_group, delay);
		}
		scc_mgr_load_dqs(read_group);

		writel(0, &sdr_scc_mgr->update);

		/*
		 * Stop searching when the read test doesn't pass AND when
		 * we've seen a passing read on every bit.
		 */
		if (use_read_test) {
			stop = !rw_mgr_mem_calibrate_read_test(rank_bgn,
				read_group, NUM_READ_PB_TESTS, PASS_ONE_BIT,
				&bit_chk, 0, 0);
		} else {
			rw_mgr_mem_calibrate_write_test(rank_bgn, write_group,
							0, PASS_ONE_BIT,
							&bit_chk, 0);
			bit_chk = bit_chk >> (RW_MGR_MEM_DQ_PER_READ_DQS *
				(read_group - (write_group *
					RW_MGR_MEM_IF_READ_DQS_WIDTH /
					RW_MGR_MEM_IF_WRITE_DQS_WIDTH)));
			stop = (bit_chk == 0);
		}
		sticky_bit_chk = sticky_bit_chk | bit_chk;
		stop = stop && (sticky_bit_chk == param->read_correct_mask);

		debug_cond(DLEVEL == 2, "%s:%d vfifo_center(right): dtap=%u => %u == \
			   %u && %u", __func__, __LINE__, d,
			   sticky_bit_chk, param->read_correct_mask, stop);

		if (stop == 1) {
			break;
		} else {
			for (i = 0; i < RW_MGR_MEM_DQ_PER_READ_DQS; i++) {
				if (bit_chk & 1) {
					/* Remember a passing test as
					the right_edge */
					right_edge[i] = d;
				} else {
					if (d != 0) {
						/* If a right edge has not been
						seen yet, then a future passing
						test will mark this edge as the
						left edge */
						if (right_edge[i] ==
						IO_IO_IN_DELAY_MAX + 1) {
							left_edge[i] = -(d + 1);
						}
					} else {
						/* d = 0 failed, but it passed
						when testing the left edge,
						so it must be marginal,
						set it to -1 */
						if (right_edge[i] ==
							IO_IO_IN_DELAY_MAX + 1 &&
							left_edge[i] !=
							IO_IO_IN_DELAY_MAX
							+ 1) {
							right_edge[i] = -1;
						}
						/* If a right edge has not been
						seen yet, then a future passing
						test will mark this edge as the
						left edge */
						else if (right_edge[i] ==
							IO_IO_IN_DELAY_MAX +
							1) {
							left_edge[i] = -(d + 1);
						}
					}
				}

				debug_cond(DLEVEL == 2, "%s:%d vfifo_center[r,\
					   d=%u]: ", __func__, __LINE__, d);
				debug_cond(DLEVEL == 2, "bit_chk_test=%d left_edge[%u]: %d ",
					   (int)(bit_chk & 1), i, left_edge[i]);
				debug_cond(DLEVEL == 2, "right_edge[%u]: %d\n", i,
					   right_edge[i]);
				bit_chk = bit_chk >> 1;
			}
		}
	}

	/* Check that all bits have a window */
	for (i = 0; i < RW_MGR_MEM_DQ_PER_READ_DQS; i++) {
		debug_cond(DLEVEL == 2, "%s:%d vfifo_center: left_edge[%u]: \
			   %d right_edge[%u]: %d", __func__, __LINE__,
			   i, left_edge[i], i, right_edge[i]);
		if ((left_edge[i] == IO_IO_IN_DELAY_MAX + 1) || (right_edge[i]
			== IO_IO_IN_DELAY_MAX + 1)) {
			/*
			 * Restore delay chain settings before letting the loop
			 * in rw_mgr_mem_calibrate_vfifo to retry different
			 * dqs/ck relationships.
			 */
			scc_mgr_set_dqs_bus_in_delay(read_group, start_dqs);
			if (IO_SHIFT_DQS_EN_WHEN_SHIFT_DQS) {
				scc_mgr_set_dqs_en_delay(read_group,
							 start_dqs_en);
			}
			scc_mgr_load_dqs(read_group);
			writel(0, &sdr_scc_mgr->update);

			debug_cond(DLEVEL == 1, "%s:%d vfifo_center: failed to \
				   find edge [%u]: %d %d", __func__, __LINE__,
				   i, left_edge[i], right_edge[i]);
			if (use_read_test) {
				set_failing_group_stage(read_group *
					RW_MGR_MEM_DQ_PER_READ_DQS + i,
					CAL_STAGE_VFIFO,
					CAL_SUBSTAGE_VFIFO_CENTER);
			} else {
				set_failing_group_stage(read_group *
					RW_MGR_MEM_DQ_PER_READ_DQS + i,
					CAL_STAGE_VFIFO_AFTER_WRITES,
					CAL_SUBSTAGE_VFIFO_CENTER);
			}
			return 0;
		}
	}

	/* Find middle of window for each DQ bit */
	mid_min = left_edge[0] - right_edge[0];
	min_index = 0;
	for (i = 1; i < RW_MGR_MEM_DQ_PER_READ_DQS; i++) {
		mid = left_edge[i] - right_edge[i];
		if (mid < mid_min) {
			mid_min = mid;
			min_index = i;
		}
	}

	/*
	 * -mid_min/2 represents the amount that we need to move DQS.
	 * If mid_min is odd and positive we'll need to add one to
	 * make sure the rounding in further calculations is correct
	 * (always bias to the right), so just add 1 for all positive values.
	 */
	if (mid_min > 0)
		mid_min++;

	mid_min = mid_min / 2;

	debug_cond(DLEVEL == 1, "%s:%d vfifo_center: mid_min=%d (index=%u)\n",
		   __func__, __LINE__, mid_min, min_index);

	/* Determine the amount we can change DQS (which is -mid_min) */
	orig_mid_min = mid_min;
	new_dqs = start_dqs - mid_min;
	if (new_dqs > IO_DQS_IN_DELAY_MAX)
		new_dqs = IO_DQS_IN_DELAY_MAX;
	else if (new_dqs < 0)
		new_dqs = 0;

	mid_min = start_dqs - new_dqs;
	debug_cond(DLEVEL == 1, "vfifo_center: new mid_min=%d new_dqs=%d\n",
		   mid_min, new_dqs);

	if (IO_SHIFT_DQS_EN_WHEN_SHIFT_DQS) {
		if (start_dqs_en - mid_min > IO_DQS_EN_DELAY_MAX)
			mid_min += start_dqs_en - mid_min - IO_DQS_EN_DELAY_MAX;
		else if (start_dqs_en - mid_min < 0)
			mid_min += start_dqs_en - mid_min;
	}
	new_dqs = start_dqs - mid_min;

	debug_cond(DLEVEL == 1, "vfifo_center: start_dqs=%d start_dqs_en=%d \
		   new_dqs=%d mid_min=%d\n", start_dqs,
		   IO_SHIFT_DQS_EN_WHEN_SHIFT_DQS ? start_dqs_en : -1,
		   new_dqs, mid_min);

	/* Initialize data for export structures */
	dqs_margin = IO_IO_IN_DELAY_MAX + 1;
	dq_margin  = IO_IO_IN_DELAY_MAX + 1;

	/* add delay to bring centre of all DQ windows to the same "level" */
	for (i = 0, p = test_bgn; i < RW_MGR_MEM_DQ_PER_READ_DQS; i++, p++) {
		/* Use values before divide by 2 to reduce round off error */
		shift_dq = (left_edge[i] - right_edge[i] -
			(left_edge[min_index] - right_edge[min_index]))/2  +
			(orig_mid_min - mid_min);

		debug_cond(DLEVEL == 2, "vfifo_center: before: \
			   shift_dq[%u]=%d\n", i, shift_dq);

		addr = SDR_PHYGRP_SCCGRP_ADDRESS | SCC_MGR_IO_IN_DELAY_OFFSET;
		temp_dq_in_delay1 = readl(addr + (p << 2));
		temp_dq_in_delay2 = readl(addr + (i << 2));

		if (shift_dq + (int32_t)temp_dq_in_delay1 >
			(int32_t)IO_IO_IN_DELAY_MAX) {
			shift_dq = (int32_t)IO_IO_IN_DELAY_MAX - temp_dq_in_delay2;
		} else if (shift_dq + (int32_t)temp_dq_in_delay1 < 0) {
			shift_dq = -(int32_t)temp_dq_in_delay1;
		}
		debug_cond(DLEVEL == 2, "vfifo_center: after: \
			   shift_dq[%u]=%d\n", i, shift_dq);
		final_dq[i] = temp_dq_in_delay1 + shift_dq;
		scc_mgr_set_dq_in_delay(write_group, p, final_dq[i]);
		scc_mgr_load_dq(p);

		debug_cond(DLEVEL == 2, "vfifo_center: margin[%u]=[%d,%d]\n", i,
			   left_edge[i] - shift_dq + (-mid_min),
			   right_edge[i] + shift_dq - (-mid_min));
		/* To determine values for export structures */
		if (left_edge[i] - shift_dq + (-mid_min) < dq_margin)
			dq_margin = left_edge[i] - shift_dq + (-mid_min);

		if (right_edge[i] + shift_dq - (-mid_min) < dqs_margin)
			dqs_margin = right_edge[i] + shift_dq - (-mid_min);
	}

	final_dqs = new_dqs;
	if (IO_SHIFT_DQS_EN_WHEN_SHIFT_DQS)
		final_dqs_en = start_dqs_en - mid_min;

	/* Move DQS-en */
	if (IO_SHIFT_DQS_EN_WHEN_SHIFT_DQS) {
		scc_mgr_set_dqs_en_delay(read_group, final_dqs_en);
		scc_mgr_load_dqs(read_group);
	}

	/* Move DQS */
	scc_mgr_set_dqs_bus_in_delay(read_group, final_dqs);
	scc_mgr_load_dqs(read_group);
	debug_cond(DLEVEL == 2, "%s:%d vfifo_center: dq_margin=%d \
		   dqs_margin=%d", __func__, __LINE__,
		   dq_margin, dqs_margin);

	/*
	 * Do not remove this line as it makes sure all of our decisions
	 * have been applied. Apply the update bit.
	 */
	writel(0, &sdr_scc_mgr->update);

	return (dq_margin >= 0) && (dqs_margin >= 0);
}

/*
 * calibrate the read valid prediction FIFO.
 *
 *  - read valid prediction will consist of finding a good DQS enable phase,
 * DQS enable delay, DQS input phase, and DQS input delay.
 *  - we also do a per-bit deskew on the DQ lines.
 */
static uint32_t rw_mgr_mem_calibrate_vfifo(uint32_t read_group,
					   uint32_t test_bgn)
{
	uint32_t p, d, rank_bgn, sr;
	uint32_t dtaps_per_ptap;
	uint32_t tmp_delay;
	uint32_t bit_chk;
	uint32_t grp_calibrated;
	uint32_t write_group, write_test_bgn;
	uint32_t failed_substage;

	debug("%s:%d: %u %u\n", __func__, __LINE__, read_group, test_bgn);

	/* update info for sims */
	reg_file_set_stage(CAL_STAGE_VFIFO);

	write_group = read_group;
	write_test_bgn = test_bgn;

	/* USER Determine number of delay taps for each phase tap */
	dtaps_per_ptap = 0;
	tmp_delay = 0;
	while (tmp_delay < IO_DELAY_PER_OPA_TAP) {
		dtaps_per_ptap++;
		tmp_delay += IO_DELAY_PER_DQS_EN_DCHAIN_TAP;
	}
	dtaps_per_ptap--;
	tmp_delay = 0;

	/* update info for sims */
	reg_file_set_group(read_group);

	grp_calibrated = 0;

	reg_file_set_sub_stage(CAL_SUBSTAGE_GUARANTEED_READ);
	failed_substage = CAL_SUBSTAGE_GUARANTEED_READ;

	for (d = 0; d <= dtaps_per_ptap && grp_calibrated == 0; d += 2) {
		/*
		 * In RLDRAMX we may be messing the delay of pins in
		 * the same write group but outside of the current read
		 * the group, but that's ok because we haven't
		 * calibrated output side yet.
		 */
		if (d > 0) {
			scc_mgr_apply_group_all_out_delay_add_all_ranks
			(write_group, write_test_bgn, d);
		}

		for (p = 0; p <= IO_DQDQS_OUT_PHASE_MAX && grp_calibrated == 0;
			p++) {
			/* set a particular dqdqs phase */
			scc_mgr_set_dqdqs_output_phase_all_ranks(read_group, p);

			debug_cond(DLEVEL == 1, "%s:%d calibrate_vfifo: g=%u \
				   p=%u d=%u\n", __func__, __LINE__,
				   read_group, p, d);

			/*
			 * Load up the patterns used by read calibration
			 * using current DQDQS phase.
			 */
			rw_mgr_mem_calibrate_read_load_patterns(0, 1);
			if (!(gbl->phy_debug_mode_flags &
				PHY_DEBUG_DISABLE_GUARANTEED_READ)) {
				if (!rw_mgr_mem_calibrate_read_test_patterns_all_ranks
				    (read_group, 1, &bit_chk)) {
					debug_cond(DLEVEL == 1, "%s:%d Guaranteed read test failed:",
						   __func__, __LINE__);
					debug_cond(DLEVEL == 1, " g=%u p=%u d=%u\n",
						   read_group, p, d);
					break;
				}
			}

/* case:56390 */
			grp_calibrated = 1;
		if (rw_mgr_mem_calibrate_vfifo_find_dqs_en_phase_sweep_dq_in_delay
		    (write_group, read_group, test_bgn)) {
				/*
				 * USER Read per-bit deskew can be done on a
				 * per shadow register basis.
				 */
				for (rank_bgn = 0, sr = 0;
					rank_bgn < RW_MGR_MEM_NUMBER_OF_RANKS;
					rank_bgn += NUM_RANKS_PER_SHADOW_REG,
					++sr) {
					/*
					 * Determine if this set of ranks
					 * should be skipped entirely.
					 */
					if (!param->skip_shadow_regs[sr]) {
						/*
						 * If doing read after write
						 * calibration, do not update
						 * FOM, now - do it then.
						 */
					if (!rw_mgr_mem_calibrate_vfifo_center
						(rank_bgn, write_group,
						read_group, test_bgn, 1, 0)) {
							grp_calibrated = 0;
							failed_substage =
						CAL_SUBSTAGE_VFIFO_CENTER;
						}
					}
				}
			} else {
				grp_calibrated = 0;
				failed_substage = CAL_SUBSTAGE_DQS_EN_PHASE;
			}
		}
	}

	if (grp_calibrated == 0) {
		set_failing_group_stage(write_group, CAL_STAGE_VFIFO,
					failed_substage);
		return 0;
	}

	/*
	 * Reset the delay chains back to zero if they have moved > 1
	 * (check for > 1 because loop will increase d even when pass in
	 * first case).
	 */
	if (d > 2)
		scc_mgr_zero_group(write_group, write_test_bgn, 1);

	return 1;
}

/* VFIFO Calibration -- Read Deskew Calibration after write deskew */
static uint32_t rw_mgr_mem_calibrate_vfifo_end(uint32_t read_group,
					       uint32_t test_bgn)
{
	uint32_t rank_bgn, sr;
	uint32_t grp_calibrated;
	uint32_t write_group;

	debug("%s:%d %u %u", __func__, __LINE__, read_group, test_bgn);

	/* update info for sims */

	reg_file_set_stage(CAL_STAGE_VFIFO_AFTER_WRITES);
	reg_file_set_sub_stage(CAL_SUBSTAGE_VFIFO_CENTER);

	write_group = read_group;

	/* update info for sims */
	reg_file_set_group(read_group);

	grp_calibrated = 1;
	/* Read per-bit deskew can be done on a per shadow register basis */
	for (rank_bgn = 0, sr = 0; rank_bgn < RW_MGR_MEM_NUMBER_OF_RANKS;
		rank_bgn += NUM_RANKS_PER_SHADOW_REG, ++sr) {
		/* Determine if this set of ranks should be skipped entirely */
		if (!param->skip_shadow_regs[sr]) {
		/* This is the last calibration round, update FOM here */
			if (!rw_mgr_mem_calibrate_vfifo_center(rank_bgn,
								write_group,
								read_group,
								test_bgn, 0,
								1)) {
				grp_calibrated = 0;
			}
		}
	}


	if (grp_calibrated == 0) {
		set_failing_group_stage(write_group,
					CAL_STAGE_VFIFO_AFTER_WRITES,
					CAL_SUBSTAGE_VFIFO_CENTER);
		return 0;
	}

	return 1;
}

/* Calibrate LFIFO to find smallest read latency */
static uint32_t rw_mgr_mem_calibrate_lfifo(void)
{
	uint32_t found_one;
	uint32_t bit_chk;

	debug("%s:%d\n", __func__, __LINE__);

	/* update info for sims */
	reg_file_set_stage(CAL_STAGE_LFIFO);
	reg_file_set_sub_stage(CAL_SUBSTAGE_READ_LATENCY);

	/* Load up the patterns used by read calibration for all ranks */
	rw_mgr_mem_calibrate_read_load_patterns(0, 1);
	found_one = 0;

	do {
		writel(gbl->curr_read_lat, &phy_mgr_cfg->phy_rlat);
		debug_cond(DLEVEL == 2, "%s:%d lfifo: read_lat=%u",
			   __func__, __LINE__, gbl->curr_read_lat);

		if (!rw_mgr_mem_calibrate_read_test_all_ranks(0,
							      NUM_READ_TESTS,
							      PASS_ALL_BITS,
							      &bit_chk, 1)) {
			break;
		}

		found_one = 1;
		/* reduce read latency and see if things are working */
		/* correctly */
		gbl->curr_read_lat--;
	} while (gbl->curr_read_lat > 0);

	/* reset the fifos to get pointers to known state */

	writel(0, &phy_mgr_cmd->fifo_reset);

	if (found_one) {
		/* add a fudge factor to the read latency that was determined */
		gbl->curr_read_lat += 2;
		writel(gbl->curr_read_lat, &phy_mgr_cfg->phy_rlat);
		debug_cond(DLEVEL == 2, "%s:%d lfifo: success: using \
			   read_lat=%u\n", __func__, __LINE__,
			   gbl->curr_read_lat);
		return 1;
	} else {
		set_failing_group_stage(0xff, CAL_STAGE_LFIFO,
					CAL_SUBSTAGE_READ_LATENCY);

		debug_cond(DLEVEL == 2, "%s:%d lfifo: failed at initial \
			   read_lat=%u\n", __func__, __LINE__,
			   gbl->curr_read_lat);
		return 0;
	}
}

/*
 * issue write test command.
 * two variants are provided. one that just tests a write pattern and
 * another that tests datamask functionality.
 */
static void rw_mgr_mem_calibrate_write_test_issue(uint32_t group,
						  uint32_t test_dm)
{
	uint32_t mcc_instruction;
	uint32_t quick_write_mode = (((STATIC_CALIB_STEPS) & CALIB_SKIP_WRITES) &&
		ENABLE_SUPER_QUICK_CALIBRATION);
	uint32_t rw_wl_nop_cycles;
	uint32_t addr;

	/*
	 * Set counter and jump addresses for the right
	 * number of NOP cycles.
	 * The number of supported NOP cycles can range from -1 to infinity
	 * Three different cases are handled:
	 *
	 * 1. For a number of NOP cycles greater than 0, the RW Mgr looping
	 *    mechanism will be used to insert the right number of NOPs
	 *
	 * 2. For a number of NOP cycles equals to 0, the micro-instruction
	 *    issuing the write command will jump straight to the
	 *    micro-instruction that turns on DQS (for DDRx), or outputs write
	 *    data (for RLD), skipping
	 *    the NOP micro-instruction all together
	 *
	 * 3. A number of NOP cycles equal to -1 indicates that DQS must be
	 *    turned on in the same micro-instruction that issues the write
	 *    command. Then we need
	 *    to directly jump to the micro-instruction that sends out the data
	 *
	 * NOTE: Implementing this mechanism uses 2 RW Mgr jump-counters
	 *       (2 and 3). One jump-counter (0) is used to perform multiple
	 *       write-read operations.
	 *       one counter left to issue this command in "multiple-group" mode
	 */

	rw_wl_nop_cycles = gbl->rw_wl_nop_cycles;

	if (rw_wl_nop_cycles == -1) {
		/*
		 * CNTR 2 - We want to execute the special write operation that
		 * turns on DQS right away and then skip directly to the
		 * instruction that sends out the data. We set the counter to a
		 * large number so that the jump is always taken.
		 */
		writel(0xFF, &sdr_rw_load_mgr_regs->load_cntr2);

		/* CNTR 3 - Not used */
		if (test_dm) {
			mcc_instruction = RW_MGR_LFSR_WR_RD_DM_BANK_0_WL_1;
			writel(RW_MGR_LFSR_WR_RD_DM_BANK_0_DATA,
			       &sdr_rw_load_jump_mgr_regs->load_jump_add2);
			writel(RW_MGR_LFSR_WR_RD_DM_BANK_0_NOP,
			       &sdr_rw_load_jump_mgr_regs->load_jump_add3);
		} else {
			mcc_instruction = RW_MGR_LFSR_WR_RD_BANK_0_WL_1;
			writel(RW_MGR_LFSR_WR_RD_BANK_0_DATA,
				&sdr_rw_load_jump_mgr_regs->load_jump_add2);
			writel(RW_MGR_LFSR_WR_RD_BANK_0_NOP,
				&sdr_rw_load_jump_mgr_regs->load_jump_add3);
		}
	} else if (rw_wl_nop_cycles == 0) {
		/*
		 * CNTR 2 - We want to skip the NOP operation and go straight
		 * to the DQS enable instruction. We set the counter to a large
		 * number so that the jump is always taken.
		 */
		writel(0xFF, &sdr_rw_load_mgr_regs->load_cntr2);

		/* CNTR 3 - Not used */
		if (test_dm) {
			mcc_instruction = RW_MGR_LFSR_WR_RD_DM_BANK_0;
			writel(RW_MGR_LFSR_WR_RD_DM_BANK_0_DQS,
			       &sdr_rw_load_jump_mgr_regs->load_jump_add2);
		} else {
			mcc_instruction = RW_MGR_LFSR_WR_RD_BANK_0;
			writel(RW_MGR_LFSR_WR_RD_BANK_0_DQS,
				&sdr_rw_load_jump_mgr_regs->load_jump_add2);
		}
	} else {
		/*
		 * CNTR 2 - In this case we want to execute the next instruction
		 * and NOT take the jump. So we set the counter to 0. The jump
		 * address doesn't count.
		 */
		writel(0x0, &sdr_rw_load_mgr_regs->load_cntr2);
		writel(0x0, &sdr_rw_load_jump_mgr_regs->load_jump_add2);

		/*
		 * CNTR 3 - Set the nop counter to the number of cycles we
		 * need to loop for, minus 1.
		 */
		writel(rw_wl_nop_cycles - 1, &sdr_rw_load_mgr_regs->load_cntr3);
		if (test_dm) {
			mcc_instruction = RW_MGR_LFSR_WR_RD_DM_BANK_0;
			writel(RW_MGR_LFSR_WR_RD_DM_BANK_0_NOP,
				&sdr_rw_load_jump_mgr_regs->load_jump_add3);
		} else {
			mcc_instruction = RW_MGR_LFSR_WR_RD_BANK_0;
			writel(RW_MGR_LFSR_WR_RD_BANK_0_NOP,
				&sdr_rw_load_jump_mgr_regs->load_jump_add3);
		}
	}

	writel(0, SDR_PHYGRP_RWMGRGRP_ADDRESS |
		  RW_MGR_RESET_READ_DATAPATH_OFFSET);

	if (quick_write_mode)
		writel(0x08, &sdr_rw_load_mgr_regs->load_cntr0);
	else
		writel(0x40, &sdr_rw_load_mgr_regs->load_cntr0);

	writel(mcc_instruction, &sdr_rw_load_jump_mgr_regs->load_jump_add0);

	/*
	 * CNTR 1 - This is used to ensure enough time elapses
	 * for read data to come back.
	 */
	writel(0x30, &sdr_rw_load_mgr_regs->load_cntr1);

	if (test_dm) {
		writel(RW_MGR_LFSR_WR_RD_DM_BANK_0_WAIT,
			&sdr_rw_load_jump_mgr_regs->load_jump_add1);
	} else {
		writel(RW_MGR_LFSR_WR_RD_BANK_0_WAIT,
			&sdr_rw_load_jump_mgr_regs->load_jump_add1);
	}

	addr = SDR_PHYGRP_RWMGRGRP_ADDRESS | RW_MGR_RUN_SINGLE_GROUP_OFFSET;
	writel(mcc_instruction, addr + (group << 2));
}

/* Test writes, can check for a single bit pass or multiple bit pass */
static uint32_t rw_mgr_mem_calibrate_write_test(uint32_t rank_bgn,
	uint32_t write_group, uint32_t use_dm, uint32_t all_correct,
	uint32_t *bit_chk, uint32_t all_ranks)
{
	uint32_t r;
	uint32_t correct_mask_vg;
	uint32_t tmp_bit_chk;
	uint32_t vg;
	uint32_t rank_end = all_ranks ? RW_MGR_MEM_NUMBER_OF_RANKS :
		(rank_bgn + NUM_RANKS_PER_SHADOW_REG);
	uint32_t addr_rw_mgr;
	uint32_t base_rw_mgr;

	*bit_chk = param->write_correct_mask;
	correct_mask_vg = param->write_correct_mask_vg;

	for (r = rank_bgn; r < rank_end; r++) {
		if (param->skip_ranks[r]) {
			/* request to skip the rank */
			continue;
		}

		/* set rank */
		set_rank_and_odt_mask(r, RW_MGR_ODT_MODE_READ_WRITE);

		tmp_bit_chk = 0;
		addr_rw_mgr = SDR_PHYGRP_RWMGRGRP_ADDRESS;
		for (vg = RW_MGR_MEM_VIRTUAL_GROUPS_PER_WRITE_DQS-1; ; vg--) {
			/* reset the fifos to get pointers to known state */
			writel(0, &phy_mgr_cmd->fifo_reset);

			tmp_bit_chk = tmp_bit_chk <<
				(RW_MGR_MEM_DQ_PER_WRITE_DQS /
				RW_MGR_MEM_VIRTUAL_GROUPS_PER_WRITE_DQS);
			rw_mgr_mem_calibrate_write_test_issue(write_group *
				RW_MGR_MEM_VIRTUAL_GROUPS_PER_WRITE_DQS+vg,
				use_dm);

			base_rw_mgr = readl(addr_rw_mgr);
			tmp_bit_chk = tmp_bit_chk | (correct_mask_vg & ~(base_rw_mgr));
			if (vg == 0)
				break;
		}
		*bit_chk &= tmp_bit_chk;
	}

	if (all_correct) {
		set_rank_and_odt_mask(0, RW_MGR_ODT_MODE_OFF);
		debug_cond(DLEVEL == 2, "write_test(%u,%u,ALL) : %u == \
			   %u => %lu", write_group, use_dm,
			   *bit_chk, param->write_correct_mask,
			   (long unsigned int)(*bit_chk ==
			   param->write_correct_mask));
		return *bit_chk == param->write_correct_mask;
	} else {
		set_rank_and_odt_mask(0, RW_MGR_ODT_MODE_OFF);
		debug_cond(DLEVEL == 2, "write_test(%u,%u,ONE) : %u != ",
		       write_group, use_dm, *bit_chk);
		debug_cond(DLEVEL == 2, "%lu" " => %lu", (long unsigned int)0,
			(long unsigned int)(*bit_chk != 0));
		return *bit_chk != 0x00;
	}
}

/*
 * center all windows. do per-bit-deskew to possibly increase size of
 * certain windows.
 */
static uint32_t rw_mgr_mem_calibrate_writes_center(uint32_t rank_bgn,
	uint32_t write_group, uint32_t test_bgn)
{
	uint32_t i, p, min_index;
	int32_t d;
	/*
	 * Store these as signed since there are comparisons with
	 * signed numbers.
	 */
	uint32_t bit_chk;
	uint32_t sticky_bit_chk;
	int32_t left_edge[RW_MGR_MEM_DQ_PER_WRITE_DQS];
	int32_t right_edge[RW_MGR_MEM_DQ_PER_WRITE_DQS];
	int32_t mid;
	int32_t mid_min, orig_mid_min;
	int32_t new_dqs, start_dqs, shift_dq;
	int32_t dq_margin, dqs_margin, dm_margin;
	uint32_t stop;
	uint32_t temp_dq_out1_delay;
	uint32_t addr;

	debug("%s:%d %u %u", __func__, __LINE__, write_group, test_bgn);

	dm_margin = 0;

	addr = SDR_PHYGRP_SCCGRP_ADDRESS | SCC_MGR_IO_OUT1_DELAY_OFFSET;
	start_dqs = readl(addr +
			  (RW_MGR_MEM_DQ_PER_WRITE_DQS << 2));

	/* per-bit deskew */

	/*
	 * set the left and right edge of each bit to an illegal value
	 * use (IO_IO_OUT1_DELAY_MAX + 1) as an illegal value.
	 */
	sticky_bit_chk = 0;
	for (i = 0; i < RW_MGR_MEM_DQ_PER_WRITE_DQS; i++) {
		left_edge[i]  = IO_IO_OUT1_DELAY_MAX + 1;
		right_edge[i] = IO_IO_OUT1_DELAY_MAX + 1;
	}

	/* Search for the left edge of the window for each bit */
	for (d = 0; d <= IO_IO_OUT1_DELAY_MAX; d++) {
		scc_mgr_apply_group_dq_out1_delay(write_group, test_bgn, d);

		writel(0, &sdr_scc_mgr->update);

		/*
		 * Stop searching when the read test doesn't pass AND when
		 * we've seen a passing read on every bit.
		 */
		stop = !rw_mgr_mem_calibrate_write_test(rank_bgn, write_group,
			0, PASS_ONE_BIT, &bit_chk, 0);
		sticky_bit_chk = sticky_bit_chk | bit_chk;
		stop = stop && (sticky_bit_chk == param->write_correct_mask);
		debug_cond(DLEVEL == 2, "write_center(left): dtap=%d => %u \
			   == %u && %u [bit_chk= %u ]\n",
			d, sticky_bit_chk, param->write_correct_mask,
			stop, bit_chk);

		if (stop == 1) {
			break;
		} else {
			for (i = 0; i < RW_MGR_MEM_DQ_PER_WRITE_DQS; i++) {
				if (bit_chk & 1) {
					/*
					 * Remember a passing test as the
					 * left_edge.
					 */
					left_edge[i] = d;
				} else {
					/*
					 * If a left edge has not been seen
					 * yet, then a future passing test will
					 * mark this edge as the right edge.
					 */
					if (left_edge[i] ==
						IO_IO_OUT1_DELAY_MAX + 1) {
						right_edge[i] = -(d + 1);
					}
				}
				debug_cond(DLEVEL == 2, "write_center[l,d=%d):", d);
				debug_cond(DLEVEL == 2, "bit_chk_test=%d left_edge[%u]: %d",
					   (int)(bit_chk & 1), i, left_edge[i]);
				debug_cond(DLEVEL == 2, "right_edge[%u]: %d\n", i,
				       right_edge[i]);
				bit_chk = bit_chk >> 1;
			}
		}
	}

	/* Reset DQ delay chains to 0 */
	scc_mgr_apply_group_dq_out1_delay(write_group, test_bgn, 0);
	sticky_bit_chk = 0;
	for (i = RW_MGR_MEM_DQ_PER_WRITE_DQS - 1;; i--) {
		debug_cond(DLEVEL == 2, "%s:%d write_center: left_edge[%u]: \
			   %d right_edge[%u]: %d\n", __func__, __LINE__,
			   i, left_edge[i], i, right_edge[i]);

		/*
		 * Check for cases where we haven't found the left edge,
		 * which makes our assignment of the the right edge invalid.
		 * Reset it to the illegal value.
		 */
		if ((left_edge[i] == IO_IO_OUT1_DELAY_MAX + 1) &&
		    (right_edge[i] != IO_IO_OUT1_DELAY_MAX + 1)) {
			right_edge[i] = IO_IO_OUT1_DELAY_MAX + 1;
			debug_cond(DLEVEL == 2, "%s:%d write_center: reset \
				   right_edge[%u]: %d\n", __func__, __LINE__,
				   i, right_edge[i]);
		}

		/*
		 * Reset sticky bit (except for bits where we have
		 * seen the left edge).
		 */
		sticky_bit_chk = sticky_bit_chk << 1;
		if ((left_edge[i] != IO_IO_OUT1_DELAY_MAX + 1))
			sticky_bit_chk = sticky_bit_chk | 1;

		if (i == 0)
			break;
	}

	/* Search for the right edge of the window for each bit */
	for (d = 0; d <= IO_IO_OUT1_DELAY_MAX - start_dqs; d++) {
		scc_mgr_apply_group_dqs_io_and_oct_out1(write_group,
							d + start_dqs);

		writel(0, &sdr_scc_mgr->update);

		/*
		 * Stop searching when the read test doesn't pass AND when
		 * we've seen a passing read on every bit.
		 */
		stop = !rw_mgr_mem_calibrate_write_test(rank_bgn, write_group,
			0, PASS_ONE_BIT, &bit_chk, 0);

		sticky_bit_chk = sticky_bit_chk | bit_chk;
		stop = stop && (sticky_bit_chk == param->write_correct_mask);

		debug_cond(DLEVEL == 2, "write_center (right): dtap=%u => %u == \
			   %u && %u\n", d, sticky_bit_chk,
			   param->write_correct_mask, stop);

		if (stop == 1) {
			if (d == 0) {
				for (i = 0; i < RW_MGR_MEM_DQ_PER_WRITE_DQS;
					i++) {
					/* d = 0 failed, but it passed when
					testing the left edge, so it must be
					marginal, set it to -1 */
					if (right_edge[i] ==
						IO_IO_OUT1_DELAY_MAX + 1 &&
						left_edge[i] !=
						IO_IO_OUT1_DELAY_MAX + 1) {
						right_edge[i] = -1;
					}
				}
			}
			break;
		} else {
			for (i = 0; i < RW_MGR_MEM_DQ_PER_WRITE_DQS; i++) {
				if (bit_chk & 1) {
					/*
					 * Remember a passing test as
					 * the right_edge.
					 */
					right_edge[i] = d;
				} else {
					if (d != 0) {
						/*
						 * If a right edge has not
						 * been seen yet, then a future
						 * passing test will mark this
						 * edge as the left edge.
						 */
						if (right_edge[i] ==
						    IO_IO_OUT1_DELAY_MAX + 1)
							left_edge[i] = -(d + 1);
					} else {
						/*
						 * d = 0 failed, but it passed
						 * when testing the left edge,
						 * so it must be marginal, set
						 * it to -1.
						 */
						if (right_edge[i] ==
						    IO_IO_OUT1_DELAY_MAX + 1 &&
						    left_edge[i] !=
						    IO_IO_OUT1_DELAY_MAX + 1)
							right_edge[i] = -1;
						/*
						 * If a right edge has not been
						 * seen yet, then a future
						 * passing test will mark this
						 * edge as the left edge.
						 */
						else if (right_edge[i] ==
							IO_IO_OUT1_DELAY_MAX +
							1)
							left_edge[i] = -(d + 1);
					}
				}
				debug_cond(DLEVEL == 2, "write_center[r,d=%d):", d);
				debug_cond(DLEVEL == 2, "bit_chk_test=%d left_edge[%u]: %d",
					   (int)(bit_chk & 1), i, left_edge[i]);
				debug_cond(DLEVEL == 2, "right_edge[%u]: %d\n", i,
					   right_edge[i]);
				bit_chk = bit_chk >> 1;
			}
		}
	}

	/* Check that all bits have a window */
	for (i = 0; i < RW_MGR_MEM_DQ_PER_WRITE_DQS; i++) {
		debug_cond(DLEVEL == 2, "%s:%d write_center: left_edge[%u]: \
			   %d right_edge[%u]: %d", __func__, __LINE__,
			   i, left_edge[i], i, right_edge[i]);
		if ((left_edge[i] == IO_IO_OUT1_DELAY_MAX + 1) ||
		    (right_edge[i] == IO_IO_OUT1_DELAY_MAX + 1)) {
			set_failing_group_stage(test_bgn + i,
						CAL_STAGE_WRITES,
						CAL_SUBSTAGE_WRITES_CENTER);
			return 0;
		}
	}

	/* Find middle of window for each DQ bit */
	mid_min = left_edge[0] - right_edge[0];
	min_index = 0;
	for (i = 1; i < RW_MGR_MEM_DQ_PER_WRITE_DQS; i++) {
		mid = left_edge[i] - right_edge[i];
		if (mid < mid_min) {
			mid_min = mid;
			min_index = i;
		}
	}

	/*
	 * -mid_min/2 represents the amount that we need to move DQS.
	 * If mid_min is odd and positive we'll need to add one to
	 * make sure the rounding in further calculations is correct
	 * (always bias to the right), so just add 1 for all positive values.
	 */
	if (mid_min > 0)
		mid_min++;
	mid_min = mid_min / 2;
	debug_cond(DLEVEL == 1, "%s:%d write_center: mid_min=%d\n", __func__,
		   __LINE__, mid_min);

	/* Determine the amount we can change DQS (which is -mid_min) */
	orig_mid_min = mid_min;
	new_dqs = start_dqs;
	mid_min = 0;
	debug_cond(DLEVEL == 1, "%s:%d write_center: start_dqs=%d new_dqs=%d \
		   mid_min=%d\n", __func__, __LINE__, start_dqs, new_dqs, mid_min);
	/* Initialize data for export structures */
	dqs_margin = IO_IO_OUT1_DELAY_MAX + 1;
	dq_margin  = IO_IO_OUT1_DELAY_MAX + 1;

	/* add delay to bring centre of all DQ windows to the same "level" */
	for (i = 0, p = test_bgn; i < RW_MGR_MEM_DQ_PER_WRITE_DQS; i++, p++) {
		/* Use values before divide by 2 to reduce round off error */
		shift_dq = (left_edge[i] - right_edge[i] -
			(left_edge[min_index] - right_edge[min_index]))/2  +
		(orig_mid_min - mid_min);

		debug_cond(DLEVEL == 2, "%s:%d write_center: before: shift_dq \
			   [%u]=%d\n", __func__, __LINE__, i, shift_dq);

		addr = SDR_PHYGRP_SCCGRP_ADDRESS | SCC_MGR_IO_OUT1_DELAY_OFFSET;
		temp_dq_out1_delay = readl(addr + (i << 2));
		if (shift_dq + (int32_t)temp_dq_out1_delay >
			(int32_t)IO_IO_OUT1_DELAY_MAX) {
			shift_dq = (int32_t)IO_IO_OUT1_DELAY_MAX - temp_dq_out1_delay;
		} else if (shift_dq + (int32_t)temp_dq_out1_delay < 0) {
			shift_dq = -(int32_t)temp_dq_out1_delay;
		}
		debug_cond(DLEVEL == 2, "write_center: after: shift_dq[%u]=%d\n",
			   i, shift_dq);
		scc_mgr_set_dq_out1_delay(write_group, i, temp_dq_out1_delay +
					  shift_dq);
		scc_mgr_load_dq(i);

		debug_cond(DLEVEL == 2, "write_center: margin[%u]=[%d,%d]\n", i,
			   left_edge[i] - shift_dq + (-mid_min),
			   right_edge[i] + shift_dq - (-mid_min));
		/* To determine values for export structures */
		if (left_edge[i] - shift_dq + (-mid_min) < dq_margin)
			dq_margin = left_edge[i] - shift_dq + (-mid_min);

		if (right_edge[i] + shift_dq - (-mid_min) < dqs_margin)
			dqs_margin = right_edge[i] + shift_dq - (-mid_min);
	}

	/* Move DQS */
	scc_mgr_apply_group_dqs_io_and_oct_out1(write_group, new_dqs);
	writel(0, &sdr_scc_mgr->update);

	/* Centre DM */
	debug_cond(DLEVEL == 2, "%s:%d write_center: DM\n", __func__, __LINE__);

	/*
	 * set the left and right edge of each bit to an illegal value,
	 * use (IO_IO_OUT1_DELAY_MAX + 1) as an illegal value,
	 */
	left_edge[0]  = IO_IO_OUT1_DELAY_MAX + 1;
	right_edge[0] = IO_IO_OUT1_DELAY_MAX + 1;
	int32_t bgn_curr = IO_IO_OUT1_DELAY_MAX + 1;
	int32_t end_curr = IO_IO_OUT1_DELAY_MAX + 1;
	int32_t bgn_best = IO_IO_OUT1_DELAY_MAX + 1;
	int32_t end_best = IO_IO_OUT1_DELAY_MAX + 1;
	int32_t win_best = 0;

	/* Search for the/part of the window with DM shift */
	for (d = IO_IO_OUT1_DELAY_MAX; d >= 0; d -= DELTA_D) {
		scc_mgr_apply_group_dm_out1_delay(write_group, d);
		writel(0, &sdr_scc_mgr->update);

		if (rw_mgr_mem_calibrate_write_test(rank_bgn, write_group, 1,
						    PASS_ALL_BITS, &bit_chk,
						    0)) {
			/* USE Set current end of the window */
			end_curr = -d;
			/*
			 * If a starting edge of our window has not been seen
			 * this is our current start of the DM window.
			 */
			if (bgn_curr == IO_IO_OUT1_DELAY_MAX + 1)
				bgn_curr = -d;

			/*
			 * If current window is bigger than best seen.
			 * Set best seen to be current window.
			 */
			if ((end_curr-bgn_curr+1) > win_best) {
				win_best = end_curr-bgn_curr+1;
				bgn_best = bgn_curr;
				end_best = end_curr;
			}
		} else {
			/* We just saw a failing test. Reset temp edge */
			bgn_curr = IO_IO_OUT1_DELAY_MAX + 1;
			end_curr = IO_IO_OUT1_DELAY_MAX + 1;
			}
		}


	/* Reset DM delay chains to 0 */
	scc_mgr_apply_group_dm_out1_delay(write_group, 0);

	/*
	 * Check to see if the current window nudges up aganist 0 delay.
	 * If so we need to continue the search by shifting DQS otherwise DQS
	 * search begins as a new search. */
	if (end_curr != 0) {
		bgn_curr = IO_IO_OUT1_DELAY_MAX + 1;
		end_curr = IO_IO_OUT1_DELAY_MAX + 1;
	}

	/* Search for the/part of the window with DQS shifts */
	for (d = 0; d <= IO_IO_OUT1_DELAY_MAX - new_dqs; d += DELTA_D) {
		/*
		 * Note: This only shifts DQS, so are we limiting ourselve to
		 * width of DQ unnecessarily.
		 */
		scc_mgr_apply_group_dqs_io_and_oct_out1(write_group,
							d + new_dqs);

		writel(0, &sdr_scc_mgr->update);
		if (rw_mgr_mem_calibrate_write_test(rank_bgn, write_group, 1,
						    PASS_ALL_BITS, &bit_chk,
						    0)) {
			/* USE Set current end of the window */
			end_curr = d;
			/*
			 * If a beginning edge of our window has not been seen
			 * this is our current begin of the DM window.
			 */
			if (bgn_curr == IO_IO_OUT1_DELAY_MAX + 1)
				bgn_curr = d;

			/*
			 * If current window is bigger than best seen. Set best
			 * seen to be current window.
			 */
			if ((end_curr-bgn_curr+1) > win_best) {
				win_best = end_curr-bgn_curr+1;
				bgn_best = bgn_curr;
				end_best = end_curr;
			}
		} else {
			/* We just saw a failing test. Reset temp edge */
			bgn_curr = IO_IO_OUT1_DELAY_MAX + 1;
			end_curr = IO_IO_OUT1_DELAY_MAX + 1;

			/* Early exit optimization: if ther remaining delay
			chain space is less than already seen largest window
			we can exit */
			if ((win_best-1) >
				(IO_IO_OUT1_DELAY_MAX - new_dqs - d)) {
					break;
				}
			}
		}

	/* assign left and right edge for cal and reporting; */
	left_edge[0] = -1*bgn_best;
	right_edge[0] = end_best;

	debug_cond(DLEVEL == 2, "%s:%d dm_calib: left=%d right=%d\n", __func__,
		   __LINE__, left_edge[0], right_edge[0]);

	/* Move DQS (back to orig) */
	scc_mgr_apply_group_dqs_io_and_oct_out1(write_group, new_dqs);

	/* Move DM */

	/* Find middle of window for the DM bit */
	mid = (left_edge[0] - right_edge[0]) / 2;

	/* only move right, since we are not moving DQS/DQ */
	if (mid < 0)
		mid = 0;

	/* dm_marign should fail if we never find a window */
	if (win_best == 0)
		dm_margin = -1;
	else
		dm_margin = left_edge[0] - mid;

	scc_mgr_apply_group_dm_out1_delay(write_group, mid);
	writel(0, &sdr_scc_mgr->update);

	debug_cond(DLEVEL == 2, "%s:%d dm_calib: left=%d right=%d mid=%d \
		   dm_margin=%d\n", __func__, __LINE__, left_edge[0],
		   right_edge[0], mid, dm_margin);
	/* Export values */
	gbl->fom_out += dq_margin + dqs_margin;

	debug_cond(DLEVEL == 2, "%s:%d write_center: dq_margin=%d \
		   dqs_margin=%d dm_margin=%d\n", __func__, __LINE__,
		   dq_margin, dqs_margin, dm_margin);

	/*
	 * Do not remove this line as it makes sure all of our
	 * decisions have been applied.
	 */
	writel(0, &sdr_scc_mgr->update);
	return (dq_margin >= 0) && (dqs_margin >= 0) && (dm_margin >= 0);
}

/* calibrate the write operations */
static uint32_t rw_mgr_mem_calibrate_writes(uint32_t rank_bgn, uint32_t g,
	uint32_t test_bgn)
{
	/* update info for sims */
	debug("%s:%d %u %u\n", __func__, __LINE__, g, test_bgn);

	reg_file_set_stage(CAL_STAGE_WRITES);
	reg_file_set_sub_stage(CAL_SUBSTAGE_WRITES_CENTER);

	reg_file_set_group(g);

	if (!rw_mgr_mem_calibrate_writes_center(rank_bgn, g, test_bgn)) {
		set_failing_group_stage(g, CAL_STAGE_WRITES,
					CAL_SUBSTAGE_WRITES_CENTER);
		return 0;
	}

	return 1;
}

/* precharge all banks and activate row 0 in bank "000..." and bank "111..." */
static void mem_precharge_and_activate(void)
{
	uint32_t r;

	for (r = 0; r < RW_MGR_MEM_NUMBER_OF_RANKS; r++) {
		if (param->skip_ranks[r]) {
			/* request to skip the rank */
			continue;
		}

		/* set rank */
		set_rank_and_odt_mask(r, RW_MGR_ODT_MODE_OFF);

		/* precharge all banks ... */
		writel(RW_MGR_PRECHARGE_ALL, SDR_PHYGRP_RWMGRGRP_ADDRESS |
					     RW_MGR_RUN_SINGLE_GROUP_OFFSET);

		writel(0x0F, &sdr_rw_load_mgr_regs->load_cntr0);
		writel(RW_MGR_ACTIVATE_0_AND_1_WAIT1,
			&sdr_rw_load_jump_mgr_regs->load_jump_add0);

		writel(0x0F, &sdr_rw_load_mgr_regs->load_cntr1);
		writel(RW_MGR_ACTIVATE_0_AND_1_WAIT2,
			&sdr_rw_load_jump_mgr_regs->load_jump_add1);

		/* activate rows */
		writel(RW_MGR_ACTIVATE_0_AND_1, SDR_PHYGRP_RWMGRGRP_ADDRESS |
						RW_MGR_RUN_SINGLE_GROUP_OFFSET);
	}
}

/* Configure various memory related parameters. */
static void mem_config(void)
{
	uint32_t rlat, wlat;
	uint32_t rw_wl_nop_cycles;
	uint32_t max_latency;

	debug("%s:%d\n", __func__, __LINE__);
	/* read in write and read latency */
	wlat = readl(&data_mgr->t_wl_add);
	wlat += readl(&data_mgr->mem_t_add);

	/* WL for hard phy does not include additive latency */

	/*
	 * add addtional write latency to offset the address/command extra
	 * clock cycle. We change the AC mux setting causing AC to be delayed
	 * by one mem clock cycle. Only do this for DDR3
	 */
	wlat = wlat + 1;

	rlat = readl(&data_mgr->t_rl_add);

	rw_wl_nop_cycles = wlat - 2;
	gbl->rw_wl_nop_cycles = rw_wl_nop_cycles;

	/*
	 * For AV/CV, lfifo is hardened and always runs at full rate so
	 * max latency in AFI clocks, used here, is correspondingly smaller.
	 */
	max_latency = (1<<MAX_LATENCY_COUNT_WIDTH)/1 - 1;
	/* configure for a burst length of 8 */

	/* write latency */
	/* Adjust Write Latency for Hard PHY */
	wlat = wlat + 1;

	/* set a pretty high read latency initially */
	gbl->curr_read_lat = rlat + 16;

	if (gbl->curr_read_lat > max_latency)
		gbl->curr_read_lat = max_latency;

	writel(gbl->curr_read_lat, &phy_mgr_cfg->phy_rlat);

	/* advertise write latency */
	gbl->curr_write_lat = wlat;
	writel(wlat - 2, &phy_mgr_cfg->afi_wlat);

	/* initialize bit slips */
	mem_precharge_and_activate();
}

/* Set VFIFO and LFIFO to instant-on settings in skip calibration mode */
static void mem_skip_calibrate(void)
{
	uint32_t vfifo_offset;
	uint32_t i, j, r;

	debug("%s:%d\n", __func__, __LINE__);
	/* Need to update every shadow register set used by the interface */
	for (r = 0; r < RW_MGR_MEM_NUMBER_OF_RANKS;
		r += NUM_RANKS_PER_SHADOW_REG) {
		/*
		 * Set output phase alignment settings appropriate for
		 * skip calibration.
		 */
		for (i = 0; i < RW_MGR_MEM_IF_READ_DQS_WIDTH; i++) {
			scc_mgr_set_dqs_en_phase(i, 0);
#if IO_DLL_CHAIN_LENGTH == 6
			scc_mgr_set_dqdqs_output_phase(i, 6);
#else
			scc_mgr_set_dqdqs_output_phase(i, 7);
#endif
			/*
			 * Case:33398
			 *
			 * Write data arrives to the I/O two cycles before write
			 * latency is reached (720 deg).
			 *   -> due to bit-slip in a/c bus
			 *   -> to allow board skew where dqs is longer than ck
			 *      -> how often can this happen!?
			 *      -> can claim back some ptaps for high freq
			 *       support if we can relax this, but i digress...
			 *
			 * The write_clk leads mem_ck by 90 deg
			 * The minimum ptap of the OPA is 180 deg
			 * Each ptap has (360 / IO_DLL_CHAIN_LENGH) deg of delay
			 * The write_clk is always delayed by 2 ptaps
			 *
			 * Hence, to make DQS aligned to CK, we need to delay
			 * DQS by:
			 *    (720 - 90 - 180 - 2 * (360 / IO_DLL_CHAIN_LENGTH))
			 *
			 * Dividing the above by (360 / IO_DLL_CHAIN_LENGTH)
			 * gives us the number of ptaps, which simplies to:
			 *
			 *    (1.25 * IO_DLL_CHAIN_LENGTH - 2)
			 */
			scc_mgr_set_dqdqs_output_phase(i, (1.25 *
				IO_DLL_CHAIN_LENGTH - 2));
		}
		writel(0xff, &sdr_scc_mgr->dqs_ena);
		writel(0xff, &sdr_scc_mgr->dqs_io_ena);

		for (i = 0; i < RW_MGR_MEM_IF_WRITE_DQS_WIDTH; i++) {
			writel(i, SDR_PHYGRP_SCCGRP_ADDRESS |
				  SCC_MGR_GROUP_COUNTER_OFFSET);
		}
		writel(0xff, &sdr_scc_mgr->dq_ena);
		writel(0xff, &sdr_scc_mgr->dm_ena);
		writel(0, &sdr_scc_mgr->update);
	}

	/* Compensate for simulation model behaviour */
	for (i = 0; i < RW_MGR_MEM_IF_READ_DQS_WIDTH; i++) {
		scc_mgr_set_dqs_bus_in_delay(i, 10);
		scc_mgr_load_dqs(i);
	}
	writel(0, &sdr_scc_mgr->update);

	/*
	 * ArriaV has hard FIFOs that can only be initialized by incrementing
	 * in sequencer.
	 */
	vfifo_offset = CALIB_VFIFO_OFFSET;
	for (j = 0; j < vfifo_offset; j++) {
		writel(0xff, &phy_mgr_cmd->inc_vfifo_hard_phy);
	}
	writel(0, &phy_mgr_cmd->fifo_reset);

	/*
	 * For ACV with hard lfifo, we get the skip-cal setting from
	 * generation-time constant.
	 */
	gbl->curr_read_lat = CALIB_LFIFO_OFFSET;
	writel(gbl->curr_read_lat, &phy_mgr_cfg->phy_rlat);
}

/* Memory calibration entry point */
static uint32_t mem_calibrate(void)
{
	uint32_t i;
	uint32_t rank_bgn, sr;
	uint32_t write_group, write_test_bgn;
	uint32_t read_group, read_test_bgn;
	uint32_t run_groups, current_run;
	uint32_t failing_groups = 0;
	uint32_t group_failed = 0;
	uint32_t sr_failed = 0;

	debug("%s:%d\n", __func__, __LINE__);
	/* Initialize the data settings */

	gbl->error_substage = CAL_SUBSTAGE_NIL;
	gbl->error_stage = CAL_STAGE_NIL;
	gbl->error_group = 0xff;
	gbl->fom_in = 0;
	gbl->fom_out = 0;

	mem_config();

	uint32_t bypass_mode = 0x1;
	for (i = 0; i < RW_MGR_MEM_IF_READ_DQS_WIDTH; i++) {
		writel(i, SDR_PHYGRP_SCCGRP_ADDRESS |
			  SCC_MGR_GROUP_COUNTER_OFFSET);
		scc_set_bypass_mode(i, bypass_mode);
	}

	if ((dyn_calib_steps & CALIB_SKIP_ALL) == CALIB_SKIP_ALL) {
		/*
		 * Set VFIFO and LFIFO to instant-on settings in skip
		 * calibration mode.
		 */
		mem_skip_calibrate();
	} else {
		for (i = 0; i < NUM_CALIB_REPEAT; i++) {
			/*
			 * Zero all delay chain/phase settings for all
			 * groups and all shadow register sets.
			 */
			scc_mgr_zero_all();

			run_groups = ~param->skip_groups;

			for (write_group = 0, write_test_bgn = 0; write_group
				< RW_MGR_MEM_IF_WRITE_DQS_WIDTH; write_group++,
				write_test_bgn += RW_MGR_MEM_DQ_PER_WRITE_DQS) {
				/* Initialized the group failure */
				group_failed = 0;

				current_run = run_groups & ((1 <<
					RW_MGR_NUM_DQS_PER_WRITE_GROUP) - 1);
				run_groups = run_groups >>
					RW_MGR_NUM_DQS_PER_WRITE_GROUP;

				if (current_run == 0)
					continue;

				writel(write_group, SDR_PHYGRP_SCCGRP_ADDRESS |
						    SCC_MGR_GROUP_COUNTER_OFFSET);
				scc_mgr_zero_group(write_group, write_test_bgn,
						   0);

				for (read_group = write_group *
					RW_MGR_MEM_IF_READ_DQS_WIDTH /
					RW_MGR_MEM_IF_WRITE_DQS_WIDTH,
					read_test_bgn = 0;
					read_group < (write_group + 1) *
					RW_MGR_MEM_IF_READ_DQS_WIDTH /
					RW_MGR_MEM_IF_WRITE_DQS_WIDTH &&
					group_failed == 0;
					read_group++, read_test_bgn +=
					RW_MGR_MEM_DQ_PER_READ_DQS) {
					/* Calibrate the VFIFO */
					if (!((STATIC_CALIB_STEPS) &
						CALIB_SKIP_VFIFO)) {
						if (!rw_mgr_mem_calibrate_vfifo
							(read_group,
							read_test_bgn)) {
							group_failed = 1;

							if (!(gbl->
							phy_debug_mode_flags &
						PHY_DEBUG_SWEEP_ALL_GROUPS)) {
								return 0;
							}
						}
					}
				}

				/* Calibrate the output side */
				if (group_failed == 0)	{
					for (rank_bgn = 0, sr = 0; rank_bgn
						< RW_MGR_MEM_NUMBER_OF_RANKS;
						rank_bgn +=
						NUM_RANKS_PER_SHADOW_REG,
						++sr) {
						sr_failed = 0;
						if (!((STATIC_CALIB_STEPS) &
						CALIB_SKIP_WRITES)) {
							if ((STATIC_CALIB_STEPS)
						& CALIB_SKIP_DELAY_SWEEPS) {
						/* not needed in quick mode! */
							} else {
						/*
						 * Determine if this set of
						 * ranks should be skipped
						 * entirely.
						 */
					if (!param->skip_shadow_regs[sr]) {
						if (!rw_mgr_mem_calibrate_writes
						(rank_bgn, write_group,
						write_test_bgn)) {
							sr_failed = 1;
							if (!(gbl->
							phy_debug_mode_flags &
						PHY_DEBUG_SWEEP_ALL_GROUPS)) {
								return 0;
									}
									}
								}
							}
						}
						if (sr_failed != 0)
							group_failed = 1;
					}
				}

				if (group_failed == 0) {
					for (read_group = write_group *
					RW_MGR_MEM_IF_READ_DQS_WIDTH /
					RW_MGR_MEM_IF_WRITE_DQS_WIDTH,
					read_test_bgn = 0;
						read_group < (write_group + 1)
						* RW_MGR_MEM_IF_READ_DQS_WIDTH
						/ RW_MGR_MEM_IF_WRITE_DQS_WIDTH &&
						group_failed == 0;
						read_group++, read_test_bgn +=
						RW_MGR_MEM_DQ_PER_READ_DQS) {
						if (!((STATIC_CALIB_STEPS) &
							CALIB_SKIP_WRITES)) {
					if (!rw_mgr_mem_calibrate_vfifo_end
						(read_group, read_test_bgn)) {
							group_failed = 1;

						if (!(gbl->phy_debug_mode_flags
						& PHY_DEBUG_SWEEP_ALL_GROUPS)) {
								return 0;
								}
							}
						}
					}
				}

				if (group_failed != 0)
					failing_groups++;
			}

			/*
			 * USER If there are any failing groups then report
			 * the failure.
			 */
			if (failing_groups != 0)
				return 0;

			/* Calibrate the LFIFO */
			if (!((STATIC_CALIB_STEPS) & CALIB_SKIP_LFIFO)) {
				/*
				 * If we're skipping groups as part of debug,
				 * don't calibrate LFIFO.
				 */
				if (param->skip_groups == 0) {
					if (!rw_mgr_mem_calibrate_lfifo())
						return 0;
				}
			}
		}
	}

	/*
	 * Do not remove this line as it makes sure all of our decisions
	 * have been applied.
	 */
	writel(0, &sdr_scc_mgr->update);
	return 1;
}

static uint32_t run_mem_calibrate(void)
{
	uint32_t pass;
	uint32_t debug_info;

	debug("%s:%d\n", __func__, __LINE__);

	/* Reset pass/fail status shown on afi_cal_success/fail */
	writel(PHY_MGR_CAL_RESET, &phy_mgr_cfg->cal_status);

	/* stop tracking manger */
	uint32_t ctrlcfg = readl(&sdr_ctrl->ctrl_cfg);

	writel(ctrlcfg & 0xFFBFFFFF, &sdr_ctrl->ctrl_cfg);

	initialize();
	rw_mgr_mem_initialize();

	pass = mem_calibrate();

	mem_precharge_and_activate();
	writel(0, &phy_mgr_cmd->fifo_reset);

	/*
	 * Handoff:
	 * Don't return control of the PHY back to AFI when in debug mode.
	 */
	if ((gbl->phy_debug_mode_flags & PHY_DEBUG_IN_DEBUG_MODE) == 0) {
		rw_mgr_mem_handoff();
		/*
		 * In Hard PHY this is a 2-bit control:
		 * 0: AFI Mux Select
		 * 1: DDIO Mux Select
		 */
		writel(0x2, &phy_mgr_cfg->mux_sel);
	}

	writel(ctrlcfg, &sdr_ctrl->ctrl_cfg);

	if (pass) {
		printf("%s: CALIBRATION PASSED\n", __FILE__);

		gbl->fom_in /= 2;
		gbl->fom_out /= 2;

		if (gbl->fom_in > 0xff)
			gbl->fom_in = 0xff;

		if (gbl->fom_out > 0xff)
			gbl->fom_out = 0xff;

		/* Update the FOM in the register file */
		debug_info = gbl->fom_in;
		debug_info |= gbl->fom_out << 8;
		writel(debug_info, &sdr_reg_file->fom);

		writel(debug_info, &phy_mgr_cfg->cal_debug_info);
		writel(PHY_MGR_CAL_SUCCESS, &phy_mgr_cfg->cal_status);
	} else {
		printf("%s: CALIBRATION FAILED\n", __FILE__);

		debug_info = gbl->error_stage;
		debug_info |= gbl->error_substage << 8;
		debug_info |= gbl->error_group << 16;

		writel(debug_info, &sdr_reg_file->failing_stage);
		writel(debug_info, &phy_mgr_cfg->cal_debug_info);
		writel(PHY_MGR_CAL_FAIL, &phy_mgr_cfg->cal_status);

		/* Update the failing group/stage in the register file */
		debug_info = gbl->error_stage;
		debug_info |= gbl->error_substage << 8;
		debug_info |= gbl->error_group << 16;
		writel(debug_info, &sdr_reg_file->failing_stage);
	}

	return pass;
}

/**
 * hc_initialize_rom_data() - Initialize ROM data
 *
 * Initialize ROM data.
 */
static void hc_initialize_rom_data(void)
{
	u32 i, addr;

	addr = SDR_PHYGRP_RWMGRGRP_ADDRESS | RW_MGR_INST_ROM_WRITE_OFFSET;
	for (i = 0; i < ARRAY_SIZE(inst_rom_init); i++)
		writel(inst_rom_init[i], addr + (i << 2));

	addr = SDR_PHYGRP_RWMGRGRP_ADDRESS | RW_MGR_AC_ROM_WRITE_OFFSET;
	for (i = 0; i < ARRAY_SIZE(ac_rom_init); i++)
		writel(ac_rom_init[i], addr + (i << 2));
}

/**
 * initialize_reg_file() - Initialize SDR register file
 *
 * Initialize SDR register file.
 */
static void initialize_reg_file(void)
{
	/* Initialize the register file with the correct data */
	writel(REG_FILE_INIT_SEQ_SIGNATURE, &sdr_reg_file->signature);
	writel(0, &sdr_reg_file->debug_data_addr);
	writel(0, &sdr_reg_file->cur_stage);
	writel(0, &sdr_reg_file->fom);
	writel(0, &sdr_reg_file->failing_stage);
	writel(0, &sdr_reg_file->debug1);
	writel(0, &sdr_reg_file->debug2);
}

/**
 * initialize_hps_phy() - Initialize HPS PHY
 *
 * Initialize HPS PHY.
 */
static void initialize_hps_phy(void)
{
	uint32_t reg;
	/*
	 * Tracking also gets configured here because it's in the
	 * same register.
	 */
	uint32_t trk_sample_count = 7500;
	uint32_t trk_long_idle_sample_count = (10 << 16) | 100;
	/*
	 * Format is number of outer loops in the 16 MSB, sample
	 * count in 16 LSB.
	 */

	reg = 0;
	reg |= SDR_CTRLGRP_PHYCTRL_PHYCTRL_0_ACDELAYEN_SET(2);
	reg |= SDR_CTRLGRP_PHYCTRL_PHYCTRL_0_DQDELAYEN_SET(1);
	reg |= SDR_CTRLGRP_PHYCTRL_PHYCTRL_0_DQSDELAYEN_SET(1);
	reg |= SDR_CTRLGRP_PHYCTRL_PHYCTRL_0_DQSLOGICDELAYEN_SET(1);
	reg |= SDR_CTRLGRP_PHYCTRL_PHYCTRL_0_RESETDELAYEN_SET(0);
	reg |= SDR_CTRLGRP_PHYCTRL_PHYCTRL_0_LPDDRDIS_SET(1);
	/*
	 * This field selects the intrinsic latency to RDATA_EN/FULL path.
	 * 00-bypass, 01- add 5 cycles, 10- add 10 cycles, 11- add 15 cycles.
	 */
	reg |= SDR_CTRLGRP_PHYCTRL_PHYCTRL_0_ADDLATSEL_SET(0);
	reg |= SDR_CTRLGRP_PHYCTRL_PHYCTRL_0_SAMPLECOUNT_19_0_SET(
		trk_sample_count);
	writel(reg, &sdr_ctrl->phy_ctrl0);

	reg = 0;
	reg |= SDR_CTRLGRP_PHYCTRL_PHYCTRL_1_SAMPLECOUNT_31_20_SET(
		trk_sample_count >>
		SDR_CTRLGRP_PHYCTRL_PHYCTRL_0_SAMPLECOUNT_19_0_WIDTH);
	reg |= SDR_CTRLGRP_PHYCTRL_PHYCTRL_1_LONGIDLESAMPLECOUNT_19_0_SET(
		trk_long_idle_sample_count);
	writel(reg, &sdr_ctrl->phy_ctrl1);

	reg = 0;
	reg |= SDR_CTRLGRP_PHYCTRL_PHYCTRL_2_LONGIDLESAMPLECOUNT_31_20_SET(
		trk_long_idle_sample_count >>
		SDR_CTRLGRP_PHYCTRL_PHYCTRL_1_LONGIDLESAMPLECOUNT_19_0_WIDTH);
	writel(reg, &sdr_ctrl->phy_ctrl2);
}

static void initialize_tracking(void)
{
	uint32_t concatenated_longidle = 0x0;
	uint32_t concatenated_delays = 0x0;
	uint32_t concatenated_rw_addr = 0x0;
	uint32_t concatenated_refresh = 0x0;
	uint32_t trk_sample_count = 7500;
	uint32_t dtaps_per_ptap;
	uint32_t tmp_delay;

	/*
	 * compute usable version of value in case we skip full
	 * computation later
	 */
	dtaps_per_ptap = 0;
	tmp_delay = 0;
	while (tmp_delay < IO_DELAY_PER_OPA_TAP) {
		dtaps_per_ptap++;
		tmp_delay += IO_DELAY_PER_DCHAIN_TAP;
	}
	dtaps_per_ptap--;

	concatenated_longidle = concatenated_longidle ^ 10;
		/*longidle outer loop */
	concatenated_longidle = concatenated_longidle << 16;
	concatenated_longidle = concatenated_longidle ^ 100;
		/*longidle sample count */
	concatenated_delays = concatenated_delays ^ 243;
		/* trfc, worst case of 933Mhz 4Gb */
	concatenated_delays = concatenated_delays << 8;
	concatenated_delays = concatenated_delays ^ 14;
		/* trcd, worst case */
	concatenated_delays = concatenated_delays << 8;
	concatenated_delays = concatenated_delays ^ 10;
		/* vfifo wait */
	concatenated_delays = concatenated_delays << 8;
	concatenated_delays = concatenated_delays ^ 4;
		/* mux delay */

	concatenated_rw_addr = concatenated_rw_addr ^ RW_MGR_IDLE;
	concatenated_rw_addr = concatenated_rw_addr << 8;
	concatenated_rw_addr = concatenated_rw_addr ^ RW_MGR_ACTIVATE_1;
	concatenated_rw_addr = concatenated_rw_addr << 8;
	concatenated_rw_addr = concatenated_rw_addr ^ RW_MGR_SGLE_READ;
	concatenated_rw_addr = concatenated_rw_addr << 8;
	concatenated_rw_addr = concatenated_rw_addr ^ RW_MGR_PRECHARGE_ALL;

	concatenated_refresh = concatenated_refresh ^ RW_MGR_REFRESH_ALL;
	concatenated_refresh = concatenated_refresh << 24;
	concatenated_refresh = concatenated_refresh ^ 1000; /* trefi */

	/* Initialize the register file with the correct data */
	writel(dtaps_per_ptap, &sdr_reg_file->dtaps_per_ptap);
	writel(trk_sample_count, &sdr_reg_file->trk_sample_count);
	writel(concatenated_longidle, &sdr_reg_file->trk_longidle);
	writel(concatenated_delays, &sdr_reg_file->delays);
	writel(concatenated_rw_addr, &sdr_reg_file->trk_rw_mgr_addr);
	writel(RW_MGR_MEM_IF_READ_DQS_WIDTH, &sdr_reg_file->trk_read_dqs_width);
	writel(concatenated_refresh, &sdr_reg_file->trk_rfsh);
}

int sdram_calibration_full(void)
{
	struct param_type my_param;
	struct gbl_type my_gbl;
	uint32_t pass;
	uint32_t i;

	param = &my_param;
	gbl = &my_gbl;

	/* Initialize the debug mode flags */
	gbl->phy_debug_mode_flags = 0;
	/* Set the calibration enabled by default */
	gbl->phy_debug_mode_flags |= PHY_DEBUG_ENABLE_CAL_RPT;
	/*
	 * Only sweep all groups (regardless of fail state) by default
	 * Set enabled read test by default.
	 */
#if DISABLE_GUARANTEED_READ
	gbl->phy_debug_mode_flags |= PHY_DEBUG_DISABLE_GUARANTEED_READ;
#endif
	/* Initialize the register file */
	initialize_reg_file();

	/* Initialize any PHY CSR */
	initialize_hps_phy();

	scc_mgr_initialize();

	initialize_tracking();

	/* USER Enable all ranks, groups */
	for (i = 0; i < RW_MGR_MEM_NUMBER_OF_RANKS; i++)
		param->skip_ranks[i] = 0;
	for (i = 0; i < NUM_SHADOW_REGS; ++i)
		param->skip_shadow_regs[i] = 0;
	param->skip_groups = 0;

	printf("%s: Preparing to start memory calibration\n", __FILE__);

	debug("%s:%d\n", __func__, __LINE__);
	debug_cond(DLEVEL == 1,
		   "DDR3 FULL_RATE ranks=%u cs/dimm=%u dq/dqs=%u,%u vg/dqs=%u,%u ",
		   RW_MGR_MEM_NUMBER_OF_RANKS, RW_MGR_MEM_NUMBER_OF_CS_PER_DIMM,
		   RW_MGR_MEM_DQ_PER_READ_DQS, RW_MGR_MEM_DQ_PER_WRITE_DQS,
		   RW_MGR_MEM_VIRTUAL_GROUPS_PER_READ_DQS,
		   RW_MGR_MEM_VIRTUAL_GROUPS_PER_WRITE_DQS);
	debug_cond(DLEVEL == 1,
		   "dqs=%u,%u dq=%u dm=%u ptap_delay=%u dtap_delay=%u ",
		   RW_MGR_MEM_IF_READ_DQS_WIDTH, RW_MGR_MEM_IF_WRITE_DQS_WIDTH,
		   RW_MGR_MEM_DATA_WIDTH, RW_MGR_MEM_DATA_MASK_WIDTH,
		   IO_DELAY_PER_OPA_TAP, IO_DELAY_PER_DCHAIN_TAP);
	debug_cond(DLEVEL == 1, "dtap_dqsen_delay=%u, dll=%u",
		   IO_DELAY_PER_DQS_EN_DCHAIN_TAP, IO_DLL_CHAIN_LENGTH);
	debug_cond(DLEVEL == 1, "max values: en_p=%u dqdqs_p=%u en_d=%u dqs_in_d=%u ",
		   IO_DQS_EN_PHASE_MAX, IO_DQDQS_OUT_PHASE_MAX,
		   IO_DQS_EN_DELAY_MAX, IO_DQS_IN_DELAY_MAX);
	debug_cond(DLEVEL == 1, "io_in_d=%u io_out1_d=%u io_out2_d=%u ",
		   IO_IO_IN_DELAY_MAX, IO_IO_OUT1_DELAY_MAX,
		   IO_IO_OUT2_DELAY_MAX);
	debug_cond(DLEVEL == 1, "dqs_in_reserve=%u dqs_out_reserve=%u\n",
		   IO_DQS_IN_RESERVE, IO_DQS_OUT_RESERVE);

	hc_initialize_rom_data();

	/* update info for sims */
	reg_file_set_stage(CAL_STAGE_NIL);
	reg_file_set_group(0);

	/*
	 * Load global needed for those actions that require
	 * some dynamic calibration support.
	 */
	dyn_calib_steps = STATIC_CALIB_STEPS;
	/*
	 * Load global to allow dynamic selection of delay loop settings
	 * based on calibration mode.
	 */
	if (!(dyn_calib_steps & CALIB_SKIP_DELAY_LOOPS))
		skip_delay_mask = 0xff;
	else
		skip_delay_mask = 0x0;

	pass = run_mem_calibrate();

	printf("%s: Calibration complete\n", __FILE__);
	return pass;
}
