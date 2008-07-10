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
#if defined(CFG_NS16550) || defined(CONFIG_MPC85xx)
#include <ns16550.h>
#endif

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
#elif defined(CONFIG_MGT5100)
#error CONFIG_PS2SERIAL must be in 1, 2 or 3
#elif CONFIG_PS2SERIAL == 4
#define PSC_BASE MPC5XXX_PSC4
#elif CONFIG_PS2SERIAL == 5
#define PSC_BASE MPC5XXX_PSC5
#elif CONFIG_PS2SERIAL == 6
#define PSC_BASE MPC5XXX_PSC6
#else
#error CONFIG_PS2SERIAL must be in 1 ... 6
#endif

#elif defined(CONFIG_MPC8540) || defined(CONFIG_MPC8541) || \
      defined(CONFIG_MPC8548) || defined(CONFIG_MPC8555)

#if CONFIG_PS2SERIAL == 1
#define COM_BASE (CFG_CCSRBAR+0x4500)
#elif CONFIG_PS2SERIAL == 2
#define COM_BASE (CFG_CCSRBAR+0x4600)
#else
#error CONFIG_PS2SERIAL must be in 1 ... 2
#endif

#endif /* CONFIG_MPC5xxx / CONFIG_MPC8540 / other */

static int	ps2ser_getc_hw(void);
static void	ps2ser_interrupt(void *dev_id);

extern struct	serial_state rs_table[]; /* in serial.c */
#if !defined(CONFIG_MPC5xxx) && !defined(CONFIG_MPC8540) && \
    !defined(CONFIG_MPC8541) && !defined(CONFIG_MPC8548) && \
    !defined(CONFIG_MPC8555)
static struct	serial_state *state;
#endif

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
#if defined(CONFIG_MGT5100)
	psc->psc_clock_select = 0xdd00;
	baseclk = (CFG_MPC5XXX_CLKIN + 16) / 32;
#elif defined(CONFIG_MPC5200)
	psc->psc_clock_select = 0;
	baseclk = (gd->ipb_clk + 16) / 32;
#endif

	/* switch to UART mode */
	psc->sicr = 0;

	/* configure parity, bit length and so on */
#if defined(CONFIG_MGT5100)
	psc->mode = PSC_MODE_ERR | PSC_MODE_8_BITS | PSC_MODE_PARNONE;
#elif defined(CONFIG_MPC5200)
	psc->mode = PSC_MODE_8_BITS | PSC_MODE_PARNONE;
#endif
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

#elif defined(CONFIG_MPC8540) || defined(CONFIG_MPC8541) || \
      defined(CONFIG_MPC8548) || defined(CONFIG_MPC8555)
int ps2ser_init(void)
{
	NS16550_t com_port = (NS16550_t)COM_BASE;

	com_port->ier = 0x00;
	com_port->lcr = LCR_BKSE | LCR_8N1;
	com_port->dll = (CFG_NS16550_CLK / 16 / PS2SER_BAUD) & 0xff;
	com_port->dlm = ((CFG_NS16550_CLK / 16 / PS2SER_BAUD) >> 8) & 0xff;
	com_port->lcr = LCR_8N1;
	com_port->mcr = (MCR_DTR | MCR_RTS);
	com_port->fcr = (FCR_FIFO_EN | FCR_RXSR | FCR_TXSR);

	return (0);
}

#else /* !CONFIG_MPC5xxx && !CONFIG_MPC8540 / other */

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
#endif /* CONFIG_MPC5xxx / CONFIG_MPC8540 / other */

void ps2ser_putc(int chr)
{
#ifdef CONFIG_MPC5xxx
	volatile struct mpc5xxx_psc *psc = (struct mpc5xxx_psc *)PSC_BASE;
#elif defined(CONFIG_MPC8540) || defined(CONFIG_MPC8541) || \
      defined(CONFIG_MPC8548) || defined(CONFIG_MPC8555)
	NS16550_t com_port = (NS16550_t)COM_BASE;
#endif
#ifdef DEBUG
	printf(">>>> 0x%02x\n", chr);
#endif

#ifdef CONFIG_MPC5xxx
	while (!(psc->psc_status & PSC_SR_TXRDY));

	psc->psc_buffer_8 = chr;
#elif defined(CONFIG_MPC8540) || defined(CONFIG_MPC8541) || \
      defined(CONFIG_MPC8548) || defined(CONFIG_MPC8555)
	while ((com_port->lsr & LSR_THRE) == 0);
	com_port->thr = chr;
#else
	while (!(ps2ser_in(UART_LSR) & UART_LSR_THRE));

	ps2ser_out(UART_TX, chr);
#endif
}

static int ps2ser_getc_hw(void)
{
#ifdef CONFIG_MPC5xxx
	volatile struct mpc5xxx_psc *psc = (struct mpc5xxx_psc *)PSC_BASE;
#elif defined(CONFIG_MPC8540) || defined(CONFIG_MPC8541) || \
      defined(CONFIG_MPC8548) || defined(CONFIG_MPC8555)
	NS16550_t com_port = (NS16550_t)COM_BASE;
#endif
	int res = -1;

#ifdef CONFIG_MPC5xxx
	if (psc->psc_status & PSC_SR_RXRDY) {
		res = (psc->psc_buffer_8);
	}
#elif defined(CONFIG_MPC8540) || defined(CONFIG_MPC8541) || \
      defined(CONFIG_MPC8548) || defined(CONFIG_MPC8555)
	if (com_port->lsr & LSR_DR) {
		res = com_port->rbr;
	}
#else
	if (ps2ser_in(UART_LSR) & UART_LSR_DR) {
		res = (ps2ser_in(UART_RX));
	}
#endif

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
#ifdef CONFIG_MPC5xxx
	volatile struct mpc5xxx_psc *psc = (struct mpc5xxx_psc *)PSC_BASE;
#elif defined(CONFIG_MPC8540) || defined(CONFIG_MPC8541) || \
      defined(CONFIG_MPC8548) || defined(CONFIG_MPC8555)
	NS16550_t com_port = (NS16550_t)COM_BASE;
#endif
	int chr;
	int status;

	do {
		chr = ps2ser_getc_hw();
#ifdef CONFIG_MPC5xxx
		status = psc->psc_status;
#elif defined(CONFIG_MPC8540) || defined(CONFIG_MPC8541) || \
      defined(CONFIG_MPC8548) || defined(CONFIG_MPC8555)
		status = com_port->lsr;
#else
		status = ps2ser_in(UART_IIR);
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
#elif defined(CONFIG_MPC8540) || defined(CONFIG_MPC8541) || \
      defined(CONFIG_MPC8548) || defined(CONFIG_MPC8555)
	} while (status & LSR_DR);
#else
	} while (status & UART_IIR_RDI);
#endif

	if (atomic_read(&ps2buf_cnt)) {
		ps2mult_callback(atomic_read(&ps2buf_cnt));
	}
}

#endif /* CONFIG_PS2SERIAL */
