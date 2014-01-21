/*
 * (C) Copyright 2008-2011 Michal Simek <monstr@monstr.eu>
 * Clean driver and add xilinx constant from header file
 *
 * (C) Copyright 2004 Atmark Techno, Inc.
 * Yasushi SHOJI <yashi@atmark-techno.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <config.h>
#include <common.h>
#include <asm/io.h>
#include <linux/compiler.h>
#include <serial.h>

#define SR_TX_FIFO_FULL		0x08 /* transmit FIFO full */
#define SR_RX_FIFO_VALID_DATA	0x01 /* data in receive FIFO */
#define SR_RX_FIFO_FULL		0x02 /* receive FIFO full */

struct uartlite {
	unsigned int rx_fifo;
	unsigned int tx_fifo;
	unsigned int status;
};

static struct uartlite *userial_ports[4] = {
#ifdef XILINX_UARTLITE_BASEADDR
	[0] = (struct uartlite *)XILINX_UARTLITE_BASEADDR,
#endif
#ifdef XILINX_UARTLITE_BASEADDR1
	[1] = (struct uartlite *)XILINX_UARTLITE_BASEADDR1,
#endif
#ifdef XILINX_UARTLITE_BASEADDR2
	[2] = (struct uartlite *)XILINX_UARTLITE_BASEADDR2,
#endif
#ifdef XILINX_UARTLITE_BASEADDR3
	[3] = (struct uartlite *)XILINX_UARTLITE_BASEADDR3
#endif
};

static void uartlite_serial_putc(const char c, const int port)
{
	struct uartlite *regs = userial_ports[port];

	if (c == '\n')
		uartlite_serial_putc('\r', port);

	while (in_be32(&regs->status) & SR_TX_FIFO_FULL)
		;
	out_be32(&regs->tx_fifo, c & 0xff);
}

static void uartlite_serial_puts(const char *s, const int port)
{
	while (*s)
		uartlite_serial_putc(*s++, port);
}

static int uartlite_serial_getc(const int port)
{
	struct uartlite *regs = userial_ports[port];

	while (!(in_be32(&regs->status) & SR_RX_FIFO_VALID_DATA))
		;
	return in_be32(&regs->rx_fifo) & 0xff;
}

static int uartlite_serial_tstc(const int port)
{
	struct uartlite *regs = userial_ports[port];

	return in_be32(&regs->status) & SR_RX_FIFO_VALID_DATA;
}

static int uartlite_serial_init(const int port)
{
	if (userial_ports[port])
		return 0;
	return -1;
}

/* Multi serial device functions */
#define DECLARE_ESERIAL_FUNCTIONS(port) \
	static int userial##port##_init(void) \
				{ return uartlite_serial_init(port); } \
	static void userial##port##_setbrg(void) {} \
	static int userial##port##_getc(void) \
				{ return uartlite_serial_getc(port); } \
	static int userial##port##_tstc(void) \
				{ return uartlite_serial_tstc(port); } \
	static void userial##port##_putc(const char c) \
				{ uartlite_serial_putc(c, port); } \
	static void userial##port##_puts(const char *s) \
				{ uartlite_serial_puts(s, port); }

/* Serial device descriptor */
#define INIT_ESERIAL_STRUCTURE(port, __name) {	\
	.name	= __name,			\
	.start	= userial##port##_init,		\
	.stop	= NULL,				\
	.setbrg	= userial##port##_setbrg,	\
	.getc	= userial##port##_getc,		\
	.tstc	= userial##port##_tstc,		\
	.putc	= userial##port##_putc,		\
	.puts	= userial##port##_puts,		\
}

DECLARE_ESERIAL_FUNCTIONS(0);
struct serial_device uartlite_serial0_device =
	INIT_ESERIAL_STRUCTURE(0, "ttyUL0");
DECLARE_ESERIAL_FUNCTIONS(1);
struct serial_device uartlite_serial1_device =
	INIT_ESERIAL_STRUCTURE(1, "ttyUL1");
DECLARE_ESERIAL_FUNCTIONS(2);
struct serial_device uartlite_serial2_device =
	INIT_ESERIAL_STRUCTURE(2, "ttyUL2");
DECLARE_ESERIAL_FUNCTIONS(3);
struct serial_device uartlite_serial3_device =
	INIT_ESERIAL_STRUCTURE(3, "ttyUL3");

__weak struct serial_device *default_serial_console(void)
{
	if (userial_ports[0])
		return &uartlite_serial0_device;
	if (userial_ports[1])
		return &uartlite_serial1_device;
	if (userial_ports[2])
		return &uartlite_serial2_device;
	if (userial_ports[3])
		return &uartlite_serial3_device;

	return NULL;
}

void uartlite_serial_initialize(void)
{
#ifdef XILINX_UARTLITE_BASEADDR
	serial_register(&uartlite_serial0_device);
#endif /* XILINX_UARTLITE_BASEADDR */
#ifdef XILINX_UARTLITE_BASEADDR1
	serial_register(&uartlite_serial1_device);
#endif /* XILINX_UARTLITE_BASEADDR1 */
#ifdef XILINX_UARTLITE_BASEADDR2
	serial_register(&uartlite_serial2_device);
#endif /* XILINX_UARTLITE_BASEADDR2 */
#ifdef XILINX_UARTLITE_BASEADDR3
	serial_register(&uartlite_serial3_device);
#endif /* XILINX_UARTLITE_BASEADDR3 */
}
