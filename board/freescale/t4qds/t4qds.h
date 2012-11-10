/*
 * Copyright 2011-2012 Freescale Semiconductor, Inc.
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

#ifndef __CORENET_DS_H__
#define __CORENET_DS_H__

void fdt_fixup_board_enet(void *blob);
void pci_of_setup(void *blob, bd_t *bd);

static const int8_t vsc3316_fsm1_tx[8][2] = { {0, 0}, {1, 1}, {6, 6}, {7, 7},
				{8, 8}, {9, 9}, {14, 14}, {15, 15} };

static const int8_t vsc3316_fsm2_tx[8][2] = { {2, 2}, {3, 3}, {4, 4}, {5, 5},
				{10, 10}, {11, 11}, {12, 12}, {13, 13} };

static const int8_t vsc3316_fsm1_rx[8][2] = { {2, 12}, {3, 13}, {4, 5}, {5, 4},
				{10, 11}, {11, 10}, {12, 2}, {13, 3} };

static const int8_t vsc3316_fsm2_rx[8][2] = { {0, 15}, {1, 14}, {6, 7}, {7, 6},
				{8, 9}, {9, 8}, {14, 1}, {15, 0} };
#endif
