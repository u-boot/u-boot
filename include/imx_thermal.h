/*
 *
 * (C) Copyright 2014 Freescale Semiconductor, Inc
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef _IMX_THERMAL_H_
#define _IMX_THERMAL_H_

struct imx_thermal_plat {
	void *regs;
	int fuse_bank;
	int fuse_word;
};

#endif	/* _IMX_THERMAL_H_ */
