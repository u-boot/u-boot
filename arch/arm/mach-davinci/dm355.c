/*
 * SoC-specific code for tms320dm355 and similar chips
 *
 * Copyright (C) 2009 David Brownell
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <asm/arch/hardware.h>


void davinci_enable_uart0(void)
{
	lpsc_on(DAVINCI_LPSC_UART0);

	/* Bringup UART0 out of reset */
	REG(UART0_PWREMU_MGMT) = 0x00006001;
}


#ifdef CONFIG_SYS_I2C_DAVINCI
void davinci_enable_i2c(void)
{
	lpsc_on(DAVINCI_LPSC_I2C);

	/* Enable I2C pin Mux */
	REG(PINMUX3) |= (1 << 20) | (1 << 19);
}
#endif
