/*
 * Copyright Altera Corporation (C) 2012-2015
 *
 * SPDX-License-Identifier:    BSD-3-Clause
 */

#ifndef _SEQUENCER_H_
#define _SEQUENCER_H_

#define RW_MGR_NUM_DM_PER_WRITE_GROUP (RW_MGR_MEM_DATA_MASK_WIDTH \
	/ RW_MGR_MEM_IF_WRITE_DQS_WIDTH)
#define RW_MGR_NUM_TRUE_DM_PER_WRITE_GROUP (RW_MGR_TRUE_MEM_DATA_MASK_WIDTH \
	/ RW_MGR_MEM_IF_WRITE_DQS_WIDTH)

#define RW_MGR_NUM_DQS_PER_WRITE_GROUP (RW_MGR_MEM_IF_READ_DQS_WIDTH \
	/ RW_MGR_MEM_IF_WRITE_DQS_WIDTH)
#define NUM_RANKS_PER_SHADOW_REG (RW_MGR_MEM_NUMBER_OF_RANKS / NUM_SHADOW_REGS)

#define RW_MGR_RUN_SINGLE_GROUP_OFFSET		0x0
#define RW_MGR_RUN_ALL_GROUPS_OFFSET		0x0400
#define RW_MGR_RESET_READ_DATAPATH_OFFSET	0x1000
#define RW_MGR_SET_CS_AND_ODT_MASK_OFFSET	0x1400
#define RW_MGR_INST_ROM_WRITE_OFFSET		0x1800
#define RW_MGR_AC_ROM_WRITE_OFFSET		0x1C00

#define RW_MGR_MEM_NUMBER_OF_RANKS	1
#define NUM_SHADOW_REGS			1

#define RW_MGR_RANK_NONE		0xFF
#define RW_MGR_RANK_ALL			0x00

#define RW_MGR_ODT_MODE_OFF		0
#define RW_MGR_ODT_MODE_READ_WRITE	1

#define NUM_CALIB_REPEAT		1

#define NUM_READ_TESTS			7
#define NUM_READ_PB_TESTS		7
#define NUM_WRITE_TESTS			15
#define NUM_WRITE_PB_TESTS		31

#define PASS_ALL_BITS			1
#define PASS_ONE_BIT			0

/* calibration stages */
#define CAL_STAGE_NIL			0
#define CAL_STAGE_VFIFO			1
#define CAL_STAGE_WLEVEL		2
#define CAL_STAGE_LFIFO			3
#define CAL_STAGE_WRITES		4
#define CAL_STAGE_FULLTEST		5
#define CAL_STAGE_REFRESH		6
#define CAL_STAGE_CAL_SKIPPED		7
#define CAL_STAGE_CAL_ABORTED		8
#define CAL_STAGE_VFIFO_AFTER_WRITES	9

/* calibration substages */
#define CAL_SUBSTAGE_NIL		0
#define CAL_SUBSTAGE_GUARANTEED_READ	1
#define CAL_SUBSTAGE_DQS_EN_PHASE	2
#define CAL_SUBSTAGE_VFIFO_CENTER	3
#define CAL_SUBSTAGE_WORKING_DELAY	1
#define CAL_SUBSTAGE_LAST_WORKING_DELAY	2
#define CAL_SUBSTAGE_WLEVEL_COPY	3
#define CAL_SUBSTAGE_WRITES_CENTER	1
#define CAL_SUBSTAGE_READ_LATENCY	1
#define CAL_SUBSTAGE_REFRESH		1

#define MAX_RANKS			(RW_MGR_MEM_NUMBER_OF_RANKS)
#define MAX_DQS				(RW_MGR_MEM_IF_WRITE_DQS_WIDTH > \
					RW_MGR_MEM_IF_READ_DQS_WIDTH ? \
					RW_MGR_MEM_IF_WRITE_DQS_WIDTH : \
					RW_MGR_MEM_IF_READ_DQS_WIDTH)
#define MAX_DQ				(RW_MGR_MEM_DATA_WIDTH)
#define MAX_DM				(RW_MGR_MEM_DATA_MASK_WIDTH)

/* length of VFIFO, from SW_MACROS */
#define VFIFO_SIZE			(READ_VALID_FIFO_SIZE)

