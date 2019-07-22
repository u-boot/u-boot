// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2019 Rockchip Electronics Co., Ltd
 */
#include <common.h>
#include <asm/io.h>
#include <asm/arch-rockchip/grf_rk3188.h>
#include <asm/arch-rockchip/hardware.h>

#define GRF_BASE	0x20008000

#ifdef CONFIG_DEBUG_UART_BOARD_INIT
void board_debug_uart_init(void)
{
	/* Enable early UART on the RK3188 */
	struct rk3188_grf * const grf = (void *)GRF_BASE;
	enum {
		GPIO1B1_SHIFT		= 2,
		GPIO1B1_MASK		= 3,
		GPIO1B1_GPIO		= 0,
		GPIO1B1_UART2_SOUT,
		GPIO1B1_JTAG_TDO,

		GPIO1B0_SHIFT		= 0,
		GPIO1B0_MASK		= 3,
		GPIO1B0_GPIO		= 0,
		GPIO1B0_UART2_SIN,
		GPIO1B0_JTAG_TDI,
	};

	rk_clrsetreg(&grf->gpio1b_iomux,
		     GPIO1B1_MASK << GPIO1B1_SHIFT |
		     GPIO1B0_MASK << GPIO1B0_SHIFT,
		     GPIO1B1_UART2_SOUT << GPIO1B1_SHIFT |
		     GPIO1B0_UART2_SIN << GPIO1B0_SHIFT);
}
#endif

int arch_cpu_init(void)
{
#ifdef CONFIG_ROCKCHIP_USB_UART
	struct rk3188_grf * const grf = (void *)GRF_BASE;

	rk_clrsetreg(&grf->uoc0_con[0],
		     SIDDQ_MASK | UOC_DISABLE_MASK | COMMON_ON_N_MASK,
		     1 << SIDDQ_SHIFT | 1 << UOC_DISABLE_SHIFT |
		     1 << COMMON_ON_N_SHIFT);
	rk_clrsetreg(&grf->uoc0_con[2],
		     SOFT_CON_SEL_MASK, 1 << SOFT_CON_SEL_SHIFT);
	rk_clrsetreg(&grf->uoc0_con[3],
		     OPMODE_MASK | XCVRSELECT_MASK |
		     TERMSEL_FULLSPEED_MASK | SUSPENDN_MASK,
		     OPMODE_NODRIVING << OPMODE_SHIFT |
		     XCVRSELECT_FSTRANSC << XCVRSELECT_SHIFT |
		     1 << TERMSEL_FULLSPEED_SHIFT |
		     1 << SUSPENDN_SHIFT);
	rk_clrsetreg(&grf->uoc0_con[0],
		     BYPASSSEL_MASK | BYPASSDMEN_MASK,
		     1 << BYPASSSEL_SHIFT | 1 << BYPASSDMEN_SHIFT);
#endif
	return 0;
}
