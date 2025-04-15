// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2024-2025 Renesas Electronics Corp.
 */

#ifndef __INCLUDE_DBSC5_H__
#define __INCLUDE_DBSC5_H__

/* The number of channels V4H has */
#define DRAM_CH_CNT			4
/* The number of slices V4H has */
#define SLICE_CNT			2
/* The number of chip select V4H has */
#define CS_CNT				2

struct renesas_dbsc5_board_config {
	/* Channels in use */
	u8 bdcfg_phyvalid;
	/* Read vref (SoC) training range */
	u32 bdcfg_vref_r;
	/* Write vref (MR14, MR15) training range */
	u16 bdcfg_vref_w;
	/* CA vref (MR12) training range */
	u16 bdcfg_vref_ca;
	/* RFM required check */
	bool bdcfg_rfm_chk;

	/* Board parameter about channels */
	struct {
		/*
		 * 0x00:  4Gb dual channel die /  2Gb single channel die
		 * 0x01:  6Gb dual channel die /  3Gb single channel die
		 * 0x02:  8Gb dual channel die /  4Gb single channel die
		 * 0x03: 12Gb dual channel die /  6Gb single channel die
		 * 0x04: 16Gb dual channel die /  8Gb single channel die
		 * 0x05: 24Gb dual channel die / 12Gb single channel die
		 * 0x06: 32Gb dual channel die / 16Gb single channel die
		 * 0x07: 24Gb single channel die
		 * 0x08: 32Gb single channel die
		 * 0xFF: NO_MEMORY
		 */
		u8 bdcfg_ddr_density[CS_CNT];
		/* SoC caX([6][5][4][3][2][1][0]) -> MEM caY: */
		u32 bdcfg_ca_swap;
		/* SoC dqsX([1][0]) -> MEM dqsY: */
		u8 bdcfg_dqs_swap;
		/* SoC dq([7][6][5][4][3][2][1][0]) -> MEM dqY/dm:  (8 means DM) */
		u32 bdcfg_dq_swap[SLICE_CNT];
		/* SoC dm -> MEM dqY/dm:  (8 means DM) */
		u8 bdcfg_dm_swap[SLICE_CNT];
		/* SoC ckeX([1][0]) -> MEM csY */
		u8 bdcfg_cs_swap;
	} ch[4];
};

#endif	/* __INCLUDE_DBSC5_H__ */
