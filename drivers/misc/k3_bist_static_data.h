/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Static Data for Texas Instruments' BIST (Built-In Self-Test) driver
 *
 * Copyright (C) 2025 Texas Instruments Incorporated - https://www.ti.com/
 *
 */

#ifndef __K3_BIST_STATIC_DATA_H
#define __K3_BIST_STATIC_DATA_H

/*
 * Registers and functions related to PBIST
 */

#define PBIST_MAX_NUM_RUNS			2
#define NUM_MAX_PBIST_TEST_ROM_RUNS		13
#define PBIST14_DFT_PBIST_CPU_0_INTR_NUM	311

/* VIM Registers */
#define VIM_STS_BASE				0x40f80404
#define VIM_RAW_BASE				0x40f80400

#define VIM_STS(i)	(VIM_STS_BASE + (i) / 32 * 0x20)
#define VIM_RAW(i)	(VIM_RAW_BASE + (i) / 32 * 0x20)
#define VIM_RAW_MASK(i)	(BIT((i) % 32))

/* PBIST Registers and Flags*/
#define PBIST_RF0L						0x00000000
#define PBIST_RF1L						0x00000004
#define PBIST_RF2L						0x00000008
#define PBIST_RF3L						0x0000000C
#define PBIST_RF4L						0x0000010
#define PBIST_RF5L						0x0000014
#define PBIST_RF6L						0x0000018
#define PBIST_RF7L						0x000001C
#define PBIST_RF8L						0x0000020
#define PBIST_RF9L						0x0000024
#define PBIST_RF10L						0x0000028
#define PBIST_RF11L						0x000002C
#define PBIST_RF12L						0x0000030
#define PBIST_RF13L						0x0000034
#define PBIST_RF14L						0x0000038
#define PBIST_RF15L						0x000003C
#define PBIST_RF0U						0x0000040
#define PBIST_RF1U						0x0000044
#define PBIST_RF2U						0x0000048
#define PBIST_RF3U						0x000004C
#define PBIST_RF4U						0x0000050
#define PBIST_RF5U						0x0000054
#define PBIST_RF6U						0x0000058
#define PBIST_RF7U						0x000005C
#define PBIST_RF8U						0x0000060
#define PBIST_RF9U						0x0000064
#define PBIST_RF10U						0x0000068
#define PBIST_RF11U						0x000006C
#define PBIST_RF12U						0x0000070
#define PBIST_RF13U						0x0000074
#define PBIST_RF14U						0x0000078
#define PBIST_RF15U						0x000007C
#define PBIST_A0						0x0000100
#define PBIST_A1						0x0000104
#define PBIST_A2						0x0000108
#define PBIST_A3						0x000010C
#define PBIST_L0						0x0000110
#define PBIST_L1						0x0000114
#define PBIST_L2						0x0000118
#define PBIST_L3						0x000011C
#define PBIST_D						0x0000120
#define PBIST_E						0x0000124
#define PBIST_CA0						0x0000130
#define PBIST_CA1						0x0000134
#define PBIST_CA2						0x0000138
#define PBIST_CA3						0x000013C
#define PBIST_CL0						0x0000140
#define PBIST_CL1						0x0000144
#define PBIST_CL2						0x0000148
#define PBIST_CL3						0x000014C
#define PBIST_I0						0x0000150
#define PBIST_I1						0x0000154
#define PBIST_I2						0x0000158
#define PBIST_I3						0x000015C
#define PBIST_RAMT						0x0000160
#define PBIST_DLR						0x0000164
#define PBIST_CMS						0x0000168
#define PBIST_STR						0x000016C
#define PBIST_SCR						0x0000170
#define PBIST_SCR_LO						0x0000170
#define PBIST_SCR_HI						0x0000174
#define PBIST_CSR						0x0000178
#define PBIST_FDLY						0x000017C
#define PBIST_PACT						0x0000180
#define PBIST_PID						0x0000184
#define PBIST_OVER						0x0000188
#define PBIST_FSRF						0x0000190
#define PBIST_FSRC						0x0000198
#define PBIST_FSRA						0x00001A0
#define PBIST_FSRDL0						0x00001A8
#define PBIST_FSRDL1						0x00001B0
#define PBIST_MARGIN_MODE					0x00001B4
#define PBIST_WRENZ						0x00001B8
#define PBIST_PAGE_PGS						0x00001BC
#define PBIST_ROM						0x00001C0
#define PBIST_ALGO						0x00001C4
#define PBIST_RINFO						0x00001C8

