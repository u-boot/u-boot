/***********************************************************************
 *
 * (C) Copyright 2004-2009
 * DENX Software Engineering
 * Wolfgang Denk, wd@denx.de
 *
 * Simple 16550A serial driver
 *
 * Originally from linux source (drivers/char/ps2ser.c)
 *
 * Used by the PS/2 multiplexer driver (ps2mult.c)
 *
 ***********************************************************************/

#include <common.h>

#include <asm/io.h>
#include <asm/atomic.h>
#include <ps2mult.h>
/* This is needed for ns16550.h */
#ifndef CONFIG_SYS_NS16550_REG_SIZE
#define CONFIG_SYS_NS16550_REG_SIZE 1
#endif
#include <ns16550.h>

DECLARE_GLOBAL_DATA_PTR;

/* #define	DEBUG */

#define PS2SER_BAUD	57600

#if CONFIG_PS2SERIAL == 1
#define COM_BASE (CONFIG_SYS_CCSRBAR+0x4500)
#elif CONFIG_PS2SERIAL == 2
#define COM_BASE (CONFIG_SYS_CCSRBAR+0x4600)
#else
#error CONFIG_PS2SERIAL must be in 1 ... 2
#endif

static int	ps2ser_getc_hw(void);
static void	ps2ser_interrupt(void *dev_id);

extern struct	serial_state rs_table[]; /* in serial.c */

static u_char	ps2buf[PS2BUF_SIZE];
static atomic_t	ps2buf_cnt;
static int	ps2buf_in_idx;
static int	ps2buf_out_idx;

int ps2ser_init(void)
{
	NS16550_t com_port = (NS16550_t)COM_BASE;

	com_port->ier = 0x00;
	com_port->lcr = UART_LCR_BKSE | UART_LCR_8N1;
	com_port->dll = (CONFIG_SYS_NS16550_CLK / 16 / PS2SER_BAUD) & 0xff;
	com_port->dlm = ((CONFIG_SYS_NS16550_CLK / 16 / PS2SER_BAUD) >> 8) & 0xff;
	com_port->lcr = UART_LCR_8N1;
	com_port->mcr = (UART_MCR_DTR | UART_MCR_RTS);
	com_port->fcr = (UART_FCR_FIFO_EN | UART_FCR_RXSR | UART_FCR_TXSR);

	return (0);
}

void ps2ser_putc(int chr)
{
	NS16550_t com_port = (NS16550_t)COM_BASE;
	debug(">>>> 0x%02x\n", chr);

	while ((com_port->lsr & UART_LSR_THRE) == 0);
	com_port->thr = chr;
}

static int ps2ser_getc_hw(void)
{
	NS16550_t com_port = (NS16550_t)COM_BASE;
	int res = -1;

	if (com_port->lsr & UART_LSR_DR) {
		res = com_port->rbr;
	}

	return res;
}

int ps2ser_getc(void)
{
	volatile int chr;
	int flags;

	debug("<< ");

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

	if (flags)
		enable_interrupts();

	debug("0x%02x\n", chr);

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
	NS16550_t com_port = (NS16550_t)COM_BASE;
	int chr;
	int status;

	do {
		chr = ps2ser_getc_hw();
		status = com_port->lsr;
		if (chr < 0) continue;

		if (atomic_read(&ps2buf_cnt) < PS2BUF_SIZE) {
			ps2buf[ps2buf_in_idx++] = chr;
			ps2buf_in_idx &= (PS2BUF_SIZE - 1);
			atomic_inc(&ps2buf_cnt);
		} else {
			printf ("ps2ser.c: buffer overflow\n");
		}
	} while (status & UART_LSR_DR);
	if (atomic_read(&ps2buf_cnt)) {
		ps2mult_callback(atomic_read(&ps2buf_cnt));
	}
}
