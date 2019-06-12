// SPDX-License-Identifier: GPL-2.0+
#include <common.h>
#include <asm/arch/sci/sci.h>

int sc_pm_setup_uart(sc_rsrc_t uart_rsrc, sc_pm_clock_rate_t clk_rate)
{
	sc_pm_clock_rate_t rate = clk_rate;
	int ret;

	/* Power up UARTn */
	ret = sc_pm_set_resource_power_mode(-1, uart_rsrc, SC_PM_PW_MODE_ON);
	if (ret)
		return ret;

	/* Set UARTn clock root to 'rate' MHz */
	ret = sc_pm_set_clock_rate(-1, uart_rsrc, SC_PM_CLK_PER, &rate);
	if (ret)
		return ret;

	/* Enable UARTn clock root */
	ret = sc_pm_clock_enable(-1, uart_rsrc, SC_PM_CLK_PER, true, false);
	if (ret)
		return ret;

	return 0;
}