#define SCC_MGR_GROUP_COUNTER_OFFSET		0x0000
#define SCC_MGR_DQS_IN_DELAY_OFFSET		0x0100
#define SCC_MGR_DQS_EN_PHASE_OFFSET		0x0200
#define SCC_MGR_DQS_EN_DELAY_OFFSET		0x0300
#define SCC_MGR_DQDQS_OUT_PHASE_OFFSET		0x0400
#define SCC_MGR_OCT_OUT1_DELAY_OFFSET		0x0500
#define SCC_MGR_IO_OUT1_DELAY_OFFSET		0x0700
#define SCC_MGR_IO_IN_DELAY_OFFSET		0x0900

/* HHP-HPS-specific versions of some commands */
#define SCC_MGR_DQS_EN_DELAY_GATE_OFFSET	0x0600
#define SCC_MGR_IO_OE_DELAY_OFFSET		0x0800
#define SCC_MGR_HHP_GLOBALS_OFFSET		0x0A00
#define SCC_MGR_HHP_RFILE_OFFSET		0x0B00
#define SCC_MGR_AFI_CAL_INIT_OFFSET		0x0D00

#define SDR_PHYGRP_SCCGRP_ADDRESS		(SOCFPGA_SDR_ADDRESS | 0x0)
#define SDR_PHYGRP_PHYMGRGRP_ADDRESS		(SOCFPGA_SDR_ADDRESS | 0x1000)
#define SDR_PHYGRP_RWMGRGRP_ADDRESS		(SOCFPGA_SDR_ADDRESS | 0x2000)
#define SDR_PHYGRP_DATAMGRGRP_ADDRESS		(SOCFPGA_SDR_ADDRESS | 0x4000)
#define SDR_PHYGRP_REGFILEGRP_ADDRESS		(SOCFPGA_SDR_ADDRESS | 0x4800)

#define SDR_CTRLGRP_PHYCTRL_PHYCTRL_0_OFFSET 0x150
#define SDR_CTRLGRP_PHYCTRL_PHYCTRL_1_OFFSET 0x154
#define SDR_CTRLGRP_PHYCTRL_PHYCTRL_2_OFFSET 0x158

#define PHY_MGR_CAL_RESET		(0)
#define PHY_MGR_CAL_SUCCESS		(1)
#define PHY_MGR_CAL_FAIL		(2)

#define CALIB_SKIP_DELAY_LOOPS		(1 << 0)
#define CALIB_SKIP_ALL_BITS_CHK		(1 << 1)
#define CALIB_SKIP_DELAY_SWEEPS		(1 << 2)
#define CALIB_SKIP_VFIFO		(1 << 3)
#define CALIB_SKIP_LFIFO		(1 << 4)
#define CALIB_SKIP_WLEVEL		(1 << 5)
#define CALIB_SKIP_WRITES		(1 << 6)
#define CALIB_SKIP_FULL_TEST		(1 << 7)
#define CALIB_SKIP_ALL			(CALIB_SKIP_VFIFO | \
				CALIB_SKIP_LFIFO | CALIB_SKIP_WLEVEL | \
				CALIB_SKIP_WRITES | CALIB_SKIP_FULL_TEST)
#define CALIB_IN_RTL_SIM			(1 << 8)

/* Scan chain manager command addresses */
#define READ_SCC_OCT_OUT2_DELAY			0
#define READ_SCC_DQ_OUT2_DELAY			0
#define READ_SCC_DQS_IO_OUT2_DELAY		0
#define READ_SCC_DM_IO_OUT2_DELAY		0

/* HHP-HPS-specific values */
#define SCC_MGR_HHP_EXTRAS_OFFSET			0
#define SCC_MGR_HHP_DQSE_MAP_OFFSET			1

/* PHY Debug mode flag constants */
#define PHY_DEBUG_IN_DEBUG_MODE 0x00000001
#define PHY_DEBUG_ENABLE_CAL_RPT 0x00000002
#define PHY_DEBUG_ENABLE_MARGIN_RPT 0x00000004
#define PHY_DEBUG_SWEEP_ALL_GROUPS 0x00000008
#define PHY_DEBUG_DISABLE_GUARANTEED_READ 0x00000010
#define PHY_DEBUG_ENABLE_NON_DESTRUCTIVE_CALIBRATION 0x00000020

/* Init and Reset delay constants - Only use if defined by sequencer_defines.h,
 * otherwise, revert to defaults
 * Default for Tinit = (0+1) * ((202+1) * (2 * 131 + 1) + 1) = 53532 =
 * 200.75us @ 266MHz
 */
#ifdef TINIT_CNTR0_VAL
#define SEQ_TINIT_CNTR0_VAL TINIT_CNTR0_VAL
#else
#define SEQ_TINIT_CNTR0_VAL 0
#endif