#define PBIST_MARGIN_MODE_PBIST_DFT_WRITE_MASK			0x00000003
#define PBIST_MARGIN_MODE_PBIST_DFT_READ_SHIFT			0x00000002
#define PBIST_MARGIN_MODE_PBIST_DFT_READ_MASK			0x0000000C
#define PBIST_PACT_PACT_MASK					0x00000001
#define PBIST_DLR_DLR0_ROM_MASK				0x00000004
#define PBIST_DLR_DLR0_CAM_MASK				0x00000010
#define PBIST_NOT_DONE						0
#define PBIST_DONE						1

/* PBIST test mode */
#define PBIST_TEST_MODE (PBIST_MARGIN_MODE_PBIST_DFT_WRITE_MASK \
			 | (1 << PBIST_MARGIN_MODE_PBIST_DFT_READ_SHIFT))

/* PBIST Failure Insertion test mode */
#define PBIST_FAILURE_INSERTION_TEST_MODE (PBIST_MARGIN_MODE_PBIST_DFT_WRITE_MASK \
					   | PBIST_MARGIN_MODE_PBIST_DFT_READ_MASK)

/**
 * struct core_under_test - structure for a core under a BIST test
 * @dev_id: Device ID of the core
 * @proc_id: Processor ID of the core
 */
struct core_under_test {
	u32 dev_id;
	u32 proc_id;
};

/*
 * struct pbist_config - Structure for different configuration used for PBIST
 * @override: Override value for memory configuration
 * @algorithms_bit_map: Bitmap to select algorithms to use for test
 * @memory_groups_bit_map: Bitmap to select memory groups to run test on
 * @scramble_value_lo: Lower scramble value to be used for test
 * @scramble_value_hi: Higher scramble value to be used for test
 */
struct pbist_config {
	u32 override;
	u32 algorithms_bit_map;
	u64 memory_groups_bit_map;
	u32 scramble_value_lo;
	u32 scramble_value_hi;
};

/*
 * struct pbist_config_neg - Structure for different configuration used for PBIST
 * for the failure insertion test to generate negative result
 * @CA0: Failure insertion value for CA0
 * @CA1: Failure insertion value for CA1
 * @CA2: Failure insertion value for CA2
 * @CA3: Failure insertion value for CA3
 * @CL0: Failure insertion value for CL0
 * @CL1: Failure insertion value for CL1
 * @CL2: Failure insertion value for CL2
 * @CL3: Failure insertion value for CL3
 * @CMS: Failure insertion value for CMS
 * @CSR: Failure insertion value for CSR
 * @I0: Failure insertion value for I0
 * @I1: Failure insertion value for I1
 * @I2: Failure insertion value for I2
 * @I3: Failure insertion value for I3
 * @RAMT: Failure insertion value for RAMT
 */
struct pbist_config_neg {
	u32 CA0;
	u32 CA1;
	u32 CA2;
	u32 CA3;
	u32 CL0;
	u32 CL1;
	u32 CL2;
	u32 CL3;
	u32 CMS;
	u32 CSR;
	u32 I0;
	u32 I1;
	u32 I2;
	u32 I3;
	u32 RAMT;
};

/*
 * struct pbist_config_neg - Structure for different configuration used for PBIST
 * test of ROM
 * @D: ROM test value for D
 * @E: ROM test value for E
 * @CA2: ROM test value for CA2
 * @CL0: ROM test value for CL0
 * @CA3: ROM test value for CA3
 * @I0: ROM test value for I0
 * @CL1: ROM test value for CL1
 * @I3: ROM test value for I3
 * @I2: ROM test value for I2
 * @CL2: ROM test value for CL2
 * @CA1: ROM test value for CA1
 * @CA0: ROM test value for CA0
 * @CL3: ROM test value for CL3
 * @I1: ROM test value for I1
 * @RAMT: ROM test value for RAMT
 * @CSR: ROM test value for CSR
 * @CMS: ROM test value for CMS
 */
