/*
 * Copyright (C) Marvell International Ltd. and its affiliates
 *
 * SPDX-License-Identifier:	GPL-2.0
 */

#include <spl.h>
#include <asm/io.h>
#include <asm/arch/cpu.h>
#include <asm/arch/soc.h>
#include <linux/delay.h>

#include "ddr3_init.h"

#define WL_ITERATION_NUM		10
#define ONE_CLOCK_ERROR_SHIFT		2
#define ALIGN_ERROR_SHIFT		-2

static u32 pup_mask_table[] = {
	0x000000ff,
	0x0000ff00,
	0x00ff0000,
	0xff000000
};

static struct write_supp_result wr_supp_res[MAX_INTERFACE_NUM][MAX_BUS_NUM];

static int ddr3_tip_dynamic_write_leveling_seq(u32 dev_num);
static int ddr3_tip_dynamic_read_leveling_seq(u32 dev_num);
static int ddr3_tip_dynamic_per_bit_read_leveling_seq(u32 dev_num);
static int ddr3_tip_wl_supp_align_err_shift(u32 dev_num, u32 if_id, u32 bus_id,
					    u32 bus_id_delta);
static int ddr3_tip_wl_supp_align_phase_shift(u32 dev_num, u32 if_id,
					      u32 bus_id, u32 offset,
					      u32 bus_id_delta);
static int ddr3_tip_xsb_compare_test(u32 dev_num, u32 if_id, u32 bus_id,
				     u32 edge_offset, u32 bus_id_delta);
static int ddr3_tip_wl_supp_one_clk_err_shift(u32 dev_num, u32 if_id,
					      u32 bus_id, u32 bus_id_delta);

u32 hws_ddr3_tip_max_cs_get(void)
{
	u32 c_cs;
	static u32 max_cs;
	struct hws_topology_map *tm = ddr3_get_topology_map();

	if (!max_cs) {
		for (c_cs = 0; c_cs < NUM_OF_CS; c_cs++) {
			VALIDATE_ACTIVE(tm->
					interface_params[0].as_bus_params[0].
					cs_bitmask, c_cs);
			max_cs++;
		}
	}

	return max_cs;
}

