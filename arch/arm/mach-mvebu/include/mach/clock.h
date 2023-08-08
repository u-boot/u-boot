/*
 * Copyright (C) 2018 Marvell International Ltd.
 *
 * SPDX-License-Identifier:    GPL-2.0+
 * https://spdx.org/licenses
 */

#ifndef _MVEBU_CLOCK_H_
#define _MVEBU_CLOCK_H_

#define KHz			1000
#define MHz			1000000
#define GHz			1000000000

u32 soc_cpu_clk_get(void);
u32 soc_ddr_clk_get(void);
u32 soc_tclk_get(void);
u32 soc_l2_clk_get(void);
u32 soc_timer_clk_get(void);

void soc_print_clock_info(void);

#endif /* _MVEBU_CLOCK_H_ */