struct pbist_config_rom {
	u32 D;
	u32 E;
	u32 CA2;
	u32 CL0;
	u32 CA3;
	u32 I0;
	u32 CL1;
	u32 I3;
	u32 I2;
	u32 CL2;
	u32 CA1;
	u32 CA0;
	u32 CL3;
	u32 I1;
	u32 RAMT;
	u32 CSR;
	u32 CMS;
};

/*
 * struct pbist_inst_info - Structure for different configuration used for PBIST
 * @num_pbist_runs: Number of runs of PBIST test
 * @intr_num: Interrupt number triggered by this PBIST instance to MCU R5 VIM
 * @pbist_config_run: Configuration for PBIST test
 * @pbist_neg_config_run: Configuration for PBIST negative test
 * @num_pbist_rom_test_runs: Number of runs of PBIST test on ROM
 * @pbist_rom_test_config_run: Configuration for PBIST test on ROM
 */
struct pbist_inst_info {
	u32 num_pbist_runs;
	u32 intr_num;
	u32 dev_id;
	struct core_under_test cut[2];
	struct pbist_config pbist_config_run[PBIST_MAX_NUM_RUNS];
	struct pbist_config_neg pbist_neg_config_run;
	u32 num_pbist_rom_test_runs;
	struct pbist_config_rom pbist_rom_test_config_run[NUM_MAX_PBIST_TEST_ROM_RUNS];
};

/*
 * Registers and functions related to LBIST
 */

#define LBIST_CTRL_DIVIDE_RATIO_MASK            0x0000001F
#define LBIST_CTRL_DIVIDE_RATIO_SHIFT           0x00000000
#define LBIST_CTRL_DIVIDE_RATIO_MAX             0x0000001F

#define LBIST_CTRL_LOAD_DIV_MASK                0x00000080
#define LBIST_CTRL_LOAD_DIV_SHIFT               0x00000007
#define LBIST_CTRL_LOAD_DIV_MAX                 0x00000001

#define LBIST_CTRL_DC_DEF_MASK                  0x00000300
#define LBIST_CTRL_DC_DEF_SHIFT                 0x00000008
#define LBIST_CTRL_DC_DEF_MAX                   0x00000003

#define LBIST_CTRL_RUNBIST_MODE_MASK            0x0000F000
#define LBIST_CTRL_RUNBIST_MODE_SHIFT           0x0000000C
#define LBIST_CTRL_RUNBIST_MODE_MAX             0x0000000F

#define LBIST_CTRL_BIST_RUN_MASK                0x0F000000
#define LBIST_CTRL_BIST_RUN_SHIFT               0x00000018
#define LBIST_CTRL_BIST_RUN_MAX                 0x0000000F

#define LBIST_CTRL_BIST_RESET_MASK              0x80000000
#define LBIST_CTRL_BIST_RESET_SHIFT             0x0000001F
#define LBIST_CTRL_BIST_RESET_MAX               0x00000001

/* LBIST_PATCOUNT */

#define LBIST_PATCOUNT_SCAN_PC_DEF_MASK         0x0000000F
#define LBIST_PATCOUNT_SCAN_PC_DEF_SHIFT        0x00000000
#define LBIST_PATCOUNT_SCAN_PC_DEF_MAX          0x0000000F

#define LBIST_PATCOUNT_RESET_PC_DEF_MASK        0x000000F0
#define LBIST_PATCOUNT_RESET_PC_DEF_SHIFT       0x00000004
#define LBIST_PATCOUNT_RESET_PC_DEF_MAX         0x0000000F

#define LBIST_PATCOUNT_SET_PC_DEF_MASK          0x00000F00
#define LBIST_PATCOUNT_SET_PC_DEF_SHIFT         0x00000008
#define LBIST_PATCOUNT_SET_PC_DEF_MAX           0x0000000F

#define LBIST_PATCOUNT_STATIC_PC_DEF_MASK       0x3FFF0000
#define LBIST_PATCOUNT_STATIC_PC_DEF_SHIFT      0x00000010
#define LBIST_PATCOUNT_STATIC_PC_DEF_MAX        0x00003FFF

/* LBIST_SEED0 */

#define LBIST_SEED0_PRPG_DEF_MASK               0xFFFFFFFF
#define LBIST_SEED0_PRPG_DEF_SHIFT              0x00000000
#define LBIST_SEED0_PRPG_DEF_MAX                0xFFFFFFFF

/* LBIST_SEED1 */

