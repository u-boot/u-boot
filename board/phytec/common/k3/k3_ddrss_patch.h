/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright (C) 2024 PHYTEC Messtechnik GmbH
 * Author: Wadim Egorov <w.egorov@phytec.de>
 */

#ifndef K3_DDRSS_PATCH
#define K3_DDRSS_PATCH

#include <linux/types.h>

struct ddr_reg {
	u32 off;
	u32 val;
};

struct ddrss {
	struct ddr_reg *ctl_regs;
	u32 ctl_regs_num;
	struct ddr_reg *pi_regs;
	u32 pi_regs_num;
	struct ddr_reg *phy_regs;
	u32 phy_regs_num;
};

int fdt_apply_ddrss_timings_patch(void *fdt, struct ddrss *ddrss);

#endif /* K3_DDRSS_PATCH */
