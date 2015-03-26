/*
 * Copyright (C) Marvell International Ltd. and its affiliates
 *
 * SPDX-License-Identifier:	GPL-2.0
 */

#ifndef _DDR3_TRAINING_IP_STATIC_H_
#define _DDR3_TRAINING_IP_STATIC_H_

#include "ddr3_training_ip_def.h"
#include "ddr3_training_ip.h"

struct trip_delay_element {
	u32 dqs_delay;		/* DQS delay (m_sec) */
	u32 ck_delay;		/* CK Delay  (m_sec) */
};

struct hws_tip_static_config_info {
	u32 silicon_delay;
	struct trip_delay_element *package_trace_arr;
	struct trip_delay_element *board_trace_arr;
};

int ddr3_tip_run_static_alg(u32 dev_num, enum hws_ddr_freq freq);
int ddr3_tip_init_static_config_db(
	u32 dev_num, struct hws_tip_static_config_info *static_config_info);
int ddr3_tip_init_specific_reg_config(u32 dev_num,
				      struct reg_data *reg_config_arr);
int ddr3_tip_static_phy_init_controller(u32 dev_num);

#endif /* _DDR3_TRAINING_IP_STATIC_H_ */
