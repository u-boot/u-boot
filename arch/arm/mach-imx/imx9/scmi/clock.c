// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2025 NXP
 */

#include <asm/arch/clock.h>
#include <dm/uclass.h>
#include <scmi_agent.h>
#include <scmi_nxp_protocols.h>
#include "common.h"

u32 get_arm_core_clk(void)
{
	u32 val;

	val = imx_clk_scmi_get_rate(SCMI_CLK(SEL_A55C0));
	if (val)
		return val;
	return imx_clk_scmi_get_rate(SCMI_CLK(A55));
}

void init_uart_clk(u32 index)
{
	u32 clock_id;

	switch (index) {
	case 0:
		clock_id = SCMI_CLK(LPUART1);
		break;
	case 1:
		clock_id = SCMI_CLK(LPUART2);
		break;
	case 2:
		clock_id = SCMI_CLK(LPUART3);
		break;
	default:
		return;
	}

	/* 24MHz */
	imx_clk_scmi_enable(clock_id, false);
	imx_clk_scmi_set_parent(clock_id, SCMI_CLK(24M));
	imx_clk_scmi_set_rate(clock_id, 24000000);
	imx_clk_scmi_enable(clock_id, true);
}

unsigned int mxc_get_clock(enum mxc_clock clk)
{
	switch (clk) {
	case MXC_ARM_CLK:
		return get_arm_core_clk();
	case MXC_IPG_CLK:
		return imx_clk_scmi_get_rate(SCMI_CLK(BUSWAKEUP));
	case MXC_CSPI_CLK:
		return imx_clk_scmi_get_rate(SCMI_CLK(LPSPI1));
	case MXC_ESDHC_CLK:
		return imx_clk_scmi_get_rate(SCMI_CLK(USDHC1));
	case MXC_ESDHC2_CLK:
		return imx_clk_scmi_get_rate(SCMI_CLK(USDHC2));
	case MXC_ESDHC3_CLK:
		return imx_clk_scmi_get_rate(SCMI_CLK(USDHC3));
	case MXC_UART_CLK:
		return imx_clk_scmi_get_rate(SCMI_CLK(LPUART1));
	case MXC_FLEXSPI_CLK:
		return imx_clk_scmi_get_rate(SCMI_CLK(FLEXSPI1));
	default:
		return -1;
	};

	return -1;
};
