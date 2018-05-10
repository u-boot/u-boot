/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) Marvell International Ltd. and its affiliates
 */

#ifndef _MV_DDR_TOPOLOGY_H
#define _MV_DDR_TOPOLOGY_H

/* ddr bus masks */
#define BUS_MASK_32BIT			0xf
#define BUS_MASK_32BIT_ECC		0x1f
#define BUS_MASK_16BIT			0x3
#define BUS_MASK_16BIT_ECC		0x13
#define BUS_MASK_16BIT_ECC_PUP3		0xb
#define MV_DDR_64BIT_BUS_MASK		0xff
#define MV_DDR_64BIT_ECC_PUP8_BUS_MASK	0x1ff
#define MV_DDR_32BIT_ECC_PUP8_BUS_MASK	0x10f

/* source of ddr configuration data */
enum mv_ddr_cfg_src {
	MV_DDR_CFG_DEFAULT,	/* based on data in mv_ddr_topology_map structure */
	MV_DDR_CFG_SPD,		/* based on data in spd */
	MV_DDR_CFG_USER,	/* based on data from user */
	MV_DDR_CFG_STATIC,	/* based on data from user in register-value format */
	MV_DDR_CFG_LAST
};

enum mv_ddr_num_of_sub_phys_per_ddr_unit {
	SINGLE_SUB_PHY = 1,
	TWO_SUB_PHYS = 2
};

enum mv_ddr_temperature {
	MV_DDR_TEMP_LOW,
	MV_DDR_TEMP_NORMAL,
	MV_DDR_TEMP_HIGH
};

enum mv_ddr_timing {
	MV_DDR_TIM_DEFAULT,
	MV_DDR_TIM_1T,
	MV_DDR_TIM_2T
};

enum mv_ddr_timing_data {
	MV_DDR_TCK_AVG_MIN, /* sdram min cycle time (t ck avg min) */
	MV_DDR_TAA_MIN, /* min cas latency time (t aa min) */
	MV_DDR_TRFC1_MIN, /* min refresh recovery delay time (t rfc1 min) */
	MV_DDR_TWR_MIN, /* min write recovery time (t wr min) */
	MV_DDR_TRCD_MIN, /* min ras to cas delay time (t rcd min) */
	MV_DDR_TRP_MIN, /* min row precharge delay time (t rp min) */
	MV_DDR_TRC_MIN, /* min active to active/refresh delay time (t rc min) */
	MV_DDR_TRAS_MIN, /* min active to precharge delay time (t ras min) */
	MV_DDR_TRRD_S_MIN, /* min activate to activate delay time (t rrd_s min), diff bank group */
	MV_DDR_TRRD_L_MIN, /* min activate to activate delay time (t rrd_l min), same bank group */
	MV_DDR_TFAW_MIN, /* min four activate window delay time (t faw min) */
	MV_DDR_TWTR_S_MIN, /* min write to read time (t wtr s min), diff bank group */
	MV_DDR_TWTR_L_MIN, /* min write to read time (t wtr l min), same bank group */
	MV_DDR_TDATA_LAST
};

enum mv_ddr_dev_width { /* sdram device width */
	MV_DDR_DEV_WIDTH_4BIT,
	MV_DDR_DEV_WIDTH_8BIT,
	MV_DDR_DEV_WIDTH_16BIT,
	MV_DDR_DEV_WIDTH_32BIT,
	MV_DDR_DEV_WIDTH_LAST
};

enum mv_ddr_die_capacity { /* total sdram capacity per die, megabits */
	MV_DDR_DIE_CAP_256MBIT,
	MV_DDR_DIE_CAP_512MBIT = 0,
	MV_DDR_DIE_CAP_1GBIT,
	MV_DDR_DIE_CAP_2GBIT,
	MV_DDR_DIE_CAP_4GBIT,
	MV_DDR_DIE_CAP_8GBIT,
	MV_DDR_DIE_CAP_16GBIT,
	MV_DDR_DIE_CAP_32GBIT,
	MV_DDR_DIE_CAP_12GBIT,
	MV_DDR_DIE_CAP_24GBIT,
	MV_DDR_DIE_CAP_LAST
};

enum mv_ddr_pkg_rank { /* number of package ranks per dimm */
	MV_DDR_PKG_RANK_1,
	MV_DDR_PKG_RANK_2,
	MV_DDR_PKG_RANK_3,
	MV_DDR_PKG_RANK_4,
	MV_DDR_PKG_RANK_5,
	MV_DDR_PKG_RANK_6,
	MV_DDR_PKG_RANK_7,
	MV_DDR_PKG_RANK_8,
	MV_DDR_PKG_RANK_LAST
};

enum mv_ddr_pri_bus_width { /* number of primary bus width bits */
	MV_DDR_PRI_BUS_WIDTH_8,
	MV_DDR_PRI_BUS_WIDTH_16,
	MV_DDR_PRI_BUS_WIDTH_32,
	MV_DDR_PRI_BUS_WIDTH_64,
	MV_DDR_PRI_BUS_WIDTH_LAST
};

enum mv_ddr_bus_width_ext { /* number of extension bus width bits */
	MV_DDR_BUS_WIDTH_EXT_0,
	MV_DDR_BUS_WIDTH_EXT_8,
	MV_DDR_BUS_WIDTH_EXT_LAST
};

enum mv_ddr_die_count {
	MV_DDR_DIE_CNT_1,
	MV_DDR_DIE_CNT_2,
	MV_DDR_DIE_CNT_3,
	MV_DDR_DIE_CNT_4,
	MV_DDR_DIE_CNT_5,
	MV_DDR_DIE_CNT_6,
	MV_DDR_DIE_CNT_7,
	MV_DDR_DIE_CNT_8,
	MV_DDR_DIE_CNT_LAST
};

unsigned int mv_ddr_cl_calc(unsigned int taa_min, unsigned int tclk);
unsigned int mv_ddr_cwl_calc(unsigned int tclk);
struct mv_ddr_topology_map *mv_ddr_topology_map_update(void);
struct dram_config *mv_ddr_dram_config_update(void);
unsigned short mv_ddr_bus_bit_mask_get(void);
unsigned int mv_ddr_if_bus_width_get(void);

#endif /* _MV_DDR_TOPOLOGY_H */
