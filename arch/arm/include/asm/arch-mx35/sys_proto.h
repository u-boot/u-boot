/*
 * (C) Copyright 2011
 * Stefano Babic, DENX Software Engineering, sbabic@denx.de.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef _SYS_PROTO_H_
#define _SYS_PROTO_H_

u32 get_cpu_rev(void);
void mx3_setup_sdram_bank(u32 start_address, u32 ddr2_config,
	u32 row, u32 col, u32 dsize, u32 refresh);
#define is_soc_rev(rev)	((get_cpu_rev() & 0xFF) - rev)

#endif
