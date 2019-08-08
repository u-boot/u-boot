// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2019 Microchip Technology Inc. and its subsidiaries
 *
 * Author: Nicolas Ferre <nicolas.ferre@microcihp.com>
 */

#include <common.h>
#include <debug_uart.h>
#include <asm/io.h>
#include <asm/arch/at91_common.h>
#include <asm/arch/atmel_pio4.h>
#include <asm/arch/atmel_mpddrc.h>
#include <asm/arch/atmel_sdhci.h>
#include <asm/arch/clk.h>
#include <asm/arch/gpio.h>
#include <asm/arch/sama5d2.h>

extern void at91_pda_detect(void);

DECLARE_GLOBAL_DATA_PTR;

#ifdef CONFIG_BOARD_LATE_INIT
int board_late_init(void)
{
#ifdef CONFIG_DM_VIDEO
	at91_video_show_board_info();
#endif
	at91_pda_detect();
	return 0;
}
#endif

#ifdef CONFIG_DEBUG_UART_BOARD_INIT
static void board_uart0_hw_init(void)
{
	atmel_pio4_set_c_periph(AT91_PIO_PORTB, 26, ATMEL_PIO_PUEN_MASK);	/* URXD0 */
	atmel_pio4_set_c_periph(AT91_PIO_PORTB, 27, 0);				/* UTXD0 */

	at91_periph_clk_enable(ATMEL_ID_UART0);
}

void board_debug_uart_init(void)
{
	board_uart0_hw_init();
}
#endif

#ifdef CONFIG_BOARD_EARLY_INIT_F
int board_early_init_f(void)
{
#ifdef CONFIG_DEBUG_UART
	debug_uart_init();
#endif

	return 0;
}
#endif

int board_init(void)
{
	/* address of boot parameters */
	gd->bd->bi_boot_params = CONFIG_SYS_SDRAM_BASE + 0x100;

	return 0;
}

#ifdef CONFIG_MISC_INIT_R
int misc_init_r(void)
{
	return 0;
}
#endif

int dram_init(void)
{
	gd->ram_size = get_ram_size((void *)CONFIG_SYS_SDRAM_BASE,
				    CONFIG_SYS_SDRAM_SIZE);
	return 0;
}
