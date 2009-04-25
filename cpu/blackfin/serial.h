/*
 * serial.h - common serial defines for early debug and serial driver.
 *            any functions defined here must be always_inline since
 *            initcode cannot have function calls.
 *
 * Copyright (c) 2004-2007 Analog Devices Inc.
 *
 * Licensed under the GPL-2 or later.
 */

#ifndef __BFIN_CPU_SERIAL_H__
#define __BFIN_CPU_SERIAL_H__

#include <asm/blackfin.h>
#include <asm/mach-common/bits/uart.h>

#ifndef CONFIG_UART_CONSOLE
# define CONFIG_UART_CONSOLE 0
#endif

#ifdef CONFIG_DEBUG_EARLY_SERIAL
# define BFIN_DEBUG_EARLY_SERIAL 1
#else
# define BFIN_DEBUG_EARLY_SERIAL 0
#endif

#define LOB(x) ((x) & 0xFF)
#define HIB(x) (((x) >> 8) & 0xFF)

#ifndef UART_LSR
# if (CONFIG_UART_CONSOLE == 3)
#  define pUART_DLH  pUART3_DLH
#  define pUART_DLL  pUART3_DLL
#  define pUART_GCTL pUART3_GCTL
#  define pUART_IER  pUART3_IER
#  define pUART_IERC pUART3_IER_CLEAR
#  define pUART_LCR  pUART3_LCR
#  define pUART_LSR  pUART3_LSR
#  define pUART_RBR  pUART3_RBR
#  define pUART_THR  pUART3_THR
#  define  UART_THR   UART3_THR
#  define  UART_LSR   UART3_LSR
# elif (CONFIG_UART_CONSOLE == 2)
#  define pUART_DLH  pUART2_DLH
#  define pUART_DLL  pUART2_DLL
#  define pUART_GCTL pUART2_GCTL
#  define pUART_IER  pUART2_IER
#  define pUART_IERC pUART2_IER_CLEAR
#  define pUART_LCR  pUART2_LCR
#  define pUART_LSR  pUART2_LSR
#  define pUART_RBR  pUART2_RBR
#  define pUART_THR  pUART2_THR
#  define  UART_THR   UART2_THR
#  define  UART_LSR   UART2_LSR
# elif (CONFIG_UART_CONSOLE == 1)
#  define pUART_DLH  pUART1_DLH
#  define pUART_DLL  pUART1_DLL
#  define pUART_GCTL pUART1_GCTL
#  define pUART_IER  pUART1_IER
#  define pUART_IERC pUART1_IER_CLEAR
#  define pUART_LCR  pUART1_LCR
#  define pUART_LSR  pUART1_LSR
#  define pUART_RBR  pUART1_RBR
#  define pUART_THR  pUART1_THR
#  define  UART_THR   UART1_THR
#  define  UART_LSR   UART1_LSR
# elif (CONFIG_UART_CONSOLE == 0)
#  define pUART_DLH  pUART0_DLH
#  define pUART_DLL  pUART0_DLL
#  define pUART_GCTL pUART0_GCTL
#  define pUART_IER  pUART0_IER
#  define pUART_IERC pUART0_IER_CLEAR
#  define pUART_LCR  pUART0_LCR
#  define pUART_LSR  pUART0_LSR
#  define pUART_RBR  pUART0_RBR
#  define pUART_THR  pUART0_THR
#  define  UART_THR   UART0_THR
#  define  UART_LSR   UART0_LSR
# endif
#endif

#ifndef __ASSEMBLY__

#ifdef __ADSPBF54x__
# define ACCESS_LATCH()
# define ACCESS_PORT_IER()
# define CLEAR_IER()       (*pUART_IERC = 0)
#else
# define ACCESS_LATCH()    (*pUART_LCR |= DLAB)
# define ACCESS_PORT_IER() (*pUART_LCR &= ~DLAB)
# define CLEAR_IER()       (*pUART_IER = 0)
#endif

