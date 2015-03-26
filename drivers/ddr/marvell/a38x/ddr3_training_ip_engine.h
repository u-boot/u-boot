/*
 * Copyright (C) Marvell International Ltd. and its affiliates
 *
 * SPDX-License-Identifier:	GPL-2.0
 */

#ifndef _DDR3_TRAINING_IP_ENGINE_H_
#define _DDR3_TRAINING_IP_ENGINE_H_

#include "ddr3_training_ip_def.h"
#include "ddr3_training_ip_flow.h"

#define EDGE_1				0
#define EDGE_2				1
#define ALL_PUP_TRAINING		0xe
#define PUP_RESULT_EDGE_1_MASK		0xff
#define PUP_RESULT_EDGE_2_MASK		(0xff << 8)
#define PUP_LOCK_RESULT_BIT		25

#define GET_TAP_RESULT(reg, edge)				 \
	(((edge) == EDGE_1) ? ((reg) & PUP_RESULT_EDGE_1_MASK) : \
	 (((reg) & PUP_RESULT_EDGE_2_MASK) >> 8));
#define GET_LOCK_RESULT(reg)						\
	(((reg) & (1<<PUP_LOCK_RESULT_BIT)) >> PUP_LOCK_RESULT_BIT)

#define EDGE_FAILURE			128
#define ALL_BITS_PER_PUP		128

#define MIN_WINDOW_SIZE			6
#define MAX_WINDOW_SIZE_RX		32
#define MAX_WINDOW_SIZE_TX		64

int ddr3_tip_training_ip_test(u32 dev_num, enum hws_training_result result_type,
			      enum hws_search_dir search_dir,
			      enum hws_dir direction,
			      enum hws_edge_compare edge,
			      u32 init_val1, u32 init_val2,
			      u32 num_of_iterations, u32 start_pattern,
			      u32 end_pattern);
int ddr3_tip_load_pattern_to_mem(u32 dev_num, enum hws_pattern pattern);
int ddr3_tip_load_pattern_to_mem_by_cpu(u32 dev_num, enum hws_pattern pattern,
					u32 offset);
int ddr3_tip_load_all_pattern_to_mem(u32 dev_num);
int ddr3_tip_read_training_result(u32 dev_num, u32 if_id,
				  enum hws_access_type pup_access_type,
				  u32 pup_num, u32 bit_num,
				  enum hws_search_dir search,
				  enum hws_dir direction,
				  enum hws_training_result result_type,
				  enum hws_training_load_op operation,
				  u32 cs_num_type, u32 **load_res,
				  int is_read_from_db, u8 cons_tap,
				  int is_check_result_validity);
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
			 enum hws_training_ip_stat *train_status);
int ddr3_tip_ip_training_wrapper(u32 dev_num, enum hws_access_type access_type,
				 u32 if_id,
				 enum hws_access_type pup_access_type,
				 u32 pup_num,
				 enum hws_training_result result_type,
				 enum hws_control_element control_element,
				 enum hws_search_dir search_dir,
				 enum hws_dir direction,
				 u32 interface_mask, u32 init_value1,
				 u32 init_value2, u32 num_iter,
				 enum hws_pattern pattern,
				 enum hws_edge_compare edge_comp,
				 enum hws_ddr_cs train_cs_type, u32 cs_num,
				 enum hws_training_ip_stat *train_status);
int is_odpg_access_done(u32 dev_num, u32 if_id);
void ddr3_tip_print_bist_res(void);
struct pattern_info *ddr3_tip_get_pattern_table(void);
u16 *ddr3_tip_get_mask_results_dq_reg(void);
u16 *ddr3_tip_get_mask_results_pup_reg_map(void);

#endif /* _DDR3_TRAINING_IP_ENGINE_H_ */
