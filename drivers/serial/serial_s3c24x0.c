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
#if defined(CONFIG_S3C2400) || defined(CONFIG_TRAB)
#include <s3c2400.h>
#elif defined(CONFIG_S3C2410)
#include <s3c2410.h>
#endif

DECLARE_GLOBAL_DATA_PTR;

#ifdef CONFIG_SERIAL1
#define UART_NR	S3C24X0_UART0

#elif defined(CONFIG_SERIAL2)
# if defined(CONFIG_TRAB)
#  error "TRAB supports only CONFIG_SERIAL1"
# endif
#define UART_NR	S3C24X0_UART1

#elif defined(CONFIG_SERIAL3)
# if defined(CONFIG_TRAB)
#  #error "TRAB supports only CONFIG_SERIAL1"
# endif
#define UART_NR	S3C24X0_UART2

#else
#error "Bad: you didn't configure serial ..."
#endif

#if defined(CONFIG_SERIAL_MULTI)
#include <serial.h>

/* Multi serial device functions */
#define DECLARE_S3C_SERIAL_FUNCTIONS(port) \
    int  s3serial##port##_init (void) {\
	return serial_init_dev(port);}\
    void s3serial##port##_setbrg (void) {\
	serial_setbrg_dev(port);}\
    int  s3serial##port##_getc (void) {\
	return serial_getc_dev(port);}\
    int  s3serial##port##_tstc (void) {\
	return serial_tstc_dev(port);}\
    void s3serial##port##_putc (const char c) {\
	serial_putc_dev(port, c);}\
    void s3serial##port##_puts (const char *s) {\
	serial_puts_dev(port, s);}

#define INIT_S3C_SERIAL_STRUCTURE(port,name,bus) {\
	name,\
	bus,\
	s3serial##port##_init,\
	s3serial##port##_setbrg,\
	s3serial##port##_getc,\
	s3serial##port##_tstc,\
	s3serial##port##_putc,\
	s3serial##port##_puts, }

#endif /* CONFIG_SERIAL_MULTI */

void _serial_setbrg(const int dev_index)
{
	S3C24X0_UART * const uart = S3C24X0_GetBase_UART(dev_index);
	unsigned int reg = 0;
	int i;

	/* value is calculated so : (int)(PCLK/16./baudrate) -1 */
	reg = get_PCLK() / (16 * gd->baudrate) - 1;

	uart->UBRDIV = reg;
	for (i = 0; i < 100; i++);
}
#if defined(CONFIG_SERIAL_MULTI)
static inline void
serial_setbrg_dev(unsigned int dev_index)
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
	S3C24X0_UART * const uart = S3C24X0_GetBase_UART(dev_index);

	/* FIFO enable, Tx/Rx FIFO clear */
	uart->UFCON = 0x07;
	uart->UMCON = 0x0;

	/* Normal,No parity,1 stop,8 bit */
	uart->ULCON = 0x3;
	/*
	 * tx=level,rx=edge,disable timeout int.,enable rx error int.,
	 * normal,interrupt or polling
	 */
	uart->UCON = 0x245;

#ifdef CONFIG_HWFLOW
	uart->UMCON = 0x1; /* RTS up */
#endif

	/* FIXME: This is sooooooooooooooooooo ugly */
#if defined(CONFIG_ARCH_GTA02_v1) || defined(CONFIG_ARCH_GTA02_v2)
	/* we need auto hw flow control on the gsm and gps port */
	if (dev_index == 0 || dev_index == 1)
		uart->UMCON = 0x10;
#endif
	_serial_setbrg(dev_index);

	return (0);
}

#if !defined(CONFIG_SERIAL_MULTI)
/* Initialise the serial port. The settings are always 8 data bits, no parity,
 * 1 stop bit, no start bits.
 */
int serial_init (void)
{
	return serial_init_dev(UART_NR);
}
#endif

/*
 * Read a single byte from the serial port. Returns 1 on success, 0
 * otherwise. When the function is succesfull, the character read is
 * written into its argument c.
 */
int _serial_getc (const int dev_index)
{
	S3C24X0_UART * const uart = S3C24X0_GetBase_UART(dev_index);

	/* wait for character to arrive */
	while (!(uart->UTRSTAT & 0x1));

	return uart->URXH & 0xff;
}
#if defined(CONFIG_SERIAL_MULTI)
static inline int serial_getc_dev(unsigned int dev_index)
{
	return _serial_getc(dev_index);
}
#else
int serial_getc (void)
{
	return _serial_getc(UART_NR);
}
#endif

#ifdef CONFIG_HWFLOW
static int hwflow = 0; /* turned off by default */
int hwflow_onoff(int on)
{
	switch(on) {
	case 0:
	default:
		break; /* return current */
	case 1:
		hwflow = 1; /* turn on */
		break;
	case -1:
		hwflow = 0; /* turn off */
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
void _serial_putc (const char c, const int dev_index)
{
	S3C24X0_UART * const uart = S3C24X0_GetBase_UART(dev_index);
#ifdef CONFIG_MODEM_SUPPORT
	if (be_quiet)
		return;
#endif

	/* wait for room in the tx FIFO */
	while (!(uart->UTRSTAT & 0x2));

#ifdef CONFIG_HWFLOW
	/* Wait for CTS up */
	while(hwflow && !(uart->UMSTAT & 0x1))
		;
#endif

	uart->UTXH = c;

	/* If \n, also do \r */
	if (c == '\n')
		serial_putc ('\r');
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
	S3C24X0_UART * const uart = S3C24X0_GetBase_UART(dev_index);

	return uart->UTRSTAT & 0x1;
}
#if defined(CONFIG_SERIAL_MULTI)
static inline int
serial_tstc_dev(unsigned int dev_index)
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
		_serial_putc (*s++, dev_index);
	}
}
#if defined(CONFIG_SERIAL_MULTI)
static inline void
serial_puts_dev(int dev_index, const char *s)
{
	_serial_puts(s, dev_index);
}
#else
void
serial_puts (const char *s)
{
	_serial_puts(s, UART_NR);
}
#endif

#if defined(CONFIG_SERIAL_MULTI)
DECLARE_S3C_SERIAL_FUNCTIONS(0);
struct serial_device s3c24xx_serial0_device =
	INIT_S3C_SERIAL_STRUCTURE(0, "s3ser0", "S3UART1");
DECLARE_S3C_SERIAL_FUNCTIONS(1);
struct serial_device s3c24xx_serial1_device =
	INIT_S3C_SERIAL_STRUCTURE(1, "s3ser1", "S3UART2");
DECLARE_S3C_SERIAL_FUNCTIONS(2);
struct serial_device s3c24xx_serial2_device =
	INIT_S3C_SERIAL_STRUCTURE(2, "s3ser2", "S3UART3");

#endif /* CONFIG_SERIAL_MULTI */
