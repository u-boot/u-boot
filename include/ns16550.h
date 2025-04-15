/*
 * NS16550 Serial Port
 * originally from linux source (arch/powerpc/boot/ns16550.h)
 *
 * Cleanup and unification
 * (C) 2009 by Detlev Zundel, DENX Software Engineering GmbH
 *
 * modified slightly to
 * have addresses as offsets from CONFIG_SYS_ISA_BASE
 * added a few more definitions
 * added prototypes for ns16550.c
 * reduced no of com ports to 2
 * modifications (c) Rob Taylor, Flying Pig Systems. 2000.
 *
 * added support for port on 64-bit bus
 * by Richard Danter (richard.danter@windriver.com), (C) 2005 Wind River Systems
 */

/*
 * Note that the following macro magic uses the fact that the compiler
 * will not allocate storage for arrays of size 0
 */

#ifndef __ns16550_h
#define __ns16550_h

#include <linux/types.h>
#include <serial.h>

#if CONFIG_IS_ENABLED(DM_SERIAL) ||  defined(CONFIG_NS16550_DYNAMIC) || \
	defined(CONFIG_DEBUG_UART)
/*
 * For driver model we always use one byte per register, and sort out the
 * differences in the driver. In the case of CONFIG_NS16550_DYNAMIC we do
 * similar, and CONFIG_DEBUG_UART is responsible for shifts in its own manner.
 */
#define UART_REG(x)	unsigned char x
#else
#if !defined(CONFIG_SYS_NS16550_REG_SIZE) || (CONFIG_SYS_NS16550_REG_SIZE == 0)
#error "Please define NS16550 registers size."
#elif (CONFIG_SYS_NS16550_REG_SIZE > 0)
#define UART_REG(x)						   \
	unsigned char prepad_##x[CONFIG_SYS_NS16550_REG_SIZE - 1]; \
	unsigned char x;
#elif (CONFIG_SYS_NS16550_REG_SIZE < 0)
#define UART_REG(x)							\
	unsigned char x;						\
	unsigned char postpad_##x[-CONFIG_SYS_NS16550_REG_SIZE - 1];
#endif
#endif /* CONFIG_NS16550_DYNAMIC */

enum ns16550_flags {
	NS16550_FLAG_IO		= 1 << 0, /* Use I/O access (else mem-mapped) */
	NS16550_FLAG_ENDIAN	= 1 << 1, /* Use out_le/be_32() */
	NS16550_FLAG_BE		= 1 << 2, /* Big-endian access (else little) */
};

/**
 * struct ns16550_plat - information about a NS16550 port
 *
 * @base:		Base register address
 * @size:		Size of register area in bytes
 * @reg_width:		IO accesses size of registers (in bytes, 1 or 4)
 * @reg_shift:		Shift size of registers (0=byte, 1=16bit, 2=32bit...)
 * @reg_offset:		Offset to start of registers (normally 0)
 * @clock:		UART base clock speed in Hz
 * @fcr:		Offset of FCR register (normally UART_FCR_DEFVAL)
 * @flags:		A few flags (enum ns16550_flags)
 * @bdf:		PCI slot/function (pci_dev_t)
 */
struct ns16550_plat {
	ulong base;
	ulong size;
	int reg_width;
	int reg_shift;
	int reg_offset;
	int clock;
	u32 fcr;
	int flags;
#if defined(CONFIG_PCI) && defined(CONFIG_SPL)
	int bdf;
#endif
};

struct udevice;

struct ns16550 {
	UART_REG(rbr);		/* 0 */
	UART_REG(ier);		/* 1 */
	UART_REG(fcr);		/* 2 */
	UART_REG(lcr);		/* 3 */
	UART_REG(mcr);		/* 4 */
	UART_REG(lsr);		/* 5 */
	UART_REG(msr);		/* 6 */
	UART_REG(spr);		/* 7 */
#ifdef CONFIG_SOC_DA8XX
	UART_REG(reg8);		/* 8 */
	UART_REG(reg9);		/* 9 */
	UART_REG(revid1);	/* A */
	UART_REG(revid2);	/* B */
	UART_REG(pwr_mgmt);	/* C */
	UART_REG(mdr1);		/* D */
#else
	UART_REG(mdr1);		/* 8 */
	UART_REG(reg9);		/* 9 */
	UART_REG(regA);		/* A */
	UART_REG(regB);		/* B */
	UART_REG(regC);		/* C */
	UART_REG(regD);		/* D */
	UART_REG(regE);		/* E */
	UART_REG(uasr);		/* F */
	UART_REG(scr);		/* 10*/
	UART_REG(ssr);		/* 11*/
#endif
#if CONFIG_IS_ENABLED(DM_SERIAL)
	struct ns16550_plat *plat;
#endif
};

