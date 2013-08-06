/*
 * serial.h - common serial defines for early debug and serial driver.
 *            any functions defined here must be always_inline since
 *            initcode cannot have function calls.
 *
 * Copyright (c) 2004-2011 Analog Devices Inc.
 *
 * Licensed under the GPL-2 or later.
 */

#ifndef __BFIN_CPU_SERIAL4_H__
#define __BFIN_CPU_SERIAL4_H__

#include <asm/mach-common/bits/uart4.h>

#ifndef __ASSEMBLY__

#include <asm/clock.h>

#define MMR_UART(n) _PASTE_UART(n, UART, REVID)
#define UART_BASE MMR_UART(CONFIG_UART_CONSOLE)

struct bfin_mmr_serial {
	u32 revid;
	u32 control;
	u32 status;
	u32 scr;
	u32 clock;
	u32 emask;
	u32 emaskst;
	u32 emaskcl;
	u32 rbr;
	u32 thr;
	u32 taip;
	u32 tsr;
	u32 rsr;
	u32 txdiv_cnt;
	u32 rxdiv_cnt;
};
#define uart_lsr_t uint32_t
#define _lsr_read(p)     bfin_read(&p->status)
#define _lsr_write(p, v) bfin_write(&p->status, v)

__attribute__((always_inline))
static inline void serial_early_do_mach_portmux(char port, int mux_mask,
	int mux_func, int port_pin)
{
	switch (port) {
	case 'D':
		bfin_write_PORTD_MUX((bfin_read_PORTD_MUX() &
			~mux_mask) | mux_func);
		bfin_write_PORTD_FER_SET(port_pin);
		break;
	case 'G':
		bfin_write_PORTG_MUX((bfin_read_PORTG_MUX() &
			~mux_mask) | mux_func);
		bfin_write_PORTG_FER_SET(port_pin);
		break;
	}
}

__attribute__((always_inline))
static inline void serial_early_do_portmux(void)
{
#if defined(__ADSPBF60x__)
	switch (CONFIG_UART_CONSOLE) {
	case 0:
		serial_early_do_mach_portmux('D', PORT_x_MUX_7_MASK,
		PORT_x_MUX_7_FUNC_2, PD7); /* TX: D; mux 7; func 2; PD7 */
		serial_early_do_mach_portmux('D', PORT_x_MUX_8_MASK,
		PORT_x_MUX_8_FUNC_2, PD8); /* RX: D; mux 8; func 2; PD8 */
		break;
	case 1:
		serial_early_do_mach_portmux('G', PORT_x_MUX_15_MASK,
		PORT_x_MUX_15_FUNC_1, PG15); /* TX: G; mux 15; func 1; PG15 */
		serial_early_do_mach_portmux('G', PORT_x_MUX_14_MASK,
		PORT_x_MUX_14_FUNC_1, PG14); /* RX: G; mux 14; func 1; PG14 */
		break;
	}
#else
# if (P_UART(RX) & P_DEFINED) || (P_UART(TX) & P_DEFINED)
#  error "missing portmux logic for UART"
# endif
#endif
	SSYNC();
}

__attribute__((always_inline))
static inline int uart_init(uint32_t uart_base)
{
	/* always enable UART to 8-bit mode */
	bfin_write(&pUART->control, UEN | UMOD_UART | WLS_8);

	SSYNC();

	return 0;
}

__attribute__((always_inline))
static inline int serial_early_init(uint32_t uart_base)
{
	/* handle portmux crap on different Blackfins */
	serial_do_portmux();

	return uart_init(uart_base);
}

__attribute__((always_inline))
static inline int serial_early_uninit(uint32_t uart_base)
{
	/* disable the UART by clearing UEN */
	bfin_write(&pUART->control, 0);

	return 0;
}

__attribute__((always_inline))
static inline void serial_set_divisor(uint32_t uart_base, uint16_t divisor)
{
	/* Program the divisor to get the baud rate we want */
	bfin_write(&pUART->clock, divisor);
	SSYNC();
}

__attribute__((always_inline))
static inline void serial_early_set_baud(uint32_t uart_base, uint32_t baud)
{
	uint16_t divisor = early_division(early_get_uart_clk(), baud * 16);

	/* Program the divisor to get the baud rate we want */
	serial_set_divisor(uart_base, divisor);
}

__attribute__((always_inline))
static inline void serial_early_put_div(uint32_t divisor)
{
	uint32_t uart_base = UART_BASE;
	bfin_write(&pUART->clock, divisor);
}

__attribute__((always_inline))
static inline uint32_t serial_early_get_div(void)
{
	uint32_t uart_base = UART_BASE;
	return bfin_read(&pUART->clock);
}

#endif

#endif
