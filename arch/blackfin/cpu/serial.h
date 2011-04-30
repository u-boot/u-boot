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

#ifndef __ASSEMBLY__

#include <asm/portmux.h>

#define LOB(x) ((x) & 0xFF)
#define HIB(x) (((x) >> 8) & 0xFF)

#if defined(__ADSPBF50x__) || defined(__ADSPBF54x__)
# define BFIN_UART_HW_VER 2
#else
# define BFIN_UART_HW_VER 1
#endif

/*
 * All Blackfin system MMRs are padded to 32bits even if the register
 * itself is only 16bits.  So use a helper macro to streamline this.
 */
#define __BFP(m) u16 m; u16 __pad_##m
struct bfin_mmr_serial {
#if BFIN_UART_HW_VER == 2
	__BFP(dll);
	__BFP(dlh);
	__BFP(gctl);
	__BFP(lcr);
	__BFP(mcr);
	__BFP(lsr);
	__BFP(msr);
	__BFP(scr);
	__BFP(ier_set);
	__BFP(ier_clear);
	__BFP(thr);
	__BFP(rbr);
#else
	union {
		u16 dll;
		u16 thr;
		const u16 rbr;
	};
	const u16 __spad0;
	union {
		u16 dlh;
		u16 ier;
	};
	const u16 __spad1;
	const __BFP(iir);
	__BFP(lcr);
	__BFP(mcr);
	__BFP(lsr);
	__BFP(msr);
	__BFP(scr);
	const u32 __spad2;
	__BFP(gctl);
#endif
};
#undef __BFP

#define __PASTE_UART(num, pfx, sfx) pfx##num##_##sfx
#define _PASTE_UART(num, pfx, sfx) __PASTE_UART(num, pfx, sfx)
#define MMR_UART(n) _PASTE_UART(n, UART, DLL)
#define _P_UART(n, pin) _PASTE_UART(n, P_UART, pin)
#define P_UART(pin) _P_UART(CONFIG_UART_CONSOLE, pin)

#ifndef UART_DLL
# define UART_DLL MMR_UART(CONFIG_UART_CONSOLE)
#else
# define UART0_DLL UART_DLL
# if CONFIG_UART_CONSOLE != 0
#  error CONFIG_UART_CONSOLE must be 0 on parts with only one UART
# endif
#endif
#define pUART ((volatile struct bfin_mmr_serial *)uart_base)

#if BFIN_UART_HW_VER == 2
# define ACCESS_LATCH()
# define ACCESS_PORT_IER()
#else
# define ACCESS_LATCH() \
	bfin_write16(&pUART->lcr, bfin_read16(&pUART->lcr) | DLAB)
# define ACCESS_PORT_IER() \
	bfin_write16(&pUART->lcr, bfin_read16(&pUART->lcr) & ~DLAB)
#endif

__attribute__((always_inline))
static inline void serial_do_portmux(void)
{
	if (!BFIN_DEBUG_EARLY_SERIAL) {
		const unsigned short pins[] = { P_UART(RX), P_UART(TX), 0, };
		peripheral_request_list(pins, "bfin-uart");
		return;
	}

#if defined(__ADSPBF50x__)
# define DO_MUX(port, mux_tx, mux_rx, tx, rx) \
	bfin_write_PORT##port##_MUX((bfin_read_PORT##port##_MUX() & ~(PORT_x_MUX_##mux_tx##_MASK | PORT_x_MUX_##mux_rx##_MASK)) | PORT_x_MUX_##mux_tx##_FUNC_1 | PORT_x_MUX_##mux_rx##_FUNC_1); \
	bfin_write_PORT##port##_FER(bfin_read_PORT##port##_FER() | P##port##tx | P##port##rx);
	switch (CONFIG_UART_CONSOLE) {
	case 0: DO_MUX(G, 7, 7, 12, 13); break;	/* Port G; mux 7; PG12 and PG13 */
	case 1: DO_MUX(F, 3, 3, 6, 7);   break;	/* Port F; mux 3; PF6 and PF7 */
	}
	SSYNC();
#elif defined(__ADSPBF51x__)
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
	const uint16_t func[] = { PFDE, PFTE, };
	bfin_write_PORT_MUX(bfin_read_PORT_MUX() & ~func[CONFIG_UART_CONSOLE]);
	bfin_write_PORTF_FER(bfin_read_PORTF_FER() |
	                     (1 << P_IDENT(P_UART(RX))) |
	                     (1 << P_IDENT(P_UART(TX))));
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
#elif defined(__ADSPBF561__)
	/* UART pins could be GPIO, but they aren't pin muxed.  */
#else
# if (P_UART(RX) & P_DEFINED) || (P_UART(TX) & P_DEFINED)
#  error "missing portmux logic for UART"
# endif
#endif
}

__attribute__((always_inline))
static inline int uart_init(uint32_t uart_base)
{
	/* always enable UART -- avoids anomalies 05000309 and 05000350 */
	bfin_write16(&pUART->gctl, UCEN);

	/* Set LCR to Word Lengh 8-bit word select */
	bfin_write16(&pUART->lcr, WLS_8);

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
	/* disable the UART by clearing UCEN */
	bfin_write16(&pUART->gctl, 0);

	return 0;
}

__attribute__((always_inline))
static inline void serial_early_put_div(uint32_t uart_base, uint16_t divisor)
{
	/* Set DLAB in LCR to Access DLL and DLH */
	ACCESS_LATCH();
	SSYNC();

	/* Program the divisor to get the baud rate we want */
	bfin_write16(&pUART->dll, LOB(divisor));
	bfin_write16(&pUART->dlh, HIB(divisor));
	SSYNC();

	/* Clear DLAB in LCR to Access THR RBR IER */
	ACCESS_PORT_IER();
	SSYNC();
}

__attribute__((always_inline))
static inline uint16_t serial_early_get_div(void)
{
	uint32_t uart_base = UART_DLL;

	/* Set DLAB in LCR to Access DLL and DLH */
	ACCESS_LATCH();
	SSYNC();

	uint8_t dll = bfin_read16(&pUART->dll);
	uint8_t dlh = bfin_read16(&pUART->dlh);
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
static inline void serial_early_set_baud(uint32_t uart_base, uint32_t baud)
{
	/* Translate from baud into divisor in terms of SCLK.  The
	 * weird multiplication is to make sure we over sample just
	 * a little rather than under sample the incoming signals.
	 */
	serial_early_put_div(uart_base,
		(get_sclk() + (baud * 8)) / (baud * 16) - ANOMALY_05000230);
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