#if CONFIG_IS_ENABLED(DM_SERIAL)
#define serial_out(value, addr)	\
	ns16550_writeb(com_port, \
		(unsigned char *)(addr) - (unsigned char *)com_port, value)
#define serial_in(addr) \
	ns16550_readb(com_port, \
		(unsigned char *)(addr) - (unsigned char *)com_port)
#endif

#define thr rbr
#define iir fcr
#define dll rbr
#define dlm ier

/*
 * These are the definitions for the FIFO Control Register
 */
#define UART_FCR_FIFO_EN	0x01 /* Fifo enable */
#define UART_FCR_CLEAR_RCVR	0x02 /* Clear the RCVR FIFO */
#define UART_FCR_CLEAR_XMIT	0x04 /* Clear the XMIT FIFO */
#define UART_FCR_DMA_SELECT	0x08 /* For DMA applications */
#define UART_FCR_TRIGGER_MASK	0xC0 /* Mask for the FIFO trigger range */
#define UART_FCR_TRIGGER_1	0x00 /* Mask for trigger set at 1 */
#define UART_FCR_TRIGGER_4	0x40 /* Mask for trigger set at 4 */
#define UART_FCR_TRIGGER_8	0x80 /* Mask for trigger set at 8 */
#define UART_FCR_TRIGGER_14	0xC0 /* Mask for trigger set at 14 */

#define UART_FCR_RXSR		0x02 /* Receiver soft reset */
#define UART_FCR_TXSR		0x04 /* Transmitter soft reset */

/* Ingenic JZ47xx specific UART-enable bit. */
#define UART_FCR_UME		0x10

/* Clear & enable FIFOs */
#define UART_FCR_DEFVAL (UART_FCR_FIFO_EN | \
			UART_FCR_RXSR |	\
			UART_FCR_TXSR)

/*
 * These are the definitions for the Modem Control Register
 */
#define UART_MCR_DTR	0x01		/* DTR   */
#define UART_MCR_RTS	0x02		/* RTS   */
#define UART_MCR_OUT1	0x04		/* Out 1 */
#define UART_MCR_OUT2	0x08		/* Out 2 */
#define UART_MCR_LOOP	0x10		/* Enable loopback test mode */
#define UART_MCR_AFE	0x20		/* Enable auto-RTS/CTS */

#define UART_MCR_DMA_EN	0x04
#define UART_MCR_TX_DFR	0x08

/*
 * These are the definitions for the Line Control Register
 *
 * Note: if the word length is 5 bits (UART_LCR_WLEN5), then setting
 * UART_LCR_STOP will select 1.5 stop bits, not 2 stop bits.
 */
#define UART_LCR_WLS_MSK 0x03		/* character length select mask */
#define UART_LCR_WLS_5	0x00		/* 5 bit character length */
#define UART_LCR_WLS_6	0x01		/* 6 bit character length */
#define UART_LCR_WLS_7	0x02		/* 7 bit character length */
#define UART_LCR_WLS_8	0x03		/* 8 bit character length */
#define UART_LCR_STB	0x04		/* # stop Bits, off=1, on=1.5 or 2) */
#define UART_LCR_PEN	0x08		/* Parity eneble */
#define UART_LCR_EPS	0x10		/* Even Parity Select */
#define UART_LCR_STKP	0x20		/* Stick Parity */
#define UART_LCR_SBRK	0x40		/* Set Break */
#define UART_LCR_BKSE	0x80		/* Bank select enable */
#define UART_LCR_DLAB	0x80		/* Divisor latch access bit */

/*
 * These are the definitions for the Line Status Register
 */
