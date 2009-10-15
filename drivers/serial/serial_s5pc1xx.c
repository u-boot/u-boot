/*
 * (C) Copyright 2009 SAMSUNG Electronics
 * Minkyu Kang <mk7.kang@samsung.com>
 * Heungjun Kim <riverful.kim@samsung.com>
 *
 * based on drivers/serial/s3c64xx.c
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
#include <asm/io.h>
#include <asm/arch/uart.h>
#include <asm/arch/clk.h>
#include <serial.h>

static inline struct s5pc1xx_uart *s5pc1xx_get_base_uart(int dev_index)
{
	u32 offset = dev_index * sizeof(struct s5pc1xx_uart);

	if (cpu_is_s5pc100())
		return (struct s5pc1xx_uart *)(S5PC100_UART_BASE + offset);
	else
		return (struct s5pc1xx_uart *)(S5PC110_UART_BASE + offset);
}

/*
 * The coefficient, used to calculate the baudrate on S5PC1XX UARTs is
 * calculated as
 * C = UBRDIV * 16 + number_of_set_bits_in_UDIVSLOT
 * however, section 31.6.11 of the datasheet doesn't recomment using 1 for 1,
 * 3 for 2, ... (2^n - 1) for n, instead, they suggest using these constants:
 */
static const int udivslot[] = {
	0,
	0x0080,
	0x0808,
	0x0888,
	0x2222,
	0x4924,
	0x4a52,
	0x54aa,
	0x5555,
	0xd555,
	0xd5d5,
	0xddd5,
	0xdddd,
	0xdfdd,
	0xdfdf,
	0xffdf,
};

void serial_setbrg_dev(const int dev_index)
{
	DECLARE_GLOBAL_DATA_PTR;
	struct s5pc1xx_uart *const uart = s5pc1xx_get_base_uart(dev_index);
	u32 pclk = get_pclk();
	u32 baudrate = gd->baudrate;
	u32 val;

	val = pclk / baudrate;

	writel(val / 16 - 1, &uart->ubrdiv);
	writew(udivslot[val % 16], &uart->udivslot);
}

/*
 * Initialise the serial port with the given baudrate. The settings
 * are always 8 data bits, no parity, 1 stop bit, no start bits.
 */
int serial_init_dev(const int dev_index)
{
	struct s5pc1xx_uart *const uart = s5pc1xx_get_base_uart(dev_index);

	/* reset and enable FIFOs, set triggers to the maximum */
	writel(0, &uart->ufcon);
	writel(0, &uart->umcon);
	/* 8N1 */
	writel(0x3, &uart->ulcon);
	/* No interrupts, no DMA, pure polling */
	writel(0x245, &uart->ucon);

	serial_setbrg_dev(dev_index);

	return 0;
}

static int serial_err_check(const int dev_index)
{
	struct s5pc1xx_uart *const uart = s5pc1xx_get_base_uart(dev_index);

	if (readl(&uart->uerstat) & 0xf)
		return 1;

	return 0;
}

/*
 * Read a single byte from the serial port. Returns 1 on success, 0
 * otherwise. When the function is succesfull, the character read is
 * written into its argument c.
 */
int serial_getc_dev(const int dev_index)
{
	struct s5pc1xx_uart *const uart = s5pc1xx_get_base_uart(dev_index);

	/* wait for character to arrive */
	while (!(readl(&uart->utrstat) & 0x1)) {
		if (serial_err_check(dev_index))
			return 0;
	}

	return (int)(readl(&uart->urxh) & 0xff);
}

/*
 * Output a single byte to the serial port.
 */
void serial_putc_dev(const char c, const int dev_index)
{
	struct s5pc1xx_uart *const uart = s5pc1xx_get_base_uart(dev_index);

	/* wait for room in the tx FIFO */
	while (!(readl(&uart->utrstat) & 0x2)) {
		if (serial_err_check(dev_index))
			return;
	}

	writel(c, &uart->utxh);

	/* If \n, also do \r */
	if (c == '\n')
		serial_putc('\r');
}

/*
 * Test whether a character is in the RX buffer
 */
int serial_tstc_dev(const int dev_index)
{
	struct s5pc1xx_uart *const uart = s5pc1xx_get_base_uart(dev_index);

	return (int)(readl(&uart->utrstat) & 0x1);
}

void serial_puts_dev(const char *s, const int dev_index)
{
	while (*s)
		serial_putc_dev(*s++, dev_index);
}

/* Multi serial device functions */
#define DECLARE_S5P_SERIAL_FUNCTIONS(port) \
int s5p_serial##port##_init(void) { return serial_init_dev(port); } \
void s5p_serial##port##_setbrg(void) { serial_setbrg_dev(port); } \
int s5p_serial##port##_getc(void) { return serial_getc_dev(port); } \
int s5p_serial##port##_tstc(void) { return serial_tstc_dev(port); } \
void s5p_serial##port##_putc(const char c) { serial_putc_dev(c, port); } \
void s5p_serial##port##_puts(const char *s) { serial_puts_dev(s, port); }

#define INIT_S5P_SERIAL_STRUCTURE(port, name, bus) { \
	name, \
	bus, \
	s5p_serial##port##_init, \
	s5p_serial##port##_setbrg, \
	s5p_serial##port##_getc, \
	s5p_serial##port##_tstc, \
	s5p_serial##port##_putc, \
	s5p_serial##port##_puts, }

DECLARE_S5P_SERIAL_FUNCTIONS(0);
struct serial_device s5pc1xx_serial0_device =
	INIT_S5P_SERIAL_STRUCTURE(0, "s5pser0", "S5PUART0");
DECLARE_S5P_SERIAL_FUNCTIONS(1);
struct serial_device s5pc1xx_serial1_device =
	INIT_S5P_SERIAL_STRUCTURE(1, "s5pser1", "S5PUART1");
DECLARE_S5P_SERIAL_FUNCTIONS(2);
struct serial_device s5pc1xx_serial2_device =
	INIT_S5P_SERIAL_STRUCTURE(2, "s5pser2", "S5PUART2");
DECLARE_S5P_SERIAL_FUNCTIONS(3);
struct serial_device s5pc1xx_serial3_device =
	INIT_S5P_SERIAL_STRUCTURE(3, "s5pser3", "S5PUART3");
