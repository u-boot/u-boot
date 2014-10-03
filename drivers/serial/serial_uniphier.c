/*
 * Copyright (C) 2012-2014 Panasonic Corporation
 *   Author: Masahiro Yamada <yamada.m@jp.panasonic.com>
 *
 * Based on serial_ns16550.c
 * (C) Copyright 2000
 * Rob Taylor, Flying Pig Systems. robt@flyingpig.com.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <serial.h>

#define UART_REG(x)					\
	u8 x;						\
	u8 postpad_##x[3];

/*
 * Note: Register map is slightly different from that of 16550.
 */
struct uniphier_serial {
	UART_REG(rbr);		/* 0x00 */
	UART_REG(ier);		/* 0x04 */
	UART_REG(iir);		/* 0x08 */
	UART_REG(fcr);		/* 0x0c */
	u8 mcr;			/* 0x10 */
	u8 lcr;
	u16 __postpad;
	UART_REG(lsr);		/* 0x14 */
	UART_REG(msr);		/* 0x18 */
	u32 __none1;
	u32 __none2;
	u16 dlr;
	u16 __postpad2;
};

#define thr rbr

/*
 * These are the definitions for the Line Control Register
 */
#define UART_LCR_WLS_8	0x03		/* 8 bit character length */

/*
 * These are the definitions for the Line Status Register
 */
#define UART_LSR_DR	0x01		/* Data ready */
#define UART_LSR_THRE	0x20		/* Xmit holding register empty */

DECLARE_GLOBAL_DATA_PTR;

static void uniphier_serial_init(struct uniphier_serial *port)
{
	const unsigned int mode_x_div = 16;
	unsigned int divisor;

	writeb(UART_LCR_WLS_8, &port->lcr);

	divisor = DIV_ROUND_CLOSEST(CONFIG_SYS_UNIPHIER_UART_CLK,
						mode_x_div * gd->baudrate);

	writew(divisor, &port->dlr);
}

static void uniphier_serial_setbrg(struct uniphier_serial *port)
{
	uniphier_serial_init(port);
}

static int uniphier_serial_tstc(struct uniphier_serial *port)
{
	return (readb(&port->lsr) & UART_LSR_DR) != 0;
}

static int uniphier_serial_getc(struct uniphier_serial *port)
{
	while (!uniphier_serial_tstc(port))
		;

	return readb(&port->rbr);
}

static void uniphier_serial_putc(struct uniphier_serial *port, const char c)
{
	if (c == '\n')
		uniphier_serial_putc(port, '\r');

	while (!(readb(&port->lsr) & UART_LSR_THRE))
		;

	writeb(c, &port->thr);
}

static struct uniphier_serial *serial_ports[4] = {
#ifdef CONFIG_SYS_UNIPHIER_SERIAL_BASE0
	(struct uniphier_serial *)CONFIG_SYS_UNIPHIER_SERIAL_BASE0,
#else
	NULL,
#endif
#ifdef CONFIG_SYS_UNIPHIER_SERIAL_BASE1
	(struct uniphier_serial *)CONFIG_SYS_UNIPHIER_SERIAL_BASE1,
#else
	NULL,
#endif
#ifdef CONFIG_SYS_UNIPHIER_SERIAL_BASE2
	(struct uniphier_serial *)CONFIG_SYS_UNIPHIER_SERIAL_BASE2,
#else
	NULL,
#endif
#ifdef CONFIG_SYS_UNIPHIER_SERIAL_BASE3
	(struct uniphier_serial *)CONFIG_SYS_UNIPHIER_SERIAL_BASE3,
#else
	NULL,
#endif
};

/* Multi serial device functions */
#define DECLARE_ESERIAL_FUNCTIONS(port) \
	static int  eserial##port##_init(void) \
	{ \
		uniphier_serial_init(serial_ports[port]); \
		return 0 ; \
	} \
	static void eserial##port##_setbrg(void) \
	{ \
		uniphier_serial_setbrg(serial_ports[port]); \
	} \
	static int  eserial##port##_getc(void) \
	{ \
		return uniphier_serial_getc(serial_ports[port]); \
	} \
	static int  eserial##port##_tstc(void) \
	{ \
		return uniphier_serial_tstc(serial_ports[port]); \
	} \
	static void eserial##port##_putc(const char c) \
	{ \
		uniphier_serial_putc(serial_ports[port], c); \
	}

/* Serial device descriptor */
#define INIT_ESERIAL_STRUCTURE(port, __name) {	\
	.name	= __name,			\
	.start	= eserial##port##_init,		\
	.stop	= NULL,				\
	.setbrg	= eserial##port##_setbrg,	\
	.getc	= eserial##port##_getc,		\
	.tstc	= eserial##port##_tstc,		\
	.putc	= eserial##port##_putc,		\
	.puts	= default_serial_puts,		\
}

#if defined(CONFIG_SYS_UNIPHIER_SERIAL_BASE0)
DECLARE_ESERIAL_FUNCTIONS(0);
struct serial_device uniphier_serial0_device =
	INIT_ESERIAL_STRUCTURE(0, "ttyS0");
#endif
#if defined(CONFIG_SYS_UNIPHIER_SERIAL_BASE1)
DECLARE_ESERIAL_FUNCTIONS(1);
struct serial_device uniphier_serial1_device =
	INIT_ESERIAL_STRUCTURE(1, "ttyS1");
#endif
#if defined(CONFIG_SYS_UNIPHIER_SERIAL_BASE2)
DECLARE_ESERIAL_FUNCTIONS(2);
struct serial_device uniphier_serial2_device =
	INIT_ESERIAL_STRUCTURE(2, "ttyS2");
#endif
#if defined(CONFIG_SYS_UNIPHIER_SERIAL_BASE3)
DECLARE_ESERIAL_FUNCTIONS(3);
struct serial_device uniphier_serial3_device =
	INIT_ESERIAL_STRUCTURE(3, "ttyS3");
#endif

__weak struct serial_device *default_serial_console(void)
{
#if defined(CONFIG_SYS_UNIPHIER_SERIAL_BASE0)
	return &uniphier_serial0_device;
#elif defined(CONFIG_SYS_UNIPHIER_SERIAL_BASE1)
	return &uniphier_serial1_device;
#elif defined(CONFIG_SYS_UNIPHIER_SERIAL_BASE2)
	return &uniphier_serial2_device;
#elif defined(CONFIG_SYS_UNIPHIER_SERIAL_BASE3)
	return &uniphier_serial3_device;
#else
#error "No uniphier serial ports configured."
#endif
}

void uniphier_serial_initialize(void)
{
#if defined(CONFIG_SYS_UNIPHIER_SERIAL_BASE0)
	serial_register(&uniphier_serial0_device);
#endif
#if defined(CONFIG_SYS_UNIPHIER_SERIAL_BASE1)
	serial_register(&uniphier_serial1_device);
#endif
#if defined(CONFIG_SYS_UNIPHIER_SERIAL_BASE2)
	serial_register(&uniphier_serial2_device);
#endif
#if defined(CONFIG_SYS_UNIPHIER_SERIAL_BASE3)
	serial_register(&uniphier_serial3_device);
#endif
}
