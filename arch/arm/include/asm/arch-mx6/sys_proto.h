/*
 * (C) Copyright 2009
 * Stefano Babic, DENX Software Engineering, sbabic@denx.de.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef _SYS_PROTO_H_
#define _SYS_PROTO_H_

#include <asm/imx-common/regs-common.h>

#define MXC_CPU_MX51		0x51
#define MXC_CPU_MX53		0x53
#define MXC_CPU_MX6SL		0x60
#define MXC_CPU_MX6DL		0x61
#define MXC_CPU_MX6SOLO		0x62
#define MXC_CPU_MX6Q		0x63

#define is_soc_rev(rev)	((get_cpu_rev() & 0xFF) - rev)
u32 get_cpu_rev(void);

/* returns MXC_CPU_ value */
#define cpu_type(rev) (((rev) >> 12)&0xff)

/* use with MXC_CPU_ constants */
#define is_cpu_type(cpu) (cpu_type(get_cpu_rev()) == cpu)

const char *get_imx_type(u32 imxtype);
unsigned imx_ddr_size(void);

void set_vddsoc(u32 mv);

/*
 * Initializes on-chip ethernet controllers.
 * to override, implement board_eth_init()
 */

int fecmxc_initialize(bd_t *bis);
u32 get_ahb_clk(void);
u32 get_periph_clk(void);

int mxs_reset_block(struct mxs_register_32 *reg);
int mxs_wait_mask_set(struct mxs_register_32 *reg,
		       uint32_t mask,
		       unsigned int timeout);
int mxs_wait_mask_clr(struct mxs_register_32 *reg,
		       uint32_t mask,
		       unsigned int timeout);
#endif