/*****************************************************************************
Dynamic read leveling
******************************************************************************/
int ddr3_tip_dynamic_read_leveling(u32 dev_num, u32 freq)
{
	u32 data, mask;
	u32 max_cs = hws_ddr3_tip_max_cs_get();
	u32 bus_num, if_id, cl_val;
	enum hws_speed_bin speed_bin_index;
	/* save current CS value */
	u32 cs_enable_reg_val[MAX_INTERFACE_NUM] = { 0 };
	int is_any_pup_fail = 0;
	u32 data_read[MAX_INTERFACE_NUM + 1] = { 0 };
	u8 rl_values[NUM_OF_CS][MAX_BUS_NUM][MAX_INTERFACE_NUM];
	struct pattern_info *pattern_table = ddr3_tip_get_pattern_table();
	u16 *mask_results_pup_reg_map = ddr3_tip_get_mask_results_pup_reg_map();
	struct hws_topology_map *tm = ddr3_get_topology_map();

	if (rl_version == 0) {
		/* OLD RL machine */
		data = 0x40;
		data |= (1 << 20);

		/* TBD multi CS */
		CHECK_STATUS(ddr3_tip_if_write(
				     dev_num, ACCESS_TYPE_MULTICAST,
				     PARAM_NOT_CARE, TRAINING_REG,
				     data, 0x11ffff));
		CHECK_STATUS(ddr3_tip_if_write(
				     dev_num, ACCESS_TYPE_MULTICAST,
				     PARAM_NOT_CARE,
				     TRAINING_PATTERN_BASE_ADDRESS_REG,
				     0, 0xfffffff8));
		CHECK_STATUS(ddr3_tip_if_write(
				     dev_num, ACCESS_TYPE_MULTICAST,
				     PARAM_NOT_CARE, TRAINING_REG,
				     (u32)(1 << 31), (u32)(1 << 31)));

		for (if_id = 0; if_id <= MAX_INTERFACE_NUM - 1; if_id++) {
			VALIDATE_ACTIVE(tm->if_act_mask, if_id);
			training_result[training_stage][if_id] = TEST_SUCCESS;
			if (ddr3_tip_if_polling
			    (dev_num, ACCESS_TYPE_UNICAST, if_id, 0,
			     (u32)(1 << 31), TRAINING_REG,
			     MAX_POLLING_ITERATIONS) != MV_OK) {
				DEBUG_LEVELING(
					DEBUG_LEVEL_ERROR,
					("RL: DDR3 poll failed(1) IF %d\n",
					 if_id));
				training_result[training_stage][if_id] =
					TEST_FAILED;

				if (debug_mode == 0)
					return MV_FAIL;
			}
		}

		/* read read-leveling result */
		CHECK_STATUS(ddr3_tip_if_read
			     (dev_num, ACCESS_TYPE_MULTICAST, PARAM_NOT_CARE,
			      TRAINING_REG, data_read, 1 << 30));
		/* exit read leveling mode */
		CHECK_STATUS(ddr3_tip_if_write
			     (dev_num, ACCESS_TYPE_MULTICAST, PARAM_NOT_CARE,
			      TRAINING_SW_2_REG, 0x8, 0x9));
		CHECK_STATUS(ddr3_tip_if_write
			     (dev_num, ACCESS_TYPE_MULTICAST, PARAM_NOT_CARE,
			      TRAINING_SW_1_REG, 1 << 16, 1 << 16));

		/* disable RL machine all Trn_CS[3:0] , [16:0] */

		CHECK_STATUS(ddr3_tip_if_write
			     (dev_num, ACCESS_TYPE_MULTICAST, PARAM_NOT_CARE,
			      TRAINING_REG, 0, 0xf1ffff));

		for (if_id = 0; if_id <= MAX_INTERFACE_NUM - 1; if_id++) {
			VALIDATE_ACTIVE(tm->if_act_mask, if_id);
			if ((data_read[if_id] & (1 << 30)) == 0) {
				DEBUG_LEVELING(
					DEBUG_LEVEL_ERROR,
					("\n_read Leveling failed for IF %d\n",
					 if_id));
				training_result[training_stage][if_id] =
					TEST_FAILED;
				if (debug_mode == 0)
					return MV_FAIL;
			}
		}
		return MV_OK;
	}

	/* NEW RL machine */
	for (effective_cs = 0; effective_cs < NUM_OF_CS; effective_cs++)
		for (bus_num = 0; bus_num < MAX_BUS_NUM; bus_num++)
			for (if_id = 0; if_id <= MAX_INTERFACE_NUM - 1; if_id++)
				rl_values[effective_cs][bus_num][if_id] = 0;

	for (effective_cs = 0; effective_cs < max_cs; effective_cs++) {
		for (if_id = 0; if_id <= MAX_INTERFACE_NUM - 1; if_id++) {
			VALIDATE_ACTIVE(tm->if_act_mask, if_id);
			training_result[training_stage][if_id] = TEST_SUCCESS;

			/* save current cs enable reg val */
			CHECK_STATUS(ddr3_tip_if_read
				     (dev_num, ACCESS_TYPE_UNICAST, if_id,
				      CS_ENABLE_REG, cs_enable_reg_val,
				      MASK_ALL_BITS));
			/* enable single cs */
			CHECK_STATUS(ddr3_tip_if_write
				     (dev_num, ACCESS_TYPE_UNICAST, if_id,
				      CS_ENABLE_REG, (1 << 3), (1 << 3)));
		}

		ddr3_tip_reset_fifo_ptr(dev_num);

		/*
		 *     Phase 1: Load pattern (using ODPG)
		 *
		 * enter Read Leveling mode
		 * only 27 bits are masked
		 * assuming non multi-CS configuration
		 * write to CS = 0 for the non multi CS configuration, note
		 * that the results shall be read back to the required CS !!!
		 */

		/* BUS count is 0 shifted 26 */
		CHECK_STATUS(ddr3_tip_if_write
			     (dev_num, ACCESS_TYPE_MULTICAST, PARAM_NOT_CARE,
			      ODPG_DATA_CONTROL_REG, 0x3, 0x3));
		CHECK_STATUS(ddr3_tip_configure_odpg
			     (dev_num, ACCESS_TYPE_MULTICAST, PARAM_NOT_CARE, 0,
			      pattern_table[PATTERN_RL].num_of_phases_tx, 0,
			      pattern_table[PATTERN_RL].num_of_phases_rx, 0, 0,
			      effective_cs, STRESS_NONE, DURATION_SINGLE));

		/* load pattern to ODPG */
		ddr3_tip_load_pattern_to_odpg(dev_num, ACCESS_TYPE_MULTICAST,
					      PARAM_NOT_CARE, PATTERN_RL,
					      pattern_table[PATTERN_RL].
					      start_addr);

		/*
		 *     Phase 2: ODPG to Read Leveling mode
		 */

		/* General Training Opcode register */
		CHECK_STATUS(ddr3_tip_if_write
			     (dev_num, ACCESS_TYPE_MULTICAST, PARAM_NOT_CARE,
			      ODPG_WRITE_READ_MODE_ENABLE_REG, 0,
			      MASK_ALL_BITS));

		CHECK_STATUS(ddr3_tip_if_write
			     (dev_num, ACCESS_TYPE_MULTICAST, PARAM_NOT_CARE,
			      ODPG_TRAINING_CONTROL_REG,
			      (0x301b01 | effective_cs << 2), 0x3c3fef));

		/* Object1 opcode register 0 & 1 */
		for (if_id = 0; if_id <= MAX_INTERFACE_NUM - 1; if_id++) {
			VALIDATE_ACTIVE(tm->if_act_mask, if_id);
			speed_bin_index =
				tm->interface_params[if_id].speed_bin_index;
			cl_val =
				cas_latency_table[speed_bin_index].cl_val[freq];
			data = (cl_val << 17) | (0x3 << 25);
			mask = (0xff << 9) | (0x1f << 17) | (0x3 << 25);
			CHECK_STATUS(ddr3_tip_if_write
				     (dev_num, ACCESS_TYPE_UNICAST, if_id,
				      ODPG_OBJ1_OPCODE_REG, data, mask));
		}

		/* Set iteration count to max value */
		CHECK_STATUS(ddr3_tip_if_write
			     (dev_num, ACCESS_TYPE_MULTICAST, PARAM_NOT_CARE,
			      TRAINING_OPCODE_1_REG, 0xd00, 0xd00));

		/*
		 *     Phase 2: Mask config
		 */

		ddr3_tip_dynamic_read_leveling_seq(dev_num);

		/*
		 *     Phase 3: Read Leveling execution
		 */

		/* temporary jira dunit=14751 */
		CHECK_STATUS(ddr3_tip_if_write
			     (dev_num, ACCESS_TYPE_MULTICAST, PARAM_NOT_CARE,
			      TRAINING_DBG_1_REG, 0, (u32)(1 << 31)));
		/* configure phy reset value */
		CHECK_STATUS(ddr3_tip_if_write
			     (dev_num, ACCESS_TYPE_MULTICAST, PARAM_NOT_CARE,
			      TRAINING_DBG_3_REG, (0x7f << 24),
			      (u32)(0xff << 24)));
		/* data pup rd reset enable  */
		CHECK_STATUS(ddr3_tip_if_write
			     (dev_num, ACCESS_TYPE_MULTICAST, PARAM_NOT_CARE,
			      SDRAM_CONFIGURATION_REG, 0, (1 << 30)));
		/* data pup rd reset disable */
		CHECK_STATUS(ddr3_tip_if_write
			     (dev_num, ACCESS_TYPE_MULTICAST, PARAM_NOT_CARE,
			      SDRAM_CONFIGURATION_REG, (1 << 30), (1 << 30)));
		/* training SW override & training RL mode */
		CHECK_STATUS(ddr3_tip_if_write
			     (dev_num, ACCESS_TYPE_MULTICAST, PARAM_NOT_CARE,
			      TRAINING_SW_2_REG, 0x1, 0x9));
		/* training enable */
		CHECK_STATUS(ddr3_tip_if_write
			     (dev_num, ACCESS_TYPE_MULTICAST, PARAM_NOT_CARE,
			      TRAINING_REG, (1 << 24) | (1 << 20),
			      (1 << 24) | (1 << 20)));
		CHECK_STATUS(ddr3_tip_if_write
			     (dev_num, ACCESS_TYPE_MULTICAST, PARAM_NOT_CARE,
			      TRAINING_REG, (u32)(1 << 31), (u32)(1 << 31)));

		/********* trigger training *******************/
		/* Trigger, poll on status and disable ODPG */
		CHECK_STATUS(ddr3_tip_if_write
			     (dev_num, ACCESS_TYPE_MULTICAST, PARAM_NOT_CARE,
			      ODPG_TRAINING_TRIGGER_REG, 0x1, 0x1));
		CHECK_STATUS(ddr3_tip_if_write
			     (dev_num, ACCESS_TYPE_MULTICAST, PARAM_NOT_CARE,
			      ODPG_TRAINING_STATUS_REG, 0x1, 0x1));

		/* check for training done + results pass */
		if (ddr3_tip_if_polling
		    (dev_num, ACCESS_TYPE_MULTICAST, PARAM_NOT_CARE, 0x2, 0x2,
		     ODPG_TRAINING_STATUS_REG,
		     MAX_POLLING_ITERATIONS) != MV_OK) {
			DEBUG_LEVELING(DEBUG_LEVEL_ERROR,
				       ("Training Done Failed\n"));
			return MV_FAIL;
		}

		for (if_id = 0; if_id <= MAX_INTERFACE_NUM - 1; if_id++) {
			VALIDATE_ACTIVE(tm->if_act_mask, if_id);
			CHECK_STATUS(ddr3_tip_if_read
				     (dev_num, ACCESS_TYPE_UNICAST,
				      if_id,
				      ODPG_TRAINING_TRIGGER_REG, data_read,
				      0x4));
			data = data_read[if_id];
			if (data != 0x0) {
				DEBUG_LEVELING(DEBUG_LEVEL_ERROR,
					       ("Training Result Failed\n"));
			}
		}

		/*disable ODPG - Back to functional mode */
		CHECK_STATUS(ddr3_tip_if_write
			     (dev_num, ACCESS_TYPE_MULTICAST, PARAM_NOT_CARE,
			      ODPG_ENABLE_REG, 0x1 << ODPG_DISABLE_OFFS,
			      (0x1 << ODPG_DISABLE_OFFS)));
		if (ddr3_tip_if_polling
		    (dev_num, ACCESS_TYPE_MULTICAST, PARAM_NOT_CARE, 0x0, 0x1,
		     ODPG_ENABLE_REG, MAX_POLLING_ITERATIONS) != MV_OK) {
			DEBUG_LEVELING(DEBUG_LEVEL_ERROR,
				       ("ODPG disable failed "));
			return MV_FAIL;
		}
		CHECK_STATUS(ddr3_tip_if_write
			     (dev_num, ACCESS_TYPE_MULTICAST, PARAM_NOT_CARE,
			      ODPG_DATA_CONTROL_REG, 0, MASK_ALL_BITS));

		/* double loop on bus, pup */
		for (if_id = 0; if_id <= MAX_INTERFACE_NUM - 1; if_id++) {
			VALIDATE_ACTIVE(tm->if_act_mask, if_id);
			/* check training done */
			is_any_pup_fail = 0;
			for (bus_num = 0;
			     bus_num < tm->num_of_bus_per_interface;
			     bus_num++) {
				VALIDATE_ACTIVE(tm->bus_act_mask, bus_num);
				if (ddr3_tip_if_polling
				    (dev_num, ACCESS_TYPE_UNICAST,
				     if_id, (1 << 25), (1 << 25),
				     mask_results_pup_reg_map[bus_num],
				     MAX_POLLING_ITERATIONS) != MV_OK) {
					DEBUG_LEVELING(DEBUG_LEVEL_ERROR,
						       ("\n_r_l: DDR3 poll failed(2) for bus %d",
							bus_num));
					is_any_pup_fail = 1;
				} else {
					/* read result per pup */
					CHECK_STATUS(ddr3_tip_if_read
						     (dev_num,
						      ACCESS_TYPE_UNICAST,
						      if_id,
						      mask_results_pup_reg_map
						      [bus_num], data_read,
						      0xff));
					rl_values[effective_cs][bus_num]
						[if_id] = (u8)data_read[if_id];
				}
			}

			if (is_any_pup_fail == 1) {
				training_result[training_stage][if_id] =
					TEST_FAILED;
				if (debug_mode == 0)
					return MV_FAIL;
			}
		}

		DEBUG_LEVELING(DEBUG_LEVEL_INFO, ("RL exit read leveling\n"));

		/*
		 *     Phase 3: Exit Read Leveling
		 */

		CHECK_STATUS(ddr3_tip_if_write
			     (dev_num, ACCESS_TYPE_MULTICAST, PARAM_NOT_CARE,
			      TRAINING_SW_2_REG, (1 << 3), (1 << 3)));
		CHECK_STATUS(ddr3_tip_if_write
			     (dev_num, ACCESS_TYPE_MULTICAST, PARAM_NOT_CARE,
			      TRAINING_SW_1_REG, (1 << 16), (1 << 16)));
		/* set ODPG to functional */
		CHECK_STATUS(ddr3_tip_if_write
			     (dev_num, ACCESS_TYPE_MULTICAST, PARAM_NOT_CARE,
			      ODPG_DATA_CONTROL_REG, 0x0, MASK_ALL_BITS));

		/*
		 * Copy the result from the effective CS search to the
		 * real Functional CS
		 */
		/*ddr3_tip_write_cs_result(dev_num, RL_PHY_REG); */
		CHECK_STATUS(ddr3_tip_if_write
			     (dev_num, ACCESS_TYPE_MULTICAST, PARAM_NOT_CARE,
			      ODPG_DATA_CONTROL_REG, 0x0, MASK_ALL_BITS));
	}

	for (effective_cs = 0; effective_cs < max_cs; effective_cs++) {
		/* double loop on bus, pup */
		for (if_id = 0; if_id <= MAX_INTERFACE_NUM - 1; if_id++) {
			VALIDATE_ACTIVE(tm->if_act_mask, if_id);
			for (bus_num = 0;
			     bus_num < tm->num_of_bus_per_interface;
			     bus_num++) {
				VALIDATE_ACTIVE(tm->bus_act_mask, bus_num);
				/* read result per pup from arry */
				data = rl_values[effective_cs][bus_num][if_id];
				data = (data & 0x1f) |
					(((data & 0xe0) >> 5) << 6);
				ddr3_tip_bus_write(dev_num,
						   ACCESS_TYPE_UNICAST,
						   if_id,
						   ACCESS_TYPE_UNICAST,
						   bus_num, DDR_PHY_DATA,
						   RL_PHY_REG +
						   ((effective_cs ==
						     0) ? 0x0 : 0x4), data);
			}
		}
	}
	/* Set to 0 after each loop to avoid illegal value may be used */
	effective_cs = 0;

	for (if_id = 0; if_id <= MAX_INTERFACE_NUM - 1; if_id++) {
		VALIDATE_ACTIVE(tm->if_act_mask, if_id);
		/* restore cs enable value */
		CHECK_STATUS(ddr3_tip_if_write
			     (dev_num, ACCESS_TYPE_UNICAST, if_id,
			      CS_ENABLE_REG, cs_enable_reg_val[if_id],
			      MASK_ALL_BITS));
		if (odt_config != 0) {
			CHECK_STATUS(ddr3_tip_write_additional_odt_setting
				     (dev_num, if_id));
		}
	}

	for (if_id = 0; if_id <= MAX_INTERFACE_NUM - 1; if_id++) {
		VALIDATE_ACTIVE(tm->if_act_mask, if_id);
		if (training_result[training_stage][if_id] == TEST_FAILED)
			return MV_FAIL;
	}

	return MV_OK;
}

/*
 * Legacy Dynamic write leveling
 */
