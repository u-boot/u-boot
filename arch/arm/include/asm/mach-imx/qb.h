/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2026 NXP
 */

#ifndef __IMX_QB_H__
#define __IMX_QB_H__

#include <stdbool.h>

bool imx_qb_check(void);
int imx_qb(const char *ifname, const char *dev, bool save);
void spl_imx_qb_save(void);

#endif
