/*
 * (C) Copyright 2010
 * Texas Instruments, <www.ti.com>
 * Aneesh V <aneesh@ti.com>
 *
 * See file CREDITS for list of people who contributed to this
 * project.
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
#ifndef _PL310_H_
#define _PL310_H_

#include <linux/types.h>

/* Register bit fields */
#define PL310_AUX_CTRL_ASSOCIATIVITY_MASK	(1 << 16)

struct pl310_regs {
	u32 pl310_cache_id;
	u32 pl310_cache_type;
	u32 pad1[62];
	u32 pl310_ctrl;
	u32 pl310_aux_ctrl;
	u32 pl310_tag_latency_ctrl;
	u32 pl310_data_latency_ctrl;
	u32 pad2[60];
	u32 pl310_event_cnt_ctrl;
	u32 pl310_event_cnt1_cfg;
	u32 pl310_event_cnt0_cfg;
	u32 pl310_event_cnt1_val;
	u32 pl310_event_cnt0_val;
	u32 pl310_intr_mask;
	u32 pl310_masked_intr_stat;
	u32 pl310_raw_intr_stat;
	u32 pl310_intr_clear;
	u32 pad3[323];
	u32 pl310_cache_sync;
	u32 pad4[15];
	u32 pl310_inv_line_pa;
	u32 pad5[2];
	u32 pl310_inv_way;
	u32 pad6[12];
	u32 pl310_clean_line_pa;
	u32 pad7[1];
	u32 pl310_clean_line_idx;
	u32 pl310_clean_way;
	u32 pad8[12];
	u32 pl310_clean_inv_line_pa;
	u32 pad9[1];
	u32 pl310_clean_inv_line_idx;
	u32 pl310_clean_inv_way;
};

void pl310_inval_all(void);
void pl310_clean_inval_all(void);
void pl310_inval_range(u32 start, u32 end);
void pl310_clean_inval_range(u32 start, u32 end);

#endif
