/*
 * COM1 NS16550 support
 * originally from linux source (arch/powerpc/boot/ns16550.c)
 * modified to use CONFIG_SYS_ISA_MEM and new defines
 *
 * further modified by Josh Huber <huber@mclx.com> to support
 * the DUART on the Galileo Eval board. (db64360)
 */

#include <config.h>
#include "ns16550.h"

#ifdef ZUMA_NTL
/* no 16550 device */
#else
const NS16550_t COM_PORTS[] = { (NS16550_t) (CONFIG_SYS_DUART_IO + 0),
	(NS16550_t) (CONFIG_SYS_DUART_IO + 0x20)
};

volatile struct NS16550 *NS16550_init (int chan, int baud_divisor)
{
	volatile struct NS16550 *com_port;

	com_port = (struct NS16550 *) COM_PORTS[chan];
	com_port->ier = 0x00;
	com_port->lcr = LCR_BKSE;	/* Access baud rate */
	com_port->dll = baud_divisor & 0xff;	/* 9600 baud */
	com_port->dlm = (baud_divisor >> 8) & 0xff;
	com_port->lcr = LCR_8N1;	/* 8 data, 1 stop, no parity */
	com_port->mcr = MCR_DTR | MCR_RTS;	/* RTS/DTR */

	/* Clear & enable FIFOs */
	com_port->fcr = FCR_FIFO_EN | FCR_RXSR | FCR_TXSR;
	return (com_port);
}

void NS16550_reinit (volatile struct NS16550 *com_port, int baud_divisor)
{
	com_port->ier = 0x00;
	com_port->lcr = LCR_BKSE;	/* Access baud rate */
	com_port->dll = baud_divisor & 0xff;	/* 9600 baud */
	com_port->dlm = (baud_divisor >> 8) & 0xff;
	com_port->lcr = LCR_8N1;	/* 8 data, 1 stop, no parity */
	com_port->mcr = MCR_DTR | MCR_RTS;	/* RTS/DTR */

	/* Clear & enable FIFOs */
	com_port->fcr = FCR_FIFO_EN | FCR_RXSR | FCR_TXSR;
}

void NS16550_putc (volatile struct NS16550 *com_port, unsigned char c)
{
	while ((com_port->lsr & LSR_THRE) == 0);
	com_port->thr = c;
}

unsigned char NS16550_getc (volatile struct NS16550 *com_port)
{
	while ((com_port->lsr & LSR_DR) == 0);
	return (com_port->rbr);
}

int NS16550_tstc (volatile struct NS16550 *com_port)
{
	return ((com_port->lsr & LSR_DR) != 0);
}
#endif
