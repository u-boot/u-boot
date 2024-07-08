/*
 * Copyright (C) Marvell International Ltd. and its affiliates
 *
 * SPDX-License-Identifier:	GPL-2.0
 */

#include <spl.h>
#include <asm/io.h>
#include <asm/arch/cpu.h>
#include <asm/arch/soc.h>

#include "ddr3_init.h"

static u32 bist_offset = 32;
enum hws_pattern sweep_pattern = PATTERN_KILLER_DQ0;

static int ddr3_tip_bist_operation(u32 dev_num,
				   enum hws_access_type access_type,
				   u32 if_id,
				   enum hws_bist_operation oper_type);

/*
 * BIST activate
 */
int ddr3_tip_bist_activate(u32 dev_num, enum hws_pattern pattern,
			   enum hws_access_type access_type, u32 if_num,
			   enum hws_dir direction,
			   enum hws_stress_jump addr_stress_jump,
			   enum hws_pattern_duration duration,
			   enum hws_bist_operation oper_type,
			   u32 offset, u32 cs_num, u32 pattern_addr_length)
{
	u32 tx_burst_size;
	u32 delay_between_burst;
	u32 rd_mode, val;
	u32 poll_cnt = 0, max_poll = 1000, i, start_if, end_if;
	struct pattern_info *pattern_table = ddr3_tip_get_pattern_table();
	u32 read_data[MAX_INTERFACE_NUM];
	struct hws_topology_map *tm = ddr3_get_topology_map();

	/* ODPG Write enable from BIST */
	CHECK_STATUS(ddr3_tip_if_write(dev_num, access_type, if_num,
				       ODPG_DATA_CONTROL_REG, 0x1, 0x1));
	/* ODPG Read enable/disable from BIST */
	CHECK_STATUS(ddr3_tip_if_write(dev_num, access_type, if_num,
				       ODPG_DATA_CONTROL_REG,
				       (direction == OPER_READ) ?
				       0x2 : 0, 0x2));
	CHECK_STATUS(ddr3_tip_load_pattern_to_odpg(dev_num, access_type, if_num,
						   pattern, offset));

	CHECK_STATUS(ddr3_tip_if_write(dev_num, access_type, if_num,
				       ODPG_DATA_BUF_SIZE_REG,
				       pattern_addr_length, MASK_ALL_BITS));
	tx_burst_size = (direction == OPER_WRITE) ?
		pattern_table[pattern].tx_burst_size : 0;
	delay_between_burst = (direction == OPER_WRITE) ? 2 : 0;
	rd_mode = (direction == OPER_WRITE) ? 1 : 0;
	CHECK_STATUS(ddr3_tip_configure_odpg
		     (dev_num, access_type, if_num, direction,
		      pattern_table[pattern].num_of_phases_tx, tx_burst_size,
		      pattern_table[pattern].num_of_phases_rx,
		      delay_between_burst,
		      rd_mode, cs_num, addr_stress_jump, duration));
	CHECK_STATUS(ddr3_tip_if_write(dev_num, access_type, if_num,
				       ODPG_PATTERN_ADDR_OFFSET_REG,
				       offset, MASK_ALL_BITS));
	if (oper_type == BIST_STOP) {
		CHECK_STATUS(ddr3_tip_bist_operation(dev_num, access_type,
						     if_num, BIST_STOP));
	} else {
		CHECK_STATUS(ddr3_tip_bist_operation(dev_num, access_type,
						     if_num, BIST_START));
		if (duration != DURATION_CONT) {
			/*
			 * This pdelay is a WA, becuase polling fives "done"
			 * also the odpg did nmot finish its task
			 */
			if (access_type == ACCESS_TYPE_MULTICAST) {
				start_if = 0;
				end_if = MAX_INTERFACE_NUM - 1;
			} else {
				start_if = if_num;
				end_if = if_num;
			}

			for (i = start_if; i <= end_if; i++) {
				VALIDATE_ACTIVE(tm->
						   if_act_mask, i);

				for (poll_cnt = 0; poll_cnt < max_poll;
				     poll_cnt++) {
					CHECK_STATUS(ddr3_tip_if_read
						     (dev_num,
						      ACCESS_TYPE_UNICAST,
						      if_num, ODPG_BIST_DONE,
						      read_data,
						      MASK_ALL_BITS));
					val = read_data[i];
					if ((val & 0x1) == 0x0) {
						/*
						 * In SOC type devices this bit
						 * is self clear so, if it was
						 * cleared all good
						 */
						break;
					}
				}

				if (poll_cnt >= max_poll) {
					DEBUG_TRAINING_BIST_ENGINE
						(DEBUG_LEVEL_ERROR,
						 ("Bist poll failure 2\n"));
					CHECK_STATUS(ddr3_tip_if_write
						     (dev_num,
						      ACCESS_TYPE_UNICAST,
						      if_num,
						      ODPG_DATA_CONTROL_REG, 0,
						      MASK_ALL_BITS));
					return MV_FAIL;
				}
			}

			CHECK_STATUS(ddr3_tip_bist_operation
				     (dev_num, access_type, if_num, BIST_STOP));
		}
	}

	CHECK_STATUS(ddr3_tip_if_write(dev_num, access_type, if_num,
				       ODPG_DATA_CONTROL_REG, 0,
				       MASK_ALL_BITS));

	return MV_OK;
}

/*
 * BIST read result
 */
int ddr3_tip_bist_read_result(u32 dev_num, u32 if_id,
			      struct bist_result *pst_bist_result)
{
	int ret;
	u32 read_data[MAX_INTERFACE_NUM];
	struct hws_topology_map *tm = ddr3_get_topology_map();

