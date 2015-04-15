/*
 * (C) Copyright 2009
 * Stefano Babic, DENX Software Engineering, sbabic@denx.de.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef _SYS_PROTO_H_
#define _SYS_PROTO_H_

#include "../arch-imx/cpu.h"

#define is_soc_rev(rev)	((get_cpu_rev() & 0xFF) - rev)
u32 get_cpu_rev(void);
unsigned imx_ddr_size(void);
void sdelay(unsigned long);
void set_chipselect_size(int const);

/*
 * Initializes on-chip ethernet controllers.
 * to override, implement board_eth_init()
 */

int fecmxc_initialize(bd_t *bis);
u32 get_ahb_clk(void);
u32 get_periph_clk(void);

#endif
