/*
 * serial.h - common serial defines for early debug and serial driver.
 *            any functions defined here must be always_inline since
 *            initcode cannot have function calls.
 *
 * Copyright (c) 2004-2011 Analog Devices Inc.
 *
 * Licensed under the GPL-2 or later.
 */

#ifndef __BFIN_CPU_SERIAL1_H__
#define __BFIN_CPU_SERIAL1_H__

#include <asm/mach-common/bits/uart.h>

#ifndef __ASSEMBLY__

#include <asm/clock.h>

#define MMR_UART(n) _PASTE_UART(n, UART, DLL)
#ifdef UART_DLL
# define UART0_DLL UART_DLL
# if CONFIG_UART_CONSOLE != 0
#  error CONFIG_UART_CONSOLE must be 0 on parts with only one UART
# endif
#endif
#define UART_BASE MMR_UART(CONFIG_UART_CONSOLE)

#define LOB(x) ((x) & 0xFF)
#define HIB(x) (((x) >> 8) & 0xFF)

/*
 * All Blackfin system MMRs are padded to 32bits even if the register
 * itself is only 16bits.  So use a helper macro to streamline this.
 */
struct bfin_mmr_serial {
#if BFIN_UART_HW_VER == 2
	u16 dll;
	u16 __pad_0;
	u16 dlh;
	u16 __pad_1;
	u16 gctl;
	u16 __pad_2;
	u16 lcr;
	u16 __pad_3;
	u16 mcr;
	u16 __pad_4;
	u16 lsr;
	u16 __pad_5;
	u16 msr;
	u16 __pad_6;
	u16 scr;
	u16 __pad_7;
	u16 ier_set;
	u16 __pad_8;
	u16 ier_clear;
	u16 __pad_9;
	u16 thr;
	u16 __pad_10;
	u16 rbr;
	u16 __pad_11;
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
	const u16 iir;
	u16 __pad_0;
	u16 lcr;
	u16 __pad_1;
	u16 mcr;
	u16 __pad_2;
	u16 lsr;
	u16 __pad_3;
	u16 msr;
	u16 __pad_4;
	u16 scr;
	u16 __pad_5;
	const u32 __spad2;
	u16 gctl;
	u16 __pad_6;
#endif
};

#define uart_lsr_t uint32_t
#define _lsr_read(p)     bfin_read(&p->lsr)
#define _lsr_write(p, v) bfin_write(&p->lsr, v)

#if BFIN_UART_HW_VER == 2
# define ACCESS_LATCH()
# define ACCESS_PORT_IER()
#else
# define ACCESS_LATCH()    bfin_write_or(&pUART->lcr, DLAB)
# define ACCESS_PORT_IER() bfin_write_and(&pUART->lcr, ~DLAB)
#endif

__attribute__((always_inline))
static inline void serial_early_do_mach_portmux(char port, int mux_mask,
	int mux_func, int port_pin)
{
	switch (port) {
#if defined(__ADSPBF54x__)
	case 'B':
		bfin_write_PORTB_MUX((bfin_read_PORTB_MUX() &
			~mux_mask) | mux_func);
		bfin_write_PORTB_FER(bfin_read_PORTB_FER() | port_pin);
		break;
	case 'E':
		bfin_write_PORTE_MUX((bfin_read_PORTE_MUX() &
			~mux_mask) | mux_func);
		bfin_write_PORTE_FER(bfin_read_PORTE_FER() | port_pin);
		break;
#endif
#if defined(__ADSPBF50x__) || defined(__ADSPBF51x__) || defined(__ADSPBF52x__)
	case 'F':
		bfin_write_PORTF_MUX((bfin_read_PORTF_MUX() &
			~mux_mask) | mux_func);
		bfin_write_PORTF_FER(bfin_read_PORTF_FER() | port_pin);
		break;
	case 'G':
		bfin_write_PORTG_MUX((bfin_read_PORTG_MUX() &
			~mux_mask) | mux_func);
		bfin_write_PORTG_FER(bfin_read_PORTG_FER() | port_pin);
		break;
	case 'H':
		bfin_write_PORTH_MUX((bfin_read_PORTH_MUX() &
			~mux_mask) | mux_func);
		bfin_write_PORTH_FER(bfin_read_PORTH_FER() | port_pin);
		break;
#endif
	default:
		break;
	}
}

