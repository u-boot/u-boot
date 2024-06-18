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

#define PATTERN_1	0x55555555
#define PATTERN_2	0xaaaaaaaa

#define VALIDATE_TRAINING_LIMIT(e1, e2)			\
	((((e2) - (e1) + 1) > 33) && ((e1) < 67))

u32 phy_reg_bk[MAX_INTERFACE_NUM][MAX_BUS_NUM][BUS_WIDTH_IN_BITS];

u32 training_res[MAX_INTERFACE_NUM * MAX_BUS_NUM * BUS_WIDTH_IN_BITS *
		 HWS_SEARCH_DIR_LIMIT];

u16 mask_results_dq_reg_map[] = {
	RESULT_CONTROL_PUP_0_BIT_0_REG, RESULT_CONTROL_PUP_0_BIT_1_REG,
	RESULT_CONTROL_PUP_0_BIT_2_REG, RESULT_CONTROL_PUP_0_BIT_3_REG,
	RESULT_CONTROL_PUP_0_BIT_4_REG, RESULT_CONTROL_PUP_0_BIT_5_REG,
	RESULT_CONTROL_PUP_0_BIT_6_REG, RESULT_CONTROL_PUP_0_BIT_7_REG,
	RESULT_CONTROL_PUP_1_BIT_0_REG, RESULT_CONTROL_PUP_1_BIT_1_REG,
	RESULT_CONTROL_PUP_1_BIT_2_REG, RESULT_CONTROL_PUP_1_BIT_3_REG,
	RESULT_CONTROL_PUP_1_BIT_4_REG, RESULT_CONTROL_PUP_1_BIT_5_REG,
	RESULT_CONTROL_PUP_1_BIT_6_REG, RESULT_CONTROL_PUP_1_BIT_7_REG,
	RESULT_CONTROL_PUP_2_BIT_0_REG, RESULT_CONTROL_PUP_2_BIT_1_REG,
	RESULT_CONTROL_PUP_2_BIT_2_REG, RESULT_CONTROL_PUP_2_BIT_3_REG,
	RESULT_CONTROL_PUP_2_BIT_4_REG, RESULT_CONTROL_PUP_2_BIT_5_REG,
	RESULT_CONTROL_PUP_2_BIT_6_REG, RESULT_CONTROL_PUP_2_BIT_7_REG,
	RESULT_CONTROL_PUP_3_BIT_0_REG, RESULT_CONTROL_PUP_3_BIT_1_REG,
	RESULT_CONTROL_PUP_3_BIT_2_REG, RESULT_CONTROL_PUP_3_BIT_3_REG,
	RESULT_CONTROL_PUP_3_BIT_4_REG, RESULT_CONTROL_PUP_3_BIT_5_REG,
	RESULT_CONTROL_PUP_3_BIT_6_REG, RESULT_CONTROL_PUP_3_BIT_7_REG,
	RESULT_CONTROL_PUP_4_BIT_0_REG, RESULT_CONTROL_PUP_4_BIT_1_REG,
	RESULT_CONTROL_PUP_4_BIT_2_REG, RESULT_CONTROL_PUP_4_BIT_3_REG,
	RESULT_CONTROL_PUP_4_BIT_4_REG, RESULT_CONTROL_PUP_4_BIT_5_REG,
	RESULT_CONTROL_PUP_4_BIT_6_REG, RESULT_CONTROL_PUP_4_BIT_7_REG,
};

u16 mask_results_pup_reg_map[] = {
	RESULT_CONTROL_BYTE_PUP_0_REG, RESULT_CONTROL_BYTE_PUP_1_REG,
	RESULT_CONTROL_BYTE_PUP_2_REG, RESULT_CONTROL_BYTE_PUP_3_REG,
	RESULT_CONTROL_BYTE_PUP_4_REG
};

u16 mask_results_dq_reg_map_pup3_ecc[] = {
	RESULT_CONTROL_PUP_0_BIT_0_REG, RESULT_CONTROL_PUP_0_BIT_1_REG,
	RESULT_CONTROL_PUP_0_BIT_2_REG, RESULT_CONTROL_PUP_0_BIT_3_REG,
	RESULT_CONTROL_PUP_0_BIT_4_REG, RESULT_CONTROL_PUP_0_BIT_5_REG,
	RESULT_CONTROL_PUP_0_BIT_6_REG, RESULT_CONTROL_PUP_0_BIT_7_REG,
	RESULT_CONTROL_PUP_1_BIT_0_REG, RESULT_CONTROL_PUP_1_BIT_1_REG,
	RESULT_CONTROL_PUP_1_BIT_2_REG, RESULT_CONTROL_PUP_1_BIT_3_REG,
	RESULT_CONTROL_PUP_1_BIT_4_REG, RESULT_CONTROL_PUP_1_BIT_5_REG,
	RESULT_CONTROL_PUP_1_BIT_6_REG, RESULT_CONTROL_PUP_1_BIT_7_REG,
	RESULT_CONTROL_PUP_2_BIT_0_REG, RESULT_CONTROL_PUP_2_BIT_1_REG,
	RESULT_CONTROL_PUP_2_BIT_2_REG, RESULT_CONTROL_PUP_2_BIT_3_REG,
	RESULT_CONTROL_PUP_2_BIT_4_REG, RESULT_CONTROL_PUP_2_BIT_5_REG,
	RESULT_CONTROL_PUP_2_BIT_6_REG, RESULT_CONTROL_PUP_2_BIT_7_REG,
	RESULT_CONTROL_PUP_4_BIT_0_REG, RESULT_CONTROL_PUP_4_BIT_1_REG,
	RESULT_CONTROL_PUP_4_BIT_2_REG, RESULT_CONTROL_PUP_4_BIT_3_REG,
	RESULT_CONTROL_PUP_4_BIT_4_REG, RESULT_CONTROL_PUP_4_BIT_5_REG,
	RESULT_CONTROL_PUP_4_BIT_6_REG, RESULT_CONTROL_PUP_4_BIT_7_REG,
	RESULT_CONTROL_PUP_4_BIT_0_REG, RESULT_CONTROL_PUP_4_BIT_1_REG,
	RESULT_CONTROL_PUP_4_BIT_2_REG, RESULT_CONTROL_PUP_4_BIT_3_REG,
	RESULT_CONTROL_PUP_4_BIT_4_REG, RESULT_CONTROL_PUP_4_BIT_5_REG,
	RESULT_CONTROL_PUP_4_BIT_6_REG, RESULT_CONTROL_PUP_4_BIT_7_REG,
};

u16 mask_results_pup_reg_map_pup3_ecc[] = {
	RESULT_CONTROL_BYTE_PUP_0_REG, RESULT_CONTROL_BYTE_PUP_1_REG,
	RESULT_CONTROL_BYTE_PUP_2_REG, RESULT_CONTROL_BYTE_PUP_4_REG,
	RESULT_CONTROL_BYTE_PUP_4_REG
};

