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
#define serial_out(x, y)	outb(x, (ulong)y)
#define serial_in(y)		inb((ulong)y)
#elif defined(CONFIG_SYS_NS16550_MEM32) && (CONFIG_SYS_NS16550_REG_SIZE > 0)
#define serial_out(x, y)	out_be32(y, x)
#define serial_in(y)		in_be32(y)
#elif defined(CONFIG_SYS_NS16550_MEM32) && (CONFIG_SYS_NS16550_REG_SIZE < 0)
#define serial_out(x, y)	out_le32(y, x)
#define serial_in(y)		in_le32(y)
#else
#define serial_out(x, y)	writeb(x, y)
#define serial_in(y)		readb(y)
#endif

#if defined(CONFIG_K2HK_EVM)
#define UART_REG_VAL_PWREMU_MGMT_UART_DISABLE   0
#define UART_REG_VAL_PWREMU_MGMT_UART_ENABLE ((1 << 14) | (1 << 13) | (1 << 0))
#undef UART_MCRVAL
#ifdef CONFIG_SERIAL_HW_FLOW_CONTROL
#define UART_MCRVAL             (UART_MCR_RTS | UART_MCR_AFE)
#else
#define UART_MCRVAL             (UART_MCR_RTS)
#endif
#endif

#ifndef CONFIG_SYS_NS16550_IER
#define CONFIG_SYS_NS16550_IER  0x00
#endif /* CONFIG_SYS_NS16550_IER */

void NS16550_init(NS16550_t com_port, int baud_divisor)
{
#if (defined(CONFIG_SPL_BUILD) && defined(CONFIG_OMAP34XX))
	/*
	 * On some OMAP3 devices when UART3 is configured for boot mode before
	 * SPL starts only THRE bit is set. We have to empty the transmitter
	 * before initialization starts.
	 */
	if ((serial_in(&com_port->lsr) & (UART_LSR_TEMT | UART_LSR_THRE))
	     == UART_LSR_THRE) {
		serial_out(UART_LCR_DLAB, &com_port->lcr);
		serial_out(baud_divisor & 0xff, &com_port->dll);
		serial_out((baud_divisor >> 8) & 0xff, &com_port->dlm);
		serial_out(UART_LCRVAL, &com_port->lcr);
		serial_out(0, &com_port->mdr1);
	}
#endif

	while (!(serial_in(&com_port->lsr) & UART_LSR_TEMT))
		;

	serial_out(CONFIG_SYS_NS16550_IER, &com_port->ier);
#if defined(CONFIG_OMAP) || defined(CONFIG_AM33XX) || \
			defined(CONFIG_TI81XX) || defined(CONFIG_AM43XX)
	serial_out(0x7, &com_port->mdr1);	/* mode select reset TL16C750*/
#endif
	serial_out(UART_LCR_BKSE | UART_LCRVAL, &com_port->lcr);
	serial_out(0, &com_port->dll);
	serial_out(0, &com_port->dlm);
	serial_out(UART_LCRVAL, &com_port->lcr);
	serial_out(UART_MCRVAL, &com_port->mcr);
	serial_out(UART_FCRVAL, &com_port->fcr);
	serial_out(UART_LCR_BKSE | UART_LCRVAL, &com_port->lcr);
	serial_out(baud_divisor & 0xff, &com_port->dll);
	serial_out((baud_divisor >> 8) & 0xff, &com_port->dlm);
	serial_out(UART_LCRVAL, &com_port->lcr);
#if (defined(CONFIG_OMAP) && !defined(CONFIG_OMAP3_ZOOM2)) || \
	defined(CONFIG_AM33XX) || defined(CONFIG_SOC_DA8XX) || \
	defined(CONFIG_TI81XX) || defined(CONFIG_AM43XX)

	/* /16 is proper to hit 115200 with 48MHz */
	serial_out(0, &com_port->mdr1);
#endif /* CONFIG_OMAP */
#if defined(CONFIG_K2HK_EVM)
	serial_out(UART_REG_VAL_PWREMU_MGMT_UART_ENABLE, &com_port->regC);
#endif
}

#ifndef CONFIG_NS16550_MIN_FUNCTIONS
void NS16550_reinit(NS16550_t com_port, int baud_divisor)
{
	serial_out(CONFIG_SYS_NS16550_IER, &com_port->ier);
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

void NS16550_putc(NS16550_t com_port, char c)
{
	while ((serial_in(&com_port->lsr) & UART_LSR_THRE) == 0)
		;
	serial_out(c, &com_port->thr);

	/*
	 * Call watchdog_reset() upon newline. This is done here in putc
	 * since the environment code uses a single puts() to print the complete
	 * environment upon "printenv". So we can't put this watchdog call
	 * in puts().
	 */
	if (c == '\n')
		WATCHDOG_RESET();
}

#ifndef CONFIG_NS16550_MIN_FUNCTIONS
char NS16550_getc(NS16550_t com_port)
{
	while ((serial_in(&com_port->lsr) & UART_LSR_DR) == 0) {
#if !defined(CONFIG_SPL_BUILD) && defined(CONFIG_USB_TTY)
		extern void usbtty_poll(void);
		usbtty_poll();
#endif
		WATCHDOG_RESET();
	}
	return serial_in(&com_port->rbr);
}

int NS16550_tstc(NS16550_t com_port)
{
	return (serial_in(&com_port->lsr) & UART_LSR_DR) != 0;
}

#endif /* CONFIG_NS16550_MIN_FUNCTIONS */