int ddr3_tip_legacy_dynamic_write_leveling(u32 dev_num)
{
	u32 c_cs, if_id, cs_mask = 0;
	u32 max_cs = hws_ddr3_tip_max_cs_get();
	struct hws_topology_map *tm = ddr3_get_topology_map();

	/*
	 * In TRAINIUNG reg (0x15b0) write 0x80000008 | cs_mask:
	 * Trn_start
	 * cs_mask = 0x1 <<20 Trn_CS0 - CS0 is included in the DDR3 training
	 * cs_mask = 0x1 <<21 Trn_CS1 - CS1 is included in the DDR3 training
	 * cs_mask = 0x1 <<22 Trn_CS2 - CS2 is included in the DDR3 training
	 * cs_mask = 0x1 <<23 Trn_CS3 - CS3 is included in the DDR3 training
	 * Trn_auto_seq =  write leveling
	 */
	for (c_cs = 0; c_cs < max_cs; c_cs++)
		cs_mask = cs_mask | 1 << (20 + c_cs);

	for (if_id = 0; if_id < MAX_INTERFACE_NUM; if_id++) {
		VALIDATE_ACTIVE(tm->if_act_mask, if_id);
		CHECK_STATUS(ddr3_tip_if_write
			     (dev_num, ACCESS_TYPE_MULTICAST, 0,
			      TRAINING_REG, (0x80000008 | cs_mask),
			      0xffffffff));
		mdelay(20);
		if (ddr3_tip_if_polling
		    (dev_num, ACCESS_TYPE_UNICAST, if_id, 0,
		     (u32)0x80000000, TRAINING_REG,
		     MAX_POLLING_ITERATIONS) != MV_OK) {
			DEBUG_LEVELING(DEBUG_LEVEL_ERROR,
				       ("polling failed for Old WL result\n"));
			return MV_FAIL;
		}
	}

	return MV_OK;
}

/*
 * Legacy Dynamic read leveling
 */
int ddr3_tip_legacy_dynamic_read_leveling(u32 dev_num)
{
	u32 c_cs, if_id, cs_mask = 0;
	u32 max_cs = hws_ddr3_tip_max_cs_get();
	struct hws_topology_map *tm = ddr3_get_topology_map();

	/*
	 * In TRAINIUNG reg (0x15b0) write 0x80000040 | cs_mask:
	 * Trn_start
	 * cs_mask = 0x1 <<20 Trn_CS0 - CS0 is included in the DDR3 training
	 * cs_mask = 0x1 <<21 Trn_CS1 - CS1 is included in the DDR3 training
	 * cs_mask = 0x1 <<22 Trn_CS2 - CS2 is included in the DDR3 training
	 * cs_mask = 0x1 <<23 Trn_CS3 - CS3 is included in the DDR3 training
	 * Trn_auto_seq =  Read Leveling using training pattern
	 */
	for (c_cs = 0; c_cs < max_cs; c_cs++)
		cs_mask = cs_mask | 1 << (20 + c_cs);

	CHECK_STATUS(ddr3_tip_if_write
		     (dev_num, ACCESS_TYPE_MULTICAST, 0, TRAINING_REG,
		      (0x80000040 | cs_mask), 0xffffffff));
	mdelay(100);

	for (if_id = 0; if_id < MAX_INTERFACE_NUM; if_id++) {
		VALIDATE_ACTIVE(tm->if_act_mask, if_id);
		if (ddr3_tip_if_polling
		    (dev_num, ACCESS_TYPE_UNICAST, if_id, 0,
		     (u32)0x80000000, TRAINING_REG,
		     MAX_POLLING_ITERATIONS) != MV_OK) {
			DEBUG_LEVELING(DEBUG_LEVEL_ERROR,
				       ("polling failed for Old RL result\n"));
			return MV_FAIL;
		}
	}

	return MV_OK;
}

/*
 * Dynamic per bit read leveling
 */