#define LBIST_SEED1_PRPG_DEF_MASK               0x001FFFFF
#define LBIST_SEED1_PRPG_DEF_SHIFT              0x00000000
#define LBIST_SEED1_PRPG_DEF_MAX                0x001FFFFF

/* LBIST_SPARE0 */

#define LBIST_SPARE0_LBIST_SELFTEST_EN_MASK     0x00000001
#define LBIST_SPARE0_LBIST_SELFTEST_EN_SHIFT    0x00000000
#define LBIST_SPARE0_LBIST_SELFTEST_EN_MAX      0x00000001

#define LBIST_SPARE0_PBIST_SELFTEST_EN_MASK     0x00000002
#define LBIST_SPARE0_PBIST_SELFTEST_EN_SHIFT    0x00000001
#define LBIST_SPARE0_PBIST_SELFTEST_EN_MAX      0x00000001

#define LBIST_SPARE0_SPARE0_MASK                0xFFFFFFFC
#define LBIST_SPARE0_SPARE0_SHIFT               0x00000002
#define LBIST_SPARE0_SPARE0_MAX                 0x3FFFFFFF

/* LBIST_SPARE1 */

#define LBIST_SPARE1_SPARE1_MASK                0xFFFFFFFF
#define LBIST_SPARE1_SPARE1_SHIFT               0x00000000
#define LBIST_SPARE1_SPARE1_MAX                 0xFFFFFFFF

/* LBIST_STAT */

#define LBIST_STAT_MISR_MUX_CTL_MASK            0x000000FF
#define LBIST_STAT_MISR_MUX_CTL_SHIFT           0x00000000
#define LBIST_STAT_MISR_MUX_CTL_MAX             0x000000FF

#define LBIST_STAT_OUT_MUX_CTL_MASK             0x00000300
#define LBIST_STAT_OUT_MUX_CTL_SHIFT            0x00000008
#define LBIST_STAT_OUT_MUX_CTL_MAX              0x00000003

#define LBIST_STAT_BIST_RUNNING_MASK            0x00008000
#define LBIST_STAT_BIST_RUNNING_SHIFT           0x0000000F
#define LBIST_STAT_BIST_RUNNING_MAX             0x00000001

#define LBIST_STAT_BIST_DONE_MASK               0x80000000
#define LBIST_STAT_BIST_DONE_SHIFT              0x0000001F
#define LBIST_STAT_BIST_DONE_MAX                0x00000001

/* LBIST_MISR */

#define LBIST_MISR_MISR_RESULT_MASK             0xFFFFFFFF
#define LBIST_MISR_MISR_RESULT_SHIFT            0x00000000
#define LBIST_MISR_MISR_RESULT_MAX              0xFFFFFFFF

#define CTRL_MMR0_CFG0_BASE                     0x00100000
#define MAIN_CTRL_MMR_CFG0_MCU2_LBIST_CTRL      0x0000C1A0
#define MAIN_R5F2_LBIST_BASE         (CTRL_MMR0_CFG0_BASE +\
	MAIN_CTRL_MMR_CFG0_MCU2_LBIST_CTRL)

#define LBIST_CTRL                              0x00000000
#define LBIST_PATCOUNT                          0x00000004
#define LBIST_SEED0                             0x00000008
#define LBIST_SEED1                             0x0000000C
#define LBIST_SPARE0                            0x00000010
#define LBIST_SPARE1                            0x00000014
#define LBIST_STAT                              0x00000018
#define LBIST_MISR                              0x0000001C

#define MAIN_CTRL_MMR_CFG0_MCU2_LBIST_SIG       0x0000C2C0
#define MAIN_R5F2_LBIST_SIG         (CTRL_MMR0_CFG0_BASE +\
	MAIN_CTRL_MMR_CFG0_MCU2_LBIST_SIG)
#define MCU_R5FSS0_CORE0_INTR_LBIST_BIST_DONE_0	    284

/* Lbist Parameters */
#define LBIST_DC_DEF                   0x3
#define LBIST_DIVIDE_RATIO             0x02
#define LBIST_STATIC_PC_DEF            0x3ac0
#define LBIST_RESET_PC_DEF             0x0f
#define LBIST_SET_PC_DEF               0x00
#define LBIST_SCAN_PC_DEF              0x04
#define LBIST_PRPG_DEF_L		0xFFFFFFFF
#define LBIST_PRPG_DEF_U               0x1FFFFF