#ifdef TINIT_CNTR1_VAL
#define SEQ_TINIT_CNTR1_VAL TINIT_CNTR1_VAL
#else
#define SEQ_TINIT_CNTR1_VAL 202
#endif

#ifdef TINIT_CNTR2_VAL
#define SEQ_TINIT_CNTR2_VAL TINIT_CNTR2_VAL
#else
#define SEQ_TINIT_CNTR2_VAL 131
#endif


/* Default for Treset = (2+1) * ((252+1) * (2 * 131 + 1) + 1) = 133563 =
 * 500.86us @ 266MHz
 */
#ifdef TRESET_CNTR0_VAL
#define SEQ_TRESET_CNTR0_VAL TRESET_CNTR0_VAL
#else
#define SEQ_TRESET_CNTR0_VAL 2
#endif

#ifdef TRESET_CNTR1_VAL
#define SEQ_TRESET_CNTR1_VAL TRESET_CNTR1_VAL
#else
#define SEQ_TRESET_CNTR1_VAL 252
#endif

#ifdef TRESET_CNTR2_VAL
#define SEQ_TRESET_CNTR2_VAL TRESET_CNTR2_VAL
#else
#define SEQ_TRESET_CNTR2_VAL 131
#endif

struct socfpga_sdr_rw_load_manager {
	u32	load_cntr0;
	u32	load_cntr1;
	u32	load_cntr2;
	u32	load_cntr3;
};

struct socfpga_sdr_rw_load_jump_manager {
	u32	load_jump_add0;
	u32	load_jump_add1;
	u32	load_jump_add2;
	u32	load_jump_add3;
};

struct socfpga_sdr_reg_file {
	u32 signature;
	u32 debug_data_addr;
	u32 cur_stage;
	u32 fom;
	u32 failing_stage;
	u32 debug1;
	u32 debug2;
	u32 dtaps_per_ptap;
	u32 trk_sample_count;
	u32 trk_longidle;
	u32 delays;
	u32 trk_rw_mgr_addr;
	u32 trk_read_dqs_width;
	u32 trk_rfsh;
};

/* parameter variable holder */
struct param_type {
	uint32_t dm_correct_mask;
	uint32_t read_correct_mask;
	uint32_t read_correct_mask_vg;
	uint32_t write_correct_mask;
	uint32_t write_correct_mask_vg;

	/* set a particular entry to 1 if we need to skip a particular rank */

	uint32_t skip_ranks[MAX_RANKS];

	/* set a particular entry to 1 if we need to skip a particular group */

	uint32_t skip_groups;

	/* set a particular entry to 1 if the shadow register
	(which represents a set of ranks) needs to be skipped */

	uint32_t skip_shadow_regs[NUM_SHADOW_REGS];

};


/* global variable holder */
struct gbl_type {
	uint32_t phy_debug_mode_flags;

	/* current read latency */

	uint32_t curr_read_lat;

	/* current write latency */

	uint32_t curr_write_lat;

	/* error code */

	uint32_t error_substage;
	uint32_t error_stage;
	uint32_t error_group;

	/* figure-of-merit in, figure-of-merit out */

	uint32_t fom_in;
	uint32_t fom_out;

	/*USER Number of RW Mgr NOP cycles between
	write command and write data */
	uint32_t rw_wl_nop_cycles;
};

struct socfpga_sdr_scc_mgr {
	u32	dqs_ena;
	u32	dqs_io_ena;
	u32	dq_ena;
	u32	dm_ena;
	u32	__padding1[4];
	u32	update;
	u32	__padding2[7];
	u32	active_rank;
};

/* PHY manager configuration registers. */
struct socfpga_phy_mgr_cfg {
	u32	phy_rlat;
	u32	reset_mem_stbl;
	u32	mux_sel;
	u32	cal_status;
	u32	cal_debug_info;
	u32	vfifo_rd_en_ovrd;
	u32	afi_wlat;
	u32	afi_rlat;
};

/* PHY manager command addresses. */
struct socfpga_phy_mgr_cmd {
	u32	inc_vfifo_fr;
	u32	inc_vfifo_hard_phy;
	u32	fifo_reset;
	u32	inc_vfifo_fr_hr;
	u32	inc_vfifo_qr;
};

struct socfpga_data_mgr {
	u32	__padding1;
	u32	t_wl_add;
	u32	mem_t_add;
	u32	t_rl_add;
};
#endif /* _SEQUENCER_H_ */
