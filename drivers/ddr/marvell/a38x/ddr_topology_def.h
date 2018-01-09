/*
 * Copyright (C) Marvell International Ltd. and its affiliates
 *
 * SPDX-License-Identifier:     GPL-2.0
 */

#ifndef _DDR_TOPOLOGY_DEF_H
#define _DDR_TOPOLOGY_DEF_H

#include "ddr3_training_ip_def.h"
#include "ddr3_topology_def.h"

#if defined(CONFIG_ARMADA_38X)
#include "ddr3_a38x.h"
#endif

/* bus width in bits */
enum hws_bus_width {
	BUS_WIDTH_4,
	BUS_WIDTH_8,
	BUS_WIDTH_16,
	BUS_WIDTH_32
};

enum hws_temperature {
	HWS_TEMP_LOW,
	HWS_TEMP_NORMAL,
	HWS_TEMP_HIGH
};

enum hws_mem_size {
	MEM_512M,
	MEM_1G,
	MEM_2G,
	MEM_4G,
	MEM_8G,
	MEM_SIZE_LAST
};

enum hws_timing {
	HWS_TIM_DEFAULT,
	HWS_TIM_1T,
	HWS_TIM_2T
};

struct bus_params {
	/* Chip Select (CS) bitmask (bits 0-CS0, bit 1- CS1 ...) */
	u8 cs_bitmask;

	/*
	 * mirror enable/disable
	 * (bits 0-CS0 mirroring, bit 1- CS1 mirroring ...)
	 */
	int mirror_enable_bitmask;

	/* DQS Swap (polarity) - true if enable */
	int is_dqs_swap;

	/* CK swap (polarity) - true if enable */
	int is_ck_swap;
};

struct if_params {
	/* bus configuration */
	struct bus_params as_bus_params[MAX_BUS_NUM];

	/* Speed Bin Table */
	enum hws_speed_bin speed_bin_index;

	/* bus width of memory */
	enum hws_bus_width bus_width;

	/* Bus memory size (MBit) */
	enum hws_mem_size memory_size;

	/* The DDR frequency for each interfaces */
	enum hws_ddr_freq memory_freq;

	/*
	 * delay CAS Write Latency
	 * - 0 for using default value (jedec suggested)
	 */
	u8 cas_wl;

	/*
	 * delay CAS Latency
	 * - 0 for using default value (jedec suggested)
	 */
	u8 cas_l;

	/* operation temperature */
	enum hws_temperature interface_temp;

	/* 2T vs 1T mode (by default computed from number of CSs) */
	enum hws_timing timing;
};

struct hws_topology_map {
	/* Number of interfaces (default is 12) */
	u8 if_act_mask;

	/* Controller configuration per interface */
	struct if_params interface_params[MAX_INTERFACE_NUM];

	/* BUS per interface (default is 4) */
	u8 num_of_bus_per_interface;

	/* Bit mask for active buses */
	u8 bus_act_mask;
};

/* DDR3 training global configuration parameters */
struct tune_train_params {
	u32 ck_delay;
	u32 ck_delay_16;
	u32 p_finger;
	u32 n_finger;
	u32 phy_reg3_val;
};

#endif /* _DDR_TOPOLOGY_DEF_H */
