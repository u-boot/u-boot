/*
 * (C) Copyright 2003
 *
 * Pantelis Antoniou <panto@intracom.gr>
 * Intracom S.A.
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <common.h>
#include <watchdog.h>

#ifdef CONFIG_MAX3100_SERIAL

/**************************************************************/

/* convienient macros */
#define MAX3100_SPI_RXD() (MAX3100_SPI_RXD_PORT & MAX3100_SPI_RXD_BIT)

#define MAX3100_SPI_TXD(x) \
	do { \
		if (x) \
			MAX3100_SPI_TXD_PORT |=  MAX3100_SPI_TXD_BIT; \
		else \
			MAX3100_SPI_TXD_PORT &= ~MAX3100_SPI_TXD_BIT; \
	} while(0)

#define MAX3100_SPI_CLK(x) \
	do { \
		if (x) \
			MAX3100_SPI_CLK_PORT |=  MAX3100_SPI_CLK_BIT; \
		else \
			MAX3100_SPI_CLK_PORT &= ~MAX3100_SPI_CLK_BIT; \
	} while(0)

#define MAX3100_SPI_CLK_TOGGLE() (MAX3100_SPI_CLK_PORT ^= MAX3100_SPI_CLK_BIT)

#define MAX3100_CS(x) \
	do { \
		if (x) \
			MAX3100_CS_PORT |=  MAX3100_CS_BIT; \
		else \
			MAX3100_CS_PORT &= ~MAX3100_CS_BIT; \
	} while(0)

/**************************************************************/

/* MAX3100 definitions */

#define MAX3100_WC	(3 << 14)		/* write configuration */
#define MAX3100_RC	(1 << 14)		/* read  configuration */
#define MAX3100_WD	(2 << 14)		/* write data          */
#define MAX3100_RD	(0 << 14)		/* read  data          */

/* configuration register bits */
#define MAX3100_FEN	(1 << 13)		/* FIFO enable           */
#define MAX3100_SHDN    (1 << 12)		/* shutdown bit          */
#define MAX3100_TM	(1 << 11)		/* T bit irq mask        */
#define MAX3100_RM	(1 << 10)		/* R bit irq mask        */
#define MAX3100_PM	(1 <<  9)		/* P bit irq mask        */
#define MAX3100_RAM	(1 <<  8)		/* mask for RA/FE bit    */
#define MAX3100_IR	(1 <<  7)		/* IRDA timing mode      */
#define MAX3100_ST	(1 <<  6)		/* transmit stop bit     */
#define MAX3100_PE	(1 <<  5)		/* parity enable bit     */
#define MAX3100_L	(1 <<  4)		/* Length bit            */
#define MAX3100_B_MASK	(0x000F)		/* baud rate bits mask   */
#define MAX3100_B(x)	((x) & 0x000F)	/* baud rate select bits */

/* data register bits (write) */
#define MAX3100_TE	(1 << 10)		/* transmit enable bit (active low)        */
#define MAX3100_RTS	(1 <<  9)		/* request-to-send bit (inverted ~RTS pin) */

/* data register bits (read) */
#define MAX3100_RA	(1 << 10)		/* receiver activity when in shutdown mode */
#define MAX3100_FE	(1 << 10)		/* framing error when in normal mode       */
#define MAX3100_CTS	(1 <<  9)		/* clear-to-send bit (inverted ~CTS pin)   */

/* data register bits (both directions) */
#define MAX3100_R 	(1 << 15)		/* receive bit    */
#define MAX3100_T	(1 << 14)		/* transmit bit   */
#define MAX3100_P	(1 <<  8)		/* parity bit     */
#define MAX3100_D_MASK	0x00FF                  /* data bits mask */
#define MAX3100_D(x)	((x) & 0x00FF)		/* data bits      */

/* these definitions are valid only for fOSC = 3.6864MHz */
#define MAX3100_B_230400        MAX3100_B(0)
#define MAX3100_B_115200        MAX3100_B(1)
#define MAX3100_B_57600         MAX3100_B(2)
#define MAX3100_B_38400         MAX3100_B(9)
#define MAX3100_B_19200         MAX3100_B(10)
#define MAX3100_B_9600          MAX3100_B(11)
#define MAX3100_B_4800          MAX3100_B(12)
#define MAX3100_B_2400          MAX3100_B(13)
#define MAX3100_B_1200          MAX3100_B(14)
#define MAX3100_B_600           MAX3100_B(15)

/**************************************************************/