struct pattern_info pattern_table_16[] = {
	/*
	 * num tx phases, tx burst, delay between, rx pattern,
	 * start_address, pattern_len
	 */
	{1, 1, 2, 1, 0x0080, 2},	/* PATTERN_PBS1 */
	{1, 1, 2, 1, 0x00c0, 2},	/* PATTERN_PBS2 */
	{1, 1, 2, 1, 0x0100, 2},	/* PATTERN_RL */
	{0xf, 0x7, 2, 0x7, 0x0140, 16},	/* PATTERN_STATIC_PBS */
	{0xf, 0x7, 2, 0x7, 0x0190, 16},	/* PATTERN_KILLER_DQ0 */
	{0xf, 0x7, 2, 0x7, 0x01d0, 16},	/* PATTERN_KILLER_DQ1 */
	{0xf, 0x7, 2, 0x7, 0x0210, 16},	/* PATTERN_KILLER_DQ2 */
	{0xf, 0x7, 2, 0x7, 0x0250, 16},	/* PATTERN_KILLER_DQ3 */
	{0xf, 0x7, 2, 0x7, 0x0290, 16},	/* PATTERN_KILLER_DQ4 */
	{0xf, 0x7, 2, 0x7, 0x02d0, 16},	/* PATTERN_KILLER_DQ5 */
	{0xf, 0x7, 2, 0x7, 0x0310, 16},	/* PATTERN_KILLER_DQ6 */
	{0xf, 0x7, 2, 0x7, 0x0350, 16},	/* PATTERN_KILLER_DQ7 */
	{1, 1, 2, 1, 0x0380, 2},	/* PATTERN_PBS3 */
	{1, 1, 2, 1, 0x0000, 2},	/* PATTERN_RL2 */
	{1, 1, 2, 1, 0x0040, 2},	/* PATTERN_TEST */
	{0xf, 0x7, 2, 0x7, 0x03c0, 16},	/* PATTERN_FULL_SSO_1T */
	{0xf, 0x7, 2, 0x7, 0x0400, 16},	/* PATTERN_FULL_SSO_2T */
	{0xf, 0x7, 2, 0x7, 0x0440, 16},	/* PATTERN_FULL_SSO_3T */
	{0xf, 0x7, 2, 0x7, 0x0480, 16},	/* PATTERN_FULL_SSO_4T */
	{0xf, 0x7, 2, 0x7, 0x04c0, 16}	/* PATTERN_VREF */
	/*Note: actual start_address is <<3 of defined addess */
};

struct pattern_info pattern_table_32[] = {
	/*
	 * num tx phases, tx burst, delay between, rx pattern,
	 * start_address, pattern_len
	 */
	{3, 3, 2, 3, 0x0080, 4},	/* PATTERN_PBS1 */
	{3, 3, 2, 3, 0x00c0, 4},	/* PATTERN_PBS2 */
	{3, 3, 2, 3, 0x0100, 4},	/* PATTERN_RL */
	{0x1f, 0xf, 2, 0xf, 0x0140, 32},	/* PATTERN_STATIC_PBS */
	{0x1f, 0xf, 2, 0xf, 0x0190, 32},	/* PATTERN_KILLER_DQ0 */
	{0x1f, 0xf, 2, 0xf, 0x01d0, 32},	/* PATTERN_KILLER_DQ1 */
	{0x1f, 0xf, 2, 0xf, 0x0210, 32},	/* PATTERN_KILLER_DQ2 */
	{0x1f, 0xf, 2, 0xf, 0x0250, 32},	/* PATTERN_KILLER_DQ3 */
	{0x1f, 0xf, 2, 0xf, 0x0290, 32},	/* PATTERN_KILLER_DQ4 */
	{0x1f, 0xf, 2, 0xf, 0x02d0, 32},	/* PATTERN_KILLER_DQ5 */
	{0x1f, 0xf, 2, 0xf, 0x0310, 32},	/* PATTERN_KILLER_DQ6 */
	{0x1f, 0xf, 2, 0xf, 0x0350, 32},	/* PATTERN_KILLER_DQ7 */
	{3, 3, 2, 3, 0x0380, 4},	/* PATTERN_PBS3 */
	{3, 3, 2, 3, 0x0000, 4},	/* PATTERN_RL2 */
	{3, 3, 2, 3, 0x0040, 4},	/* PATTERN_TEST */
	{0x1f, 0xf, 2, 0xf, 0x03c0, 32},	/* PATTERN_FULL_SSO_1T */
	{0x1f, 0xf, 2, 0xf, 0x0400, 32},	/* PATTERN_FULL_SSO_2T */
	{0x1f, 0xf, 2, 0xf, 0x0440, 32},	/* PATTERN_FULL_SSO_3T */
	{0x1f, 0xf, 2, 0xf, 0x0480, 32},	/* PATTERN_FULL_SSO_4T */
	{0x1f, 0xf, 2, 0xf, 0x04c0, 32}	/* PATTERN_VREF */
	/*Note: actual start_address is <<3 of defined addess */
};

u32 train_dev_num;
enum hws_ddr_cs traintrain_cs_type;
u32 train_pup_num;
enum hws_training_result train_result_type;
enum hws_control_element train_control_element;
enum hws_search_dir traine_search_dir;
enum hws_dir train_direction;
u32 train_if_select;
u32 train_init_value;
u32 train_number_iterations;
enum hws_pattern train_pattern;
enum hws_edge_compare train_edge_compare;
u32 train_cs_num;
u32 train_if_acess, train_if_id, train_pup_access;
u32 max_polling_for_done = 1000000;

u32 *ddr3_tip_get_buf_ptr(u32 dev_num, enum hws_search_dir search,
			  enum hws_training_result result_type,
			  u32 interface_num)
{
	u32 *buf_ptr = NULL;

	buf_ptr = &training_res
		[MAX_INTERFACE_NUM * MAX_BUS_NUM * BUS_WIDTH_IN_BITS * search +
		 interface_num * MAX_BUS_NUM * BUS_WIDTH_IN_BITS];

	return buf_ptr;
}

/*
 * IP Training search
 * Note: for one edge search only from fail to pass, else jitter can
 * be be entered into solution.
 */
int ddr3_tip_ip_training(u32 dev_num, enum hws_access_type access_type,
			 u32 interface_num,
			 enum hws_access_type pup_access_type,
			 u32 pup_num, enum hws_training_result result_type,
			 enum hws_control_element control_element,
			 enum hws_search_dir search_dir, enum hws_dir direction,
			 u32 interface_mask, u32 init_value, u32 num_iter,
			 enum hws_pattern pattern,
			 enum hws_edge_compare edge_comp,
			 enum hws_ddr_cs cs_type, u32 cs_num,
			 enum hws_training_ip_stat *train_status)
{
	u32 mask_dq_num_of_regs, mask_pup_num_of_regs, index_cnt, poll_cnt,
		reg_data, pup_id;
	u32 tx_burst_size;
	u32 delay_between_burst;
	u32 rd_mode;
	u32 read_data[MAX_INTERFACE_NUM];
	struct pattern_info *pattern_table = ddr3_tip_get_pattern_table();
	u16 *mask_results_pup_reg_map = ddr3_tip_get_mask_results_pup_reg_map();
	u16 *mask_results_dq_reg_map = ddr3_tip_get_mask_results_dq_reg();
	struct hws_topology_map *tm = ddr3_get_topology_map();

	if (pup_num >= tm->num_of_bus_per_interface) {
		DEBUG_TRAINING_IP_ENGINE(DEBUG_LEVEL_ERROR,
					 ("pup_num %d not valid\n", pup_num));
	}
	if (interface_num >= MAX_INTERFACE_NUM) {
		DEBUG_TRAINING_IP_ENGINE(DEBUG_LEVEL_ERROR,
					 ("if_id %d not valid\n",
					  interface_num));
	}
	if (train_status == NULL) {
		DEBUG_TRAINING_IP_ENGINE(DEBUG_LEVEL_ERROR,
					 ("error param 4\n"));
		return MV_BAD_PARAM;
	}

	/* load pattern */
	if (cs_type == CS_SINGLE) {
		/* All CSs to CS0     */
		CHECK_STATUS(ddr3_tip_if_write
			     (dev_num, access_type, interface_num,
			      CS_ENABLE_REG, 1 << 3, 1 << 3));
		/* All CSs to CS0     */
		CHECK_STATUS(ddr3_tip_if_write
			     (dev_num, access_type, interface_num,
			      ODPG_DATA_CONTROL_REG,
			      (0x3 | (effective_cs << 26)), 0xc000003));
	} else {
		CHECK_STATUS(ddr3_tip_if_write
			     (dev_num, access_type, interface_num,
			      CS_ENABLE_REG, 0, 1 << 3));
		/*  CS select */
		CHECK_STATUS(ddr3_tip_if_write
			     (dev_num, access_type, interface_num,
			      ODPG_DATA_CONTROL_REG, 0x3 | cs_num << 26,
			      0x3 | 3 << 26));
	}

