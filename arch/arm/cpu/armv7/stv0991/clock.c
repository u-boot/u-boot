/*
 * (C) Copyright 2014
 * Vikas Manocha, ST Micoelectronics, vikas.manocha@st.com.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <asm/io.h>
#include <asm/arch/hardware.h>
#include <asm/arch/stv0991_cgu.h>
#include<asm/arch/stv0991_periph.h>

static struct stv0991_cgu_regs *const stv0991_cgu_regs = \
				(struct stv0991_cgu_regs *) (CGU_BASE_ADDR);

void clock_setup(int peripheral)
{
	switch (peripheral) {
	case UART_CLOCK_CFG:
		writel(UART_CLK_CFG, &stv0991_cgu_regs->uart_freq);
		break;
	case ETH_CLOCK_CFG:
		break;
	default:
		break;
	}
}
