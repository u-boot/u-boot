// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2026 Renesas Electronics Corp.
 *
 * Portions Copyright (C) 2026 Synopsys, Inc. Used with permission. All rights reserved.
 */

#ifndef __INCLUDE_DBSC5_H__
#define __INCLUDE_DBSC5_H__

/* The number of channels X5H has */
#define DRAM_CH_CNT			16
/* The number of slices X5H has */
#define SLICE_CNT			2
/* The number of chip select X5H has */
#define CS_CNT				2

struct renesas_dbsc5_board_config {
	u32 bdcfg_phyvalid;
	u32 bdcfg_tx_drv;
	u32 bdcfg_tx_ffc;
	u32 bdcfg_rx_odt;
	u8 bdcfg_rx_dfe;
	u8 bdcfg_tx_odt;
	u8 bdcfg_tx_ntodt;
	u8 bdcfg_tx_dfe;
	u8 bdcfg_rx_dca;
	u8 bdcfg_rx_drv;
	u32 bdcfg_rx_emphasis;
	u8 bdcfg_tx_dca;
	u8 bdcfg_ca_vref;
	u32 bdcfg_rx_vref;
	u32 bdcfg_rx_vref_step;
	u32 bdcfg_tx_vref;
	u8 bdcfg_rfm_chk;

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
		/* SoC dq([7][6][5][4][3][2][1][0]) -> MEM dqY/dm: (8 means DM) */
		u32 bdcfg_dq_swap[SLICE_CNT];
		/* SoC dm -> MEM dqY/dm: (8 means DM) */
		u8 bdcfg_dm_swap[SLICE_CNT];
	} ch[DRAM_CH_CNT];
};

#endif	/* __INCLUDE_DBSC5_H__ */
