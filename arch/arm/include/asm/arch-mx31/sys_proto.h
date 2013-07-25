/*
 * (C) Copyright 2011
 * Helmut Raiger, HALE electronic GmbH, helmut.raiger@hale.at
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef _SYS_PROTO_H_
#define _SYS_PROTO_H_

struct mxc_weimcs {
	u32 upper;
	u32 lower;
	u32 additional;
};

void mxc_setup_weimcs(int cs, const struct mxc_weimcs *weimcs);
int mxc_mmc_init(bd_t *bis);
u32 get_cpu_rev(void);
#endif