	/* load pattern to ODPG */
	ddr3_tip_load_pattern_to_odpg(dev_num, access_type, interface_num,
				      pattern,
				      pattern_table[pattern].start_addr);
	tx_burst_size =	(direction == OPER_WRITE) ?
		pattern_table[pattern].tx_burst_size : 0;
	delay_between_burst = (direction == OPER_WRITE) ? 2 : 0;
	rd_mode = (direction == OPER_WRITE) ? 1 : 0;
	CHECK_STATUS(ddr3_tip_configure_odpg
		     (dev_num, access_type, interface_num, direction,
		      pattern_table[pattern].num_of_phases_tx, tx_burst_size,
		      pattern_table[pattern].num_of_phases_rx,
		      delay_between_burst, rd_mode, effective_cs, STRESS_NONE,
		      DURATION_SINGLE));
	reg_data = (direction == OPER_READ) ? 0 : (0x3 << 30);
	reg_data |= (direction == OPER_READ) ? 0x60 : 0xfa;
	CHECK_STATUS(ddr3_tip_if_write
		     (dev_num, access_type, interface_num,
		      ODPG_WRITE_READ_MODE_ENABLE_REG, reg_data,
		      MASK_ALL_BITS));
	reg_data = (edge_comp == EDGE_PF || edge_comp == EDGE_FP) ? 0 : 1 << 6;
	reg_data |= (edge_comp == EDGE_PF || edge_comp == EDGE_PFP) ?
		(1 << 7) : 0;

	/* change from Pass to Fail will lock the result */
	if (pup_access_type == ACCESS_TYPE_MULTICAST)
		reg_data |= 0xe << 14;
	else
		reg_data |= pup_num << 14;

	if (edge_comp == EDGE_FP) {
		/* don't search for readl edge change, only the state */
		reg_data |= (0 << 20);
	} else if (edge_comp == EDGE_FPF) {
		reg_data |= (0 << 20);
	} else {
		reg_data |= (3 << 20);
	}

	CHECK_STATUS(ddr3_tip_if_write
		     (dev_num, access_type, interface_num,
		      ODPG_TRAINING_CONTROL_REG,
		      reg_data | (0x7 << 8) | (0x7 << 11),
		      (0x3 | (0x3 << 2) | (0x3 << 6) | (1 << 5) | (0x7 << 8) |
		       (0x7 << 11) | (0xf << 14) | (0x3 << 18) | (3 << 20))));
	reg_data = (search_dir == HWS_LOW2HIGH) ? 0 : (1 << 8);
	CHECK_STATUS(ddr3_tip_if_write
		     (dev_num, access_type, interface_num, ODPG_OBJ1_OPCODE_REG,
		      1 | reg_data | init_value << 9 | (1 << 25) | (1 << 26),
		      0xff | (1 << 8) | (0xffff << 9) | (1 << 25) | (1 << 26)));

	/*
	 * Write2_dunit(0x10b4, Number_iteration , [15:0])
	 * Max number of iterations
	 */
	CHECK_STATUS(ddr3_tip_if_write(dev_num, access_type, interface_num,
				       ODPG_OBJ1_ITER_CNT_REG, num_iter,
				       0xffff));
	if (control_element == HWS_CONTROL_ELEMENT_DQ_SKEW &&
	    direction == OPER_READ) {
		/*
		 * Write2_dunit(0x10c0, 0x5f , [7:0])
		 * MC PBS Reg Address at DDR PHY
		 */
		reg_data = 0x5f +
			effective_cs * CALIBRATED_OBJECTS_REG_ADDR_OFFSET;
	} else if (control_element == HWS_CONTROL_ELEMENT_DQ_SKEW &&
		   direction == OPER_WRITE) {
		reg_data = 0x1f +
			effective_cs * CALIBRATED_OBJECTS_REG_ADDR_OFFSET;
	} else if (control_element == HWS_CONTROL_ELEMENT_ADLL &&
		   direction == OPER_WRITE) {
		/*
		 * LOOP         0x00000001 + 4*n:
		 * where n (0-3) represents M_CS number
		 */
		/*
		 * Write2_dunit(0x10c0, 0x1 , [7:0])
		 * ADLL WR Reg Address at DDR PHY
		 */
		reg_data = 1 + effective_cs * CS_REGISTER_ADDR_OFFSET;
	} else if (control_element == HWS_CONTROL_ELEMENT_ADLL &&
		   direction == OPER_READ) {
		/* ADLL RD Reg Address at DDR PHY */
		reg_data = 3 + effective_cs * CS_REGISTER_ADDR_OFFSET;
	} else if (control_element == HWS_CONTROL_ELEMENT_DQS_SKEW &&
		   direction == OPER_WRITE) {
		/* TBD not defined in 0.5.0 requirement  */
	} else if (control_element == HWS_CONTROL_ELEMENT_DQS_SKEW &&
		   direction == OPER_READ) {
		/* TBD not defined in 0.5.0 requirement */
	}

	reg_data |= (0x6 << 28);
	CHECK_STATUS(ddr3_tip_if_write
		     (dev_num, access_type, interface_num, CALIB_OBJ_PRFA_REG,
		      reg_data | (init_value << 8),
		      0xff | (0xffff << 8) | (0xf << 24) | (u32) (0xf << 28)));

	mask_dq_num_of_regs = tm->num_of_bus_per_interface * BUS_WIDTH_IN_BITS;
	mask_pup_num_of_regs = tm->num_of_bus_per_interface;

	if (result_type == RESULT_PER_BIT) {
		for (index_cnt = 0; index_cnt < mask_dq_num_of_regs;
		     index_cnt++) {
			CHECK_STATUS(ddr3_tip_if_write
				     (dev_num, access_type, interface_num,
				      mask_results_dq_reg_map[index_cnt], 0,
				      1 << 24));
		}

		/* Mask disabled buses */
		for (pup_id = 0; pup_id < tm->num_of_bus_per_interface;
		     pup_id++) {
			if (IS_ACTIVE(tm->bus_act_mask, pup_id) == 1)
				continue;

			for (index_cnt = (mask_dq_num_of_regs - pup_id * 8);
			     index_cnt <
				     (mask_dq_num_of_regs - (pup_id + 1) * 8);
			     index_cnt++) {
				CHECK_STATUS(ddr3_tip_if_write
					     (dev_num, access_type,
					      interface_num,
					      mask_results_dq_reg_map
					      [index_cnt], (1 << 24), 1 << 24));
			}
		}

		for (index_cnt = 0; index_cnt < mask_pup_num_of_regs;
		     index_cnt++) {
			CHECK_STATUS(ddr3_tip_if_write
				     (dev_num, access_type, interface_num,
				      mask_results_pup_reg_map[index_cnt],
				      (1 << 24), 1 << 24));
		}
	} else if (result_type == RESULT_PER_BYTE) {
		/* write to adll */
		for (index_cnt = 0; index_cnt < mask_pup_num_of_regs;
		     index_cnt++) {
			CHECK_STATUS(ddr3_tip_if_write
				     (dev_num, access_type, interface_num,
				      mask_results_pup_reg_map[index_cnt], 0,
				      1 << 24));
		}
		for (index_cnt = 0; index_cnt < mask_dq_num_of_regs;
		     index_cnt++) {
			CHECK_STATUS(ddr3_tip_if_write
				     (dev_num, access_type, interface_num,
				      mask_results_dq_reg_map[index_cnt],
				      (1 << 24), (1 << 24)));
		}
	}

	/* Start Training Trigger */
	CHECK_STATUS(ddr3_tip_if_write(dev_num, access_type, interface_num,
				       ODPG_TRAINING_TRIGGER_REG, 1, 1));
	/* wait for all RFU tests to finish (or timeout) */
	/* WA for 16 bit mode, more investigation needed */
	mdelay(1);