int ddr3_tip_dynamic_per_bit_read_leveling(u32 dev_num, u32 freq)
{
	u32 data, mask;
	u32 bus_num, if_id, cl_val, bit_num;
	u32 curr_numb, curr_min_delay;
	int adll_array[3] = { 0, -0xa, 0x14 };
	u32 phyreg3_arr[MAX_INTERFACE_NUM][MAX_BUS_NUM];
	enum hws_speed_bin speed_bin_index;
	int is_any_pup_fail = 0;
	int break_loop = 0;
	u32 cs_enable_reg_val[MAX_INTERFACE_NUM]; /* save current CS value */
	u32 data_read[MAX_INTERFACE_NUM];
	int per_bit_rl_pup_status[MAX_INTERFACE_NUM][MAX_BUS_NUM];
	u32 data2_write[MAX_INTERFACE_NUM][MAX_BUS_NUM];
	struct pattern_info *pattern_table = ddr3_tip_get_pattern_table();
	u16 *mask_results_dq_reg_map = ddr3_tip_get_mask_results_dq_reg();
	struct hws_topology_map *tm = ddr3_get_topology_map();

	for (if_id = 0; if_id < MAX_INTERFACE_NUM; if_id++) {
		VALIDATE_ACTIVE(tm->if_act_mask, if_id);
		for (bus_num = 0;
		     bus_num <= tm->num_of_bus_per_interface; bus_num++) {
			VALIDATE_ACTIVE(tm->bus_act_mask, bus_num);
			per_bit_rl_pup_status[if_id][bus_num] = 0;
			data2_write[if_id][bus_num] = 0;
			/* read current value of phy register 0x3 */
			CHECK_STATUS(ddr3_tip_bus_read
				     (dev_num, if_id, ACCESS_TYPE_UNICAST,
				      bus_num, DDR_PHY_DATA,
				      READ_CENTRALIZATION_PHY_REG,
				      &phyreg3_arr[if_id][bus_num]));
		}
	}

	/* NEW RL machine */
	for (if_id = 0; if_id <= MAX_INTERFACE_NUM - 1; if_id++) {
		VALIDATE_ACTIVE(tm->if_act_mask, if_id);
		training_result[training_stage][if_id] = TEST_SUCCESS;

		/* save current cs enable reg val */
		CHECK_STATUS(ddr3_tip_if_read
			     (dev_num, ACCESS_TYPE_UNICAST, if_id,
			      CS_ENABLE_REG, &cs_enable_reg_val[if_id],
			      MASK_ALL_BITS));
		/* enable single cs */
		CHECK_STATUS(ddr3_tip_if_write
			     (dev_num, ACCESS_TYPE_UNICAST, if_id,
			      CS_ENABLE_REG, (1 << 3), (1 << 3)));
	}

	ddr3_tip_reset_fifo_ptr(dev_num);
	for (curr_numb = 0; curr_numb < 3; curr_numb++) {
		/*
		 *     Phase 1: Load pattern (using ODPG)
		 *
		 * enter Read Leveling mode
		 * only 27 bits are masked
		 * assuming non multi-CS configuration
		 * write to CS = 0 for the non multi CS configuration, note that
		 * the results shall be read back to the required CS !!!
		 */

		/* BUS count is 0 shifted 26 */
		CHECK_STATUS(ddr3_tip_if_write
			     (dev_num, ACCESS_TYPE_MULTICAST, PARAM_NOT_CARE,
			      ODPG_DATA_CONTROL_REG, 0x3, 0x3));
		CHECK_STATUS(ddr3_tip_configure_odpg
			     (dev_num, ACCESS_TYPE_MULTICAST, PARAM_NOT_CARE, 0,
			      pattern_table[PATTERN_TEST].num_of_phases_tx, 0,
			      pattern_table[PATTERN_TEST].num_of_phases_rx, 0,
			      0, 0, STRESS_NONE, DURATION_SINGLE));

		/* load pattern to ODPG */
		ddr3_tip_load_pattern_to_odpg(dev_num, ACCESS_TYPE_MULTICAST,
					      PARAM_NOT_CARE, PATTERN_TEST,
					      pattern_table[PATTERN_TEST].
					      start_addr);

		/*
		 *     Phase 2: ODPG to Read Leveling mode
		 */

		/* General Training Opcode register */
		CHECK_STATUS(ddr3_tip_if_write
			     (dev_num, ACCESS_TYPE_MULTICAST, PARAM_NOT_CARE,
			      ODPG_WRITE_READ_MODE_ENABLE_REG, 0,
			      MASK_ALL_BITS));
		CHECK_STATUS(ddr3_tip_if_write
			     (dev_num, ACCESS_TYPE_MULTICAST, PARAM_NOT_CARE,
			      ODPG_TRAINING_CONTROL_REG, 0x301b01, 0x3c3fef));

		/* Object1 opcode register 0 & 1 */
		for (if_id = 0; if_id <= MAX_INTERFACE_NUM - 1; if_id++) {
			VALIDATE_ACTIVE(tm->if_act_mask, if_id);
			speed_bin_index =
				tm->interface_params[if_id].speed_bin_index;
			cl_val =
				cas_latency_table[speed_bin_index].cl_val[freq];
			data = (cl_val << 17) | (0x3 << 25);
			mask = (0xff << 9) | (0x1f << 17) | (0x3 << 25);
			CHECK_STATUS(ddr3_tip_if_write
				     (dev_num, ACCESS_TYPE_UNICAST, if_id,
				      ODPG_OBJ1_OPCODE_REG, data, mask));
		}

		/* Set iteration count to max value */
		CHECK_STATUS(ddr3_tip_if_write
			     (dev_num, ACCESS_TYPE_MULTICAST, PARAM_NOT_CARE,
			      TRAINING_OPCODE_1_REG, 0xd00, 0xd00));

		/*
		 *     Phase 2: Mask config
		 */

		ddr3_tip_dynamic_per_bit_read_leveling_seq(dev_num);

		/*
		 *     Phase 3: Read Leveling execution
		 */

		/* temporary jira dunit=14751 */
		CHECK_STATUS(ddr3_tip_if_write
			     (dev_num, ACCESS_TYPE_MULTICAST, PARAM_NOT_CARE,
			      TRAINING_DBG_1_REG, 0, (u32)(1 << 31)));
		/* configure phy reset value */
		CHECK_STATUS(ddr3_tip_if_write
			     (dev_num, ACCESS_TYPE_MULTICAST, PARAM_NOT_CARE,
			      TRAINING_DBG_3_REG, (0x7f << 24),
			      (u32)(0xff << 24)));
		/* data pup rd reset enable  */
		CHECK_STATUS(ddr3_tip_if_write
			     (dev_num, ACCESS_TYPE_MULTICAST, PARAM_NOT_CARE,
			      SDRAM_CONFIGURATION_REG, 0, (1 << 30)));
		/* data pup rd reset disable */
		CHECK_STATUS(ddr3_tip_if_write
			     (dev_num, ACCESS_TYPE_MULTICAST, PARAM_NOT_CARE,
			      SDRAM_CONFIGURATION_REG, (1 << 30), (1 << 30)));
		/* training SW override & training RL mode */
		CHECK_STATUS(ddr3_tip_if_write
			     (dev_num, ACCESS_TYPE_MULTICAST, PARAM_NOT_CARE,
			      TRAINING_SW_2_REG, 0x1, 0x9));
		/* training enable */
		CHECK_STATUS(ddr3_tip_if_write
			     (dev_num, ACCESS_TYPE_MULTICAST, PARAM_NOT_CARE,
			      TRAINING_REG, (1 << 24) | (1 << 20),
			      (1 << 24) | (1 << 20)));
		CHECK_STATUS(ddr3_tip_if_write
			     (dev_num, ACCESS_TYPE_MULTICAST, PARAM_NOT_CARE,
			      TRAINING_REG, (u32)(1 << 31), (u32)(1 << 31)));

		/********* trigger training *******************/
		/* Trigger, poll on status and disable ODPG */
		CHECK_STATUS(ddr3_tip_if_write
			     (dev_num, ACCESS_TYPE_MULTICAST, PARAM_NOT_CARE,
			      ODPG_TRAINING_TRIGGER_REG, 0x1, 0x1));
		CHECK_STATUS(ddr3_tip_if_write
			     (dev_num, ACCESS_TYPE_MULTICAST, PARAM_NOT_CARE,
			      ODPG_TRAINING_STATUS_REG, 0x1, 0x1));

		/*check for training done + results pass */
		if (ddr3_tip_if_polling
		    (dev_num, ACCESS_TYPE_MULTICAST, PARAM_NOT_CARE, 0x2, 0x2,
		     ODPG_TRAINING_STATUS_REG,
		     MAX_POLLING_ITERATIONS) != MV_OK) {
			DEBUG_LEVELING(DEBUG_LEVEL_ERROR,
				       ("Training Done Failed\n"));
			return MV_FAIL;
		}

		for (if_id = 0; if_id <= MAX_INTERFACE_NUM - 1; if_id++) {
			VALIDATE_ACTIVE(tm->if_act_mask, if_id);
			CHECK_STATUS(ddr3_tip_if_read
				     (dev_num, ACCESS_TYPE_UNICAST,
				      if_id,
				      ODPG_TRAINING_TRIGGER_REG, data_read,
				      0x4));
			data = data_read[if_id];
			if (data != 0x0) {
				DEBUG_LEVELING(DEBUG_LEVEL_ERROR,
					       ("Training Result Failed\n"));
			}
		}

		/*disable ODPG - Back to functional mode */
		CHECK_STATUS(ddr3_tip_if_write
			     (dev_num, ACCESS_TYPE_MULTICAST, PARAM_NOT_CARE,
			      ODPG_ENABLE_REG, 0x1 << ODPG_DISABLE_OFFS,
			      (0x1 << ODPG_DISABLE_OFFS)));
		if (ddr3_tip_if_polling
		    (dev_num, ACCESS_TYPE_MULTICAST, PARAM_NOT_CARE, 0x0, 0x1,
		     ODPG_ENABLE_REG, MAX_POLLING_ITERATIONS) != MV_OK) {
			DEBUG_LEVELING(DEBUG_LEVEL_ERROR,
				       ("ODPG disable failed "));
			return MV_FAIL;
		}
		CHECK_STATUS(ddr3_tip_if_write
			     (dev_num, ACCESS_TYPE_MULTICAST, PARAM_NOT_CARE,
			      ODPG_DATA_CONTROL_REG, 0, MASK_ALL_BITS));

		/* double loop on bus, pup */
		for (if_id = 0; if_id <= MAX_INTERFACE_NUM - 1; if_id++) {
			VALIDATE_ACTIVE(tm->if_act_mask, if_id);
			/* check training done */
			for (bus_num = 0;
			     bus_num < tm->num_of_bus_per_interface;
			     bus_num++) {
				VALIDATE_ACTIVE(tm->bus_act_mask, bus_num);

				if (per_bit_rl_pup_status[if_id][bus_num]
				    == 0) {
					curr_min_delay = 0;
					for (bit_num = 0; bit_num < 8;
					     bit_num++) {
						if (ddr3_tip_if_polling
						    (dev_num,
						     ACCESS_TYPE_UNICAST,
						     if_id, (1 << 25),
						     (1 << 25),
						     mask_results_dq_reg_map
						     [bus_num * 8 + bit_num],
						     MAX_POLLING_ITERATIONS) !=
						    MV_OK) {
							DEBUG_LEVELING
								(DEBUG_LEVEL_ERROR,
								 ("\n_r_l: DDR3 poll failed(2) for bus %d bit %d\n",
								  bus_num,
								  bit_num));
						} else {
							/* read result per pup */
							CHECK_STATUS
								(ddr3_tip_if_read
								 (dev_num,
								  ACCESS_TYPE_UNICAST,
								  if_id,
								  mask_results_dq_reg_map
								  [bus_num * 8 +
								   bit_num],
								  data_read,
								  MASK_ALL_BITS));
							data =
								(data_read
								 [if_id] &
								 0x1f) |
								((data_read
								  [if_id] &
								  0xe0) << 1);
							if (curr_min_delay == 0)
								curr_min_delay =
									data;
							else if (data <
								 curr_min_delay)
								curr_min_delay =
									data;
							if (data > data2_write[if_id][bus_num])
								data2_write
									[if_id]
									[bus_num] =
									data;
						}
					}

					if (data2_write[if_id][bus_num] <=
					    (curr_min_delay +
					     MAX_DQ_READ_LEVELING_DELAY)) {
						per_bit_rl_pup_status[if_id]
							[bus_num] = 1;
					}
				}
			}
		}

		/* check if there is need to search new phyreg3 value */
		if (curr_numb < 2) {
			/* if there is DLL that is not checked yet */
			for (if_id = 0; if_id <= MAX_INTERFACE_NUM - 1;
			     if_id++) {
				VALIDATE_ACTIVE(tm->if_act_mask, if_id);
				for (bus_num = 0;
				     bus_num < tm->num_of_bus_per_interface;
				     bus_num++) {
					VALIDATE_ACTIVE(tm->bus_act_mask,
							bus_num);
					if (per_bit_rl_pup_status[if_id]
					    [bus_num] != 1) {
						/* go to next ADLL value */
						CHECK_STATUS
							(ddr3_tip_bus_write
							 (dev_num,
							  ACCESS_TYPE_UNICAST,
							  if_id,
							  ACCESS_TYPE_UNICAST,
							  bus_num, DDR_PHY_DATA,
							  READ_CENTRALIZATION_PHY_REG,
							  (phyreg3_arr[if_id]
							   [bus_num] +
							   adll_array[curr_numb])));
						break_loop = 1;
						break;
					}
				}
				if (break_loop)
					break;
			}
		}		/* if (curr_numb < 2) */
		if (!break_loop)
			break;
	}		/* for ( curr_numb = 0; curr_numb <3; curr_numb++) */

	for (if_id = 0; if_id <= MAX_INTERFACE_NUM - 1; if_id++) {
		VALIDATE_ACTIVE(tm->if_act_mask, if_id);
		for (bus_num = 0; bus_num < tm->num_of_bus_per_interface;
		     bus_num++) {
			VALIDATE_ACTIVE(tm->bus_act_mask, bus_num);
			if (per_bit_rl_pup_status[if_id][bus_num] == 1)
				ddr3_tip_bus_write(dev_num,
						   ACCESS_TYPE_UNICAST,
						   if_id,
						   ACCESS_TYPE_UNICAST,
						   bus_num, DDR_PHY_DATA,
						   RL_PHY_REG +
						   CS_REG_VALUE(effective_cs),
						   data2_write[if_id]
						   [bus_num]);
			else
				is_any_pup_fail = 1;
		}

		/* TBD flow does not support multi CS */
		/*
		 * cs_bitmask = tm->interface_params[if_id].
		 * as_bus_params[bus_num].cs_bitmask;
		 */
		/* divide by 4 is used for retrieving the CS number */
		/*
		 * TBD BC2 - what is the PHY address for other
		 * CS ddr3_tip_write_cs_result() ???
		 */
		/*
		 * find what should be written to PHY
		 * - max delay that is less than threshold
		 */
		if (is_any_pup_fail == 1) {
			training_result[training_stage][if_id] = TEST_FAILED;
			if (debug_mode == 0)
				return MV_FAIL;
		}
	}
	DEBUG_LEVELING(DEBUG_LEVEL_INFO, ("RL exit read leveling\n"));

	/*
	 *     Phase 3: Exit Read Leveling
	 */

	CHECK_STATUS(ddr3_tip_if_write
		     (dev_num, ACCESS_TYPE_MULTICAST, PARAM_NOT_CARE,
		      TRAINING_SW_2_REG, (1 << 3), (1 << 3)));
	CHECK_STATUS(ddr3_tip_if_write
		     (dev_num, ACCESS_TYPE_MULTICAST, PARAM_NOT_CARE,
		      TRAINING_SW_1_REG, (1 << 16), (1 << 16)));
	/* set ODPG to functional */
	CHECK_STATUS(ddr3_tip_if_write
		     (dev_num, ACCESS_TYPE_MULTICAST, PARAM_NOT_CARE,
		      ODPG_DATA_CONTROL_REG, 0x0, MASK_ALL_BITS));
	/*
	 * Copy the result from the effective CS search to the real
	 * Functional CS
	 */
	ddr3_tip_write_cs_result(dev_num, RL_PHY_REG);
	CHECK_STATUS(ddr3_tip_if_write
		     (dev_num, ACCESS_TYPE_MULTICAST, PARAM_NOT_CARE,
		      ODPG_DATA_CONTROL_REG, 0x0, MASK_ALL_BITS));

	for (if_id = 0; if_id <= MAX_INTERFACE_NUM - 1; if_id++) {
		VALIDATE_ACTIVE(tm->if_act_mask, if_id);
		/* restore cs enable value */
		CHECK_STATUS(ddr3_tip_if_write
			     (dev_num, ACCESS_TYPE_UNICAST, if_id,
			      CS_ENABLE_REG, cs_enable_reg_val[if_id],
			      MASK_ALL_BITS));
		if (odt_config != 0) {
			CHECK_STATUS(ddr3_tip_write_additional_odt_setting
				     (dev_num, if_id));
		}
	}

	for (if_id = 0; if_id <= MAX_INTERFACE_NUM - 1; if_id++) {
		VALIDATE_ACTIVE(tm->if_act_mask, if_id);
		if (training_result[training_stage][if_id] == TEST_FAILED)
			return MV_FAIL;
	}

	return MV_OK;
}

