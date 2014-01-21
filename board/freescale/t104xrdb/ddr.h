/*
 * Copyright 2013 Freescale Semiconductor, Inc.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __DDR_H__
#define __DDR_H__

dimm_params_t ddr_raw_timing = {
	.n_ranks = 2,
	.rank_density = 2147483648u,
	.capacity = 4294967296u,
	.primary_sdram_width = 64,
	.ec_sdram_width = 8,
	.registered_dimm = 0,
	.mirrored_dimm = 1,
	.n_row_addr = 15,
	.n_col_addr = 10,
	.n_banks_per_sdram_device = 8,
	.edc_config = 2,	/* ECC */
	.burst_lengths_bitmask = 0x0c,

	.tckmin_x_ps = 1071,
	.caslat_x = 0x2fe << 4,	/* 5,6,7,8,9,10,11,13 */
	.taa_ps = 13910,
	.twr_ps = 15000,
	.trcd_ps = 13910,
	.trrd_ps = 6000,
	.trp_ps = 13910,
	.tras_ps = 34000,
	.trc_ps = 48910,
	.trfc_ps = 260000,
	.twtr_ps = 7500,
	.trtp_ps = 7500,
	.refresh_rate_ps = 7800000,
	.tfaw_ps = 35000,
};

struct board_specific_parameters {
	u32 n_ranks;
	u32 datarate_mhz_high;
	u32 rank_gb;
	u32 clk_adjust;
	u32 wrlvl_start;
	u32 wrlvl_ctl_2;
	u32 wrlvl_ctl_3;
	u32 cpo;
	u32 write_data_delay;
	u32 force_2t;
};

/*
 * These tables contain all valid speeds we want to override with board
 * specific parameters. datarate_mhz_high values need to be in ascending order
 * for each n_ranks group.
 */

static const struct board_specific_parameters udimm0[] = {
	/*
	 * memory controller 0
	 *   num|  hi| rank|  clk| wrlvl |   wrlvl   |  wrlvl | cpo  |wrdata|2T
	 * ranks| mhz| GB  |adjst| start |   ctl2    |  ctl3  |      |delay |
	 */
	{2,  1066, 4, 8,     4, 0x05070609, 0x08090a08,   0xff,    2,  0},
	{2,  1350, 4, 4,     8, 0x0809090b, 0x0c0c0d0a,   0xff,    2,  0},
	{2,  1350, 0, 5,     7, 0x0709090b, 0x0c0c0d09,   0xff,    2,  0},
	{2,  1666, 4, 4,     8, 0x080a0a0d, 0x0d10100b,   0xff,    2,  0},
	{2,  1666, 0, 5,     7, 0x080a0a0c, 0x0d0d0e0a,   0xff,    2,  0},
	{}
};

static const struct board_specific_parameters *udimms[] = {
	udimm0,
};
#endif