	/* Training "Done ?" */
	for (index_cnt = 0; index_cnt < MAX_INTERFACE_NUM; index_cnt++) {
		if (IS_ACTIVE(tm->if_act_mask, index_cnt) == 0)
			continue;

		if (interface_mask & (1 << index_cnt)) {
			/* need to check results for this Dunit */
			for (poll_cnt = 0; poll_cnt < max_polling_for_done;
			     poll_cnt++) {
				CHECK_STATUS(ddr3_tip_if_read
					     (dev_num, ACCESS_TYPE_UNICAST,
					      index_cnt,
					      ODPG_TRAINING_STATUS_REG,
					      &reg_data, MASK_ALL_BITS));
				if ((reg_data & 0x2) != 0) {
					/*done */
					train_status[index_cnt] =
						HWS_TRAINING_IP_STATUS_SUCCESS;
					break;
				}
			}

			if (poll_cnt == max_polling_for_done) {
				train_status[index_cnt] =
					HWS_TRAINING_IP_STATUS_TIMEOUT;
			}
		}
		/* Be sure that ODPG done */
		CHECK_STATUS(is_odpg_access_done(dev_num, index_cnt));
	}

	/* Write ODPG done in Dunit */
	CHECK_STATUS(ddr3_tip_if_write
		     (dev_num, ACCESS_TYPE_MULTICAST, PARAM_NOT_CARE,
		      ODPG_STATUS_DONE_REG, 0, 0x1));

	/* wait for all Dunit tests to finish (or timeout) */
	/* Training "Done ?" */
	/* Training "Pass ?" */
	for (index_cnt = 0; index_cnt < MAX_INTERFACE_NUM; index_cnt++) {
		if (IS_ACTIVE(tm->if_act_mask, index_cnt) == 0)
			continue;

		if (interface_mask & (1 << index_cnt)) {
			/* need to check results for this Dunit */
			for (poll_cnt = 0; poll_cnt < max_polling_for_done;
			     poll_cnt++) {
				CHECK_STATUS(ddr3_tip_if_read
					     (dev_num, ACCESS_TYPE_UNICAST,
					      index_cnt,
					      ODPG_TRAINING_TRIGGER_REG,
					      read_data, MASK_ALL_BITS));
				reg_data = read_data[index_cnt];
				if ((reg_data & 0x2) != 0) {
					/* done */
					if ((reg_data & 0x4) == 0) {
						train_status[index_cnt] =
							HWS_TRAINING_IP_STATUS_SUCCESS;
					} else {
						train_status[index_cnt] =
							HWS_TRAINING_IP_STATUS_FAIL;
					}
					break;
				}
			}

			if (poll_cnt == max_polling_for_done) {
				train_status[index_cnt] =
					HWS_TRAINING_IP_STATUS_TIMEOUT;
			}
		}
	}

	CHECK_STATUS(ddr3_tip_if_write
		     (dev_num, ACCESS_TYPE_MULTICAST, PARAM_NOT_CARE,
		      ODPG_DATA_CONTROL_REG, 0, MASK_ALL_BITS));

	return MV_OK;
}

/*
 * Load expected Pattern to ODPG
 */
int ddr3_tip_load_pattern_to_odpg(u32 dev_num, enum hws_access_type access_type,
				  u32 if_id, enum hws_pattern pattern,
				  u32 load_addr)
{
	u32 pattern_length_cnt = 0;
	struct pattern_info *pattern_table = ddr3_tip_get_pattern_table();

	for (pattern_length_cnt = 0;
	     pattern_length_cnt < pattern_table[pattern].pattern_len;
	     pattern_length_cnt++) {
		CHECK_STATUS(ddr3_tip_if_write
			     (dev_num, access_type, if_id,
			      ODPG_PATTERN_DATA_LOW_REG,
			      pattern_table_get_word(dev_num, pattern,
						     (u8) (pattern_length_cnt *
							   2)), MASK_ALL_BITS));
		CHECK_STATUS(ddr3_tip_if_write
			     (dev_num, access_type, if_id,
			      ODPG_PATTERN_DATA_HI_REG,
			      pattern_table_get_word(dev_num, pattern,
						     (u8) (pattern_length_cnt *
							   2 + 1)),
			      MASK_ALL_BITS));
		CHECK_STATUS(ddr3_tip_if_write
			     (dev_num, access_type, if_id,
			      ODPG_PATTERN_ADDR_REG, pattern_length_cnt,
			      MASK_ALL_BITS));
	}

	CHECK_STATUS(ddr3_tip_if_write
		     (dev_num, access_type, if_id,
		      ODPG_PATTERN_ADDR_OFFSET_REG, load_addr, MASK_ALL_BITS));

	return MV_OK;
}

/*
 * Configure ODPG
 */
int ddr3_tip_configure_odpg(u32 dev_num, enum hws_access_type access_type,
			    u32 if_id, enum hws_dir direction, u32 tx_phases,
			    u32 tx_burst_size, u32 rx_phases,
			    u32 delay_between_burst, u32 rd_mode, u32 cs_num,
			    u32 addr_stress_jump, u32 single_pattern)
{
	u32 data_value = 0;
	int ret;

	data_value = ((single_pattern << 2) | (tx_phases << 5) |
		      (tx_burst_size << 11) | (delay_between_burst << 15) |
		      (rx_phases << 21) | (rd_mode << 25) | (cs_num << 26) |
		      (addr_stress_jump << 29));
	ret = ddr3_tip_if_write(dev_num, access_type, if_id,
				ODPG_DATA_CONTROL_REG, data_value, 0xaffffffc);
	if (ret != MV_OK)
		return ret;

	return MV_OK;
}

int ddr3_tip_process_result(u32 *ar_result, enum hws_edge e_edge,
			    enum hws_edge_search e_edge_search,
			    u32 *edge_result)
{
	u32 i, res;
	int tap_val, max_val = -10000, min_val = 10000;
	int lock_success = 1;

	for (i = 0; i < BUS_WIDTH_IN_BITS; i++) {
		res = GET_LOCK_RESULT(ar_result[i]);
		if (res == 0) {
			lock_success = 0;
			break;
		}
		DEBUG_TRAINING_IP_ENGINE(DEBUG_LEVEL_ERROR,
					 ("lock failed for bit %d\n", i));
	}

	if (lock_success == 1) {
		for (i = 0; i < BUS_WIDTH_IN_BITS; i++) {
			tap_val = GET_TAP_RESULT(ar_result[i], e_edge);
			if (tap_val > max_val)
				max_val = tap_val;
			if (tap_val < min_val)
				min_val = tap_val;
			if (e_edge_search == TRAINING_EDGE_MAX)
				*edge_result = (u32) max_val;
			else
				*edge_result = (u32) min_val;

			DEBUG_TRAINING_IP_ENGINE(DEBUG_LEVEL_ERROR,
						 ("i %d ar_result[i] 0x%x tap_val %d max_val %d min_val %d Edge_result %d\n",
						  i, ar_result[i], tap_val,
						  max_val, min_val,
						  *edge_result));
		}
	} else {
		return MV_FAIL;
	}

	return MV_OK;
}

/*
 * Read training search result
 */