int ddr3_tip_calc_cs_mask(u32 dev_num, u32 if_id, u32 effective_cs,
			  u32 *cs_mask)
{
	u32 all_bus_cs = 0, same_bus_cs;
	u32 bus_cnt;
	struct hws_topology_map *tm = ddr3_get_topology_map();

	*cs_mask = same_bus_cs = CS_BIT_MASK;

	/*
	 * In some of the devices (such as BC2), the CS is per pup and there
	 * for mixed mode is valid on like other devices where CS configuration
	 * is per interface.
	 * In order to know that, we do 'Or' and 'And' operation between all
	 * CS (of the pups).
	 * If they are they are not the same then it's mixed mode so all CS
	 * should be configured (when configuring the MRS)
	 */
	for (bus_cnt = 0; bus_cnt < tm->num_of_bus_per_interface; bus_cnt++) {
		VALIDATE_ACTIVE(tm->bus_act_mask, bus_cnt);

		all_bus_cs |= tm->interface_params[if_id].
			as_bus_params[bus_cnt].cs_bitmask;
		same_bus_cs &= tm->interface_params[if_id].
			as_bus_params[bus_cnt].cs_bitmask;

		/* cs enable is active low */
		*cs_mask &= ~tm->interface_params[if_id].
			as_bus_params[bus_cnt].cs_bitmask;
	}

	if (all_bus_cs == same_bus_cs)
		*cs_mask = (*cs_mask | (~(1 << effective_cs))) & CS_BIT_MASK;

	return MV_OK;
}

/*
 * Dynamic write leveling
 */
int ddr3_tip_dynamic_write_leveling(u32 dev_num)
{
	u32 reg_data = 0, iter, if_id, bus_cnt;
	u32 cs_enable_reg_val[MAX_INTERFACE_NUM] = { 0 };
	u32 cs_mask[MAX_INTERFACE_NUM];
	u32 read_data_sample_delay_vals[MAX_INTERFACE_NUM] = { 0 };
	u32 read_data_ready_delay_vals[MAX_INTERFACE_NUM] = { 0 };
	/* 0 for failure */
	u32 res_values[MAX_INTERFACE_NUM * MAX_BUS_NUM] = { 0 };
	u32 test_res = 0;	/* 0 - success for all pup */
	u32 data_read[MAX_INTERFACE_NUM];
	u8 wl_values[NUM_OF_CS][MAX_BUS_NUM][MAX_INTERFACE_NUM];
	u16 *mask_results_pup_reg_map = ddr3_tip_get_mask_results_pup_reg_map();
	u32 cs_mask0[MAX_INTERFACE_NUM] = { 0 };
	u32 max_cs = hws_ddr3_tip_max_cs_get();
	struct hws_topology_map *tm = ddr3_get_topology_map();

	for (if_id = 0; if_id <= MAX_INTERFACE_NUM - 1; if_id++) {
		VALIDATE_ACTIVE(tm->if_act_mask, if_id);

		training_result[training_stage][if_id] = TEST_SUCCESS;

		/* save Read Data Sample Delay */
		CHECK_STATUS(ddr3_tip_if_read
			     (dev_num, ACCESS_TYPE_UNICAST, if_id,
			      READ_DATA_SAMPLE_DELAY,
			      read_data_sample_delay_vals, MASK_ALL_BITS));
		/* save Read Data Ready Delay */
		CHECK_STATUS(ddr3_tip_if_read
			     (dev_num, ACCESS_TYPE_UNICAST, if_id,
			      READ_DATA_READY_DELAY, read_data_ready_delay_vals,
			      MASK_ALL_BITS));
		/* save current cs reg val */
		CHECK_STATUS(ddr3_tip_if_read
			     (dev_num, ACCESS_TYPE_UNICAST, if_id,
			      CS_ENABLE_REG, cs_enable_reg_val, MASK_ALL_BITS));
	}

	/*
	 *     Phase 1: DRAM 2 Write Leveling mode
	 */

	/*Assert 10 refresh commands to DRAM to all CS */
	for (iter = 0; iter < WL_ITERATION_NUM; iter++) {
		for (if_id = 0; if_id <= MAX_INTERFACE_NUM - 1; if_id++) {
			VALIDATE_ACTIVE(tm->if_act_mask, if_id);
			CHECK_STATUS(ddr3_tip_if_write
				     (dev_num, ACCESS_TYPE_UNICAST,
				      if_id, SDRAM_OPERATION_REG,
				      (u32)((~(0xf) << 8) | 0x2), 0xf1f));
		}
	}
	/* check controller back to normal */
	for (if_id = 0; if_id <= MAX_INTERFACE_NUM - 1; if_id++) {
		VALIDATE_ACTIVE(tm->if_act_mask, if_id);
		if (ddr3_tip_if_polling
		    (dev_num, ACCESS_TYPE_UNICAST, if_id, 0, 0x1f,
		     SDRAM_OPERATION_REG, MAX_POLLING_ITERATIONS) != MV_OK) {
			DEBUG_LEVELING(DEBUG_LEVEL_ERROR,
				       ("WL: DDR3 poll failed(3)"));
		}
	}

	for (effective_cs = 0; effective_cs < max_cs; effective_cs++) {
		/*enable write leveling to all cs  - Q off , WL n */
		/* calculate interface cs mask */
		CHECK_STATUS(ddr3_tip_write_mrs_cmd(dev_num, cs_mask0, MRS1_CMD,
						    0x1000, 0x1080));

		for (if_id = 0; if_id <= MAX_INTERFACE_NUM - 1; if_id++) {
			VALIDATE_ACTIVE(tm->if_act_mask, if_id);
			/* cs enable is active low */
			ddr3_tip_calc_cs_mask(dev_num, if_id, effective_cs,
					      &cs_mask[if_id]);
		}

		/* Enable Output buffer to relevant CS - Q on , WL on */
		CHECK_STATUS(ddr3_tip_write_mrs_cmd
			     (dev_num, cs_mask, MRS1_CMD, 0x80, 0x1080));

		/*enable odt for relevant CS */
		CHECK_STATUS(ddr3_tip_if_write
			     (dev_num, ACCESS_TYPE_MULTICAST, PARAM_NOT_CARE,
			      0x1498, (0x3 << (effective_cs * 2)), 0xf));

		/*
		 *     Phase 2: Set training IP to write leveling mode
		 */

		CHECK_STATUS(ddr3_tip_dynamic_write_leveling_seq(dev_num));

		/*
		 *     Phase 3: Trigger training
		 */

		CHECK_STATUS(ddr3_tip_if_write
			     (dev_num, ACCESS_TYPE_MULTICAST, PARAM_NOT_CARE,
			      ODPG_TRAINING_TRIGGER_REG, 0x1, 0x1));

		for (if_id = 0; if_id < MAX_INTERFACE_NUM; if_id++) {
			VALIDATE_ACTIVE(tm->if_act_mask, if_id);

			/* training done */
			if (ddr3_tip_if_polling
			    (dev_num, ACCESS_TYPE_UNICAST, if_id,
			     (1 << 1), (1 << 1), ODPG_TRAINING_STATUS_REG,
			     MAX_POLLING_ITERATIONS) != MV_OK) {
				DEBUG_LEVELING(
					DEBUG_LEVEL_ERROR,
					("WL: DDR3 poll (4) failed (Data: 0x%x)\n",
					 reg_data));
			}
#if !defined(CONFIG_ARMADA_38X)	/*Disabled. JIRA #1498 */
			else {
				CHECK_STATUS(ddr3_tip_if_read
					     (dev_num, ACCESS_TYPE_UNICAST,
					      if_id,
					      ODPG_TRAINING_TRIGGER_REG,
					      &reg_data, (1 << 2)));
				if (reg_data != 0) {
					DEBUG_LEVELING(
						DEBUG_LEVEL_ERROR,
						("WL: WL failed IF %d reg_data=0x%x\n",
						 if_id, reg_data));
				}
			}
#endif
		}

		for (if_id = 0; if_id <= MAX_INTERFACE_NUM - 1; if_id++) {
			VALIDATE_ACTIVE(tm->if_act_mask, if_id);
			/* training done */
			if (ddr3_tip_if_polling
			    (dev_num, ACCESS_TYPE_UNICAST, if_id,
			     (1 << 1), (1 << 1), ODPG_TRAINING_STATUS_REG,
			     MAX_POLLING_ITERATIONS) != MV_OK) {
				DEBUG_LEVELING(
					DEBUG_LEVEL_ERROR,
					("WL: DDR3 poll (4) failed (Data: 0x%x)\n",
					 reg_data));
			} else {
#if !defined(CONFIG_ARMADA_38X)	/*Disabled. JIRA #1498 */
				CHECK_STATUS(ddr3_tip_if_read
					     (dev_num, ACCESS_TYPE_UNICAST,
					      if_id,
					      ODPG_TRAINING_STATUS_REG,
					      data_read, (1 << 2)));
				reg_data = data_read[if_id];
				if (reg_data != 0) {
					DEBUG_LEVELING(
						DEBUG_LEVEL_ERROR,
						("WL: WL failed IF %d reg_data=0x%x\n",
						 if_id, reg_data));
				}
#endif

				/* check for training completion per bus */
				for (bus_cnt = 0;
				     bus_cnt < tm->num_of_bus_per_interface;
				     bus_cnt++) {
					VALIDATE_ACTIVE(tm->bus_act_mask,
							bus_cnt);
					/* training status */
					CHECK_STATUS(ddr3_tip_if_read
						     (dev_num,
						      ACCESS_TYPE_UNICAST,
						      if_id,
						      mask_results_pup_reg_map
						      [bus_cnt], data_read,
						      (1 << 25)));
					reg_data = data_read[if_id];
					DEBUG_LEVELING(
						DEBUG_LEVEL_TRACE,
						("WL: IF %d BUS %d reg 0x%x\n",
						 if_id, bus_cnt, reg_data));
					if (reg_data == 0) {
						res_values[
							(if_id *
							 tm->num_of_bus_per_interface)
							+ bus_cnt] = 1;
					}
					CHECK_STATUS(ddr3_tip_if_read
						     (dev_num,
						      ACCESS_TYPE_UNICAST,
						      if_id,
						      mask_results_pup_reg_map
						      [bus_cnt], data_read,
						      0xff));
					/*
					 * Save the read value that should be
					 * write to PHY register
					 */
					wl_values[effective_cs]
						[bus_cnt][if_id] =
						(u8)data_read[if_id];
				}
			}
		}

		/*
		 *     Phase 4: Exit write leveling mode
		 */

		/* disable DQs toggling */
		CHECK_STATUS(ddr3_tip_if_write
			     (dev_num, ACCESS_TYPE_MULTICAST, PARAM_NOT_CARE,
			      WR_LEVELING_DQS_PATTERN_REG, 0x0, 0x1));

		/* Update MRS 1 (WL off) */
		CHECK_STATUS(ddr3_tip_write_mrs_cmd(dev_num, cs_mask0, MRS1_CMD,
						    0x1000, 0x1080));

		/* Update MRS 1 (return to functional mode - Q on , WL off) */
		CHECK_STATUS(ddr3_tip_write_mrs_cmd
			     (dev_num, cs_mask0, MRS1_CMD, 0x0, 0x1080));

		/* set phy to normal mode */
		CHECK_STATUS(ddr3_tip_if_write
			     (dev_num, ACCESS_TYPE_MULTICAST, PARAM_NOT_CARE,
			      TRAINING_SW_2_REG, 0x5, 0x7));

		/* exit sw override mode  */
		CHECK_STATUS(ddr3_tip_if_write
			     (dev_num, ACCESS_TYPE_MULTICAST, PARAM_NOT_CARE,
			      TRAINING_SW_2_REG, 0x4, 0x7));
	}

	/*
	 *     Phase 5: Load WL values to each PHY
	 */

	for (effective_cs = 0; effective_cs < max_cs; effective_cs++) {
		for (if_id = 0; if_id <= MAX_INTERFACE_NUM - 1; if_id++) {
			VALIDATE_ACTIVE(tm->if_act_mask, if_id);
			test_res = 0;
			for (bus_cnt = 0;
			     bus_cnt < tm->num_of_bus_per_interface;
			     bus_cnt++) {
				VALIDATE_ACTIVE(tm->bus_act_mask, bus_cnt);
				/* check if result == pass */
				if (res_values
				    [(if_id *
				      tm->num_of_bus_per_interface) +
				     bus_cnt] == 0) {
					/*
					 * read result control register
					 * according to pup
					 */
					reg_data =
						wl_values[effective_cs][bus_cnt]
						[if_id];
					/*
					 * Write into write leveling register
					 * ([4:0] ADLL, [8:6] Phase, [15:10]
					 * (centralization) ADLL + 0x10)
					 */
					reg_data =
						(reg_data & 0x1f) |
						(((reg_data & 0xe0) >> 5) << 6) |
						(((reg_data & 0x1f) +
						  phy_reg1_val) << 10);
					ddr3_tip_bus_write(
						dev_num,
						ACCESS_TYPE_UNICAST,
						if_id,
						ACCESS_TYPE_UNICAST,
						bus_cnt,
						DDR_PHY_DATA,
						WL_PHY_REG +
						effective_cs *
						CS_REGISTER_ADDR_OFFSET,
						reg_data);
				} else {
					test_res = 1;
					/*
					 * read result control register
					 * according to pup
					 */
					CHECK_STATUS(ddr3_tip_if_read
						     (dev_num,
						      ACCESS_TYPE_UNICAST,
						      if_id,
						      mask_results_pup_reg_map
						      [bus_cnt], data_read,
						      0xff));
					reg_data = data_read[if_id];
					DEBUG_LEVELING(
						DEBUG_LEVEL_ERROR,
						("WL: IF %d BUS %d failed, reg 0x%x\n",
						 if_id, bus_cnt, reg_data));
				}
			}

			if (test_res != 0) {
				training_result[training_stage][if_id] =
					TEST_FAILED;
			}
		}
	}
	/* Set to 0 after each loop to avoid illegal value may be used */
	effective_cs = 0;

	/*
	 * Copy the result from the effective CS search to the real
	 * Functional CS
	 */
	/* ddr3_tip_write_cs_result(dev_num, WL_PHY_REG); */
	/* restore saved values */
	for (if_id = 0; if_id <= MAX_INTERFACE_NUM - 1; if_id++) {
		VALIDATE_ACTIVE(tm->if_act_mask, if_id);
		/* restore Read Data Sample Delay */
		CHECK_STATUS(ddr3_tip_if_write
			     (dev_num, ACCESS_TYPE_UNICAST, if_id,
			      READ_DATA_SAMPLE_DELAY,
			      read_data_sample_delay_vals[if_id],
			      MASK_ALL_BITS));

		/* restore Read Data Ready Delay */
		CHECK_STATUS(ddr3_tip_if_write
			     (dev_num, ACCESS_TYPE_UNICAST, if_id,
			      READ_DATA_READY_DELAY,
			      read_data_ready_delay_vals[if_id],
			      MASK_ALL_BITS));

		/* enable multi cs */
		CHECK_STATUS(ddr3_tip_if_write
			     (dev_num, ACCESS_TYPE_UNICAST, if_id,
			      CS_ENABLE_REG, cs_enable_reg_val[if_id],
			      MASK_ALL_BITS));
	}

	/* Disable modt0 for CS0 training - need to adjust for multy CS */
	CHECK_STATUS(ddr3_tip_if_write
		     (dev_num, ACCESS_TYPE_MULTICAST, PARAM_NOT_CARE, 0x1498,
		      0x0, 0xf));

	for (if_id = 0; if_id <= MAX_INTERFACE_NUM - 1; if_id++) {
		VALIDATE_ACTIVE(tm->if_act_mask, if_id);
		if (training_result[training_stage][if_id] == TEST_FAILED)
			return MV_FAIL;
	}

	return MV_OK;
}

