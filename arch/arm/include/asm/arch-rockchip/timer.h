/*
 * (C) Copyright 2015 Rockchip Electronics Co., Ltd
 *
 * SPDX-License-Identifier:     GPL-2.0+
 */

#ifndef __ASM_ARCH_TIMER_H
#define __ASM_ARCH_TIMER_H

struct rk_timer {
	unsigned int timer_load_count0;
	unsigned int timer_load_count1;
	unsigned int timer_curr_value0;
	unsigned int timer_curr_value1;
	unsigned int timer_ctrl_reg;
	unsigned int timer_int_status;
};

void rockchip_timer_init(void);
void rockchip_udelay(unsigned int usec);

#endif
