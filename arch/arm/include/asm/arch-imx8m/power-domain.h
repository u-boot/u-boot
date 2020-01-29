/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright 2017 NXP
 */

#ifndef _ASM_ARCH_IMX8M_POWER_DOMAIN_H
#define _ASM_ARCH_IMX8M_POWER_DOMAIN_H

struct imx8m_power_domain_platdata {
	int resource_id;
	int has_pd;
	struct power_domain pd;
};

#endif
