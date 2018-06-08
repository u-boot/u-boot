/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2018  Cisco Systems, Inc.
 *
 * Author: Thomas Fitzsimmons <fitzsim@fitzsim.org>
 */

#ifndef _BCMSTB_PRIOR_STAGE_H
#define _BCMSTB_PRIOR_STAGE_H

#ifndef __ASSEMBLY__

#include <linux/types.h>

struct bcmstb_boot_parameters {
	u32 r0;
	u32 r1;
	u32 r2;
	u32 r3;
	u32 sp;
	u32 lr;
};

extern struct bcmstb_boot_parameters bcmstb_boot_parameters;

extern phys_addr_t prior_stage_fdt_address;

#endif /* __ASSEMBLY__ */

#endif /* _BCMSTB_PRIOR_STAGE_H */
