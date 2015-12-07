/*
 * Copyright (C) Marvell International Ltd. and its affiliates
 *
 * SPDX-License-Identifier:	GPL-2.0
 */

#ifndef _DDR3_TRAINING_IP_H_
#define _DDR3_TRAINING_IP_H_

#include "ddr3_training_ip_def.h"
#include "ddr_topology_def.h"
#include "ddr_training_ip_db.h"

#define DDR3_TIP_VERSION_STRING "DDR3 Training Sequence - Ver TIP-1.29."

#define MAX_CS_NUM		4
#define MAX_TOTAL_BUS_NUM	(MAX_INTERFACE_NUM * MAX_BUS_NUM)
#define MAX_DQ_NUM		40

#define GET_MIN(arg1, arg2)	((arg1) < (arg2)) ? (arg1) : (arg2)
#define GET_MAX(arg1, arg2)	((arg1) < (arg2)) ? (arg2) : (arg1)

#define INIT_CONTROLLER_MASK_BIT	0x00000001
#define STATIC_LEVELING_MASK_BIT	0x00000002
#define SET_LOW_FREQ_MASK_BIT		0x00000004
#define LOAD_PATTERN_MASK_BIT		0x00000008
#define SET_MEDIUM_FREQ_MASK_BIT	0x00000010
#define WRITE_LEVELING_MASK_BIT		0x00000020
#define LOAD_PATTERN_2_MASK_BIT		0x00000040
#define READ_LEVELING_MASK_BIT		0x00000080
#define SW_READ_LEVELING_MASK_BIT	0x00000100
#define WRITE_LEVELING_SUPP_MASK_BIT	0x00000200
#define PBS_RX_MASK_BIT			0x00000400
#define PBS_TX_MASK_BIT			0x00000800
#define SET_TARGET_FREQ_MASK_BIT	0x00001000
#define ADJUST_DQS_MASK_BIT		0x00002000
#define WRITE_LEVELING_TF_MASK_BIT	0x00004000
#define LOAD_PATTERN_HIGH_MASK_BIT	0x00008000
#define READ_LEVELING_TF_MASK_BIT	0x00010000
#define WRITE_LEVELING_SUPP_TF_MASK_BIT	0x00020000
#define DM_PBS_TX_MASK_BIT		0x00040000
#define CENTRALIZATION_RX_MASK_BIT	0x00100000
#define CENTRALIZATION_TX_MASK_BIT	0x00200000
#define TX_EMPHASIS_MASK_BIT		0x00400000
#define PER_BIT_READ_LEVELING_TF_MASK_BIT	0x00800000
#define VREF_CALIBRATION_MASK_BIT	0x01000000

enum hws_result {
	TEST_FAILED = 0,
	TEST_SUCCESS = 1,
	NO_TEST_DONE = 2
};

enum hws_training_result {
	RESULT_PER_BIT,
	RESULT_PER_BYTE
};

enum auto_tune_stage {
	INIT_CONTROLLER,
	STATIC_LEVELING,
	SET_LOW_FREQ,
	LOAD_PATTERN,
	SET_MEDIUM_FREQ,
	WRITE_LEVELING,
	LOAD_PATTERN_2,
	READ_LEVELING,
	WRITE_LEVELING_SUPP,
	PBS_RX,
	PBS_TX,
	SET_TARGET_FREQ,
	ADJUST_DQS,
	WRITE_LEVELING_TF,
	READ_LEVELING_TF,
	WRITE_LEVELING_SUPP_TF,
	DM_PBS_TX,
	VREF_CALIBRATION,
	CENTRALIZATION_RX,
	CENTRALIZATION_TX,
	TX_EMPHASIS,
	LOAD_PATTERN_HIGH,
	PER_BIT_READ_LEVELING_TF,
	MAX_STAGE_LIMIT
};

enum hws_access_type {
	ACCESS_TYPE_UNICAST = 0,
	ACCESS_TYPE_MULTICAST = 1
};

enum hws_algo_type {
	ALGO_TYPE_DYNAMIC,
	ALGO_TYPE_STATIC
};

struct init_cntr_param {
	int is_ctrl64_bit;
	int do_mrs_phy;
	int init_phy;
	int msys_init;
};

struct pattern_info {
	u8 num_of_phases_tx;
	u8 tx_burst_size;
	u8 delay_between_bursts;
	u8 num_of_phases_rx;
	u32 start_addr;
	u8 pattern_len;
};

/* CL value for each frequency */
struct cl_val_per_freq {
	u8 cl_val[DDR_FREQ_LIMIT];
};

struct cs_element {
	u8 cs_num;
	u8 num_of_cs;
};

struct mode_info {
	/* 32 bits representing MRS bits */
	u32 reg_mr0[MAX_INTERFACE_NUM];
	u32 reg_mr1[MAX_INTERFACE_NUM];
	u32 reg_mr2[MAX_INTERFACE_NUM];
	u32 reg_m_r3[MAX_INTERFACE_NUM];
	/*
	 * Each element in array represent read_data_sample register delay for
	 * a specific interface.
	 * Each register, 4 bits[0+CS*8 to 4+CS*8] represent Number of DDR
	 * cycles from read command until data is ready to be fetched from
	 * the PHY, when accessing CS.
	 */
	u32 read_data_sample[MAX_INTERFACE_NUM];
	/*
	 * Each element in array represent read_data_sample register delay for
	 * a specific interface.
	 * Each register, 4 bits[0+CS*8 to 4+CS*8] represent the total delay
	 * from read command until opening the read mask, when accessing CS.
	 * This field defines the delay in DDR cycles granularity.
	 */
	u32 read_data_ready[MAX_INTERFACE_NUM];
};

struct hws_tip_freq_config_info {
	u8 is_supported;
	u8 bw_per_freq;
	u8 rate_per_freq;
};

struct hws_cs_config_info {
	u32 cs_reg_value;
	u32 cs_cbe_value;
};

struct dfx_access {
	u8 pipe;
	u8 client;
};

struct hws_xsb_info {
	struct dfx_access *dfx_table;
};

int ddr3_tip_register_dq_table(u32 dev_num, u32 *table);
int hws_ddr3_tip_select_ddr_controller(u32 dev_num, int enable);
int hws_ddr3_tip_init_controller(u32 dev_num,
				 struct init_cntr_param *init_cntr_prm);
int hws_ddr3_tip_load_topology_map(u32 dev_num,
				   struct hws_topology_map *topology);
int hws_ddr3_tip_run_alg(u32 dev_num, enum hws_algo_type algo_type);
int hws_ddr3_tip_mode_read(u32 dev_num, struct mode_info *mode_info);
int hws_ddr3_tip_read_training_result(u32 dev_num,
		enum hws_result result[MAX_STAGE_LIMIT][MAX_INTERFACE_NUM]);
int ddr3_tip_is_pup_lock(u32 *pup_buf, enum hws_training_result read_mode);
u8 ddr3_tip_get_buf_min(u8 *buf_ptr);
u8 ddr3_tip_get_buf_max(u8 *buf_ptr);

#endif /* _DDR3_TRAINING_IP_H_ */