int ddr3_tip_read_training_result(u32 dev_num, u32 if_id,
				  enum hws_access_type pup_access_type,
				  u32 pup_num, u32 bit_num,
				  enum hws_search_dir search,
				  enum hws_dir direction,
				  enum hws_training_result result_type,
				  enum hws_training_load_op operation,
				  u32 cs_num_type, u32 **load_res,
				  int is_read_from_db, u8 cons_tap,
				  int is_check_result_validity)
{
	u32 reg_offset, pup_cnt, start_pup, end_pup, start_reg, end_reg;
	u32 *interface_train_res = NULL;
	u16 *reg_addr = NULL;
	u32 read_data[MAX_INTERFACE_NUM];
	u16 *mask_results_pup_reg_map = ddr3_tip_get_mask_results_pup_reg_map();
	u16 *mask_results_dq_reg_map = ddr3_tip_get_mask_results_dq_reg();
	struct hws_topology_map *tm = ddr3_get_topology_map();

	/*
	 * Agreed assumption: all CS mask contain same number of bits,
	 * i.e. in multi CS, the number of CS per memory is the same for
	 * all pups
	 */
	CHECK_STATUS(ddr3_tip_if_write
		     (dev_num, ACCESS_TYPE_UNICAST, if_id, CS_ENABLE_REG,
		      (cs_num_type == 0) ? 1 << 3 : 0, (1 << 3)));
	CHECK_STATUS(ddr3_tip_if_write
		     (dev_num, ACCESS_TYPE_UNICAST, if_id,
		      ODPG_DATA_CONTROL_REG, (cs_num_type << 26), (3 << 26)));
	DEBUG_TRAINING_IP_ENGINE(DEBUG_LEVEL_TRACE,
				 ("Read_from_d_b %d cs_type %d oper %d result_type %d direction %d search %d pup_num %d if_id %d pup_access_type %d\n",
				  is_read_from_db, cs_num_type, operation,
				  result_type, direction, search, pup_num,
				  if_id, pup_access_type));

	if ((load_res == NULL) && (is_read_from_db == 1)) {
		DEBUG_TRAINING_IP_ENGINE(DEBUG_LEVEL_ERROR,
					 ("ddr3_tip_read_training_result load_res = NULL"));
		return MV_FAIL;
	}
	if (pup_num >= tm->num_of_bus_per_interface) {
		DEBUG_TRAINING_IP_ENGINE(DEBUG_LEVEL_ERROR,
					 ("pup_num %d not valid\n", pup_num));
	}
	if (if_id >= MAX_INTERFACE_NUM) {
		DEBUG_TRAINING_IP_ENGINE(DEBUG_LEVEL_ERROR,
					 ("if_id %d not valid\n", if_id));
	}
	if (result_type == RESULT_PER_BIT)
		reg_addr = mask_results_dq_reg_map;
	else
		reg_addr = mask_results_pup_reg_map;
	if (pup_access_type == ACCESS_TYPE_UNICAST) {
		start_pup = pup_num;
		end_pup = pup_num;
	} else {		/*pup_access_type == ACCESS_TYPE_MULTICAST) */

		start_pup = 0;
		end_pup = tm->num_of_bus_per_interface - 1;
	}

	for (pup_cnt = start_pup; pup_cnt <= end_pup; pup_cnt++) {
		VALIDATE_ACTIVE(tm->bus_act_mask, pup_cnt);
		DEBUG_TRAINING_IP_ENGINE(
			DEBUG_LEVEL_TRACE,
			("if_id %d start_pup %d end_pup %d pup_cnt %d\n",
			 if_id, start_pup, end_pup, pup_cnt));
		if (result_type == RESULT_PER_BIT) {
			if (bit_num == ALL_BITS_PER_PUP) {
				start_reg = pup_cnt * BUS_WIDTH_IN_BITS;
				end_reg = (pup_cnt + 1) * BUS_WIDTH_IN_BITS - 1;
			} else {
				start_reg =
					pup_cnt * BUS_WIDTH_IN_BITS + bit_num;
				end_reg = pup_cnt * BUS_WIDTH_IN_BITS + bit_num;
			}
		} else {
			start_reg = pup_cnt;
			end_reg = pup_cnt;
		}

		interface_train_res =
			ddr3_tip_get_buf_ptr(dev_num, search, result_type,
					     if_id);
		DEBUG_TRAINING_IP_ENGINE(
			DEBUG_LEVEL_TRACE,
			("start_reg %d end_reg %d interface %p\n",
			 start_reg, end_reg, interface_train_res));
		if (interface_train_res == NULL) {
			DEBUG_TRAINING_IP_ENGINE(
				DEBUG_LEVEL_ERROR,
				("interface_train_res is NULL\n"));
			return MV_FAIL;
		}

		for (reg_offset = start_reg; reg_offset <= end_reg;
		     reg_offset++) {
			if (operation == TRAINING_LOAD_OPERATION_UNLOAD) {
				if (is_read_from_db == 0) {
					CHECK_STATUS(ddr3_tip_if_read
						     (dev_num,
						      ACCESS_TYPE_UNICAST,
						      if_id,
						      reg_addr[reg_offset],
						      read_data,
						      MASK_ALL_BITS));
					if (is_check_result_validity == 1) {
						if ((read_data[if_id] &
						     0x02000000) == 0) {
							interface_train_res
								[reg_offset] =
								0x02000000 +
								64 + cons_tap;
						} else {
							interface_train_res
								[reg_offset] =
								read_data
								[if_id] +
								cons_tap;
						}
					} else {
						interface_train_res[reg_offset]
							= read_data[if_id] +
							cons_tap;
					}
					DEBUG_TRAINING_IP_ENGINE
						(DEBUG_LEVEL_TRACE,
						 ("reg_offset %d value 0x%x addr %p\n",
						  reg_offset,
						  interface_train_res
						  [reg_offset],
						  &interface_train_res
						  [reg_offset]));
				} else {
					*load_res =
						&interface_train_res[start_reg];
					DEBUG_TRAINING_IP_ENGINE
						(DEBUG_LEVEL_TRACE,
						 ("*load_res %p\n", *load_res));
				}
			} else {
				DEBUG_TRAINING_IP_ENGINE(DEBUG_LEVEL_TRACE,
							 ("not supported\n"));
			}
		}
	}

	return MV_OK;
}

/*
 * Load all pattern to memory using ODPG
 */
int ddr3_tip_load_all_pattern_to_mem(u32 dev_num)
{
	u32 pattern = 0, if_id;
	struct hws_topology_map *tm = ddr3_get_topology_map();

	for (if_id = 0; if_id <= MAX_INTERFACE_NUM - 1; if_id++) {
		VALIDATE_ACTIVE(tm->if_act_mask, if_id);
		training_result[training_stage][if_id] = TEST_SUCCESS;
	}

	for (if_id = 0; if_id <= MAX_INTERFACE_NUM - 1; if_id++) {
		VALIDATE_ACTIVE(tm->if_act_mask, if_id);
		/* enable single cs */
		CHECK_STATUS(ddr3_tip_if_write
			     (dev_num, ACCESS_TYPE_UNICAST, if_id,
			      CS_ENABLE_REG, (1 << 3), (1 << 3)));
	}

	for (pattern = 0; pattern < PATTERN_LIMIT; pattern++)
		ddr3_tip_load_pattern_to_mem(dev_num, pattern);

	return MV_OK;
}

/*
 * Wait till ODPG access is ready
 */
int is_odpg_access_done(u32 dev_num, u32 if_id)
{
	u32 poll_cnt = 0, data_value;
	u32 read_data[MAX_INTERFACE_NUM];

	for (poll_cnt = 0; poll_cnt < MAX_POLLING_ITERATIONS; poll_cnt++) {
		CHECK_STATUS(ddr3_tip_if_read
			     (dev_num, ACCESS_TYPE_UNICAST, if_id,
			      ODPG_BIST_DONE, read_data, MASK_ALL_BITS));
		data_value = read_data[if_id];
		if (((data_value >> ODPG_BIST_DONE_BIT_OFFS) & 0x1) ==
		    ODPG_BIST_DONE_BIT_VALUE) {
				data_value = data_value & 0xfffffffe;
				CHECK_STATUS(ddr3_tip_if_write
					     (dev_num, ACCESS_TYPE_UNICAST,
					      if_id, ODPG_BIST_DONE, data_value,
					      MASK_ALL_BITS));
				break;
			}
	}

	if (poll_cnt >= MAX_POLLING_ITERATIONS) {
		DEBUG_TRAINING_IP_ENGINE(DEBUG_LEVEL_ERROR,
					 ("Bist Activate: poll failure 2\n"));
		return MV_FAIL;
	}

	return MV_OK;
}

