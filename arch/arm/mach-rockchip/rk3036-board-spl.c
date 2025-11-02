// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2015-2019 Rockchip Electronics Co., Ltd
 */

#include <debug_uart.h>
#include <init.h>
#include <asm/arch-rockchip/bootrom.h>
#include <asm/arch-rockchip/sdram_rk3036.h>
#include <asm/arch-rockchip/timer.h>

void board_init_f(ulong dummy)
{
#ifdef CONFIG_DEBUG_UART
	debug_uart_init();
#endif

	/* Init secure timer */
	rockchip_stimer_init();
	/* Init ARM arch timer in arch/arm/cpu/armv7/arch_timer.c */
	timer_init();

	sdram_init();

	/* return to maskrom */
	back_to_bootrom(BROM_BOOT_NEXTSTAGE);
}

/* Place Holders */
void board_init_r(gd_t *id, ulong dest_addr)
{
	/*
	 * Function attribute is no-return
	 * This Function never executes
	 */
	while (1)
		;
}