/*
 * Dynamic write leveling supplementary
 */
int ddr3_tip_dynamic_write_leveling_supp(u32 dev_num)
{
	int adll_offset;
	u32 if_id, bus_id, data, data_tmp;
	int is_if_fail = 0;
	struct hws_topology_map *tm = ddr3_get_topology_map();

	for (if_id = 0; if_id <= MAX_INTERFACE_NUM - 1; if_id++) {
		VALIDATE_ACTIVE(tm->if_act_mask, if_id);
		is_if_fail = 0;

		for (bus_id = 0; bus_id < GET_TOPOLOGY_NUM_OF_BUSES();
		     bus_id++) {
			VALIDATE_ACTIVE(tm->bus_act_mask, bus_id);
			wr_supp_res[if_id][bus_id].is_pup_fail = 1;
			CHECK_STATUS(ddr3_tip_bus_read
				     (dev_num, if_id, ACCESS_TYPE_UNICAST,
				      bus_id, DDR_PHY_DATA,
				      WRITE_CENTRALIZATION_PHY_REG +
				      effective_cs * CS_REGISTER_ADDR_OFFSET,
				      &data));
			DEBUG_LEVELING(
				DEBUG_LEVEL_TRACE,
				("WL Supp: adll_offset=0 data delay = %d\n",
				 data));
			if (ddr3_tip_wl_supp_align_phase_shift
			    (dev_num, if_id, bus_id, 0, 0) == MV_OK) {
				DEBUG_LEVELING(
					DEBUG_LEVEL_TRACE,
					("WL Supp: IF %d bus_id %d adll_offset=0 Success !\n",
					 if_id, bus_id));
				continue;
			}

			/* change adll */
			adll_offset = 5;
			CHECK_STATUS(ddr3_tip_bus_write
				     (dev_num, ACCESS_TYPE_UNICAST, if_id,
				      ACCESS_TYPE_UNICAST, bus_id, DDR_PHY_DATA,
				      WRITE_CENTRALIZATION_PHY_REG +
				      effective_cs * CS_REGISTER_ADDR_OFFSET,
				      data + adll_offset));
			CHECK_STATUS(ddr3_tip_bus_read
				     (dev_num, if_id, ACCESS_TYPE_UNICAST,
				      bus_id, DDR_PHY_DATA,
				      WRITE_CENTRALIZATION_PHY_REG +
				      effective_cs * CS_REGISTER_ADDR_OFFSET,
				      &data_tmp));
			DEBUG_LEVELING(
				DEBUG_LEVEL_TRACE,
				("WL Supp: adll_offset= %d data delay = %d\n",
				 adll_offset, data_tmp));

			if (ddr3_tip_wl_supp_align_phase_shift
			    (dev_num, if_id, bus_id, adll_offset, 0) == MV_OK) {
				DEBUG_LEVELING(
					DEBUG_LEVEL_TRACE,
					("WL Supp: IF %d bus_id %d adll_offset= %d Success !\n",
					 if_id, bus_id, adll_offset));
				continue;
			}

			/* change adll */
			adll_offset = -5;
			CHECK_STATUS(ddr3_tip_bus_write
				     (dev_num, ACCESS_TYPE_UNICAST, if_id,
				      ACCESS_TYPE_UNICAST, bus_id, DDR_PHY_DATA,
				      WRITE_CENTRALIZATION_PHY_REG +
				      effective_cs * CS_REGISTER_ADDR_OFFSET,
				      data + adll_offset));
			CHECK_STATUS(ddr3_tip_bus_read
				     (dev_num, if_id, ACCESS_TYPE_UNICAST,
				      bus_id, DDR_PHY_DATA,
				      WRITE_CENTRALIZATION_PHY_REG +
				      effective_cs * CS_REGISTER_ADDR_OFFSET,
				      &data_tmp));
			DEBUG_LEVELING(
				DEBUG_LEVEL_TRACE,
				("WL Supp: adll_offset= %d data delay = %d\n",
				 adll_offset, data_tmp));
			if (ddr3_tip_wl_supp_align_phase_shift
			    (dev_num, if_id, bus_id, adll_offset, 0) == MV_OK) {
				DEBUG_LEVELING(
					DEBUG_LEVEL_TRACE,
					("WL Supp: IF %d bus_id %d adll_offset= %d Success !\n",
					 if_id, bus_id, adll_offset));
				continue;
			} else {
				DEBUG_LEVELING(
					DEBUG_LEVEL_ERROR,
					("WL Supp: IF %d bus_id %d Failed !\n",
					 if_id, bus_id));
				is_if_fail = 1;
			}
		}
		DEBUG_LEVELING(DEBUG_LEVEL_TRACE,
			       ("WL Supp: IF %d bus_id %d is_pup_fail %d\n",
				if_id, bus_id, is_if_fail));

		if (is_if_fail == 1) {
			DEBUG_LEVELING(DEBUG_LEVEL_ERROR,
				       ("WL Supp: IF %d failed\n", if_id));
			training_result[training_stage][if_id] = TEST_FAILED;
		} else {
			training_result[training_stage][if_id] = TEST_SUCCESS;
		}
	}

	for (if_id = 0; if_id <= MAX_INTERFACE_NUM - 1; if_id++) {
		VALIDATE_ACTIVE(tm->if_act_mask, if_id);
		if (training_result[training_stage][if_id] == TEST_FAILED)
			return MV_FAIL;
	}

	return MV_OK;
}

