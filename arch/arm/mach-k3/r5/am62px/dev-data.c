// SPDX-License-Identifier: GPL-2.0+
/*
 * AM62PX specific device platform data
 *
 * This file is auto generated. Please do not hand edit and report any issues
 * to Bryan Brattlof <bb@ti.com>.
 *
 * Copyright (C) 2020-2023 Texas Instruments Incorporated - https://www.ti.com/
 */

#include "k3-dev.h"

static struct ti_psc soc_psc_list[] = {
	[0] = PSC(0, 0x00400000),
};

static struct ti_pd soc_pd_list[] = {
	[0] = PSC_PD(0, &soc_psc_list[0], NULL),
	[1] = PSC_PD(3, &soc_psc_list[0], &soc_pd_list[0]),
	[2] = PSC_PD(4, &soc_psc_list[0], &soc_pd_list[1]),
	[3] = PSC_PD(13, &soc_psc_list[0], &soc_pd_list[0]),
};

static struct ti_lpsc soc_lpsc_list[] = {
	[0] = PSC_LPSC(0, &soc_psc_list[0], &soc_pd_list[0], NULL),
	[1] = PSC_LPSC(12, &soc_psc_list[0], &soc_pd_list[0], &soc_lpsc_list[5]),
	[2] = PSC_LPSC(13, &soc_psc_list[0], &soc_pd_list[0], &soc_lpsc_list[6]),
	[3] = PSC_LPSC(20, &soc_psc_list[0], &soc_pd_list[0], &soc_lpsc_list[8]),
	[4] = PSC_LPSC(21, &soc_psc_list[0], &soc_pd_list[0], &soc_lpsc_list[8]),
	[5] = PSC_LPSC(23, &soc_psc_list[0], &soc_pd_list[0], &soc_lpsc_list[8]),
	[6] = PSC_LPSC(24, &soc_psc_list[0], &soc_pd_list[0], &soc_lpsc_list[8]),
	[7] = PSC_LPSC(28, &soc_psc_list[0], &soc_pd_list[0], &soc_lpsc_list[8]),
	[8] = PSC_LPSC(34, &soc_psc_list[0], &soc_pd_list[0], &soc_lpsc_list[8]),
	[9] = PSC_LPSC(53, &soc_psc_list[0], &soc_pd_list[1], &soc_lpsc_list[8]),
	[10] = PSC_LPSC(56, &soc_psc_list[0], &soc_pd_list[2], &soc_lpsc_list[9]),
	[11] = PSC_LPSC(72, &soc_psc_list[0], &soc_pd_list[3], &soc_lpsc_list[8]),
	[12] = PSC_LPSC(73, &soc_psc_list[0], &soc_pd_list[3], &soc_lpsc_list[11]),
	[13] = PSC_LPSC(74, &soc_psc_list[0], &soc_pd_list[3], &soc_lpsc_list[12]),
};

static struct ti_dev soc_dev_list[] = {
	PSC_DEV(16, &soc_lpsc_list[0]),
	PSC_DEV(77, &soc_lpsc_list[0]),
	PSC_DEV(61, &soc_lpsc_list[0]),
	PSC_DEV(178, &soc_lpsc_list[1]),
	PSC_DEV(179, &soc_lpsc_list[2]),
	PSC_DEV(57, &soc_lpsc_list[3]),
	PSC_DEV(58, &soc_lpsc_list[4]),
	PSC_DEV(161, &soc_lpsc_list[5]),
	PSC_DEV(162, &soc_lpsc_list[6]),
	PSC_DEV(75, &soc_lpsc_list[7]),
	PSC_DEV(36, &soc_lpsc_list[8]),
	PSC_DEV(102, &soc_lpsc_list[8]),
	PSC_DEV(146, &soc_lpsc_list[8]),
	PSC_DEV(166, &soc_lpsc_list[9]),
	PSC_DEV(135, &soc_lpsc_list[10]),
	PSC_DEV(170, &soc_lpsc_list[11]),
	PSC_DEV(177, &soc_lpsc_list[12]),
	PSC_DEV(55, &soc_lpsc_list[13]),
};

const struct ti_k3_pd_platdata am62px_pd_platdata = {
	.psc = soc_psc_list,
	.pd = soc_pd_list,
	.lpsc = soc_lpsc_list,
	.devs = soc_dev_list,
	.num_psc = ARRAY_SIZE(soc_psc_list),
	.num_pd = ARRAY_SIZE(soc_pd_list),
	.num_lpsc = ARRAY_SIZE(soc_lpsc_list),
	.num_devs = ARRAY_SIZE(soc_dev_list),
};