/*
 * LBIST setup parameters for each core
 */

#define LBIST_MAIN_R5_STATIC_PC_DEF    LBIST_STATIC_PC_DEF
#define LBIST_C7X_STATIC_PC_DEF        0x3fc0
#define LBIST_A72_STATIC_PC_DEF        0x3fc0
#define LBIST_DMPAC_STATIC_PC_DEF      0x1880
#define LBIST_VPAC_STATIC_PC_DEF       0x3fc0
#define LBIST_A72SS_STATIC_PC_DEF      0x13c0

/*
 * LBIST expected MISR's (using parameters above)
 */

#define MAIN_R5_MISR_EXP_VAL           0x71d66f87
#define A72_MISR_EXP_VAL               0x14df0200
#define C7X_MISR_EXP_VAL               0x57b0478f
#define VPAC_MISR_EXP_VAL              0xec6abe22
#define VPAC0_MISR_EXP_VAL             0x5c43b468
#define DMPAC_MISR_EXP_VAL             0x53e1ef7b
#define A72SS_MISR_EXP_VAL             0x87da5a92

/**
 * lbist_set_clock_delay() - Set seed for LBIST
 * @ctrl_mmr_base: CTRL MMR base
 * @clock_delay: clock delay
 *
 * Return: 0 if all went fine, else corresponding error.
 */
static void lbist_set_clock_delay(void *ctrl_mmr_base, u32 clock_delay)
{
	u32 reg_val;

	reg_val = readl(ctrl_mmr_base);
	writel(reg_val & LBIST_CTRL_DC_DEF_MASK, ctrl_mmr_base);

	reg_val = readl(ctrl_mmr_base);
	writel(reg_val | ((clock_delay & LBIST_CTRL_DC_DEF_MAX)
	       << LBIST_CTRL_DC_DEF_SHIFT), ctrl_mmr_base);
}

/**
 * lbist_set_seed() - Set seed for LBIST
 * @config: lbist_config structure for LBIST test
 * @seed: seed
 *
 * Return: 0 if all went fine, else corresponding error.
 */
static void lbist_set_seed(void *ctrl_mmr_base, u32 seed_l, u32 seed_u)
{
	writel(seed_l & LBIST_SEED0_PRPG_DEF_MASK, ctrl_mmr_base + LBIST_SEED0);
	writel(seed_u & LBIST_SEED1_PRPG_DEF_MASK, ctrl_mmr_base + LBIST_SEED1);
}

/**
 * set_num_chain_test_patterns() - Set chain test patterns
 * @ctrl_mmr_base: CTRL MMR base
 * @chain_test_patterns: chain test patterns
 *
 * Return: 0 if all went fine, else corresponding error.
 */
static void lbist_set_num_chain_test_patterns(void *ctrl_mmr_base, u32 chain_test_patterns)
{
	u32 reg_val;

	reg_val = readl(ctrl_mmr_base + LBIST_PATCOUNT);
	writel(reg_val & (~(LBIST_PATCOUNT_SCAN_PC_DEF_MASK)),
	       ctrl_mmr_base + LBIST_PATCOUNT);

	reg_val = readl(ctrl_mmr_base + LBIST_PATCOUNT);
	writel(reg_val | ((chain_test_patterns & LBIST_PATCOUNT_SCAN_PC_DEF_MAX)
	       << LBIST_PATCOUNT_SCAN_PC_DEF_SHIFT), ctrl_mmr_base + LBIST_PATCOUNT);
}

/**
 * set_num_reset_patterns() - Set reset patterns
 * @ctrl_mmr_base: CTRL MMR base
 * @reset_patterns: reset patterns
 *
 * Return: 0 if all went fine, else corresponding error.
 */
static void lbist_set_num_reset_patterns(void *ctrl_mmr_base, u32 reset_patterns)
{
	u32 reg_val;

	reg_val = readl(ctrl_mmr_base + LBIST_PATCOUNT);
	writel(reg_val & (~(LBIST_PATCOUNT_RESET_PC_DEF_MASK)),
	       ctrl_mmr_base + LBIST_PATCOUNT);

	reg_val = readl(ctrl_mmr_base + LBIST_PATCOUNT);
	writel(reg_val | ((reset_patterns & LBIST_PATCOUNT_RESET_PC_DEF_MAX)
	       << LBIST_PATCOUNT_RESET_PC_DEF_SHIFT), ctrl_mmr_base + LBIST_PATCOUNT);
}

