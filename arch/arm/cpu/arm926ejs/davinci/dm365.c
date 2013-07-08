/*
 * SoC-specific code for tms320dm365 and similar chips
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <asm/arch/hardware.h>

void davinci_enable_uart0(void)
{
	lpsc_on(DAVINCI_LPSC_UART0);
}

#ifdef CONFIG_DRIVER_DAVINCI_I2C
void davinci_enable_i2c(void)
{
	lpsc_on(DAVINCI_LPSC_I2C);
}
#endif