/*
 * Load specific pattern to memory using ODPG
 */
int ddr3_tip_load_pattern_to_mem(u32 dev_num, enum hws_pattern pattern)
{
	u32 reg_data, if_id;
	struct pattern_info *pattern_table = ddr3_tip_get_pattern_table();
	struct hws_topology_map *tm = ddr3_get_topology_map();

	/* load pattern to memory */
	/*
	 * Write Tx mode, CS0, phases, Tx burst size, delay between burst,
	 * rx pattern phases
	 */
	reg_data =
		0x1 | (pattern_table[pattern].num_of_phases_tx << 5) |
		(pattern_table[pattern].tx_burst_size << 11) |
		(pattern_table[pattern].delay_between_bursts << 15) |
		(pattern_table[pattern].num_of_phases_rx << 21) | (0x1 << 25) |
		(effective_cs << 26);
	CHECK_STATUS(ddr3_tip_if_write
		     (dev_num, ACCESS_TYPE_MULTICAST, PARAM_NOT_CARE,
		      ODPG_DATA_CONTROL_REG, reg_data, MASK_ALL_BITS));
	/* ODPG Write enable from BIST */
	CHECK_STATUS(ddr3_tip_if_write
		     (dev_num, ACCESS_TYPE_MULTICAST, PARAM_NOT_CARE,
		      ODPG_DATA_CONTROL_REG, (0x1 | (effective_cs << 26)),
		      0xc000003));
	/* disable error injection */
	CHECK_STATUS(ddr3_tip_if_write
		     (dev_num, ACCESS_TYPE_MULTICAST, PARAM_NOT_CARE,
		      ODPG_WRITE_DATA_ERROR_REG, 0, 0x1));
	/* load pattern to ODPG */
	ddr3_tip_load_pattern_to_odpg(dev_num, ACCESS_TYPE_MULTICAST,
				      PARAM_NOT_CARE, pattern,
				      pattern_table[pattern].start_addr);

	for (if_id = 0; if_id < MAX_INTERFACE_NUM; if_id++) {
		if (IS_ACTIVE(tm->if_act_mask, if_id) == 0)
			continue;

		CHECK_STATUS(ddr3_tip_if_write
			     (dev_num, ACCESS_TYPE_UNICAST, if_id, 0x1498,
			      0x3, 0xf));
	}

	CHECK_STATUS(ddr3_tip_if_write
		     (dev_num, ACCESS_TYPE_MULTICAST, PARAM_NOT_CARE,
		      ODPG_ENABLE_REG, 0x1 << ODPG_ENABLE_OFFS,
		      (0x1 << ODPG_ENABLE_OFFS)));

	mdelay(1);

	for (if_id = 0; if_id <= MAX_INTERFACE_NUM - 1; if_id++) {
		VALIDATE_ACTIVE(tm->if_act_mask, if_id);
		CHECK_STATUS(is_odpg_access_done(dev_num, if_id));
	}

	/* Disable ODPG and stop write to memory */
	CHECK_STATUS(ddr3_tip_if_write
		     (dev_num, ACCESS_TYPE_MULTICAST, PARAM_NOT_CARE,
		      ODPG_DATA_CONTROL_REG, (0x1 << 30), (u32) (0x3 << 30)));

	/* return to default */
	CHECK_STATUS(ddr3_tip_if_write
		     (dev_num, ACCESS_TYPE_MULTICAST, PARAM_NOT_CARE,
		      ODPG_DATA_CONTROL_REG, 0, MASK_ALL_BITS));

	/* Disable odt0 for CS0 training - need to adjust for multy CS */
	CHECK_STATUS(ddr3_tip_if_write
		     (dev_num, ACCESS_TYPE_MULTICAST, PARAM_NOT_CARE, 0x1498,
		      0x0, 0xf));

	/* temporary added */
	mdelay(1);

	return MV_OK;
}

/*
 * Load specific pattern to memory using CPU
 */
int ddr3_tip_load_pattern_to_mem_by_cpu(u32 dev_num, enum hws_pattern pattern,
					u32 offset)
{
	/* eranba - TBD */
	return MV_OK;
}

/*
 * Training search routine
 */
int ddr3_tip_ip_training_wrapper_int(u32 dev_num,
				     enum hws_access_type access_type,
				     u32 if_id,
				     enum hws_access_type pup_access_type,
				     u32 pup_num, u32 bit_num,
				     enum hws_training_result result_type,
				     enum hws_control_element control_element,
				     enum hws_search_dir search_dir,
				     enum hws_dir direction,
				     u32 interface_mask, u32 init_value_l2h,
				     u32 init_value_h2l, u32 num_iter,
				     enum hws_pattern pattern,
				     enum hws_edge_compare edge_comp,
				     enum hws_ddr_cs train_cs_type, u32 cs_num,
				     enum hws_training_ip_stat *train_status)
{
	u32 interface_num = 0, start_if, end_if, init_value_used;
	enum hws_search_dir search_dir_id, start_search, end_search;
	enum hws_edge_compare edge_comp_used;
	u8 cons_tap = (direction == OPER_WRITE) ? (64) : (0);
	struct hws_topology_map *tm = ddr3_get_topology_map();

	if (train_status == NULL) {
		DEBUG_TRAINING_IP_ENGINE(DEBUG_LEVEL_ERROR,
					 ("train_status is NULL\n"));
		return MV_FAIL;
	}

	if ((train_cs_type > CS_NON_SINGLE) ||
	    (edge_comp >= EDGE_PFP) ||
	    (pattern >= PATTERN_LIMIT) ||
	    (direction > OPER_WRITE_AND_READ) ||
	    (search_dir > HWS_HIGH2LOW) ||
	    (control_element > HWS_CONTROL_ELEMENT_DQS_SKEW) ||
	    (result_type > RESULT_PER_BYTE) ||
	    (pup_num >= tm->num_of_bus_per_interface) ||
	    (pup_access_type > ACCESS_TYPE_MULTICAST) ||
	    (if_id > 11) || (access_type > ACCESS_TYPE_MULTICAST)) {
		DEBUG_TRAINING_IP_ENGINE(
			DEBUG_LEVEL_ERROR,
			("wrong parameter train_cs_type %d edge_comp %d pattern %d direction %d search_dir %d control_element %d result_type %d pup_num %d pup_access_type %d if_id %d access_type %d\n",
			 train_cs_type, edge_comp, pattern, direction,
			 search_dir, control_element, result_type, pup_num,
			 pup_access_type, if_id, access_type));
		return MV_FAIL;
	}

	if (edge_comp == EDGE_FPF) {
		start_search = HWS_LOW2HIGH;
		end_search = HWS_HIGH2LOW;
		edge_comp_used = EDGE_FP;
	} else {
		start_search = search_dir;
		end_search = search_dir;
		edge_comp_used = edge_comp;
	}

	for (search_dir_id = start_search; search_dir_id <= end_search;
	     search_dir_id++) {
		init_value_used = (search_dir_id == HWS_LOW2HIGH) ?
			init_value_l2h : init_value_h2l;
		DEBUG_TRAINING_IP_ENGINE(
			DEBUG_LEVEL_TRACE,
			("dev_num %d, access_type %d, if_id %d, pup_access_type %d,pup_num %d, result_type %d, control_element %d search_dir_id %d, direction %d, interface_mask %d,init_value_used %d, num_iter %d, pattern %d, edge_comp_used %d, train_cs_type %d, cs_num %d\n",
			 dev_num, access_type, if_id, pup_access_type, pup_num,
			 result_type, control_element, search_dir_id,
			 direction, interface_mask, init_value_used, num_iter,
			 pattern, edge_comp_used, train_cs_type, cs_num));

		ddr3_tip_ip_training(dev_num, access_type, if_id,
				     pup_access_type, pup_num, result_type,
				     control_element, search_dir_id, direction,
				     interface_mask, init_value_used, num_iter,
				     pattern, edge_comp_used, train_cs_type,
				     cs_num, train_status);
		if (access_type == ACCESS_TYPE_MULTICAST) {
			start_if = 0;
			end_if = MAX_INTERFACE_NUM - 1;
		} else {
			start_if = if_id;
			end_if = if_id;
		}

		for (interface_num = start_if; interface_num <= end_if;
		     interface_num++) {
			VALIDATE_ACTIVE(tm->if_act_mask, interface_num);
			cs_num = 0;
			CHECK_STATUS(ddr3_tip_read_training_result
				     (dev_num, interface_num, pup_access_type,
				      pup_num, bit_num, search_dir_id,
				      direction, result_type,
				      TRAINING_LOAD_OPERATION_UNLOAD,
				      train_cs_type, NULL, 0, cons_tap,
				      0));
		}
	}

