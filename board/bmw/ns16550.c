/*
 * COM1 NS16550 support
 * originally from linux source (arch/ppc/boot/ns16550.c)
 * modified to use CFG_ISA_MEM and new defines
 */

#include <config.h>
#include "ns16550.h"

typedef struct NS16550 *NS16550_t;

const NS16550_t COM_PORTS[] =
	{ (NS16550_t) ((CFG_EUMB_ADDR) + 0x4500),
(NS16550_t) ((CFG_EUMB_ADDR) + 0x4600) };

volatile struct NS16550 *NS16550_init (int chan, int baud_divisor)
{
	volatile struct NS16550 *com_port;

	com_port = (struct NS16550 *) COM_PORTS[chan];
	com_port->ier = 0x00;
	com_port->lcr = LCR_BKSE;	/* Access baud rate */
	com_port->dll = baud_divisor & 0xff;	/* 9600 baud */
	com_port->dlm = (baud_divisor >> 8) & 0xff;
	com_port->lcr = LCR_8N1;	/* 8 data, 1 stop, no parity */
	com_port->mcr = MCR_RTS;	/* RTS/DTR */
	com_port->fcr = FCR_FIFO_EN | FCR_RXSR | FCR_TXSR;	/* Clear & enable FIFOs */
	return (com_port);
}

void NS16550_reinit (volatile struct NS16550 *com_port, int baud_divisor)
{
	com_port->ier = 0x00;
	com_port->lcr = LCR_BKSE;	/* Access baud rate */
	com_port->dll = baud_divisor & 0xff;	/* 9600 baud */
	com_port->dlm = (baud_divisor >> 8) & 0xff;
	com_port->lcr = LCR_8N1;	/* 8 data, 1 stop, no parity */
	com_port->mcr = MCR_RTS;	/* RTS/DTR */
	com_port->fcr = FCR_FIFO_EN | FCR_RXSR | FCR_TXSR;	/* Clear & enable FIFOs */
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