#define UART_LSR_DR	0x01		/* Data ready */
#define UART_LSR_OE	0x02		/* Overrun */
#define UART_LSR_PE	0x04		/* Parity error */
#define UART_LSR_FE	0x08		/* Framing error */
#define UART_LSR_BI	0x10		/* Break */
#define UART_LSR_THRE	0x20		/* Xmit holding register empty */
#define UART_LSR_TEMT	0x40		/* Xmitter empty */
#define UART_LSR_ERR	0x80		/* Error */

#define UART_MSR_DCD	0x80		/* Data Carrier Detect */
#define UART_MSR_RI	0x40		/* Ring Indicator */
#define UART_MSR_DSR	0x20		/* Data Set Ready */
#define UART_MSR_CTS	0x10		/* Clear to Send */
#define UART_MSR_DDCD	0x08		/* Delta DCD */
#define UART_MSR_TERI	0x04		/* Trailing edge ring indicator */
#define UART_MSR_DDSR	0x02		/* Delta DSR */
#define UART_MSR_DCTS	0x01		/* Delta CTS */

/*
 * These are the definitions for the Interrupt Identification Register
 */
#define UART_IIR_NO_INT	0x01	/* No interrupts pending */
#define UART_IIR_ID	0x06	/* Mask for the interrupt ID */

#define UART_IIR_MSI	0x00	/* Modem status interrupt */
#define UART_IIR_THRI	0x02	/* Transmitter holding register empty */
#define UART_IIR_RDI	0x04	/* Receiver data interrupt */
#define UART_IIR_RLSI	0x06	/* Receiver line status interrupt */

/*
 * These are the definitions for the Interrupt Enable Register
 */
#define UART_IER_MSI	0x08	/* Enable Modem status interrupt */
#define UART_IER_RLSI	0x04	/* Enable receiver line status interrupt */
#define UART_IER_THRI	0x02	/* Enable Transmitter holding register int. */
#define UART_IER_RDI	0x01	/* Enable receiver data interrupt */

/* useful defaults for LCR */
#define UART_LCR_8N1	0x03

void ns16550_init(struct ns16550 *com_port, int baud_divisor);
void ns16550_putc(struct ns16550 *com_port, char c);
char ns16550_getc(struct ns16550 *com_port);
int ns16550_tstc(struct ns16550 *com_port);
void ns16550_reinit(struct ns16550 *com_port, int baud_divisor);
int ns16550_serial_putc(struct udevice *dev, const char ch);
int ns16550_serial_pending(struct udevice *dev, bool input);
int ns16550_serial_getc(struct udevice *dev);
int ns16550_serial_setbrg(struct udevice *dev, int baudrate);
int ns16550_serial_setconfig(struct udevice *dev, uint serial_config);
int ns16550_serial_getinfo(struct udevice *dev, struct serial_device_info *info);
void ns16550_writeb(struct ns16550 *port, int offset, int value);
void ns16550_setbrg(struct ns16550 *com_port, int baud_divisor);

/**
 * ns16550_calc_divisor() - calculate the divisor given clock and baud rate
 *
 * Given the UART input clock and required baudrate, calculate the divisor
 * that should be used.
 *
 * @port:	UART port
 * @clock:	UART input clock speed in Hz
 * @baudrate:	Required baud rate
 * Return: baud rate divisor that should be used
 */
int ns16550_calc_divisor(struct ns16550 *port, int clock, int baudrate);

/**
 * ns16550_serial_of_to_plat() - convert DT to platform data
 *
 * Decode a device tree node for an ns16550 device. This includes the
 * register base address and register shift properties. The caller must set
 * up the clock frequency.
 *
 * @dev:	dev to decode platform data for
 * @return:	0 if OK, -EINVAL on error
 */
int ns16550_serial_of_to_plat(struct udevice *dev);

/**
 * ns16550_serial_probe() - probe a serial port
 *
 * This sets up the serial port ready for use, except for the baud rate
 * Return: 0, or -ve on error
 */
int ns16550_serial_probe(struct udevice *dev);

/**
 * struct ns16550_serial_ops - ns16550 serial operations
 *
 * These should be used by the client driver for the driver's 'ops' member
 */
extern const struct dm_serial_ops ns16550_serial_ops;

#endif /* __ns16550_h */
