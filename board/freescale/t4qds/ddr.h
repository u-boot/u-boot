/*
 * Copyright 2013 Freescale Semiconductor, Inc.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
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
	u32 force_2T;
};

/*
 * These tables contain all valid speeds we want to override with board
 * specific parameters. datarate_mhz_high values need to be in ascending order
 * for each n_ranks group.
 */

#ifdef CONFIG_T4240QDS
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

static const struct board_specific_parameters rdimm0[] = {
	/*
	 * memory controller 0
	 *   num|  hi| rank|  clk| wrlvl |   wrlvl   |  wrlvl | cpo  |wrdata|2T
	 * ranks| mhz| GB  |adjst| start |   ctl2    |  ctl3  |      |delay |
	 */
	{4,  1350, 0, 5,     9, 0x08070605, 0x07080805,   0xff,    2,  0},
	{4,  1666, 0, 5,     8, 0x08070605, 0x07080805,   0xff,    2,  0},
	{4,  2140, 0, 5,     8, 0x08070605, 0x07081805,   0xff,    2,  0},
	{2,  1350, 0, 5,     7, 0x0809090b, 0x0c0c0d09,   0xff,    2,  0},
	{2,  1666, 0, 5,     8, 0x080a0a0c, 0x0c0d0e0a,   0xff,    2,  0},
	{2,  2140, 0, 5,     8, 0x090a0b0c, 0x0e0f100b,   0xff,    2,  0},
	{1,  1350, 0, 5,     8, 0x0809090b, 0x0c0c0d0a,   0xff,    2,  0},
	{1,  1700, 0, 5,     8, 0x080a0a0c, 0x0c0d0e0a,   0xff,    2,  0},
	{1,  1900, 0, 4,     8, 0x080a0a0c, 0x0e0e0f0a,   0xff,    2,  0},
	{1,  2140, 0, 4,     8, 0x090a0b0c, 0x0e0f100b,   0xff,    2,  0},
	{}
};

#else	/* CONFIG_T4240EMU */
static const struct board_specific_parameters udimm0[] = {
	/*
	 * memory controller 0
	 *   num|  hi| rank|  clk| wrlvl |   wrlvl   |  wrlvl | cpo  |wrdata|2T
	 * ranks| mhz| GB  |adjst| start |   ctl2    |  ctl3  |      |delay |
	 */
	{2,  2140, 0, 4,     8, 0x0, 0x0,   0xff,    2,  0},
	{1,  2140, 0, 4,     8, 0x0, 0x0,   0xff,    2,  0},
	{}
};

static const struct board_specific_parameters rdimm0[] = {
	/*
	 * memory controller 0
	 *   num|  hi| rank|  clk| wrlvl |   wrlvl   |  wrlvl | cpo  |wrdata|2T
	 * ranks| mhz| GB  |adjst| start |   ctl2    |  ctl3  |      |delay |
	 */
	{4,  2140, 0, 5,     8, 0x0, 0x0,   0xff,    2,  0},
	{2,  2140, 0, 5,     8, 0x0, 0x0,   0xff,    2,  0},
	{1,  2140, 0, 4,     8, 0x0, 0x0,   0xff,    2,  0},
	{}
};
#endif	/* CONFIG_T4240EMU */

/*
 * The three slots have slightly different timing. The center values are good
 * for all slots. We use identical speed tables for them. In future use, if
 * DIMMs require separated tables, make more entries as needed.
 */
static const struct board_specific_parameters *udimms[] = {
	udimm0,
};

/*
 * The three slots have slightly different timing. See comments above.
 */
static const struct board_specific_parameters *rdimms[] = {
	rdimm0,
};


#endif
