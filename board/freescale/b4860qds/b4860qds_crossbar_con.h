/*
 * Copyright 2012 Freescale Semiconductor, Inc.
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

#ifndef __CROSSBAR_CONNECTIONS_H__
#define __CROSSBAR_CONNECTIONS_H__

#define NUM_CON_VSC3316	8
#define NUM_CON_VSC3308	4

static const int8_t vsc16_tx_amc[8][2] = { {15, 3}, {0, 2}, {7, 4}, {9, 10},
				{5, 11}, {4, 5}, {2, 6}, {12, 9} };

static const int8_t vsc16_tx_sfp[8][2] = { {15, 7}, {0, 1}, {7, 8}, {9, 0},
				{-1, -1}, {-1, -1}, {-1, -1}, {-1, -1} };

static const int8_t vsc16_tx_4sfp_sgmii_12_56[8][2] = { {15, 7}, {0, 1},
				{7, 8}, {9, 0}, {2, 14}, {12, 15},
				{-1, -1}, {-1, -1} };

static const int8_t vsc16_tx_4sfp_sgmii_34[8][2] = { {15, 7}, {0, 1},
				{7, 8}, {9, 0}, {5, 14}, {4, 15},
				{-1, -1}, {-1, -1} };

#ifdef CONFIG_PPC_B4420
static const int8_t vsc16_tx_sgmii_lane_cd[8][2] = { {5, 14}, {4, 15},
		{-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1} };
#endif

static const int8_t vsc16_tx_aurora[8][2] = { {2, 13}, {12, 12}, {-1, -1},
			{-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1} };

static const int8_t vsc16_rx_amc[8][2] = { {3, 15}, {2, 1}, {4, 8}, {10, 9},
				{11, 11}, {5, 10}, {6, 3}, {9, 12} };

static const int8_t vsc16_rx_sfp[8][2] = { {8, 15}, {0, 1}, {7, 8}, {1, 9},
				{-1, -1}, {-1, -1}, {-1, -1}, {-1, -1} };

static const int8_t vsc16_rx_4sfp_sgmii_12_56[8][2] = { {8, 15}, {0, 1},
				{7, 8}, {1, 9}, {14, 3}, {15, 12},
				{-1, -1}, {-1, -1} };

static const int8_t vsc16_rx_4sfp_sgmii_34[8][2] = { {8, 15}, {0, 1},
				{7, 8}, {1, 9}, {14, 11}, {15, 10},
				{-1, -1}, {-1, -1} };

#ifdef CONFIG_PPC_B4420
static const int8_t vsc16_rx_sgmii_lane_cd[8][2] = { {14, 11}, {15, 10},
		{-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1} };
#endif

static const int8_t vsc16_rx_aurora[8][2] = { {13, 3}, {12, 12}, {-1, -1},
			{-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}, {-1, -1} };

static const int8_t vsc08_tx_amc[4][2] = { {2, 2}, {3, 3}, {7, 4}, {1, 5} };

static const int8_t vsc08_tx_sfp[4][2] = { {2, 1}, {3, 0}, {7, 6}, {1, 7} };

static const int8_t vsc08_rx_amc[4][2] = { {2, 3}, {3, 4}, {4, 7}, {5, 1} };

static const int8_t vsc08_rx_sfp[4][2] = { {1, 3}, {0, 4}, {6, 7}, {7, 1} };

#endif