/*
 * Phase Shift
 */
static int ddr3_tip_wl_supp_align_phase_shift(u32 dev_num, u32 if_id,
					      u32 bus_id, u32 offset,
					      u32 bus_id_delta)
{
	wr_supp_res[if_id][bus_id].stage = PHASE_SHIFT;
	if (ddr3_tip_xsb_compare_test(dev_num, if_id, bus_id,
				      0, bus_id_delta) == MV_OK) {
		wr_supp_res[if_id][bus_id].is_pup_fail = 0;
		return MV_OK;
	} else if (ddr3_tip_xsb_compare_test(dev_num, if_id, bus_id,
					     ONE_CLOCK_ERROR_SHIFT,
					     bus_id_delta) == MV_OK) {
		/* 1 clock error */
		wr_supp_res[if_id][bus_id].stage = CLOCK_SHIFT;
		DEBUG_LEVELING(DEBUG_LEVEL_TRACE,
			       ("Supp: 1 error clock for if %d pup %d with ofsset %d success\n",
				if_id, bus_id, offset));
		ddr3_tip_wl_supp_one_clk_err_shift(dev_num, if_id, bus_id, 0);
		wr_supp_res[if_id][bus_id].is_pup_fail = 0;
		return MV_OK;
	} else if (ddr3_tip_xsb_compare_test(dev_num, if_id, bus_id,
					     ALIGN_ERROR_SHIFT,
					     bus_id_delta) == MV_OK) {
		/* align error */
		DEBUG_LEVELING(DEBUG_LEVEL_TRACE,
			       ("Supp: align error for if %d pup %d with ofsset %d success\n",
				if_id, bus_id, offset));
		wr_supp_res[if_id][bus_id].stage = ALIGN_SHIFT;
		ddr3_tip_wl_supp_align_err_shift(dev_num, if_id, bus_id, 0);
		wr_supp_res[if_id][bus_id].is_pup_fail = 0;
		return MV_OK;
	} else {
		wr_supp_res[if_id][bus_id].is_pup_fail = 1;
		return MV_FAIL;
	}
}

/*
 * Compare Test
 */
static int ddr3_tip_xsb_compare_test(u32 dev_num, u32 if_id, u32 bus_id,
				     u32 edge_offset, u32 bus_id_delta)
{
	u32 num_of_succ_byte_compare, word_in_pattern, abs_offset;
	u32 word_offset, i;
	u32 read_pattern[TEST_PATTERN_LENGTH * 2];
	struct pattern_info *pattern_table = ddr3_tip_get_pattern_table();
	u32 pattern_test_pattern_table[8];

	for (i = 0; i < 8; i++) {
		pattern_test_pattern_table[i] =
			pattern_table_get_word(dev_num, PATTERN_TEST, (u8)i);
	}

	/* extern write, than read and compare */
	CHECK_STATUS(ddr3_tip_ext_write
		     (dev_num, if_id,
		      (pattern_table[PATTERN_TEST].start_addr +
		       ((SDRAM_CS_SIZE + 1) * effective_cs)), 1,
		      pattern_test_pattern_table));

	CHECK_STATUS(ddr3_tip_reset_fifo_ptr(dev_num));

	CHECK_STATUS(ddr3_tip_ext_read
		     (dev_num, if_id,
		      (pattern_table[PATTERN_TEST].start_addr +
		       ((SDRAM_CS_SIZE + 1) * effective_cs)), 1, read_pattern));

	DEBUG_LEVELING(
		DEBUG_LEVEL_TRACE,
		("XSB-compt: IF %d bus_id %d 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x\n",
		 if_id, bus_id, read_pattern[0], read_pattern[1],
		 read_pattern[2], read_pattern[3], read_pattern[4],
		 read_pattern[5], read_pattern[6], read_pattern[7]));

	/* compare byte per pup */
	num_of_succ_byte_compare = 0;
	for (word_in_pattern = start_xsb_offset;
	     word_in_pattern < (TEST_PATTERN_LENGTH * 2); word_in_pattern++) {
		word_offset = word_in_pattern + edge_offset;
		if ((word_offset > (TEST_PATTERN_LENGTH * 2 - 1)) ||
		    (word_offset < 0))
			continue;

		if ((read_pattern[word_in_pattern] & pup_mask_table[bus_id]) ==
		    (pattern_test_pattern_table[word_offset] &
		     pup_mask_table[bus_id]))
			num_of_succ_byte_compare++;
	}

	abs_offset = (edge_offset > 0) ? edge_offset : -edge_offset;
	if (num_of_succ_byte_compare == ((TEST_PATTERN_LENGTH * 2) -
					 abs_offset - start_xsb_offset)) {
		DEBUG_LEVELING(
			DEBUG_LEVEL_TRACE,
			("XSB-compt: IF %d bus_id %d num_of_succ_byte_compare %d - Success\n",
			 if_id, bus_id, num_of_succ_byte_compare));
		return MV_OK;
	} else {
		DEBUG_LEVELING(
			DEBUG_LEVEL_TRACE,
			("XSB-compt: IF %d bus_id %d num_of_succ_byte_compare %d - Fail !\n",
			 if_id, bus_id, num_of_succ_byte_compare));

		DEBUG_LEVELING(
			DEBUG_LEVEL_TRACE,
			("XSB-compt: expected 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x\n",
			 pattern_test_pattern_table[0],
			 pattern_test_pattern_table[1],
			 pattern_test_pattern_table[2],
			 pattern_test_pattern_table[3],
			 pattern_test_pattern_table[4],
			 pattern_test_pattern_table[5],
			 pattern_test_pattern_table[6],
			 pattern_test_pattern_table[7]));
		DEBUG_LEVELING(
			DEBUG_LEVEL_TRACE,
			("XSB-compt: recieved 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x\n",
			 read_pattern[0], read_pattern[1],
			 read_pattern[2], read_pattern[3],
			 read_pattern[4], read_pattern[5],
			 read_pattern[6], read_pattern[7]));

		DEBUG_LEVELING(
			DEBUG_LEVEL_TRACE,
			("XSB-compt: IF %d bus_id %d num_of_succ_byte_compare %d - Fail !\n",
			 if_id, bus_id, num_of_succ_byte_compare));

		return MV_FAIL;
	}
}

/*
 * Clock error shift - function moves the write leveling delay 1cc forward
 */
static int ddr3_tip_wl_supp_one_clk_err_shift(u32 dev_num, u32 if_id,
					      u32 bus_id, u32 bus_id_delta)
{
	int phase, adll;
	u32 data;
	DEBUG_LEVELING(DEBUG_LEVEL_TRACE, ("One_clk_err_shift\n"));

	CHECK_STATUS(ddr3_tip_bus_read
		     (dev_num, if_id, ACCESS_TYPE_UNICAST, bus_id,
		      DDR_PHY_DATA, WL_PHY_REG, &data));
	phase = ((data >> 6) & 0x7);
	adll = data & 0x1f;
	DEBUG_LEVELING(DEBUG_LEVEL_TRACE,
		       ("One_clk_err_shift: IF %d bus_id %d phase %d adll %d\n",
			if_id, bus_id, phase, adll));

	if ((phase == 0) || (phase == 1)) {
		CHECK_STATUS(ddr3_tip_bus_read_modify_write
			     (dev_num, ACCESS_TYPE_UNICAST, if_id, bus_id,
			      DDR_PHY_DATA, 0, (phase + 2), 0x1f));
	} else if (phase == 2) {
		if (adll < 6) {
			data = (3 << 6) + (0x1f);
			CHECK_STATUS(ddr3_tip_bus_read_modify_write
				     (dev_num, ACCESS_TYPE_UNICAST, if_id,
				      bus_id, DDR_PHY_DATA, 0, data,
				      (0x7 << 6 | 0x1f)));
			data = 0x2f;
			CHECK_STATUS(ddr3_tip_bus_read_modify_write
				     (dev_num, ACCESS_TYPE_UNICAST, if_id,
				      bus_id, DDR_PHY_DATA, 1, data, 0x3f));
		}
	} else {
		/* phase 3 */
		return MV_FAIL;
	}

	return MV_OK;
}

/*
 * Align error shift
 */
