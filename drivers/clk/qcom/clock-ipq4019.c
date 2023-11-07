// SPDX-License-Identifier: GPL-2.0+
/*
 * Clock drivers for Qualcomm IPQ40xx
 *
 * Copyright (c) 2020 Sartura Ltd.
 *
 * Author: Robert Marko <robert.marko@sartura.hr>
 *
 */

#include <clk-uclass.h>
#include <common.h>
#include <dm.h>
#include <errno.h>
#include <dt-bindings/clock/qcom,ipq4019-gcc.h>

#include "clock-qcom.h"

ulong msm_set_rate(struct clk *clk, ulong rate)
{
	switch (clk->id) {
	case GCC_BLSP1_UART1_APPS_CLK: /*UART1*/
		/* This clock is already initialized by SBL1 */
		return 0;
	default:
		return -EINVAL;
	}
}

int msm_enable(struct clk *clk)
{
	switch (clk->id) {
	case GCC_BLSP1_QUP1_SPI_APPS_CLK: /*SPI1*/
		/* This clock is already initialized by SBL1 */
		return 0;
	case GCC_PRNG_AHB_CLK: /*PRNG*/
		/* This clock is already initialized by SBL1 */
		return 0;
	case GCC_USB3_MASTER_CLK:
	case GCC_USB3_SLEEP_CLK:
	case GCC_USB3_MOCK_UTMI_CLK:
	case GCC_USB2_MASTER_CLK:
	case GCC_USB2_SLEEP_CLK:
	case GCC_USB2_MOCK_UTMI_CLK:
		/* These clocks is already initialized by SBL1 */
		return 0;
	default:
		return -EINVAL;
	}
}

