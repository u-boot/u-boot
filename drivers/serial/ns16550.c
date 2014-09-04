/*
 * COM1 NS16550 support
 * originally from linux source (arch/powerpc/boot/ns16550.c)
 * modified to use CONFIG_SYS_ISA_MEM and new defines
 */

#include <common.h>
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

#if defined(CONFIG_SOC_KEYSTONE)
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

int ns16550_calc_divisor(NS16550_t port, int clock, int baudrate)
{
	const unsigned int mode_x_div = 16;

#ifdef CONFIG_OMAP1510
	/* If can't cleanly clock 115200 set div to 1 */
	if ((clock == 12000000) && (baudrate == 115200)) {
		port->osc_12m_sel = OSC_12M_SEL;  /* enable 6.5 * divisor */
		return 1;			/* return 1 for base divisor */
	}
	port->osc_12m_sel = 0;			/* clear if previsouly set */
#endif

	return DIV_ROUND_CLOSEST(clock, mode_x_div * baudrate);
}

static void NS16550_setbrg(NS16550_t com_port, int baud_divisor)
{
	serial_out(UART_LCR_BKSE | UART_LCRVAL, &com_port->lcr);
	serial_out(baud_divisor & 0xff, &com_port->dll);
	serial_out((baud_divisor >> 8) & 0xff, &com_port->dlm);
	serial_out(UART_LCRVAL, &com_port->lcr);
}

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
		NS16550_setbrg(com_port, baud_divisor);
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
	NS16550_setbrg(com_port, 0);
	serial_out(UART_MCRVAL, &com_port->mcr);
	serial_out(UART_FCRVAL, &com_port->fcr);
	NS16550_setbrg(com_port, baud_divisor);
#if defined(CONFIG_OMAP) || \
	defined(CONFIG_AM33XX) || defined(CONFIG_SOC_DA8XX) || \
	defined(CONFIG_TI81XX) || defined(CONFIG_AM43XX)

	/* /16 is proper to hit 115200 with 48MHz */
	serial_out(0, &com_port->mdr1);
#endif /* CONFIG_OMAP */
#if defined(CONFIG_SOC_KEYSTONE)
	serial_out(UART_REG_VAL_PWREMU_MGMT_UART_ENABLE, &com_port->regC);
#endif
}

#ifndef CONFIG_NS16550_MIN_FUNCTIONS
void NS16550_reinit(NS16550_t com_port, int baud_divisor)
{
	serial_out(CONFIG_SYS_NS16550_IER, &com_port->ier);
	NS16550_setbrg(com_port, 0);
	serial_out(UART_MCRVAL, &com_port->mcr);
	serial_out(UART_FCRVAL, &com_port->fcr);
	NS16550_setbrg(com_port, baud_divisor);
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
