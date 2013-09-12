/*
 * Copyright 2013 Freescale Semiconductor, Inc.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __DDR_H__
#define __DDR_H__
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
	{2,  1350, 4, 4,     8, 0x0809090b, 0x0c0c0d0a,   0xff,    2,  0},
	{2,  1350, 0, 5,     7, 0x0709090b, 0x0c0c0d09,   0xff,    2,  0},
	{2,  1666, 4, 4,     8, 0x080a0a0d, 0x0d10100b,   0xff,    2,  0},
	{2,  1666, 0, 5,     7, 0x080a0a0c, 0x0d0d0e0a,   0xff,    2,  0},
	{2,  1900, 0, 4,     8, 0x090a0b0e, 0x0f11120c,   0xff,    2,  0},
	{2,  2140, 0, 4,     8, 0x090a0b0e, 0x0f11120c,   0xff,    2,  0},
	{1,  1350, 0, 5,     8, 0x0809090b, 0x0c0c0d0a,   0xff,    2,  0},
	{1,  1700, 0, 5,     8, 0x080a0a0c, 0x0c0d0e0a,   0xff,    2,  0},
	{1,  1900, 0, 4,     8, 0x080a0a0c, 0x0e0e0f0a,   0xff,    2,  0},
	{1,  2140, 0, 4,     8, 0x090a0b0c, 0x0e0f100b,   0xff,    2,  0},
	{}
};

static const struct board_specific_parameters *udimms[] = {
	udimm0,
};
#endif