static inline unsigned int max3100_transfer(unsigned int val)
{
	unsigned int rx;
	int b;

	MAX3100_SPI_CLK(0);
	MAX3100_CS(0);

	rx = 0; b = 16;
	while (--b >= 0) {
		MAX3100_SPI_TXD(val & 0x8000);
		val <<= 1;
		MAX3100_SPI_CLK_TOGGLE();
		udelay(1);
		rx <<= 1;
		if (MAX3100_SPI_RXD())
			rx |= 1;
		MAX3100_SPI_CLK_TOGGLE();
		udelay(1);
	}

	MAX3100_SPI_CLK(1);
	MAX3100_CS(1);

	return rx;
}

/**************************************************************/

/* must be power of 2 */
#define RXFIFO_SZ	16

static int rxfifo_cnt;
static int rxfifo_in;
static int rxfifo_out;
static unsigned char rxfifo_buf[16];

static void max3100_putc(int c)
{
	unsigned int rx;

	while (((rx = max3100_transfer(MAX3100_RC)) & MAX3100_T) == 0)
		WATCHDOG_RESET();

	rx = max3100_transfer(MAX3100_WD | (c & 0xff));
	if ((rx & MAX3100_RD) != 0 && rxfifo_cnt < RXFIFO_SZ) {
		rxfifo_cnt++;
		rxfifo_buf[rxfifo_in++] = rx & 0xff;
		rxfifo_in &= RXFIFO_SZ - 1;
	}
}

static int max3100_getc(void)
{
	int c;
	unsigned int rx;

	while (rxfifo_cnt == 0) {
		rx = max3100_transfer(MAX3100_RD);
		if ((rx & MAX3100_R) != 0) {
			do {
				rxfifo_cnt++;
				rxfifo_buf[rxfifo_in++] = rx & 0xff;
				rxfifo_in &= RXFIFO_SZ - 1;

				if (rxfifo_cnt >= RXFIFO_SZ)
					break;
			} while (((rx = max3100_transfer(MAX3100_RD)) & MAX3100_R) != 0);
		}
		WATCHDOG_RESET();
	}

	rxfifo_cnt--;
	c = rxfifo_buf[rxfifo_out++];
	rxfifo_out &= RXFIFO_SZ - 1;
	return c;
}

static int max3100_tstc(void)
{
	unsigned int rx;

	if (rxfifo_cnt > 0)
		return 1;

	rx = max3100_transfer(MAX3100_RD);
	if ((rx & MAX3100_R) == 0)
		return 0;

	do {
		rxfifo_cnt++;
		rxfifo_buf[rxfifo_in++] = rx & 0xff;
		rxfifo_in &= RXFIFO_SZ - 1;

		if (rxfifo_cnt >= RXFIFO_SZ)
			break;
	} while (((rx = max3100_transfer(MAX3100_RD)) & MAX3100_R) != 0);

	return 1;
}

int serial_init(void)
{
	unsigned int wconf, rconf;
	int i;
	DECLARE_GLOBAL_DATA_PTR;

	wconf = 0;

	/* Set baud rate */
	switch (gd->baudrate) {
		case 1200:
			wconf = MAX3100_B_1200;
			break;
		case 2400:
			wconf = MAX3100_B_2400;
			break;
		case 4800:
			wconf = MAX3100_B_4800;
			break;
		case 9600:
			wconf = MAX3100_B_9600;
			break;
		case 19200:
			wconf = MAX3100_B_19200;
			break;
		case 38400:
			wconf = MAX3100_B_38400;
			break;
		case 57600:
			wconf = MAX3100_B_57600;
			break;
		default:
		case 115200:
			wconf = MAX3100_B_115200;
			break;
		case 230400:
			wconf = MAX3100_B_230400;
			break;
	}

	/* try for 10ms, with a 100us gap */
	for (i = 0; i < 10000; i += 100) {

		max3100_transfer(MAX3100_WC | wconf);
		rconf = max3100_transfer(MAX3100_RC) & 0x3fff;

		if (rconf == wconf)
			break;
		udelay(100);
	}

	rxfifo_in = rxfifo_out = rxfifo_cnt = 0;

	return (0);
}

void serial_putc(const char c)
{
	if (c == '\n')
		max3100_putc('\r');

	max3100_putc(c);
}

void serial_puts(const char *s)
{
	while (*s)
		serial_putc (*s++);
}

int serial_getc(void)
{
	return max3100_getc();
}

int serial_tstc(void)
{
	return max3100_tstc();
}

/* XXX WTF? */
void serial_setbrg(void)
{
}

#endif
