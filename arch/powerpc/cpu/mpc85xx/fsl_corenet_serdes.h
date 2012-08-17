/*
 * Copyright 2009-2010 Freescale Semiconductor, Inc.
 *
 * Author: Roy Zang <tie-fei.zang@freescale.com>
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

#ifndef __FSL_CORENET_SERDES_H
#define __FSL_CORENET_SERDES_H

#define SRDS_MAX_LANES		18
#define SRDS_MAX_BANK		3

enum srds_bank {
	FSL_SRDS_BANK_1  = 0,
	FSL_SRDS_BANK_2  = 1,
	FSL_SRDS_BANK_3  = 2,
};

int is_serdes_prtcl_valid(u32 prtcl);
int serdes_get_lane_idx(int lane);
int serdes_get_bank_by_lane(int lane);
int serdes_lane_enabled(int lane);
enum srds_prtcl serdes_get_prtcl(int cfg, int lane);

#ifdef CONFIG_SYS_P4080_ERRATUM_SERDES8
extern uint16_t srds_lpd_b[SRDS_MAX_BANK];
#endif

#endif /* __FSL_CORENET_SERDES_H */
