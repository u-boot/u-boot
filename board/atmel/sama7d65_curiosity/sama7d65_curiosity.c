// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2024 Microchip Technology, Inc. and its subsidiaries
 *
 * Author: Ryan Wanner <ryan.wanner@microchip.com>
 *
 */

#include <debug_uart.h>
#include <init.h>
#include <fdtdec.h>
#include <asm/global_data.h>
#include <asm/io.h>
#include <asm/arch/at91_common.h>
#include <asm/arch/atmel_pio4.h>
#include <asm/arch/clk.h>
#include <asm/arch/gpio.h>
#include <asm/arch/sama7d65.h>
#include <asm/mach-types.h>

DECLARE_GLOBAL_DATA_PTR;

static void board_leds_init(void)
{
	atmel_pio4_set_pio_output(AT91_PIO_PORTA, 21, 0);	/* LED BLUE */
	atmel_pio4_set_pio_output(AT91_PIO_PORTB, 17, 0);	/* LED RED */
	atmel_pio4_set_pio_output(AT91_PIO_PORTB, 15, 1);	/* LED GREEN */
}

int board_late_init(void)
{
	return 0;
}

#if (IS_ENABLED(CONFIG_DEBUG_UART_BOARD_INIT))
static void board_uart0_hw_init(void)
{
	/* FLEXCOM6 IO0 */
	atmel_pio4_set_b_periph(AT91_PIO_PORTD, 18, 0);
	/* FLEXCOM6 IO1 */
	atmel_pio4_set_b_periph(AT91_PIO_PORTD, 19, 0);

	at91_periph_clk_enable(ATMEL_ID_FLEXCOM6);
}

void board_debug_uart_init(void)
{
	board_uart0_hw_init();
}
#endif

int board_early_init_f(void)
{
	return 0;
}

int board_init(void)
{
	/* address of boot parameters */
	gd->bd->bi_boot_params = gd->bd->bi_dram[0].start + 0x100;

	board_leds_init();

	return 0;
}

int dram_init_banksize(void)
{
	return fdtdec_setup_memory_banksize();
}

int dram_init(void)
{
	return fdtdec_setup_mem_size_base();
}