__attribute__((always_inline))
static inline void serial_do_portmux(void)
{
#if defined(__ADSPBF51x__)
# define DO_MUX(port, mux_tx, mux_rx, tx, rx) \
	bfin_write_PORT##port##_MUX((bfin_read_PORT##port##_MUX() & ~(PORT_x_MUX_##mux_tx##_MASK | PORT_x_MUX_##mux_rx##_MASK)) | PORT_x_MUX_##mux_tx##_FUNC_2 | PORT_x_MUX_##mux_rx##_FUNC_2); \
	bfin_write_PORT##port##_FER(bfin_read_PORT##port##_FER() | P##port##tx | P##port##rx);
	switch (CONFIG_UART_CONSOLE) {
	case 0: DO_MUX(G, 5, 5, 9, 10);  break;	/* Port G; mux 5; PG9 and PG10 */
	case 1: DO_MUX(F, 2, 3, 14, 15); break;	/* Port H; mux 2/3; PH14 and PH15 */
	}
	SSYNC();
#elif defined(__ADSPBF52x__)
# define DO_MUX(port, mux, tx, rx) \
	bfin_write_PORT##port##_MUX((bfin_read_PORT##port##_MUX() & ~PORT_x_MUX_##mux##_MASK) | PORT_x_MUX_##mux##_FUNC_3); \
	bfin_write_PORT##port##_FER(bfin_read_PORT##port##_FER() | P##port##tx | P##port##rx);
	switch (CONFIG_UART_CONSOLE) {
	case 0: DO_MUX(G, 2, 7, 8);   break;	/* Port G; mux 2; PG2 and PG8 */
	case 1: DO_MUX(F, 5, 14, 15); break;	/* Port F; mux 5; PF14 and PF15 */
	}
	SSYNC();
#elif defined(__ADSPBF537__) || defined(__ADSPBF536__) || defined(__ADSPBF534__)
# define DO_MUX(func, tx, rx) \
	bfin_write_PORT_MUX(bfin_read_PORT_MUX() & ~(func)); \
	bfin_write_PORTF_FER(bfin_read_PORTF_FER() | PF##tx | PF##rx);
	switch (CONFIG_UART_CONSOLE) {
	case 0: DO_MUX(PFDE, 0, 1); break;
	case 1: DO_MUX(PFTE, 2, 3); break;
	}
	SSYNC();
#elif defined(__ADSPBF54x__)
# define DO_MUX(port, tx, rx) \
	bfin_write_PORT##port##_MUX((bfin_read_PORT##port##_MUX() & ~(PORT_x_MUX_##tx##_MASK | PORT_x_MUX_##rx##_MASK)) | PORT_x_MUX_##tx##_FUNC_1 | PORT_x_MUX_##rx##_FUNC_1); \
	bfin_write_PORT##port##_FER(bfin_read_PORT##port##_FER() | P##port##tx | P##port##rx);
	switch (CONFIG_UART_CONSOLE) {
	case 0: DO_MUX(E, 7, 8); break;	/* Port E; PE7 and PE8 */
	case 1: DO_MUX(H, 0, 1); break;	/* Port H; PH0 and PH1 */
	case 2: DO_MUX(B, 4, 5); break;	/* Port B; PB4 and PB5 */
	case 3: DO_MUX(B, 6, 7); break;	/* Port B; PB6 and PB7 */
	}
	SSYNC();
#endif
}

__attribute__((always_inline))
static inline void serial_early_init(void)
{
	/* handle portmux crap on different Blackfins */
	serial_do_portmux();

	/* always enable UART -- avoids anomalies 05000309 and 05000350 */
	*pUART_GCTL = UCEN;

	/* Set LCR to Word Lengh 8-bit word select */
	*pUART_LCR = WLS_8;

	SSYNC();
}

__attribute__((always_inline))
static inline void serial_early_put_div(uint16_t divisor)
{
	/* Set DLAB in LCR to Access DLL and DLH */
	ACCESS_LATCH();
	SSYNC();

	/* Program the divisor to get the baud rate we want */
	*pUART_DLL = LOB(divisor);
	*pUART_DLH = HIB(divisor);
	SSYNC();

	/* Clear DLAB in LCR to Access THR RBR IER */
	ACCESS_PORT_IER();
	SSYNC();
}

__attribute__((always_inline))
static inline uint16_t serial_early_get_div(void)
{
	/* Set DLAB in LCR to Access DLL and DLH */
	ACCESS_LATCH();
	SSYNC();

	uint8_t dll = *pUART_DLL;
	uint8_t dlh = *pUART_DLH;
	uint16_t divisor = (dlh << 8) | dll;

	/* Clear DLAB in LCR to Access THR RBR IER */
	ACCESS_PORT_IER();
	SSYNC();

	return divisor;
}

/* We cannot use get_sclk() early on as it uses caches in external memory */
#if defined(BFIN_IN_INITCODE) || defined(CONFIG_DEBUG_EARLY_SERIAL)
# define get_sclk() (CONFIG_CLKIN_HZ * CONFIG_VCO_MULT / CONFIG_SCLK_DIV)
#endif

__attribute__((always_inline))
static inline void serial_early_set_baud(uint32_t baud)
{
	/* Translate from baud into divisor in terms of SCLK.  The
	 * weird multiplication is to make sure we over sample just
	 * a little rather than under sample the incoming signals.
	 */
	serial_early_put_div((get_sclk() + (baud * 8)) / (baud * 16) - ANOMALY_05000230);
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
#ifdef CONFIG_DEBUG_EARLY_SERIAL
	call _serial_initialize;
#endif
.endm

.macro serial_early_set_baud
#ifdef CONFIG_DEBUG_EARLY_SERIAL
	R0.L = LO(CONFIG_BAUDRATE);
	R0.H = HI(CONFIG_BAUDRATE);
	call _serial_set_baud;
#endif
.endm

/* Since we embed the string right into our .text section, we need
 * to find its address.  We do this by getting our PC and adding 2
 * bytes (which is the length of the jump instruction).  Then we
 * pass this address to serial_puts().
 */
#ifdef CONFIG_DEBUG_EARLY_SERIAL
# define serial_early_puts(str) \
	call _get_pc; \
	jump 1f; \
	.ascii "Early:"; \
	.ascii __FILE__; \
	.ascii ": "; \
	.ascii str; \
	.asciz "\n"; \
	.align 4; \
1: \
	R0 += 2; \
	call _serial_puts;
#else
# define serial_early_puts(str)
#endif

#endif

#endif
