/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2015 Rockchip Electronics Co., Ltd
 */

#ifndef __ASM_ARCH_TIMER_H
#define __ASM_ARCH_TIMER_H

struct rk_timer {
	u32 timer_load_count0;
	u32 timer_load_count1;
	u32 timer_curr_value0;
	u32 timer_curr_value1;
	u32 timer_ctrl_reg;
	u32 timer_int_status;
};

/** rockchip_stimer_init() - Set up the timer ready for use */
void rockchip_stimer_init(void);

#endif
