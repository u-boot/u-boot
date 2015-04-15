/*
 * sun9i specific clock code
 *
 * (C) Copyright 2015 Hans de Goede <hdegoede@redhat.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <asm/io.h>
#include <asm/arch/clock.h>
#include <asm/arch/prcm.h>
#include <asm/arch/sys_proto.h>

void clock_init_uart(void)
{
	struct sunxi_ccm_reg *const ccm =
		(struct sunxi_ccm_reg *)SUNXI_CCM_BASE;

	/* open the clock for uart */
	setbits_le32(&ccm->apb1_gate,
		     CLK_GATE_OPEN << (APB1_GATE_UART_SHIFT +
				       CONFIG_CONS_INDEX - 1));
	/* deassert uart reset */
	setbits_le32(&ccm->apb1_reset_cfg,
		     1 << (APB1_RESET_UART_SHIFT +
			   CONFIG_CONS_INDEX - 1));

	/* Dup with clock_init_safe(), drop once sun9i SPL support lands */
	writel(PLL4_CFG_DEFAULT, &ccm->pll4_periph0_cfg);
}

int clock_twi_onoff(int port, int state)
{
	struct sunxi_ccm_reg *const ccm =
		(struct sunxi_ccm_reg *)SUNXI_CCM_BASE;

	if (port > 4)
		return -1;

	/* set the apb reset and clock gate for twi */
	if (state) {
		setbits_le32(&ccm->apb1_gate,
			     CLK_GATE_OPEN << (APB1_GATE_TWI_SHIFT + port));
		setbits_le32(&ccm->apb1_reset_cfg,
			     1 << (APB1_RESET_UART_SHIFT + port));
	} else {
		clrbits_le32(&ccm->apb1_reset_cfg,
			     1 << (APB1_RESET_UART_SHIFT + port));
		clrbits_le32(&ccm->apb1_gate,
			     CLK_GATE_OPEN << (APB1_GATE_TWI_SHIFT + port));
	}

	return 0;
}

unsigned int clock_get_pll4_periph0(void)
{
	struct sunxi_ccm_reg *const ccm =
		(struct sunxi_ccm_reg *)SUNXI_CCM_BASE;
	uint32_t rval = readl(&ccm->pll4_periph0_cfg);
	int n = ((rval & CCM_PLL4_CTRL_N_MASK) >> CCM_PLL4_CTRL_N_SHIFT);
	int p = ((rval & CCM_PLL4_CTRL_P_MASK) >> CCM_PLL4_CTRL_P_SHIFT);
	int m = ((rval & CCM_PLL4_CTRL_M_MASK) >> CCM_PLL4_CTRL_M_SHIFT) + 1;
	const int k = 1;

	return ((24000000 * n * k) >> p) / m;
}
