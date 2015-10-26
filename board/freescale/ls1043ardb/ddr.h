/*
 * Copyright 2015 Freescale Semiconductor, Inc.
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
	u32 cpo_override;
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
#ifdef CONFIG_SYS_FSL_DDR4
	{1,  1666, 0, 6,     7, 0x07090800, 0x00000000,},
	{1,  1900, 0, 6,     7, 0x07090800, 0x00000000,},
	{1,  2200, 0, 6,     7, 0x07090800, 0x00000000,},
#endif
	{}
};

static const struct board_specific_parameters *udimms[] = {
	udimm0,
};

#endif
