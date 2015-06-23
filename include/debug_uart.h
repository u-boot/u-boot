/*
 * Early debug UART support
 *
 * (C) Copyright 2014 Google, Inc
 * Writte by Simon Glass <sjg@chromium.org>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef _DEBUG_UART_H
#define _DEBUG_UART_H

/*
 * The debug UART is intended for use very early in U-Boot to debug problems
 * when an ICE or other debug mechanism is not available.
 *
 * To use it you should:
 * - Make sure your UART supports this interface
 * - Enable CONFIG_DEBUG_UART
 * - Enable the CONFIG for your UART to tell it to provide this interface
 *       (e.g. CONFIG_DEBUG_UART_NS16550)
 * - Define the required settings as needed (see below)
 * - Call debug_uart_init() before use
 * - Call printch() to output a character
 *
 * Depending on your platform it may be possible to use this UART before a
 * stack is available.
 *
 * If your UART does not support this interface you can probably add support
 * quite easily. Remember that you cannot use driver model and it is preferred
 * to use no stack.
 *
 * You must not use this UART once driver model is working and the serial
 * drivers are up and running (done in serial_init()). Otherwise the drivers
 * may conflict and you will get strange output.
 *
 *
 * To enable the debug UART in your serial driver:
 *
 * - #include <debug_uart.h>
 * - Define debug_uart_init(), trying to avoid using the stack
 * - Define _debug_uart_putc() as static inline (avoiding stack usage)
 * - Immediately afterwards, add DEBUG_UART_FUNCS to define the rest of the
 *     functionality (printch(), etc.)
 */

/**
 * debug_uart_init() - Set up the debug UART ready for use
 *
 * This sets up the UART with the correct baud rate, etc.
 *
 * Available CONFIG is:
 *
 *    - CONFIG_DEBUG_UART_BASE: Base address of UART
 *    - CONFIG_BAUDRATE: Requested baud rate
 *    - CONFIG_DEBUG_UART_CLOCK: Input clock for UART
 */
void debug_uart_init(void);

/**
 * printch() - Output a character to the debug UART
 *
 * @ch:		Character to output
 */
void printch(int ch);

/**
 * printascii() - Output an ASCII string to the debug UART
 *
 * @str:	String to output
 */
void printascii(const char *str);

/**
 * printhex2() - Output a 2-digit hex value
 *
 * @value:	Value to output
 */
void printhex2(uint value);

/**
 * printhex4() - Output a 4-digit hex value
 *
 * @value:	Value to output
 */
void printhex4(uint value);

/**
 * printhex8() - Output a 8-digit hex value
 *
 * @value:	Value to output
 */
void printhex8(uint value);

/*
 * Now define some functions - this should be inserted into the serial driver
 */
#define DEBUG_UART_FUNCS \
	void printch(int ch) \
	{ \
		_debug_uart_putc(ch); \
	} \
\
	void printascii(const char *str) \
	{ \
		while (*str) \
			_debug_uart_putc(*str++); \
	} \
\
	static inline void printhex1(uint digit) \
	{ \
		digit &= 0xf; \
		_debug_uart_putc(digit > 9 ? digit - 10 + 'a' : digit + '0'); \
	} \
\
	static inline void printhex(uint value, int digits) \
	{ \
		while (digits-- > 0) \
			printhex1(value >> (4 * digits)); \
	} \
\
	void printhex2(uint value) \
	{ \
		printhex(value, 2); \
	} \
\
	void printhex4(uint value) \
	{ \
		printhex(value, 4); \
	} \
\
	void printhex8(uint value) \
	{ \
		printhex(value, 8); \
	}

#endif