__attribute__((always_inline))
static inline void serial_early_do_portmux(void)
{
#if defined(__ADSPBF50x__)
	switch (CONFIG_UART_CONSOLE) {
	case 0:
		serial_early_do_mach_portmux('G', PORT_x_MUX_7_MASK,
		PORT_x_MUX_7_FUNC_1, PG12); /* TX: G; mux 7; func 1; PG12 */
		serial_early_do_mach_portmux('G', PORT_x_MUX_7_MASK,
		PORT_x_MUX_7_FUNC_1, PG13); /* RX: G; mux 7; func 1; PG13 */
		break;
	case 1:
		serial_early_do_mach_portmux('F', PORT_x_MUX_3_MASK,
		PORT_x_MUX_3_FUNC_1, PF7); /* TX: F; mux 3; func 1; PF6 */
		serial_early_do_mach_portmux('F', PORT_x_MUX_3_MASK,
		PORT_x_MUX_3_FUNC_1, PF6); /* RX: F; mux 3; func 1; PF7 */
		break;
	}
#elif defined(__ADSPBF51x__)
	switch (CONFIG_UART_CONSOLE) {
	case 0:
		serial_early_do_mach_portmux('G', PORT_x_MUX_5_MASK,
		PORT_x_MUX_5_FUNC_2, PG9); /* TX: G; mux 5; func 2; PG9 */
		serial_early_do_mach_portmux('G', PORT_x_MUX_5_MASK,
		PORT_x_MUX_5_FUNC_2, PG10); /* RX: G; mux 5; func 2; PG10 */
		break;
	case 1:
		serial_early_do_mach_portmux('H', PORT_x_MUX_3_MASK,
		PORT_x_MUX_3_FUNC_2, PH7); /* TX: H; mux 3; func 2; PH6 */
		serial_early_do_mach_portmux('H', PORT_x_MUX_3_MASK,
		PORT_x_MUX_3_FUNC_2, PH6); /* RX: H; mux 3; func 2; PH7 */
		break;
	}
#elif defined(__ADSPBF52x__)
	switch (CONFIG_UART_CONSOLE) {
	case 0:
		serial_early_do_mach_portmux('G', PORT_x_MUX_2_MASK,
		PORT_x_MUX_2_FUNC_3, PG7); /* TX: G; mux 2; func 3; PG7 */
		serial_early_do_mach_portmux('G', PORT_x_MUX_2_MASK,
		PORT_x_MUX_2_FUNC_3, PG8); /* RX: G; mux 2; func 3; PG8 */
		break;
	case 1:
		serial_early_do_mach_portmux('F', PORT_x_MUX_5_MASK,
		PORT_x_MUX_5_FUNC_3, PF14); /* TX: F; mux 5; func 3; PF14 */
		serial_early_do_mach_portmux('F', PORT_x_MUX_5_MASK,
		PORT_x_MUX_5_FUNC_3, PF15); /* RX: F; mux 5; func 3; PF15 */
		break;
	}
#elif defined(__ADSPBF537__) || defined(__ADSPBF536__) || defined(__ADSPBF534__)
	const uint16_t func[] = { PFDE, PFTE, };
	bfin_write_PORT_MUX(bfin_read_PORT_MUX() & ~func[CONFIG_UART_CONSOLE]);
	bfin_write_PORTF_FER(bfin_read_PORTF_FER() |
			(1 << P_IDENT(P_UART(RX))) |
			(1 << P_IDENT(P_UART(TX))));
#elif defined(__ADSPBF54x__)
	switch (CONFIG_UART_CONSOLE) {
	case 0:
		serial_early_do_mach_portmux('E', PORT_x_MUX_7_MASK,
		PORT_x_MUX_7_FUNC_1, PE7); /* TX: E; mux 7; func 1; PE7 */
		serial_early_do_mach_portmux('E', PORT_x_MUX_8_MASK,
		PORT_x_MUX_8_FUNC_1, PE8); /* RX: E; mux 8; func 1; PE8 */
		break;
	case 1:
		serial_early_do_mach_portmux('H', PORT_x_MUX_0_MASK,
		PORT_x_MUX_0_FUNC_1, PH0); /* TX: H; mux 0; func 1; PH0 */
		serial_early_do_mach_portmux('H', PORT_x_MUX_1_MASK,
		PORT_x_MUX_1_FUNC_1, PH1); /* RX: H; mux 1; func 1; PH1 */
		break;
	case 2:
		serial_early_do_mach_portmux('B', PORT_x_MUX_4_MASK,
		PORT_x_MUX_4_FUNC_1, PB4); /* TX: B; mux 4; func 1; PB4 */
		serial_early_do_mach_portmux('B', PORT_x_MUX_5_MASK,
		PORT_x_MUX_5_FUNC_1, PB5); /* RX: B; mux 5; func 1; PB5 */
		break;
	case 3:
		serial_early_do_mach_portmux('B', PORT_x_MUX_6_MASK,
		PORT_x_MUX_6_FUNC_1, PB6); /* TX: B; mux 6; func 1; PB6 */
		serial_early_do_mach_portmux('B', PORT_x_MUX_7_MASK,
		PORT_x_MUX_7_FUNC_1, PB7); /* RX: B; mux 7; func 1; PB7 */
		break;
	}
#elif defined(__ADSPBF561__)
	/* UART pins could be GPIO, but they aren't pin muxed.  */
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
	/* always enable UART -- avoids anomalies 05000309 and 05000350 */
	bfin_write(&pUART->gctl, UCEN);

	/* Set LCR to Word Lengh 8-bit word select */
	bfin_write(&pUART->lcr, WLS_8);

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
	bfin_write(&pUART->gctl, 0);

	return 0;
}

