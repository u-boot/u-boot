/*
 * Copyright (c) 2017 Theobroma Systems Design und Consulting GmbH
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __RK_VOP_H__
#define __RK_VOP_H__

#include <asm/arch/vop_rk3288.h>

struct rk_vop_priv {
	void *grf;
	void *regs;
};

enum vop_features {
	VOP_FEATURE_OUTPUT_10BIT = (1 << 0),
};

struct rkvop_driverdata {
	/* configuration */
	u32 features;
	/* block-specific setters/getters */
	void (*set_pin_polarity)(struct udevice *, enum vop_modes, u32);
};

int rk_vop_probe(struct udevice *dev);
int rk_vop_bind(struct udevice *dev);
void rk_vop_probe_regulators(struct udevice *dev,
			     const char * const *names, int cnt);

#endif
