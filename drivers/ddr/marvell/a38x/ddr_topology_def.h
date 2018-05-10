/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) Marvell International Ltd. and its affiliates
 */

#ifndef _DDR_TOPOLOGY_DEF_H
#define _DDR_TOPOLOGY_DEF_H

#include "ddr3_training_ip_def.h"
#include "ddr3_topology_def.h"

#if defined(CONFIG_ARMADA_38X) || defined(CONFIG_ARMADA_39X)
#include "mv_ddr_plat.h"
#endif

#include "mv_ddr_topology.h"
#include "mv_ddr_spd.h"
#include "ddr3_logging_def.h"

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

	/* sdram device width */
	enum mv_ddr_dev_width bus_width;

	/* total sdram capacity per die, megabits */
	enum mv_ddr_die_capacity memory_size;

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
	enum mv_ddr_temperature interface_temp;

	/* 2T vs 1T mode (by default computed from number of CSs) */
	enum mv_ddr_timing timing;
};

struct mv_ddr_topology_map {
	/* debug level configuration */
	enum mv_ddr_debug_level debug_level;

	/* Number of interfaces (default is 12) */
	u8 if_act_mask;

	/* Controller configuration per interface */
	struct if_params interface_params[MAX_INTERFACE_NUM];

	/* Bit mask for active buses */
	u16 bus_act_mask;

	/* source of ddr configuration data */
	enum mv_ddr_cfg_src cfg_src;

	/* raw spd data */
	union mv_ddr_spd_data spd_data;

	/* timing parameters */
	unsigned int timing_data[MV_DDR_TDATA_LAST];
};

/* DDR3 training global configuration parameters */
struct tune_train_params {
	u32 ck_delay;
	u32 phy_reg3_val;
	u32 g_zpri_data;
	u32 g_znri_data;
	u32 g_zpri_ctrl;
	u32 g_znri_ctrl;
	u32 g_zpodt_data;
	u32 g_znodt_data;
	u32 g_zpodt_ctrl;
	u32 g_znodt_ctrl;
	u32 g_dic;
	u32 g_odt_config;
	u32 g_rtt_nom;
	u32 g_rtt_wr;
	u32 g_rtt_park;
};

#endif /* _DDR_TOPOLOGY_DEF_H */
