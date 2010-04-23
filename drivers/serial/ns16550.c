/*
 * COM1 NS16550 support
 * originally from linux source (arch/powerpc/boot/ns16550.c)
 * modified to use CONFIG_SYS_ISA_MEM and new defines
 */

#include <config.h>
#include <ns16550.h>
#include <watchdog.h>
#include <linux/types.h>
#include <asm/io.h>

#define UART_LCRVAL UART_LCR_8N1		/* 8 data, 1 stop, no parity */
#define UART_MCRVAL (UART_MCR_DTR | \
		     UART_MCR_RTS)		/* RTS/DTR */
#define UART_FCRVAL (UART_FCR_FIFO_EN |	\
		     UART_FCR_RXSR |	\
		     UART_FCR_TXSR)		/* Clear & enable FIFOs */
#ifdef CONFIG_SYS_NS16550_PORT_MAPPED
#define serial_out(x,y)	outb(x,(ulong)y)
#define serial_in(y)	inb((ulong)y)
#else
#define serial_out(x,y) writeb(x,y)
#define serial_in(y) 	readb(y)
#endif

void NS16550_init (NS16550_t com_port, int baud_divisor)
{
	serial_out(0x00, &com_port->ier);
#if defined(CONFIG_OMAP) && !defined(CONFIG_OMAP3_ZOOM2)
	serial_out(0x7, &com_port->mdr1);	/* mode select reset TL16C750*/
#endif
	serial_out(UART_LCR_BKSE | UART_LCRVAL, (ulong)&com_port->lcr);
	serial_out(0, &com_port->dll);
	serial_out(0, &com_port->dlm);
	serial_out(UART_LCRVAL, &com_port->lcr);
	serial_out(UART_MCRVAL, &com_port->mcr);
	serial_out(UART_FCRVAL, &com_port->fcr);
	serial_out(UART_LCR_BKSE | UART_LCRVAL, &com_port->lcr);
	serial_out(baud_divisor & 0xff, &com_port->dll);
	serial_out((baud_divisor >> 8) & 0xff, &com_port->dlm);
	serial_out(UART_LCRVAL, &com_port->lcr);
#if defined(CONFIG_OMAP) && !defined(CONFIG_OMAP3_ZOOM2)
#if defined(CONFIG_APTIX)
	serial_out(3, &com_port->mdr1);	/* /13 mode so Aptix 6MHz can hit 115200 */
#else
	serial_out(0, &com_port->mdr1);	/* /16 is proper to hit 115200 with 48MHz */
#endif
#endif /* CONFIG_OMAP */
}

#ifndef CONFIG_NS16550_MIN_FUNCTIONS
void NS16550_reinit (NS16550_t com_port, int baud_divisor)
{
	serial_out(0x00, &com_port->ier);
	serial_out(UART_LCR_BKSE | UART_LCRVAL, &com_port->lcr);
	serial_out(0, &com_port->dll);
	serial_out(0, &com_port->dlm);
	serial_out(UART_LCRVAL, &com_port->lcr);
	serial_out(UART_MCRVAL, &com_port->mcr);
	serial_out(UART_FCRVAL, &com_port->fcr);
	serial_out(UART_LCR_BKSE, &com_port->lcr);
	serial_out(baud_divisor & 0xff, &com_port->dll);
	serial_out((baud_divisor >> 8) & 0xff, &com_port->dlm);
	serial_out(UART_LCRVAL, &com_port->lcr);
}
#endif /* CONFIG_NS16550_MIN_FUNCTIONS */

void NS16550_putc (NS16550_t com_port, char c)
{
	while ((serial_in(&com_port->lsr) & UART_LSR_THRE) == 0);
	serial_out(c, &com_port->thr);
}

#ifndef CONFIG_NS16550_MIN_FUNCTIONS
char NS16550_getc (NS16550_t com_port)
{
	while ((serial_in(&com_port->lsr) & UART_LSR_DR) == 0) {
#ifdef CONFIG_USB_TTY
		extern void usbtty_poll(void);
		usbtty_poll();
#endif
		WATCHDOG_RESET();
	}
	return serial_in(&com_port->rbr);
}

int NS16550_tstc (NS16550_t com_port)
{
	return ((serial_in(&com_port->lsr) & UART_LSR_DR) != 0);
}

#endif /* CONFIG_NS16550_MIN_FUNCTIONS */
