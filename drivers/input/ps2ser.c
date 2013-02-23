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

#ifdef CONFIG_MPC5xxx
#if CONFIG_PS2SERIAL == 1
#define PSC_BASE MPC5XXX_PSC1
#elif CONFIG_PS2SERIAL == 2
#define PSC_BASE MPC5XXX_PSC2
#elif CONFIG_PS2SERIAL == 3
#define PSC_BASE MPC5XXX_PSC3
#elif CONFIG_PS2SERIAL == 4
#define PSC_BASE MPC5XXX_PSC4
#elif CONFIG_PS2SERIAL == 5
#define PSC_BASE MPC5XXX_PSC5
#elif CONFIG_PS2SERIAL == 6
#define PSC_BASE MPC5XXX_PSC6
#else
#error CONFIG_PS2SERIAL must be in 1 ... 6
#endif

#else

#if CONFIG_PS2SERIAL == 1
#define COM_BASE (CONFIG_SYS_CCSRBAR+0x4500)
#elif CONFIG_PS2SERIAL == 2
#define COM_BASE (CONFIG_SYS_CCSRBAR+0x4600)
#else
#error CONFIG_PS2SERIAL must be in 1 ... 2
#endif

#endif /* CONFIG_MPC5xxx / other */

static int	ps2ser_getc_hw(void);
static void	ps2ser_interrupt(void *dev_id);

extern struct	serial_state rs_table[]; /* in serial.c */

static u_char	ps2buf[PS2BUF_SIZE];
static atomic_t	ps2buf_cnt;
static int	ps2buf_in_idx;
static int	ps2buf_out_idx;

#ifdef CONFIG_MPC5xxx
int ps2ser_init(void)
{
	volatile struct mpc5xxx_psc *psc = (struct mpc5xxx_psc *)PSC_BASE;
	unsigned long baseclk;
	int div;

	/* reset PSC */
	psc->command = PSC_SEL_MODE_REG_1;

	/* select clock sources */
	psc->psc_clock_select = 0;
	baseclk = (gd->arch.ipb_clk + 16) / 32;

	/* switch to UART mode */
	psc->sicr = 0;

	/* configure parity, bit length and so on */
	psc->mode = PSC_MODE_8_BITS | PSC_MODE_PARNONE;
	psc->mode = PSC_MODE_ONE_STOP;

	/* set up UART divisor */
	div = (baseclk + (PS2SER_BAUD/2)) / PS2SER_BAUD;
	psc->ctur = (div >> 8) & 0xff;
	psc->ctlr = div & 0xff;

	/* disable all interrupts */
	psc->psc_imr = 0;

	/* reset and enable Rx/Tx */
	psc->command = PSC_RST_RX;
	psc->command = PSC_RST_TX;
	psc->command = PSC_RX_ENABLE | PSC_TX_ENABLE;

	return (0);
}

#else

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

#endif /* CONFIG_MPC5xxx / other */

void ps2ser_putc(int chr)
{
#ifdef CONFIG_MPC5xxx
	volatile struct mpc5xxx_psc *psc = (struct mpc5xxx_psc *)PSC_BASE;
#else
	NS16550_t com_port = (NS16550_t)COM_BASE;
#endif
	debug(">>>> 0x%02x\n", chr);

#ifdef CONFIG_MPC5xxx
	while (!(psc->psc_status & PSC_SR_TXRDY));

	psc->psc_buffer_8 = chr;
#else
	while ((com_port->lsr & UART_LSR_THRE) == 0);
	com_port->thr = chr;
#endif
}

static int ps2ser_getc_hw(void)
{
#ifdef CONFIG_MPC5xxx
	volatile struct mpc5xxx_psc *psc = (struct mpc5xxx_psc *)PSC_BASE;
#else
	NS16550_t com_port = (NS16550_t)COM_BASE;
#endif
	int res = -1;

#ifdef CONFIG_MPC5xxx
	if (psc->psc_status & PSC_SR_RXRDY) {
		res = (psc->psc_buffer_8);
	}
#else
	if (com_port->lsr & UART_LSR_DR) {
		res = com_port->rbr;
	}
#endif

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
#ifdef CONFIG_MPC5xxx
	volatile struct mpc5xxx_psc *psc = (struct mpc5xxx_psc *)PSC_BASE;
#else
	NS16550_t com_port = (NS16550_t)COM_BASE;
#endif
	int chr;
	int status;

	do {
		chr = ps2ser_getc_hw();
#ifdef CONFIG_MPC5xxx
		status = psc->psc_status;
#else
		status = com_port->lsr;
#endif
		if (chr < 0) continue;

		if (atomic_read(&ps2buf_cnt) < PS2BUF_SIZE) {
			ps2buf[ps2buf_in_idx++] = chr;
			ps2buf_in_idx &= (PS2BUF_SIZE - 1);
			atomic_inc(&ps2buf_cnt);
		} else {
			printf ("ps2ser.c: buffer overflow\n");
		}
#ifdef CONFIG_MPC5xxx
	} while (status & PSC_SR_RXRDY);
#else
	} while (status & UART_LSR_DR);
#endif
	if (atomic_read(&ps2buf_cnt)) {
		ps2mult_callback(atomic_read(&ps2buf_cnt));
	}
}
