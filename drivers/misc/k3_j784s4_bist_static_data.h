/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Static Data for Texas Instruments' BIST logic for J784S4
 *
 * Copyright (C) 2025 Texas Instruments Incorporated - https://www.ti.com/
 *
 */

/* Device IDs of IPs that can be tested under BIST */
#define TISCI_DEV_MCU_R5FSS2_CORE0		343
#define TISCI_DEV_MCU_R5FSS2_CORE1		344
#define TISCI_DEV_RTI32					365
#define TISCI_DEV_RTI33					366

/* WKUP CTRL MMR Registers */
#define WKUP_CTRL_MMR_CFG0_WKUP_POST_STAT				0x0000C2C0
#define WKUP_CTRL_MMR_CFG0_WKUP_POST_STAT_POST_MCU_PBIST_DONE_SHIFT	0x00000008
#define WKUP_CTRL_MMR_CFG0_WKUP_POST_STAT_POST_MCU_LBIST_DONE_SHIFT	0x00000001
#define WKUP_CTRL_MMR_CFG0_WKUP_POST_STAT_POST_MCU_PBIST_TIMEOUT_SHIFT	0x00000009
#define WKUP_CTRL_MMR_CFG0_WKUP_POST_STAT_POST_MCU_LBIST_TIMEOUT_SHIFT	0x00000005
#define WKUP_CTRL_MMR_CFG0_WKUP_POST_STAT_POST_MCU_PBIST_FAIL_MASK	0x00008000

/* MCU CTRL MMR Register */
#define MCU_CTRL_MMR0_CFG0_BASE					0x40f00000
#define MCU_CTRL_MMR_CFG0_MCU_LBIST_CTRL				0x0000c000
#define MCU_CTRL_MMR_CFG0_MCU_LBIST_SIG				0x0000c280
#define MCU_LBIST_BASE			(MCU_CTRL_MMR0_CFG0_BASE + \
					 MCU_CTRL_MMR_CFG0_MCU_LBIST_CTRL)

/* Properties of PBIST instances in: PBIST14 */
#define PBIST14_DEV_ID					      234
#define PBIST14_NUM_TEST_VECTORS				      0x1
#define PBIST14_ALGO_BITMAP_0					      0x00000003
#define PBIST14_MEM_BITMAP_0					      0x000CCCCC
#define PBIST14_FAIL_INSERTION_TEST_VECTOR_CA0			      0x00000000
#define PBIST14_FAIL_INSERTION_TEST_VECTOR_CA1			      0x000001FF
#define PBIST14_FAIL_INSERTION_TEST_VECTOR_CA2			      0x000001FF
#define PBIST14_FAIL_INSERTION_TEST_VECTOR_CA3			      0x00000000
#define PBIST14_FAIL_INSERTION_TEST_VECTOR_CL0			      0x0000007F
#define PBIST14_FAIL_INSERTION_TEST_VECTOR_CL1			      0x00000003
#define PBIST14_FAIL_INSERTION_TEST_VECTOR_CL2			      0x00000008
#define PBIST14_FAIL_INSERTION_TEST_VECTOR_CL3			      0x000001FF
#define PBIST14_FAIL_INSERTION_TEST_VECTOR_CMS			      0x00000000
#define PBIST14_FAIL_INSERTION_TEST_VECTOR_CSR			      0x20000000
#define PBIST14_FAIL_INSERTION_TEST_VECTOR_I0			      0x00000001
#define PBIST14_FAIL_INSERTION_TEST_VECTOR_I1			      0x00000004
#define PBIST14_FAIL_INSERTION_TEST_VECTOR_I2			      0x00000008
#define PBIST14_FAIL_INSERTION_TEST_VECTOR_I3			      0x00000000
#define PBIST14_FAIL_INSERTION_TEST_VECTOR_RAMT		      0x011D2528

