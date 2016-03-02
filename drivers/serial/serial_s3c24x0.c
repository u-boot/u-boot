/*
 * (C) Copyright 2002
 * Gary Jennejohn, DENX Software Engineering, <garyj@denx.de>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <linux/compiler.h>
#include <asm/arch/s3c24x0_cpu.h>

DECLARE_GLOBAL_DATA_PTR;

#ifdef CONFIG_SERIAL1
#define UART_NR	S3C24X0_UART0

#elif defined(CONFIG_SERIAL2)
#define UART_NR	S3C24X0_UART1

#elif defined(CONFIG_SERIAL3)
#define UART_NR	S3C24X0_UART2

#else
#error "Bad: you didn't configure serial ..."
#endif

#include <asm/io.h>
#include <serial.h>

/* Multi serial device functions */
#define DECLARE_S3C_SERIAL_FUNCTIONS(port) \
	int s3serial##port##_init(void) \
	{ \
		return serial_init_dev(port); \
	} \
	void s3serial##port##_setbrg(void) \
	{ \
		serial_setbrg_dev(port); \
	} \
	int s3serial##port##_getc(void) \
	{ \
		return serial_getc_dev(port); \
	} \
	int s3serial##port##_tstc(void) \
	{ \
		return serial_tstc_dev(port); \
	} \
	void s3serial##port##_putc(const char c) \
	{ \
		serial_putc_dev(port, c); \
	} \
	void s3serial##port##_puts(const char *s) \
	{ \
		serial_puts_dev(port, s); \
	}

#define INIT_S3C_SERIAL_STRUCTURE(port, __name) {	\
	.name	= __name,				\
	.start	= s3serial##port##_init,		\
	.stop	= NULL,					\
	.setbrg	= s3serial##port##_setbrg,		\
	.getc	= s3serial##port##_getc,		\
	.tstc	= s3serial##port##_tstc,		\
	.putc	= s3serial##port##_putc,		\
	.puts	= s3serial##port##_puts,		\
}

static void _serial_setbrg(const int dev_index)
{
	struct s3c24x0_uart *uart = s3c24x0_get_base_uart(dev_index);
	unsigned int reg = 0;
	int i;

	/* value is calculated so : (int)(PCLK/16./baudrate) -1 */
	reg = get_PCLK() / (16 * gd->baudrate) - 1;

	writel(reg, &uart->ubrdiv);
	for (i = 0; i < 100; i++)
		/* Delay */ ;
}

static inline void serial_setbrg_dev(unsigned int dev_index)
{
	_serial_setbrg(dev_index);
}

/* Initialise the serial port. The settings are always 8 data bits, no parity,
 * 1 stop bit, no start bits.
 */
static int serial_init_dev(const int dev_index)
{
	struct s3c24x0_uart *uart = s3c24x0_get_base_uart(dev_index);

	/* FIFO enable, Tx/Rx FIFO clear */
	writel(0x07, &uart->ufcon);
	writel(0x0, &uart->umcon);

	/* Normal,No parity,1 stop,8 bit */
	writel(0x3, &uart->ulcon);
	/*
	 * tx=level,rx=edge,disable timeout int.,enable rx error int.,
	 * normal,interrupt or polling
	 */
	writel(0x245, &uart->ucon);

	_serial_setbrg(dev_index);

	return (0);
}

/*
 * Read a single byte from the serial port. Returns 1 on success, 0
 * otherwise. When the function is succesfull, the character read is
 * written into its argument c.
 */
static int _serial_getc(const int dev_index)
{
	struct s3c24x0_uart *uart = s3c24x0_get_base_uart(dev_index);

	while (!(readl(&uart->utrstat) & 0x1))
		/* wait for character to arrive */ ;

	return readb(&uart->urxh) & 0xff;
}

static inline int serial_getc_dev(unsigned int dev_index)
{
	return _serial_getc(dev_index);
}

/*
 * Output a single byte to the serial port.
 */
static void _serial_putc(const char c, const int dev_index)
{
	struct s3c24x0_uart *uart = s3c24x0_get_base_uart(dev_index);

	/* If \n, also do \r */
	if (c == '\n')
		serial_putc('\r');

	while (!(readl(&uart->utrstat) & 0x2))
		/* wait for room in the tx FIFO */ ;

	writeb(c, &uart->utxh);
}

static inline void serial_putc_dev(unsigned int dev_index, const char c)
{
	_serial_putc(c, dev_index);
}

/*
 * Test whether a character is in the RX buffer
 */
static int _serial_tstc(const int dev_index)
{
	struct s3c24x0_uart *uart = s3c24x0_get_base_uart(dev_index);

	return readl(&uart->utrstat) & 0x1;
}

static inline int serial_tstc_dev(unsigned int dev_index)
{
	return _serial_tstc(dev_index);
}

static void _serial_puts(const char *s, const int dev_index)
{
	while (*s) {
		_serial_putc(*s++, dev_index);
	}
}

static inline void serial_puts_dev(int dev_index, const char *s)
{
	_serial_puts(s, dev_index);
}

DECLARE_S3C_SERIAL_FUNCTIONS(0);
struct serial_device s3c24xx_serial0_device =
INIT_S3C_SERIAL_STRUCTURE(0, "s3ser0");
DECLARE_S3C_SERIAL_FUNCTIONS(1);
struct serial_device s3c24xx_serial1_device =
INIT_S3C_SERIAL_STRUCTURE(1, "s3ser1");
DECLARE_S3C_SERIAL_FUNCTIONS(2);
struct serial_device s3c24xx_serial2_device =
INIT_S3C_SERIAL_STRUCTURE(2, "s3ser2");

__weak struct serial_device *default_serial_console(void)
{
#if defined(CONFIG_SERIAL1)
	return &s3c24xx_serial0_device;
#elif defined(CONFIG_SERIAL2)
	return &s3c24xx_serial1_device;
#elif defined(CONFIG_SERIAL3)
	return &s3c24xx_serial2_device;
#else
#error "CONFIG_SERIAL? missing."
#endif
}

void s3c24xx_serial_initialize(void)
{
	serial_register(&s3c24xx_serial0_device);
	serial_register(&s3c24xx_serial1_device);
	serial_register(&s3c24xx_serial2_device);
}
