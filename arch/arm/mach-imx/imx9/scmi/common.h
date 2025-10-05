/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2025 NXP
 */

#ifndef _SCMI_CLOCK_COMMON_H_
#define _SCMI_CLOCK_COMMON_H_

#ifdef CONFIG_IMX94
#define IMX_PLAT 94
#include <imx94-clock.h>
#include <imx94-power.h>

#define IMX94_CLK_FLEXSPI1 IMX94_CLK_XSPI1
#endif

#ifdef CONFIG_IMX95
#define IMX_PLAT 95
#include <imx95-clock.h>
#include <imx95-power.h>

#define IMX95_PD_M70 IMX95_PD_M7
#endif

#define IMX_PLAT_STR__(plat) # plat
#define IMX_PLAT_STR_(IMX_PLAT) IMX_PLAT_STR__(IMX_PLAT)
#define IMX_PLAT_STR IMX_PLAT_STR_(IMX_PLAT)

#define SCMI_CLK__(plat, clk) IMX ## plat ## _CLK_ ## clk
#define SCMI_CLK_(plat, clk) SCMI_CLK__(plat, clk)
#define SCMI_CLK(clk) SCMI_CLK_(IMX_PLAT, clk)

#define SCMI_PD__(plat, pd) IMX ## plat ## _PD_ ## pd
#define SCMI_PD_(plat, pd) SCMI_PD__(plat, pd)
#define SCMI_PD(pd) SCMI_PD_(IMX_PLAT, pd)

#define SCMI_CPU__(plat) MXC_CPU_IMX ## plat
#define SCMI_CPU_(plat) SCMI_CPU__(plat)
#define SCMI_CPU SCMI_CPU_(IMX_PLAT)

#endif