/**
 * set_num_set_patterns() - Set patterns
 * @ctrl_mmr_base: CTRL MMR base
 * @set_patterns: set patterns
 *
 * Return: 0 if all went fine, else corresponding error.
 */
static void lbist_set_num_set_patterns(void *ctrl_mmr_base, u32 set_patterns)
{
	u32 reg_val;

	reg_val = readl(ctrl_mmr_base + LBIST_PATCOUNT);
	writel(reg_val & (~(LBIST_PATCOUNT_SET_PC_DEF_MASK)),
	       ctrl_mmr_base + LBIST_PATCOUNT);

	reg_val = readl(ctrl_mmr_base + LBIST_PATCOUNT);
	writel(reg_val | ((set_patterns & LBIST_PATCOUNT_RESET_PC_DEF_MAX)
	       << LBIST_PATCOUNT_SET_PC_DEF_SHIFT), ctrl_mmr_base + LBIST_PATCOUNT);
}

/**
 * set_num_stuck_at_patterns() - Set
 * @ctrl_mmr_base: CTRL MMR base
 * @stuck_at_patterns: set patterns
 *
 * Return: 0 if all went fine, else corresponding error.
 */
static void lbist_set_num_stuck_at_patterns(void *ctrl_mmr_base, u32 stuck_at_patterns)
{
	u32 reg_val;

	reg_val = readl(ctrl_mmr_base + LBIST_PATCOUNT);
	writel(reg_val & (~(LBIST_PATCOUNT_STATIC_PC_DEF_MASK)),
	       ctrl_mmr_base + LBIST_PATCOUNT);

	reg_val = readl(ctrl_mmr_base + LBIST_PATCOUNT);
	writel(reg_val | ((stuck_at_patterns & LBIST_PATCOUNT_STATIC_PC_DEF_MAX)
	       << LBIST_PATCOUNT_STATIC_PC_DEF_SHIFT), ctrl_mmr_base + LBIST_PATCOUNT);
}

/**
 * set_divide_ratio() - Set divide ratio
 * @ctrl_mmr_base: CTRL MMR base
 * @divide_ratio: divide ratio
 *
 * Return: 0 if all went fine, else corresponding error.
 */
static void lbist_set_divide_ratio(void *ctrl_mmr_base, u32 divide_ratio)
{
	u32 reg_val;

	reg_val = readl(ctrl_mmr_base + LBIST_CTRL);
	writel(reg_val & (~(LBIST_CTRL_DIVIDE_RATIO_MASK)), ctrl_mmr_base + LBIST_CTRL);

	reg_val = readl(ctrl_mmr_base + LBIST_CTRL);
	writel(reg_val | (divide_ratio & LBIST_CTRL_DIVIDE_RATIO_MASK),
	       ctrl_mmr_base + LBIST_CTRL);
}

/**
 * clear_load_div() - Clear load div
 * @ctrl_mmr_base: CTRL MMR base
 *
 * Return: 0 if all went fine, else corresponding error.
 */
static void lbist_clear_load_div(void *ctrl_mmr_base)
{
	u32 reg_val;

	reg_val = readl(ctrl_mmr_base + LBIST_CTRL);
	writel(reg_val & (~(LBIST_CTRL_LOAD_DIV_MASK)), ctrl_mmr_base + LBIST_CTRL);
}

/**
 * set_load_div() - Set load div
 * @ctrl_mmr_base: CTRL MMR base
 *
 * Return: 0 if all went fine, else corresponding error.
 */
static void lbist_set_load_div(void *ctrl_mmr_base)
{
	u32 reg_val;

	reg_val = readl(ctrl_mmr_base + LBIST_CTRL);
	writel(reg_val | (LBIST_CTRL_LOAD_DIV_MASK), ctrl_mmr_base + LBIST_CTRL);
}

/* MACRO DEFINES */
#define LBIST_STAT_MISR_MUX_CTL_COMPACT_MISR            0x0

#define LBIST_STAT_OUT_MUX_CTL_CTRLMMR_PID              0x0
#define LBIST_STAT_OUT_MUX_CTL_CTRL_ID                  0x1
#define LBIST_STAT_OUT_MUX_CTL_MISR_VALUE_1             0x2
#define LBIST_STAT_OUT_MUX_CTL_MISR_VALUE_2             0x3

