/*
 * (C) Copyright 2002
 * Gary Jennejohn, DENX Software Engineering, <garyj@denx.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
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

#if defined(CONFIG_SERIAL_MULTI)
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

#define INIT_S3C_SERIAL_STRUCTURE(port, name) { \
	name, \
	s3serial##port##_init, \
	NULL,\
	s3serial##port##_setbrg, \
	s3serial##port##_getc, \
	s3serial##port##_tstc, \
	s3serial##port##_putc, \
	s3serial##port##_puts, \
}

#endif /* CONFIG_SERIAL_MULTI */

#ifdef CONFIG_HWFLOW
static int hwflow;
#endif

void _serial_setbrg(const int dev_index)
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

#if defined(CONFIG_SERIAL_MULTI)
static inline void serial_setbrg_dev(unsigned int dev_index)
{
	_serial_setbrg(dev_index);
}
#else
void serial_setbrg(void)
{
	_serial_setbrg(UART_NR);
}
#endif


/* Initialise the serial port. The settings are always 8 data bits, no parity,
 * 1 stop bit, no start bits.
 */
static int serial_init_dev(const int dev_index)
{
	struct s3c24x0_uart *uart = s3c24x0_get_base_uart(dev_index);

#ifdef CONFIG_HWFLOW
	hwflow = 0;	/* turned off by default */
#endif

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

#ifdef CONFIG_HWFLOW
	writel(0x1, &uart->umcon);	/* rts up */
#endif

	/* FIXME: This is sooooooooooooooooooo ugly */
#if defined(CONFIG_ARCH_GTA02_v1) || defined(CONFIG_ARCH_GTA02_v2)
	/* we need auto hw flow control on the gsm and gps port */
	if (dev_index == 0 || dev_index == 1)
		writel(0x10, &uart->umcon);
#endif
	_serial_setbrg(dev_index);

	return (0);
}

#if !defined(CONFIG_SERIAL_MULTI)
/* Initialise the serial port. The settings are always 8 data bits, no parity,
 * 1 stop bit, no start bits.
 */
int serial_init(void)
{
	return serial_init_dev(UART_NR);
}
#endif

/*
 * Read a single byte from the serial port. Returns 1 on success, 0
 * otherwise. When the function is succesfull, the character read is
 * written into its argument c.
 */
int _serial_getc(const int dev_index)
{
	struct s3c24x0_uart *uart = s3c24x0_get_base_uart(dev_index);

	while (!(readl(&uart->utrstat) & 0x1))
		/* wait for character to arrive */ ;

	return readb(&uart->urxh) & 0xff;
}

#if defined(CONFIG_SERIAL_MULTI)
static inline int serial_getc_dev(unsigned int dev_index)
{
	return _serial_getc(dev_index);
}
#else
int serial_getc(void)
{
	return _serial_getc(UART_NR);
}
#endif

#ifdef CONFIG_HWFLOW
int hwflow_onoff(int on)
{
	switch (on) {
	case 0:
	default:
		break;		/* return current */
	case 1:
		hwflow = 1;	/* turn on */
		break;
	case -1:
		hwflow = 0;	/* turn off */
		break;
	}
	return hwflow;
}
#endif

#ifdef CONFIG_MODEM_SUPPORT
static int be_quiet = 0;
void disable_putc(void)
{
	be_quiet = 1;
}

void enable_putc(void)
{
	be_quiet = 0;
}
#endif


/*
 * Output a single byte to the serial port.
 */
void _serial_putc(const char c, const int dev_index)
{
	struct s3c24x0_uart *uart = s3c24x0_get_base_uart(dev_index);
#ifdef CONFIG_MODEM_SUPPORT
	if (be_quiet)
		return;
#endif

	while (!(readl(&uart->utrstat) & 0x2))
		/* wait for room in the tx FIFO */ ;

#ifdef CONFIG_HWFLOW
	while (hwflow && !(readl(&uart->umstat) & 0x1))
		/* Wait for CTS up */ ;
#endif

	writeb(c, &uart->utxh);

	/* If \n, also do \r */
	if (c == '\n')
		serial_putc('\r');
}

#if defined(CONFIG_SERIAL_MULTI)
static inline void serial_putc_dev(unsigned int dev_index, const char c)
{
	_serial_putc(c, dev_index);
}
#else
void serial_putc(const char c)
{
	_serial_putc(c, UART_NR);
}
#endif


/*
 * Test whether a character is in the RX buffer
 */
int _serial_tstc(const int dev_index)
{
	struct s3c24x0_uart *uart = s3c24x0_get_base_uart(dev_index);

	return readl(&uart->utrstat) & 0x1;
}

#if defined(CONFIG_SERIAL_MULTI)
static inline int serial_tstc_dev(unsigned int dev_index)
{
	return _serial_tstc(dev_index);
}
#else
int serial_tstc(void)
{
	return _serial_tstc(UART_NR);
}
#endif

void _serial_puts(const char *s, const int dev_index)
{
	while (*s) {
		_serial_putc(*s++, dev_index);
	}
}

#if defined(CONFIG_SERIAL_MULTI)
static inline void serial_puts_dev(int dev_index, const char *s)
{
	_serial_puts(s, dev_index);
}
#else
void serial_puts(const char *s)
{
	_serial_puts(s, UART_NR);
}
#endif

#if defined(CONFIG_SERIAL_MULTI)
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
#endif /* CONFIG_SERIAL_MULTI */