static struct pbist_inst_info pbist14_inst_info = {
	/* Main Pulsar 2 Instance 1 or MAIN_R52_x */
	.num_pbist_runs = 1,
	.intr_num = PBIST14_DFT_PBIST_CPU_0_INTR_NUM,
	.dev_id = TISCI_DEV_PBIST14,
	.cut = {
		{
			.dev_id = TISCI_DEV_R5FSS2_CORE0,
			.proc_id = PROC_ID_MCU_R5FSS2_CORE0,
		},
		{
			.dev_id = TISCI_DEV_R5FSS2_CORE1,
			.proc_id = PROC_ID_MCU_R5FSS2_CORE1,
		}
	},
	.pbist_config_run = {
		{
			.override = 0,
			.algorithms_bit_map = PBIST14_ALGO_BITMAP_0,
			.memory_groups_bit_map = PBIST14_MEM_BITMAP_0,
			.scramble_value_lo = 0x76543210,
			.scramble_value_hi = 0xFEDCBA98,
		},
		{
			.override = 0,
			.algorithms_bit_map = 0,
			.memory_groups_bit_map = 0,
			.scramble_value_lo = 0,
			.scramble_value_hi = 0,
		},
	},
	.pbist_neg_config_run = {
		.CA0   = PBIST14_FAIL_INSERTION_TEST_VECTOR_CA0,
		.CA1   = PBIST14_FAIL_INSERTION_TEST_VECTOR_CA1,
		.CA2   = PBIST14_FAIL_INSERTION_TEST_VECTOR_CA2,
		.CA3   = PBIST14_FAIL_INSERTION_TEST_VECTOR_CA3,
		.CL0   = PBIST14_FAIL_INSERTION_TEST_VECTOR_CL0,
		.CL1   = PBIST14_FAIL_INSERTION_TEST_VECTOR_CL1,
		.CL2   = PBIST14_FAIL_INSERTION_TEST_VECTOR_CL2,
		.CL3   = PBIST14_FAIL_INSERTION_TEST_VECTOR_CL3,
		.CMS   = PBIST14_FAIL_INSERTION_TEST_VECTOR_CMS,
		.CSR   = PBIST14_FAIL_INSERTION_TEST_VECTOR_CSR,
		.I0    = PBIST14_FAIL_INSERTION_TEST_VECTOR_I0,
		.I1    = PBIST14_FAIL_INSERTION_TEST_VECTOR_I1,
		.I2    = PBIST14_FAIL_INSERTION_TEST_VECTOR_I2,
		.I3    = PBIST14_FAIL_INSERTION_TEST_VECTOR_I3,
		.RAMT  = PBIST14_FAIL_INSERTION_TEST_VECTOR_RAMT
	},
	.num_pbist_rom_test_runs = 1,
	.pbist_rom_test_config_run = {
		{
			.D = 0xF412605Eu,
			.E = 0xF412605Eu,
			.CA2 = 0x7FFFu,
			.CL0 = 0x3FFu,
			.CA3 = 0x0u,
			.I0 = 0x1u,
			.CL1 = 0x1Fu,
			.I3 = 0x0u,
			.I2 = 0xEu,
			.CL2 = 0xEu,
			.CA1 = 0x7FFFu,
			.CA0 = 0x0u,
			.CL3 = 0x7FFFu,
			.I1 = 0x20u,
			.RAMT = 0x08002020u,
			.CSR = 0x00000001u,
			.CMS = 0x01u
		},
		{
			.D = 0x0u,
			.E = 0x0u,
			.CA2 = 0x0u,
			.CL0 = 0x0u,
			.CA3 = 0x0u,
			.I0 = 0x0u,
			.CL1 = 0x0u,
			.I3 = 0x0u,
			.I2 = 0x0u,
			.CL2 = 0x0u,
			.CA1 = 0x0u,
			.CA0 = 0x0u,
			.CL3 = 0x0u,
			.I1 = 0x0u,
			.RAMT = 0x0u,
			.CSR = 0x0u,
			.CMS = 0x0u
		},
		{
			.D = 0x0u,
			.E = 0x0u,
			.CA2 = 0x0u,
			.CL0 = 0x0u,
			.CA3 = 0x0u,
			.I0 = 0x0u,
			.CL1 = 0x0u,
			.I3 = 0x0u,
			.I2 = 0x0u,
			.CL2 = 0x0u,
			.CA1 = 0x0u,
			.CA0 = 0x0u,
			.CL3 = 0x0u,
			.I1 = 0x0u,
			.RAMT = 0x0u,
			.CSR = 0x0u,
			.CMS = 0x0u
		},
		{
			.D = 0x0u,
			.E = 0x0u,
			.CA2 = 0x0u,
			.CL0 = 0x0u,
			.CA3 = 0x0u,
			.I0 = 0x0u,
			.CL1 = 0x0u,
			.I3 = 0x0u,
			.I2 = 0x0u,
			.CL2 = 0x0u,
			.CA1 = 0x0u,
			.CA0 = 0x0u,
			.CL3 = 0x0u,
			.I1 = 0x0u,
			.RAMT = 0x0u,
			.CSR = 0x0u,
			.CMS = 0x0u
		},
		{
			.D = 0x0u,
			.E = 0x0u,
			.CA2 = 0x0u,
			.CL0 = 0x0u,
			.CA3 = 0x0u,
			.I0 = 0x0u,
			.CL1 = 0x0u,
			.I3 = 0x0u,
			.I2 = 0x0u,
			.CL2 = 0x0u,
			.CA1 = 0x0u,
			.CA0 = 0x0u,
			.CL3 = 0x0u,
			.I1 = 0x0u,
			.RAMT = 0x0u,
			.CSR = 0x0u,
			.CMS = 0x0u
		},
		{
			.D = 0x0u,
			.E = 0x0u,
			.CA2 = 0x0u,
			.CL0 = 0x0u,
			.CA3 = 0x0u,
			.I0 = 0x0u,
			.CL1 = 0x0u,
			.I3 = 0x0u,
			.I2 = 0x0u,
			.CL2 = 0x0u,
			.CA1 = 0x0u,
			.CA0 = 0x0u,
			.CL3 = 0x0u,
			.I1 = 0x0u,
			.RAMT = 0x0u,
			.CSR = 0x0u,
			.CMS = 0x0u
		},
		{
			.D = 0x0u,
			.E = 0x0u,
			.CA2 = 0x0u,
			.CL0 = 0x0u,
			.CA3 = 0x0u,
			.I0 = 0x0u,
			.CL1 = 0x0u,
			.I3 = 0x0u,
			.I2 = 0x0u,
			.CL2 = 0x0u,
			.CA1 = 0x0u,
			.CA0 = 0x0u,
			.CL3 = 0x0u,
			.I1 = 0x0u,
			.RAMT = 0x0u,
			.CSR = 0x0u,
			.CMS = 0x0u
		},
		{
			.D = 0x0u,
			.E = 0x0u,
			.CA2 = 0x0u,
			.CL0 = 0x0u,
			.CA3 = 0x0u,
			.I0 = 0x0u,
			.CL1 = 0x0u,
			.I3 = 0x0u,
			.I2 = 0x0u,
			.CL2 = 0x0u,
			.CA1 = 0x0u,
			.CA0 = 0x0u,
			.CL3 = 0x0u,
			.I1 = 0x0u,
			.RAMT = 0x0u,
			.CSR = 0x0u,
			.CMS = 0x0u
		},
		{
			.D = 0x0u,
			.E = 0x0u,
			.CA2 = 0x0u,
			.CL0 = 0x0u,
			.CA3 = 0x0u,
			.I0 = 0x0u,
			.CL1 = 0x0u,
			.I3 = 0x0u,
			.I2 = 0x0u,
			.CL2 = 0x0u,
			.CA1 = 0x0u,
			.CA0 = 0x0u,
			.CL3 = 0x0u,
			.I1 = 0x0u,
			.RAMT = 0x0u,
			.CSR = 0x0u,
			.CMS = 0x0u
		},
		{
			.D = 0x0u,
			.E = 0x0u,
			.CA2 = 0x0u,
			.CL0 = 0x0u,
			.CA3 = 0x0u,
			.I0 = 0x0u,
			.CL1 = 0x0u,
			.I3 = 0x0u,
			.I2 = 0x0u,
			.CL2 = 0x0u,
			.CA1 = 0x0u,
			.CA0 = 0x0u,
			.CL3 = 0x0u,
			.I1 = 0x0u,
			.RAMT = 0x0u,
			.CSR = 0x0u,
			.CMS = 0x0u
		},
		{
			.D = 0x0u,
			.E = 0x0u,
			.CA2 = 0x0u,
			.CL0 = 0x0u,
			.CA3 = 0x0u,
			.I0 = 0x0u,
			.CL1 = 0x0u,
			.I3 = 0x0u,
			.I2 = 0x0u,
			.CL2 = 0x0u,
			.CA1 = 0x0u,
			.CA0 = 0x0u,
			.CL3 = 0x0u,
			.I1 = 0x0u,
			.RAMT = 0x0u,
			.CSR = 0x0u,
			.CMS = 0x0u
		},
		{
			.D = 0x0u,
			.E = 0x0u,
			.CA2 = 0x0u,
			.CL0 = 0x0u,
			.CA3 = 0x0u,
			.I0 = 0x0u,
			.CL1 = 0x0u,
			.I3 = 0x0u,
			.I2 = 0x0u,
			.CL2 = 0x0u,
			.CA1 = 0x0u,
			.CA0 = 0x0u,
			.CL3 = 0x0u,
			.I1 = 0x0u,
			.RAMT = 0x0u,
			.CSR = 0x0u,
			.CMS = 0x0u
		},
		{
			.D = 0x0u,
			.E = 0x0u,
			.CA2 = 0x0u,
			.CL0 = 0x0u,
			.CA3 = 0x0u,
			.I0 = 0x0u,
			.CL1 = 0x0u,
			.I3 = 0x0u,
			.I2 = 0x0u,
			.CL2 = 0x0u,
			.CA1 = 0x0u,
			.CA0 = 0x0u,
			.CL3 = 0x0u,
			.I1 = 0x0u,
			.RAMT = 0x0u,
			.CSR = 0x0u,
			.CMS = 0x0u
		},
	},
};

static struct lbist_inst_info lbist_inst_info_main_r5f2_x = {
	/* Main Pulsar 2 Instance 1 or MAIN_R52_x */
	.lbist_signature = (u32 *)(MAIN_R5F2_LBIST_SIG),
	.intr_num = MCU_R5FSS0_CORE0_INTR_LBIST_BIST_DONE_0,
	.expected_misr = MAIN_R5_MISR_EXP_VAL,
	.lbist_conf = {
		.dc_def        = LBIST_DC_DEF,
		.divide_ratio  = LBIST_DIVIDE_RATIO,
		.static_pc_def = LBIST_MAIN_R5_STATIC_PC_DEF,
		.set_pc_def    = LBIST_SET_PC_DEF,
		.reset_pc_def  = LBIST_RESET_PC_DEF,
		.scan_pc_def   = LBIST_SCAN_PC_DEF,
		.prpg_def_l      = LBIST_PRPG_DEF_L,
		.prpg_def_u      = LBIST_PRPG_DEF_U,
	},
	.cut = {
		.dev_id = TISCI_DEV_R5FSS2_CORE0,
		.proc_id = PROC_ID_MCU_R5FSS2_CORE0,
	},
};