/**
 * lbist_get_misr() - Get MISR
 * @ctrl_mmr_base: CTRL MMR base
 *
 * Return: 0 if all went fine, else corresponding error.
 */
static void lbist_get_misr(void *ctrl_mmr_base, u32 *p_misr_val)
{
	u32 reg_val;
	u32 mux_val;

	reg_val = LBIST_STAT_MISR_MUX_CTL_COMPACT_MISR;
	mux_val = LBIST_STAT_OUT_MUX_CTL_MISR_VALUE_1;
	reg_val |= (mux_val << LBIST_STAT_OUT_MUX_CTL_SHIFT);
	writel(reg_val, ctrl_mmr_base + LBIST_STAT);
	*p_misr_val = readl(ctrl_mmr_base + LBIST_MISR);
}

/**
 * lbist_clear_run_bist_mode() - Clear RUN_BIST_MODE
 * @ctrl_mmr_base: CTRL MMR base
 *
 * Return: 0 if all went fine, else corresponding error.
 */
static void lbist_clear_run_bist_mode(void *ctrl_mmr_base)
{
	u32 reg_val;

	reg_val = readl(ctrl_mmr_base + LBIST_CTRL);
	writel(reg_val & (~(LBIST_CTRL_RUNBIST_MODE_MAX << LBIST_CTRL_RUNBIST_MODE_SHIFT)),
	       ctrl_mmr_base + LBIST_CTRL);
}

/**
 * lbist_stop() - Stop running LBIST
 * @ctrl_mmr_base: CTRL MMR base
 *
 * Return: 0 if all went fine, else corresponding error.
 */
static void lbist_stop(void *ctrl_mmr_base)
{
	u32 reg_val;

	reg_val = readl(ctrl_mmr_base + LBIST_CTRL);
	writel(reg_val & (~(LBIST_CTRL_BIST_RUN_MAX << LBIST_CTRL_BIST_RUN_SHIFT)),
	       ctrl_mmr_base + LBIST_CTRL);
}

/**
 * lbist_reset() - Reset LBIST
 * @ctrl_mmr_base: CTRL MMR base
 *
 * Return: 0 if all went fine, else corresponding error.
 */
static void lbist_reset(void *ctrl_mmr_base)
{
	u32 reg_val;

	reg_val = readl(ctrl_mmr_base + LBIST_CTRL);
	writel(reg_val & (~(LBIST_CTRL_BIST_RESET_MAX << LBIST_CTRL_BIST_RESET_SHIFT)),
	       ctrl_mmr_base + LBIST_CTRL);
}

/*
 * struct lbist_config - Structure containing different configuration used for LBIST
 * @dc_def: Clock delay after scan_enable switching
 * @divide_ratio: LBIST clock divide ratio
 * @static_pc_def: Bitmap of stuck-at patterns to run
 * @set_pc_def: Bitmap of set patterns to run
 * @reset_pc_def: Bitmap of reset patterns to run
 * @scan_pc_def: Bitmap of chain test patterns to run
 * @prpg_def: Initial seed for Pseudo Random Pattern generator (PRPG)
 */
struct lbist_config {
	u32 dc_def;
	u32 divide_ratio;
	u32 static_pc_def;
	u32 set_pc_def;
	u32 reset_pc_def;
	u32 scan_pc_def;
	u32 prpg_def_l;
	u32 prpg_def_u;
};

/*
 * struct lbist_inst_info - Structure for different configuration used for LBIST
 * @lbist_signature: Pointer to LBIST signature
 * @intr_num: Interrupt number triggered by this LBIST instance to MCU R5 VIM
 * @expected_misr: Expected signature
 * @lbist_config: Configuration for LBIST test
 */
struct lbist_inst_info {
	u32 *lbist_signature;
	u32 intr_num;
	u32 expected_misr;
	struct lbist_config lbist_conf;
	struct core_under_test cut;
};

#if IS_ENABLED(CONFIG_SOC_K3_J784S4)

#include "k3_j784s4_bist_static_data.h"

#endif /* CONFIG_SOC_K3_J784S4 */
#endif /* __K3_BIST_STATIC_DATA_H */
