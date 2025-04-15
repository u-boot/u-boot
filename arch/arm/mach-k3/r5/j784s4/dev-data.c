// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * J784S4 specific device platform data
 *
 * This file is auto generated. Please do not hand edit and report any issues
 * to Bryan Brattlof <bb@ti.com>.
 *
 * Copyright (C) 2020-2024 Texas Instruments Incorporated - https://www.ti.com/
 */

#include "k3-dev.h"

static struct ti_psc soc_psc_list[] = {
	[0] = PSC(0, 0x42000000),
	[1] = PSC(1, 0x00420000),
	[2] = PSC(2, 0x00400000),
};

static struct ti_pd soc_pd_list[] = {
	[0] = PSC_PD(0, &soc_psc_list[0], NULL),
	[1] = PSC_PD(3, &soc_psc_list[1], NULL),
	[2] = PSC_PD(0, &soc_psc_list[2], NULL),
	[3] = PSC_PD(1, &soc_psc_list[2], &soc_pd_list[2]),
	[4] = PSC_PD(14, &soc_psc_list[2], NULL),
	[5] = PSC_PD(15, &soc_psc_list[2], &soc_pd_list[4]),
	[6] = PSC_PD(16, &soc_psc_list[2], &soc_pd_list[4]),
	[7] = PSC_PD(38, &soc_psc_list[2], NULL),
};

static struct ti_lpsc soc_lpsc_list[] = {
	[0] = PSC_LPSC(0, &soc_psc_list[0], &soc_pd_list[0], NULL),
	[1] = PSC_LPSC(3, &soc_psc_list[0], &soc_pd_list[0], NULL),
	[2] = PSC_LPSC(10, &soc_psc_list[0], &soc_pd_list[0], NULL),
	[3] = PSC_LPSC(11, &soc_psc_list[0], &soc_pd_list[0], NULL),
	[4] = PSC_LPSC(12, &soc_psc_list[0], &soc_pd_list[0], NULL),
	[5] = PSC_LPSC(12, &soc_psc_list[1], &soc_pd_list[1], &soc_lpsc_list[6]),
	[6] = PSC_LPSC(13, &soc_psc_list[1], &soc_pd_list[1], NULL),
	[7] = PSC_LPSC(0, &soc_psc_list[2], &soc_pd_list[2], NULL),
	[8] = PSC_LPSC(9, &soc_psc_list[2], &soc_pd_list[2], &soc_lpsc_list[2]),
	[9] = PSC_LPSC(14, &soc_psc_list[2], &soc_pd_list[2], &soc_lpsc_list[10]),
	[10] = PSC_LPSC(15, &soc_psc_list[2], &soc_pd_list[2], NULL),
	[11] = PSC_LPSC(16, &soc_psc_list[2], &soc_pd_list[2], &soc_lpsc_list[12]),
	[12] = PSC_LPSC(17, &soc_psc_list[2], &soc_pd_list[2], NULL),
	[13] = PSC_LPSC(20, &soc_psc_list[2], &soc_pd_list[2], &soc_lpsc_list[5]),
	[14] = PSC_LPSC(23, &soc_psc_list[2], &soc_pd_list[2], &soc_lpsc_list[5]),
	[15] = PSC_LPSC(25, &soc_psc_list[2], &soc_pd_list[2], &soc_lpsc_list[5]),
	[16] = PSC_LPSC(43, &soc_psc_list[2], &soc_pd_list[3], NULL),
	[17] = PSC_LPSC(45, &soc_psc_list[2], &soc_pd_list[3], NULL),
	[18] = PSC_LPSC(78, &soc_psc_list[2], &soc_pd_list[4], NULL),
	[19] = PSC_LPSC(80, &soc_psc_list[2], &soc_pd_list[5], &soc_lpsc_list[18]),
	[20] = PSC_LPSC(81, &soc_psc_list[2], &soc_pd_list[6], &soc_lpsc_list[18]),
	[21] = PSC_LPSC(120, &soc_psc_list[2], &soc_pd_list[7], &soc_lpsc_list[22]),
	[22] = PSC_LPSC(121, &soc_psc_list[2], &soc_pd_list[7], NULL),
};

static struct ti_dev soc_dev_list[] = {
	PSC_DEV(35, &soc_lpsc_list[0]),
	PSC_DEV(160, &soc_lpsc_list[0]),
	PSC_DEV(161, &soc_lpsc_list[0]),
	PSC_DEV(162, &soc_lpsc_list[0]),
	PSC_DEV(243, &soc_lpsc_list[0]),
	PSC_DEV(149, &soc_lpsc_list[0]),
	PSC_DEV(167, &soc_lpsc_list[1]),
	PSC_DEV(279, &soc_lpsc_list[1]),
	PSC_DEV(397, &soc_lpsc_list[1]),
	PSC_DEV(161, &soc_lpsc_list[2]),
	PSC_DEV(162, &soc_lpsc_list[3]),
	PSC_DEV(160, &soc_lpsc_list[4]),
	PSC_DEV(139, &soc_lpsc_list[5]),
	PSC_DEV(194, &soc_lpsc_list[6]),
	PSC_DEV(78, &soc_lpsc_list[7]),
	PSC_DEV(61, &soc_lpsc_list[8]),
	PSC_DEV(131, &soc_lpsc_list[9]),
	PSC_DEV(191, &soc_lpsc_list[10]),
	PSC_DEV(132, &soc_lpsc_list[11]),
	PSC_DEV(192, &soc_lpsc_list[12]),
	PSC_DEV(398, &soc_lpsc_list[13]),
	PSC_DEV(141, &soc_lpsc_list[14]),
	PSC_DEV(140, &soc_lpsc_list[15]),
	PSC_DEV(146, &soc_lpsc_list[16]),
	PSC_DEV(392, &soc_lpsc_list[17]),
	PSC_DEV(395, &soc_lpsc_list[17]),
	PSC_DEV(198, &soc_lpsc_list[18]),
	PSC_DEV(202, &soc_lpsc_list[19]),
	PSC_DEV(203, &soc_lpsc_list[20]),
	PSC_DEV(133, &soc_lpsc_list[21]),
	PSC_DEV(193, &soc_lpsc_list[22]),
};

const struct ti_k3_pd_platdata j784s4_pd_platdata = {
	.psc = soc_psc_list,
	.pd = soc_pd_list,
	.lpsc = soc_lpsc_list,
	.devs = soc_dev_list,
	.num_psc = ARRAY_SIZE(soc_psc_list),
	.num_pd = ARRAY_SIZE(soc_pd_list),
	.num_lpsc = ARRAY_SIZE(soc_lpsc_list),
	.num_devs = ARRAY_SIZE(soc_dev_list),
};