	return MV_OK;
}

/*
 * Training search & read result routine
 */
int ddr3_tip_ip_training_wrapper(u32 dev_num, enum hws_access_type access_type,
				 u32 if_id,
				 enum hws_access_type pup_access_type,
				 u32 pup_num,
				 enum hws_training_result result_type,
				 enum hws_control_element control_element,
				 enum hws_search_dir search_dir,
				 enum hws_dir direction, u32 interface_mask,
				 u32 init_value_l2h, u32 init_value_h2l,
				 u32 num_iter, enum hws_pattern pattern,
				 enum hws_edge_compare edge_comp,
				 enum hws_ddr_cs train_cs_type, u32 cs_num,
				 enum hws_training_ip_stat *train_status)
{
	u8 e1, e2;
	u32 interface_cnt, bit_id, start_if, end_if, bit_end = 0;
	u32 *result[HWS_SEARCH_DIR_LIMIT] = { 0 };
	u8 cons_tap = (direction == OPER_WRITE) ? (64) : (0);
	u8 bit_bit_mask[MAX_BUS_NUM] = { 0 }, bit_bit_mask_active = 0;
	u8 pup_id;
	struct hws_topology_map *tm = ddr3_get_topology_map();

	if (pup_num >= tm->num_of_bus_per_interface) {
		DEBUG_TRAINING_IP_ENGINE(DEBUG_LEVEL_ERROR,
					 ("pup_num %d not valid\n", pup_num));
	}

	if (if_id >= MAX_INTERFACE_NUM) {
		DEBUG_TRAINING_IP_ENGINE(DEBUG_LEVEL_ERROR,
					 ("if_id %d not valid\n", if_id));
	}

	CHECK_STATUS(ddr3_tip_ip_training_wrapper_int
		     (dev_num, access_type, if_id, pup_access_type, pup_num,
		      ALL_BITS_PER_PUP, result_type, control_element,
		      search_dir, direction, interface_mask, init_value_l2h,
		      init_value_h2l, num_iter, pattern, edge_comp,
		      train_cs_type, cs_num, train_status));

	if (access_type == ACCESS_TYPE_MULTICAST) {
		start_if = 0;
		end_if = MAX_INTERFACE_NUM - 1;
	} else {
		start_if = if_id;
		end_if = if_id;
	}

	for (interface_cnt = start_if; interface_cnt <= end_if;
	     interface_cnt++) {
		VALIDATE_ACTIVE(tm->if_act_mask, interface_cnt);
		for (pup_id = 0;
		     pup_id <= (tm->num_of_bus_per_interface - 1); pup_id++) {
			VALIDATE_ACTIVE(tm->bus_act_mask, pup_id);
			if (result_type == RESULT_PER_BIT)
				bit_end = BUS_WIDTH_IN_BITS - 1;
			else
				bit_end = 0;

			bit_bit_mask[pup_id] = 0;
			for (bit_id = 0; bit_id <= bit_end; bit_id++) {
				enum hws_search_dir search_dir_id;
				for (search_dir_id = HWS_LOW2HIGH;
				     search_dir_id <= HWS_HIGH2LOW;
				     search_dir_id++) {
					CHECK_STATUS
						(ddr3_tip_read_training_result
						 (dev_num, interface_cnt,
						  ACCESS_TYPE_UNICAST, pup_id,
						  bit_id, search_dir_id,
						  direction, result_type,
						  TRAINING_LOAD_OPERATION_UNLOAD,
						  CS_SINGLE,
						  &result[search_dir_id],
						  1, 0, 0));
				}
				e1 = GET_TAP_RESULT(result[HWS_LOW2HIGH][0],
						    EDGE_1);
				e2 = GET_TAP_RESULT(result[HWS_HIGH2LOW][0],
						    EDGE_1);
				DEBUG_TRAINING_IP_ENGINE(
					DEBUG_LEVEL_INFO,
					("wrapper if_id %d pup_id %d bit %d l2h 0x%x (e1 0x%x) h2l 0x%x (e2 0x%x)\n",
					 interface_cnt, pup_id, bit_id,
					 result[HWS_LOW2HIGH][0], e1,
					 result[HWS_HIGH2LOW][0], e2));
				/* TBD validate is valid only for tx */
				if (VALIDATE_TRAINING_LIMIT(e1, e2) == 1 &&
				    GET_LOCK_RESULT(result[HWS_LOW2HIGH][0]) &&
				    GET_LOCK_RESULT(result[HWS_LOW2HIGH][0])) {
					/* Mark problem bits */
					bit_bit_mask[pup_id] |= 1 << bit_id;
					bit_bit_mask_active = 1;
				}
			}	/* For all bits */
		}		/* For all PUPs */

		/* Fix problem bits */
		if (bit_bit_mask_active != 0) {
			u32 *l2h_if_train_res = NULL;
			u32 *h2l_if_train_res = NULL;
			l2h_if_train_res =
				ddr3_tip_get_buf_ptr(dev_num, HWS_LOW2HIGH,
						     result_type,
						     interface_cnt);
			h2l_if_train_res =
				ddr3_tip_get_buf_ptr(dev_num, HWS_HIGH2LOW,
						     result_type,
						     interface_cnt);

			ddr3_tip_ip_training(dev_num, ACCESS_TYPE_UNICAST,
					     interface_cnt,
					     ACCESS_TYPE_MULTICAST,
					     PARAM_NOT_CARE, result_type,
					     control_element, HWS_LOW2HIGH,
					     direction, interface_mask,
					     num_iter / 2, num_iter / 2,
					     pattern, EDGE_FP, train_cs_type,
					     cs_num, train_status);

			for (pup_id = 0;
			     pup_id <= (tm->num_of_bus_per_interface - 1);
			     pup_id++) {
				VALIDATE_ACTIVE(tm->bus_act_mask, pup_id);

				if (bit_bit_mask[pup_id] == 0)
					continue;

				for (bit_id = 0; bit_id <= bit_end; bit_id++) {
					if ((bit_bit_mask[pup_id] &
					     (1 << bit_id)) == 0)
						continue;
					CHECK_STATUS
						(ddr3_tip_read_training_result
						 (dev_num, interface_cnt,
						  ACCESS_TYPE_UNICAST, pup_id,
						  bit_id, HWS_LOW2HIGH,
						  direction,
						  result_type,
						  TRAINING_LOAD_OPERATION_UNLOAD,
						  CS_SINGLE, &l2h_if_train_res,
						  0, 0, 1));
				}
			}

			ddr3_tip_ip_training(dev_num, ACCESS_TYPE_UNICAST,
					     interface_cnt,
					     ACCESS_TYPE_MULTICAST,
					     PARAM_NOT_CARE, result_type,
					     control_element, HWS_HIGH2LOW,
					     direction, interface_mask,
					     num_iter / 2, num_iter / 2,
					     pattern, EDGE_FP, train_cs_type,
					     cs_num, train_status);

			for (pup_id = 0;
			     pup_id <= (tm->num_of_bus_per_interface - 1);
			     pup_id++) {
				VALIDATE_ACTIVE(tm->bus_act_mask, pup_id);

				if (bit_bit_mask[pup_id] == 0)
					continue;

				for (bit_id = 0; bit_id <= bit_end; bit_id++) {
					if ((bit_bit_mask[pup_id] &
					     (1 << bit_id)) == 0)
						continue;
					CHECK_STATUS
						(ddr3_tip_read_training_result
						 (dev_num, interface_cnt,
						  ACCESS_TYPE_UNICAST, pup_id,
						  bit_id, HWS_HIGH2LOW, direction,
						  result_type,
						  TRAINING_LOAD_OPERATION_UNLOAD,
						  CS_SINGLE, &h2l_if_train_res,
						  0, cons_tap, 1));
				}
			}
		}		/* if bit_bit_mask_active */
	}			/* For all Interfacess */