	if (IS_ACTIVE(tm->if_act_mask, if_id) == 0)
		return MV_NOT_SUPPORTED;
	DEBUG_TRAINING_BIST_ENGINE(DEBUG_LEVEL_TRACE,
				   ("ddr3_tip_bist_read_result if_id %d\n",
				    if_id));
	ret = ddr3_tip_if_read(dev_num, ACCESS_TYPE_UNICAST, if_id,
			       ODPG_BIST_FAILED_DATA_HI_REG, read_data,
			       MASK_ALL_BITS);
	if (ret != MV_OK)
		return ret;
	pst_bist_result->bist_fail_high = read_data[if_id];
	ret = ddr3_tip_if_read(dev_num, ACCESS_TYPE_UNICAST, if_id,
			       ODPG_BIST_FAILED_DATA_LOW_REG, read_data,
			       MASK_ALL_BITS);
	if (ret != MV_OK)
		return ret;
	pst_bist_result->bist_fail_low = read_data[if_id];

	ret = ddr3_tip_if_read(dev_num, ACCESS_TYPE_UNICAST, if_id,
			       ODPG_BIST_LAST_FAIL_ADDR_REG, read_data,
			       MASK_ALL_BITS);
	if (ret != MV_OK)
		return ret;
	pst_bist_result->bist_last_fail_addr = read_data[if_id];
	ret = ddr3_tip_if_read(dev_num, ACCESS_TYPE_UNICAST, if_id,
			       ODPG_BIST_DATA_ERROR_COUNTER_REG, read_data,
			       MASK_ALL_BITS);
	if (ret != MV_OK)
		return ret;
	pst_bist_result->bist_error_cnt = read_data[if_id];

	return MV_OK;
}

/*
 * BIST flow - Activate & read result
 */
int hws_ddr3_run_bist(u32 dev_num, enum hws_pattern pattern, u32 *result,
		      u32 cs_num)
{
	int ret;
	u32 i = 0;
	u32 win_base;
	struct bist_result st_bist_result;
	struct hws_topology_map *tm = ddr3_get_topology_map();

	for (i = 0; i < MAX_INTERFACE_NUM; i++) {
		VALIDATE_ACTIVE(tm->if_act_mask, i);
		hws_ddr3_cs_base_adr_calc(i, cs_num, &win_base);
		ret = ddr3_tip_bist_activate(dev_num, pattern,
					     ACCESS_TYPE_UNICAST,
					     i, OPER_WRITE, STRESS_NONE,
					     DURATION_SINGLE, BIST_START,
					     bist_offset + win_base,
					     cs_num, 15);
		if (ret != MV_OK) {
			printf("ddr3_tip_bist_activate failed (0x%x)\n", ret);
			return ret;
		}

		ret = ddr3_tip_bist_activate(dev_num, pattern,
					     ACCESS_TYPE_UNICAST,
					     i, OPER_READ, STRESS_NONE,
					     DURATION_SINGLE, BIST_START,
					     bist_offset + win_base,
					     cs_num, 15);
		if (ret != MV_OK) {
			printf("ddr3_tip_bist_activate failed (0x%x)\n", ret);
			return ret;
		}

		ret = ddr3_tip_bist_read_result(dev_num, i, &st_bist_result);
		if (ret != MV_OK) {
			printf("ddr3_tip_bist_read_result failed\n");
			return ret;
		}
		result[i] = st_bist_result.bist_error_cnt;
	}

	return MV_OK;
}

/*
 * Set BIST Operation
 */

static int ddr3_tip_bist_operation(u32 dev_num,
				   enum hws_access_type access_type,
				   u32 if_id, enum hws_bist_operation oper_type)
{
	if (oper_type == BIST_STOP) {
		CHECK_STATUS(ddr3_tip_if_write(dev_num, access_type, if_id,
					       ODPG_BIST_DONE, 1 << 8, 1 << 8));
	} else {
		CHECK_STATUS(ddr3_tip_if_write(dev_num, access_type, if_id,
					       ODPG_BIST_DONE, 1, 1));
	}

	return MV_OK;
}

/*
 * Print BIST result
 */
void ddr3_tip_print_bist_res(void)
{
	u32 dev_num = 0;
	u32 i;
	struct bist_result st_bist_result[MAX_INTERFACE_NUM];
	int res;
	struct hws_topology_map *tm = ddr3_get_topology_map();

	for (i = 0; i < MAX_INTERFACE_NUM; i++) {
		if (IS_ACTIVE(tm->if_act_mask, i) == 0)
			continue;

		res = ddr3_tip_bist_read_result(dev_num, i, &st_bist_result[i]);
		if (res != MV_OK) {
			DEBUG_TRAINING_BIST_ENGINE(
				DEBUG_LEVEL_ERROR,
				("ddr3_tip_bist_read_result failed\n"));
			return;
		}
	}

	DEBUG_TRAINING_BIST_ENGINE(
		DEBUG_LEVEL_INFO,
		("interface | error_cnt | fail_low | fail_high | fail_addr\n"));

	for (i = 0; i < MAX_INTERFACE_NUM; i++) {
		if (IS_ACTIVE(tm->if_act_mask, i) ==
		    0)
			continue;

		DEBUG_TRAINING_BIST_ENGINE(
			DEBUG_LEVEL_INFO,
			("%d |  0x%08x  |  0x%08x  |  0x%08x  | 0x%08x\n",
			 i, st_bist_result[i].bist_error_cnt,
			 st_bist_result[i].bist_fail_low,
			 st_bist_result[i].bist_fail_high,
			 st_bist_result[i].bist_last_fail_addr));
	}
}
