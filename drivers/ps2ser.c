/***********************************************************************
 *
 * (C) Copyright 2004
 * DENX Software Engineering
 * Wolfgang Denk, wd@denx.de
 * All rights reserved.
 *
 * Simple 16550A serial driver
 *
 * Originally from linux source (drivers/char/ps2ser.c)
 *
 * Used by the PS/2 multiplexer driver (ps2mult.c)
 *
 ***********************************************************************/

#include <common.h>

#ifdef CONFIG_PS2SERIAL

#include <asm/io.h>
#include <asm/atomic.h>
#include <ps2mult.h>

/* #define	DEBUG */

#define PS2SER_BAUD	57600

static int	ps2ser_getc_hw(void);
static void	ps2ser_interrupt(void *dev_id);

extern struct	serial_state rs_table[]; /* in serial.c */
static struct	serial_state *state;

static u_char	ps2buf[PS2BUF_SIZE];
static atomic_t	ps2buf_cnt;
static int	ps2buf_in_idx;
static int	ps2buf_out_idx;


static inline unsigned int ps2ser_in(int offset)
{
	return readb((unsigned long) state->iomem_base + offset);
}

static inline void ps2ser_out(int offset, int value)
{
	writeb(value, (unsigned long) state->iomem_base + offset);
}

int ps2ser_init(void)
{
	int quot;
	unsigned cval;

	state = rs_table + CONFIG_PS2SERIAL;

	quot = state->baud_base / PS2SER_BAUD;
	cval = 0x3; /* 8N1 - 8 data bits, no parity bits, 1 stop bit */

	  /* Set speed, enable interrupts, enable FIFO
	   */
	ps2ser_out(UART_LCR, cval | UART_LCR_DLAB);
	ps2ser_out(UART_DLL, quot & 0xff);
	ps2ser_out(UART_DLM, quot >> 8);
	ps2ser_out(UART_LCR, cval);
	ps2ser_out(UART_IER, UART_IER_RDI);
	ps2ser_out(UART_MCR, UART_MCR_OUT2 | UART_MCR_DTR | UART_MCR_RTS);
	ps2ser_out(UART_FCR,
	    UART_FCR_ENABLE_FIFO | UART_FCR_CLEAR_RCVR | UART_FCR_CLEAR_XMIT);

	/* If we read 0xff from the LSR, there is no UART here
	 */
	if (ps2ser_in(UART_LSR) == 0xff) {
		printf ("ps2ser.c: no UART found\n");
		return -1;
	}

	irq_install_handler(state->irq, ps2ser_interrupt, NULL);

	return 0;
}

void ps2ser_putc(int chr)
{
#ifdef DEBUG
	printf(">>>> 0x%02x\n", chr);
#endif

	while (!(ps2ser_in(UART_LSR) & UART_LSR_THRE));

	ps2ser_out(UART_TX, chr);
}

static int ps2ser_getc_hw(void)
{
	int res = -1;

	if (ps2ser_in(UART_LSR) & UART_LSR_DR) {
		res = (ps2ser_in(UART_RX));
	}

	return res;
}

int ps2ser_getc(void)
{
	volatile int chr;
	int flags;

#ifdef DEBUG
	printf("<< ");
#endif

	flags = disable_interrupts();

	do {
		if (atomic_read(&ps2buf_cnt) != 0) {
			chr = ps2buf[ps2buf_out_idx++];
			ps2buf_out_idx &= (PS2BUF_SIZE - 1);
			atomic_dec(&ps2buf_cnt);
		} else {
			chr = ps2ser_getc_hw();
		}
	}
	while (chr < 0);

	if (flags) enable_interrupts();

#ifdef DEBUG
	printf("0x%02x\n", chr);
#endif

	return chr;
}

int ps2ser_check(void)
{
	int flags;

	flags = disable_interrupts();
	ps2ser_interrupt(NULL);
	if (flags) enable_interrupts();

	return atomic_read(&ps2buf_cnt);
}

static void ps2ser_interrupt(void *dev_id)
{
	int chr;
	int iir;

	do {
		chr = ps2ser_getc_hw();
		iir = ps2ser_in(UART_IIR);
		if (chr < 0) continue;

		if (atomic_read(&ps2buf_cnt) < PS2BUF_SIZE) {
			ps2buf[ps2buf_in_idx++] = chr;
			ps2buf_in_idx &= (PS2BUF_SIZE - 1);
			atomic_inc(&ps2buf_cnt);
		} else {
			printf ("ps2ser.c: buffer overflow\n");
		}
	} while (iir & UART_IIR_RDI);

	if (atomic_read(&ps2buf_cnt)) {
		ps2mult_callback(atomic_read(&ps2buf_cnt));
	}
}

#endif /* CONFIG_PS2SERIAL */
