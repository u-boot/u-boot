/*
 * serial.h - common serial defines for early debug and serial driver.
 *            any functions defined here must be always_inline since
 *            initcode cannot have function calls.
 *
 * Copyright (c) 2004-2011 Analog Devices Inc.
 *
 * Licensed under the GPL-2 or later.
 */

#ifndef __BFIN_CPU_SERIAL_H__
#define __BFIN_CPU_SERIAL_H__

#include <asm/blackfin.h>
#include <asm/portmux.h>

#ifndef CONFIG_UART_CONSOLE
# define CONFIG_UART_CONSOLE 0
#endif

#ifdef CONFIG_DEBUG_EARLY_SERIAL
# define BFIN_DEBUG_EARLY_SERIAL 1
#else
# define BFIN_DEBUG_EARLY_SERIAL 0
#endif

#if defined(__ADSPBF60x__)
# define BFIN_UART_HW_VER 4
#elif defined(__ADSPBF50x__) || defined(__ADSPBF54x__)
# define BFIN_UART_HW_VER 2
#else
# define BFIN_UART_HW_VER 1
#endif

#define __PASTE_UART(num, pfx, sfx) pfx##num##_##sfx
#define _PASTE_UART(num, pfx, sfx) __PASTE_UART(num, pfx, sfx)
#define _P_UART(n, pin) _PASTE_UART(n, P_UART, pin)
#define P_UART(pin) _P_UART(CONFIG_UART_CONSOLE, pin)

#define pUART ((volatile struct bfin_mmr_serial *)uart_base)

#ifndef __ASSEMBLY__
__attribute__((always_inline))
static inline void serial_do_portmux(void);
#endif

#if BFIN_UART_HW_VER < 4
# include "serial1.h"
#else
# include "serial4.h"
#endif

#ifndef __ASSEMBLY__

__attribute__((always_inline))
static inline void serial_do_portmux(void)
{
	if (!BFIN_DEBUG_EARLY_SERIAL) {
		const unsigned short pins[] = { P_UART(RX), P_UART(TX), 0, };
		peripheral_request_list(pins, "bfin-uart");
		return;
	}

	serial_early_do_portmux();
}

#ifndef BFIN_IN_INITCODE
__attribute__((always_inline))
static inline void serial_early_puts(const char *s)
{
	if (BFIN_DEBUG_EARLY_SERIAL) {
		serial_puts("Early: ");
		serial_puts(s);
	}
}
#endif

#else

.macro serial_early_init
#if defined(CONFIG_DEBUG_EARLY_SERIAL) && !defined(CONFIG_UART_MEM)
	call __serial_early_init;
#endif
.endm

.macro serial_early_set_baud
#if defined(CONFIG_DEBUG_EARLY_SERIAL) && !defined(CONFIG_UART_MEM)
	R0.L = LO(CONFIG_BAUDRATE);
	R0.H = HI(CONFIG_BAUDRATE);
	call __serial_early_set_baud;
#endif
.endm

#if CONFIG_BFIN_BOOT_MODE == BFIN_BOOT_BYPASS
#define update_serial_early_string_addr \
	R1.L = _start; \
	R1.H = _start; \
	R0 = R0 - R1; \
	R1.L = 0; \
	R1.H = 0x2000; \
	R0 = R0 + R1;
#else
#define update_serial_early_string_addr
#endif

/* Since we embed the string right into our .text section, we need
 * to find its address.  We do this by getting our PC and adding 2
 * bytes (which is the length of the jump instruction).  Then we
 * pass this address to serial_puts().
 */
#ifdef CONFIG_DEBUG_EARLY_SERIAL
# define serial_early_puts(str) \
	.section .rodata; \
	7: \
	.ascii "Early:"; \
	.ascii __FILE__; \
	.ascii ": "; \
	.ascii str; \
	.asciz "\n"; \
	.previous; \
	R0.L = 7b; \
	R0.H = 7b; \
	update_serial_early_string_addr \
	call _uart_early_puts;
#else
# define serial_early_puts(str)
#endif

#endif

#endif