__attribute__((always_inline))
static inline void serial_set_divisor(uint32_t uart_base, uint16_t divisor)
{
	/* Set DLAB in LCR to Access DLL and DLH */
	ACCESS_LATCH();
	SSYNC();

	/* Program the divisor to get the baud rate we want */
	bfin_write(&pUART->dll, LOB(divisor));
	bfin_write(&pUART->dlh, HIB(divisor));
	SSYNC();

	/* Clear DLAB in LCR to Access THR RBR IER */
	ACCESS_PORT_IER();
	SSYNC();
}

__attribute__((always_inline))
static inline void serial_early_set_baud(uint32_t uart_base, uint32_t baud)
{
	/* Translate from baud into divisor in terms of SCLK.  The
	 * weird multiplication is to make sure we over sample just
	 * a little rather than under sample the incoming signals.
	 */
#if CONFIG_BFIN_BOOT_MODE == BFIN_BOOT_BYPASS
	uint16_t divisor = (early_get_uart_clk() + baud * 8) / (baud * 16)
			- ANOMALY_05000230;
#else
	uint16_t divisor = early_division(early_get_uart_clk() + (baud * 8),
			baud * 16) - ANOMALY_05000230;
#endif

	serial_set_divisor(uart_base, divisor);
}

__attribute__((always_inline))
static inline void serial_early_put_div(uint16_t divisor)
{
	uint32_t uart_base = UART_BASE;

	/* Set DLAB in LCR to Access DLL and DLH */
	ACCESS_LATCH();
	SSYNC();

	/* Program the divisor to get the baud rate we want */
	bfin_write(&pUART->dll, LOB(divisor));
	bfin_write(&pUART->dlh, HIB(divisor));
	SSYNC();

	/* Clear DLAB in LCR to Access THR RBR IER */
	ACCESS_PORT_IER();
	SSYNC();
}

__attribute__((always_inline))
static inline uint16_t serial_early_get_div(void)
{
	uint32_t uart_base = UART_BASE;

	/* Set DLAB in LCR to Access DLL and DLH */
	ACCESS_LATCH();
	SSYNC();

	uint8_t dll = bfin_read(&pUART->dll);
	uint8_t dlh = bfin_read(&pUART->dlh);
	uint16_t divisor = (dlh << 8) | dll;

	/* Clear DLAB in LCR to Access THR RBR IER */
	ACCESS_PORT_IER();
	SSYNC();

	return divisor;
}

#endif

#endif