	return MV_OK;
}

/*
 * Load phy values
 */
int ddr3_tip_load_phy_values(int b_load)
{
	u32 bus_cnt = 0, if_id, dev_num = 0;
	struct hws_topology_map *tm = ddr3_get_topology_map();

	for (if_id = 0; if_id <= MAX_INTERFACE_NUM - 1; if_id++) {
		VALIDATE_ACTIVE(tm->if_act_mask, if_id);
		for (bus_cnt = 0; bus_cnt < GET_TOPOLOGY_NUM_OF_BUSES();
		     bus_cnt++) {
			VALIDATE_ACTIVE(tm->bus_act_mask, bus_cnt);
			if (b_load == 1) {
				CHECK_STATUS(ddr3_tip_bus_read
					     (dev_num, if_id,
					      ACCESS_TYPE_UNICAST, bus_cnt,
					      DDR_PHY_DATA,
					      WRITE_CENTRALIZATION_PHY_REG +
					      (effective_cs *
					       CS_REGISTER_ADDR_OFFSET),
					      &phy_reg_bk[if_id][bus_cnt]
					      [0]));
				CHECK_STATUS(ddr3_tip_bus_read
					     (dev_num, if_id,
					      ACCESS_TYPE_UNICAST, bus_cnt,
					      DDR_PHY_DATA,
					      RL_PHY_REG +
					      (effective_cs *
					       CS_REGISTER_ADDR_OFFSET),
					      &phy_reg_bk[if_id][bus_cnt]
					      [1]));
				CHECK_STATUS(ddr3_tip_bus_read
					     (dev_num, if_id,
					      ACCESS_TYPE_UNICAST, bus_cnt,
					      DDR_PHY_DATA,
					      READ_CENTRALIZATION_PHY_REG +
					      (effective_cs *
					       CS_REGISTER_ADDR_OFFSET),
					      &phy_reg_bk[if_id][bus_cnt]
					      [2]));
			} else {
				CHECK_STATUS(ddr3_tip_bus_write
					     (dev_num, ACCESS_TYPE_UNICAST,
					      if_id, ACCESS_TYPE_UNICAST,
					      bus_cnt, DDR_PHY_DATA,
					      WRITE_CENTRALIZATION_PHY_REG +
					      (effective_cs *
					       CS_REGISTER_ADDR_OFFSET),
					      phy_reg_bk[if_id][bus_cnt]
					      [0]));
				CHECK_STATUS(ddr3_tip_bus_write
					     (dev_num, ACCESS_TYPE_UNICAST,
					      if_id, ACCESS_TYPE_UNICAST,
					      bus_cnt, DDR_PHY_DATA,
					      RL_PHY_REG +
					      (effective_cs *
					       CS_REGISTER_ADDR_OFFSET),
					      phy_reg_bk[if_id][bus_cnt]
					      [1]));
				CHECK_STATUS(ddr3_tip_bus_write
					     (dev_num, ACCESS_TYPE_UNICAST,
					      if_id, ACCESS_TYPE_UNICAST,
					      bus_cnt, DDR_PHY_DATA,
					      READ_CENTRALIZATION_PHY_REG +
					      (effective_cs *
					       CS_REGISTER_ADDR_OFFSET),
					      phy_reg_bk[if_id][bus_cnt]
					      [2]));
			}
		}
	}

	return MV_OK;
}

int ddr3_tip_training_ip_test(u32 dev_num, enum hws_training_result result_type,
			      enum hws_search_dir search_dir,
			      enum hws_dir direction,
			      enum hws_edge_compare edge,
			      u32 init_val1, u32 init_val2,
			      u32 num_of_iterations,
			      u32 start_pattern, u32 end_pattern)
{
	u32 pattern, if_id, pup_id;
	enum hws_training_ip_stat train_status[MAX_INTERFACE_NUM];
	u32 *res = NULL;
	u32 search_state = 0;
	struct hws_topology_map *tm = ddr3_get_topology_map();

	ddr3_tip_load_phy_values(1);

	for (pattern = start_pattern; pattern <= end_pattern; pattern++) {
		for (search_state = 0; search_state < HWS_SEARCH_DIR_LIMIT;
		     search_state++) {
			ddr3_tip_ip_training_wrapper(dev_num,
						     ACCESS_TYPE_MULTICAST, 0,
						     ACCESS_TYPE_MULTICAST, 0,
						     result_type,
						     HWS_CONTROL_ELEMENT_ADLL,
						     search_dir, direction,
						     0xfff, init_val1,
						     init_val2,
						     num_of_iterations, pattern,
						     edge, CS_SINGLE,
						     PARAM_NOT_CARE,
						     train_status);

			for (if_id = 0; if_id <= MAX_INTERFACE_NUM - 1;
			     if_id++) {
				VALIDATE_ACTIVE(tm->if_act_mask, if_id);
				for (pup_id = 0; pup_id <
					     tm->num_of_bus_per_interface;
				     pup_id++) {
					VALIDATE_ACTIVE(tm->bus_act_mask,
							pup_id);
					CHECK_STATUS
						(ddr3_tip_read_training_result
						 (dev_num, if_id,
						  ACCESS_TYPE_UNICAST, pup_id,
						  ALL_BITS_PER_PUP,
						  search_state,
						  direction, result_type,
						  TRAINING_LOAD_OPERATION_UNLOAD,
						  CS_SINGLE, &res, 1, 0,
						  0));
					if (result_type == RESULT_PER_BYTE) {
						DEBUG_TRAINING_IP_ENGINE
							(DEBUG_LEVEL_INFO,
							 ("search_state %d if_id %d pup_id %d 0x%x\n",
							  search_state, if_id,
							  pup_id, res[0]));
					} else {
						DEBUG_TRAINING_IP_ENGINE
							(DEBUG_LEVEL_INFO,
							 ("search_state %d if_id %d pup_id %d 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x\n",
							  search_state, if_id,
							  pup_id, res[0],
							  res[1], res[2],
							  res[3], res[4],
							  res[5], res[6],
							  res[7]));
					}
				}
			}	/* interface */
		}		/* search */
	}			/* pattern */

	ddr3_tip_load_phy_values(0);

	return MV_OK;
}

struct pattern_info *ddr3_tip_get_pattern_table()
{
	struct hws_topology_map *tm = ddr3_get_topology_map();

	if (DDR3_IS_16BIT_DRAM_MODE(tm->bus_act_mask) == 0)
		return pattern_table_32;
	else
		return pattern_table_16;
}

u16 *ddr3_tip_get_mask_results_dq_reg()
{
	struct hws_topology_map *tm = ddr3_get_topology_map();

	if (DDR3_IS_ECC_PUP3_MODE(tm->bus_act_mask))
		return mask_results_dq_reg_map_pup3_ecc;
	else
		return mask_results_dq_reg_map;
}

u16 *ddr3_tip_get_mask_results_pup_reg_map()
{
	struct hws_topology_map *tm = ddr3_get_topology_map();

	if (DDR3_IS_ECC_PUP3_MODE(tm->bus_act_mask))
		return mask_results_pup_reg_map_pup3_ecc;
	else
		return mask_results_pup_reg_map;
}