static int ddr3_tip_wl_supp_align_err_shift(u32 dev_num, u32 if_id,
					    u32 bus_id, u32 bus_id_delta)
{
	int phase, adll;
	u32 data;

	/* Shift WL result 1 phase back */
	CHECK_STATUS(ddr3_tip_bus_read(dev_num, if_id, ACCESS_TYPE_UNICAST,
				       bus_id, DDR_PHY_DATA, WL_PHY_REG,
				       &data));
	phase = ((data >> 6) & 0x7);
	adll = data & 0x1f;
	DEBUG_LEVELING(
		DEBUG_LEVEL_TRACE,
		("Wl_supp_align_err_shift: IF %d bus_id %d phase %d adll %d\n",
		 if_id, bus_id, phase, adll));

	if (phase < 2) {
		if (adll > 0x1a) {
			if (phase == 0)
				return MV_FAIL;

			if (phase == 1) {
				data = 0;
				CHECK_STATUS(ddr3_tip_bus_read_modify_write
					     (dev_num, ACCESS_TYPE_UNICAST,
					      if_id, bus_id, DDR_PHY_DATA,
					      0, data, (0x7 << 6 | 0x1f)));
				data = 0xf;
				CHECK_STATUS(ddr3_tip_bus_read_modify_write
					     (dev_num, ACCESS_TYPE_UNICAST,
					      if_id, bus_id, DDR_PHY_DATA,
					      1, data, 0x1f));
				return MV_OK;
			}
		} else {
			return MV_FAIL;
		}
	} else if ((phase == 2) || (phase == 3)) {
		phase = phase - 2;
		data = (phase << 6) + (adll & 0x1f);
		CHECK_STATUS(ddr3_tip_bus_read_modify_write
			     (dev_num, ACCESS_TYPE_UNICAST, if_id, bus_id,
			      DDR_PHY_DATA, 0, data, (0x7 << 6 | 0x1f)));
		return MV_OK;
	} else {
		DEBUG_LEVELING(DEBUG_LEVEL_ERROR,
			       ("Wl_supp_align_err_shift: unexpected phase\n"));

		return MV_FAIL;
	}

	return MV_OK;
}

/*
 * Dynamic write leveling sequence
 */
static int ddr3_tip_dynamic_write_leveling_seq(u32 dev_num)
{
	u32 bus_id, dq_id;
	u16 *mask_results_pup_reg_map = ddr3_tip_get_mask_results_pup_reg_map();
	u16 *mask_results_dq_reg_map = ddr3_tip_get_mask_results_dq_reg();
	struct hws_topology_map *tm = ddr3_get_topology_map();

	CHECK_STATUS(ddr3_tip_if_write
		     (dev_num, ACCESS_TYPE_MULTICAST, PARAM_NOT_CARE,
		      TRAINING_SW_2_REG, 0x1, 0x5));
	CHECK_STATUS(ddr3_tip_if_write
		     (dev_num, ACCESS_TYPE_MULTICAST, PARAM_NOT_CARE,
		      TRAINING_WRITE_LEVELING_REG, 0x50, 0xff));
	CHECK_STATUS(ddr3_tip_if_write
		     (dev_num, ACCESS_TYPE_MULTICAST, PARAM_NOT_CARE,
		      TRAINING_WRITE_LEVELING_REG, 0x5c, 0xff));
	CHECK_STATUS(ddr3_tip_if_write
		     (dev_num, ACCESS_TYPE_MULTICAST, PARAM_NOT_CARE,
		      ODPG_TRAINING_CONTROL_REG, 0x381b82, 0x3c3faf));
	CHECK_STATUS(ddr3_tip_if_write
		     (dev_num, ACCESS_TYPE_MULTICAST, PARAM_NOT_CARE,
		      ODPG_OBJ1_OPCODE_REG, (0x3 << 25), (0x3ffff << 9)));
	CHECK_STATUS(ddr3_tip_if_write
		     (dev_num, ACCESS_TYPE_MULTICAST, PARAM_NOT_CARE,
		      ODPG_OBJ1_ITER_CNT_REG, 0x80, 0xffff));
	CHECK_STATUS(ddr3_tip_if_write
		     (dev_num, ACCESS_TYPE_MULTICAST, PARAM_NOT_CARE,
		      ODPG_WRITE_LEVELING_DONE_CNTR_REG, 0x14, 0xff));
	CHECK_STATUS(ddr3_tip_if_write
		     (dev_num, ACCESS_TYPE_MULTICAST, PARAM_NOT_CARE,
		      TRAINING_WRITE_LEVELING_REG, 0xff5c, 0xffff));

	/* mask PBS */
	for (dq_id = 0; dq_id < MAX_DQ_NUM; dq_id++) {
		CHECK_STATUS(ddr3_tip_if_write
			     (dev_num, ACCESS_TYPE_MULTICAST, PARAM_NOT_CARE,
			      mask_results_dq_reg_map[dq_id], 0x1 << 24,
			      0x1 << 24));
	}

	/* Mask all results */
	for (bus_id = 0; bus_id < tm->num_of_bus_per_interface; bus_id++) {
		CHECK_STATUS(ddr3_tip_if_write
			     (dev_num, ACCESS_TYPE_MULTICAST, PARAM_NOT_CARE,
			      mask_results_pup_reg_map[bus_id], 0x1 << 24,
			      0x1 << 24));
	}

	/* Unmask only wanted */
	for (bus_id = 0; bus_id < tm->num_of_bus_per_interface; bus_id++) {
		VALIDATE_ACTIVE(tm->bus_act_mask, bus_id);
		CHECK_STATUS(ddr3_tip_if_write
			     (dev_num, ACCESS_TYPE_MULTICAST, PARAM_NOT_CARE,
			      mask_results_pup_reg_map[bus_id], 0, 0x1 << 24));
	}

	CHECK_STATUS(ddr3_tip_if_write
		     (dev_num, ACCESS_TYPE_MULTICAST, PARAM_NOT_CARE,
		      WR_LEVELING_DQS_PATTERN_REG, 0x1, 0x1));

	return MV_OK;
}

/*
 * Dynamic read leveling sequence
 */
static int ddr3_tip_dynamic_read_leveling_seq(u32 dev_num)
{
	u32 bus_id, dq_id;
	u16 *mask_results_pup_reg_map = ddr3_tip_get_mask_results_pup_reg_map();
	u16 *mask_results_dq_reg_map = ddr3_tip_get_mask_results_dq_reg();
	struct hws_topology_map *tm = ddr3_get_topology_map();

	/* mask PBS */
	for (dq_id = 0; dq_id < MAX_DQ_NUM; dq_id++) {
		CHECK_STATUS(ddr3_tip_if_write
			     (dev_num, ACCESS_TYPE_MULTICAST, PARAM_NOT_CARE,
			      mask_results_dq_reg_map[dq_id], 0x1 << 24,
			      0x1 << 24));
	}

	/* Mask all results */
	for (bus_id = 0; bus_id < tm->num_of_bus_per_interface; bus_id++) {
		CHECK_STATUS(ddr3_tip_if_write
			     (dev_num, ACCESS_TYPE_MULTICAST, PARAM_NOT_CARE,
			      mask_results_pup_reg_map[bus_id], 0x1 << 24,
			      0x1 << 24));
	}

	/* Unmask only wanted */
	for (bus_id = 0; bus_id < tm->num_of_bus_per_interface; bus_id++) {
		VALIDATE_ACTIVE(tm->bus_act_mask, bus_id);
		CHECK_STATUS(ddr3_tip_if_write
			     (dev_num, ACCESS_TYPE_MULTICAST, PARAM_NOT_CARE,
			      mask_results_pup_reg_map[bus_id], 0, 0x1 << 24));
	}

	return MV_OK;
}

/*
 * Dynamic read leveling sequence
 */
static int ddr3_tip_dynamic_per_bit_read_leveling_seq(u32 dev_num)
{
	u32 bus_id, dq_id;
	u16 *mask_results_pup_reg_map = ddr3_tip_get_mask_results_pup_reg_map();
	u16 *mask_results_dq_reg_map = ddr3_tip_get_mask_results_dq_reg();
	struct hws_topology_map *tm = ddr3_get_topology_map();

	/* mask PBS */
	for (dq_id = 0; dq_id < MAX_DQ_NUM; dq_id++) {
		CHECK_STATUS(ddr3_tip_if_write
			     (dev_num, ACCESS_TYPE_MULTICAST, PARAM_NOT_CARE,
			      mask_results_dq_reg_map[dq_id], 0x1 << 24,
			      0x1 << 24));
	}

	/* Mask all results */
	for (bus_id = 0; bus_id < tm->num_of_bus_per_interface; bus_id++) {
		CHECK_STATUS(ddr3_tip_if_write
			     (dev_num, ACCESS_TYPE_MULTICAST, PARAM_NOT_CARE,
			      mask_results_pup_reg_map[bus_id], 0x1 << 24,
			      0x1 << 24));
	}

	/* Unmask only wanted */
	for (dq_id = 0; dq_id < MAX_DQ_NUM; dq_id++) {
		VALIDATE_ACTIVE(tm->bus_act_mask, dq_id / 8);
		CHECK_STATUS(ddr3_tip_if_write
			     (dev_num, ACCESS_TYPE_MULTICAST, PARAM_NOT_CARE,
			      mask_results_dq_reg_map[dq_id], 0x0 << 24,
			      0x1 << 24));
	}

	return MV_OK;
}

/*
 * Print write leveling supplementary results
 */
int ddr3_tip_print_wl_supp_result(u32 dev_num)
{
	u32 bus_id = 0, if_id = 0;
	struct hws_topology_map *tm = ddr3_get_topology_map();

	DEBUG_LEVELING(DEBUG_LEVEL_INFO,
		       ("I/F0 PUP0 Result[0 - success, 1-fail] ...\n"));

	for (if_id = 0; if_id <= MAX_INTERFACE_NUM - 1; if_id++) {
		VALIDATE_ACTIVE(tm->if_act_mask, if_id);
		for (bus_id = 0; bus_id < tm->num_of_bus_per_interface;
		     bus_id++) {
			VALIDATE_ACTIVE(tm->bus_act_mask, bus_id);
			DEBUG_LEVELING(DEBUG_LEVEL_INFO,
				       ("%d ,", wr_supp_res[if_id]
					[bus_id].is_pup_fail));
		}
	}
	DEBUG_LEVELING(
		DEBUG_LEVEL_INFO,
		("I/F0 PUP0 Stage[0-phase_shift, 1-clock_shift, 2-align_shift] ...\n"));

	for (if_id = 0; if_id <= MAX_INTERFACE_NUM - 1; if_id++) {
		VALIDATE_ACTIVE(tm->if_act_mask, if_id);
		for (bus_id = 0; bus_id < tm->num_of_bus_per_interface;
		     bus_id++) {
			VALIDATE_ACTIVE(tm->bus_act_mask, bus_id);
			DEBUG_LEVELING(DEBUG_LEVEL_INFO,
				       ("%d ,", wr_supp_res[if_id]
					[bus_id].stage));
		}
	}

	return MV_OK;
}
