/*
 * Altera NiosII YANU serial interface by Imagos
 * please see  http://www.opencores.org/project,yanu for
 * information/downloads
 *
 * Copyright 2010, Renato Andreola <renato.andreola@imagos.it>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <watchdog.h>
#include <asm/io.h>
#include <serial.h>

DECLARE_GLOBAL_DATA_PTR;

/*-----------------------------------------------------------------*/
/* YANU Imagos serial port */
/*-----------------------------------------------------------------*/

#define YANU_MAX_PRESCALER_N   ((1 << 4) - 1)	/* 15 */
#define YANU_MAX_PRESCALER_M   ((1 << 11) -1)	/* 2047 */
#define YANU_FIFO_SIZE         (16)
#define YANU_RXFIFO_SIZE       (YANU_FIFO_SIZE)
#define YANU_TXFIFO_SIZE       (YANU_FIFO_SIZE)

#define YANU_RXFIFO_DLY        (10*11)
#define YANU_TXFIFO_THR        (10)
#define YANU_DATA_CHAR_MASK    (0xFF)

/* data register */
#define YANU_DATA_OFFSET       (0)	/* data register offset */

#define YANU_CONTROL_OFFSET    (4)	/* control register offset */
/* interrupt enable */
#define YANU_CONTROL_IE_RRDY   (1<<0)	/* ie on received character ready */
#define YANU_CONTROL_IE_OE     (1<<1)	/* ie on rx overrun    */
#define YANU_CONTROL_IE_BRK    (1<<2)	/* ie on break detect  */
#define YANU_CONTROL_IE_FE     (1<<3)	/* ie on framing error */
#define YANU_CONTROL_IE_PE     (1<<4)	/* ie on parity error  */
#define YANU_CONTROL_IE_TRDY   (1<<5)	/* ie interrupt on tranmitter ready */
/* control bits */
#define YANU_CONTROL_BITS_POS  (6)	/* bits number pos */
#define YANU_CONTROL_BITS      (1<<YANU_CONTROL_BITS_POS)	/* number of rx/tx bits per word. 3 bit unsigned integer */
#define YANU_CONTROL_BITS_N    (3)	/* ... its bit filed length */
#define YANU_CONTROL_PARENA    (1<<9)	/*  enable parity bit transmission/reception */
#define YANU_CONTROL_PAREVEN   (1<<10)	/* parity even */
#define YANU_CONTROL_STOPS     (1<<11)	/* number of stop bits */
#define YANU_CONTROL_HHENA     (1<<12)	/* Harware Handshake enable... */
#define YANU_CONTROL_FORCEBRK  (1<<13)	/* if set than txd = active (0) */
/* tuning part */
#define YANU_CONTROL_RDYDLY    (1<<14)	/* delay from "first" before setting rrdy (in bit) */
#define YANU_CONTROL_RDYDLY_N  (8)	/* ... its bit filed length */
#define YANU_CONTROL_TXTHR     (1<<22)	/* tx interrupt threshold: the trdy set if txfifo_chars<= txthr (chars) */
#define YANU_CONTROL_TXTHR_N   (4)	/* ... its bit field length */

#define YANU_BAUD_OFFSET  (8)	/* baud register offset */
#define YANU_BAUDM        (1<<0)	/* baud mantissa lsb */
#define YANU_BAUDM_N      (12)	/* ...its bit filed length */
#define YANU_BAUDE        (1<<12)	/* baud exponent lsb */
#define YANU_BAUDE_N      (4)	/* ...its bit field length */

#define YANU_ACTION_OFFSET   (12)	/* action register... write only */
#define YANU_ACTION_RRRDY    (1<<0)	/* reset rrdy */
#define YANU_ACTION_ROE      (1<<1)	/* reset oe */
#define YANU_ACTION_RBRK     (1<<2)	/* reset brk */
#define YANU_ACTION_RFE      (1<<3)	/* reset fe  */
#define YANU_ACTION_RPE      (1<<4)	/* reset pe  */
#define YANU_ACTION_SRRDY    (1<<5)	/* set rrdy  */
#define YANU_ACTION_SOE      (1<<6)	/* set oe    */
#define YANU_ACTION_SBRK     (1<<7)	/* set brk   */
#define YANU_ACTION_SFE      (1<<8)	/* set fe    */
#define YANU_ACTION_SPE      (1<<9)	/* set pe    */
#define YANU_ACTION_RFIFO_PULL  (1<<10)	/* pull a char from rx fifo we MUST do it before taking a char */
#define YANU_ACTION_RFIFO_CLEAR (1<<11)	/* clear rx fifo */
#define YANU_ACTION_TFIFO_CLEAR (1<<12)	/* clear tx fifo */
#define YANU_ACTION_RTRDY       (1<<13)	/* clear trdy    */
#define YANU_ACTION_STRDY       (1<<14)	/* set trdy      */

#define YANU_STATUS_OFFSET   (16)
#define YANU_STATUS_RRDY     (1<<0)	/* rxrdy flag */
#define YANU_STATUS_TRDY     (1<<1)	/* txrdy flag */
#define YANU_STATUS_OE       (1<<2)	/* rx overrun error */
#define YANU_STATUS_BRK      (1<<3)	/* rx break detect flag */
#define YANU_STATUS_FE       (1<<4)	/* rx framing error flag */
#define YANU_STATUS_PE       (1<<5)	/* rx parity erro flag */
#define YANU_RFIFO_CHARS_POS (6)
#define YANU_RFIFO_CHARS     (1<<RFIFO_CHAR_POS)	/* number of chars into rx fifo */
#define YANU_RFIFO_CHARS_N   (5)	/* ...its bit field length: 32 chars */
#define YANU_TFIFO_CHARS_POS (11)
#define YANU_TFIFO_CHARS     (1<<TFIFO_CHAR_POS)	/* number of chars into tx fifo */
#define YANU_TFIFO_CHARS_N   (5)	/* ...its bit field length: 32 chars */

typedef volatile struct {
	volatile unsigned data;
	volatile unsigned control;	/* control register (RW) 32-bit   */
	volatile unsigned baud;	/* baud/prescaler register (RW) 32-bit */
	volatile unsigned action;	/* action register (W) 32-bit */
	volatile unsigned status;	/* status register (R) 32-bit */
	volatile unsigned magic;	/* magic register (R) 32-bit */
} yanu_uart_t;

static yanu_uart_t *uart = (yanu_uart_t *)CONFIG_SYS_NIOS_CONSOLE;

static void oc_serial_setbrg(void)
{
	int n, k;
	const unsigned max_uns = 0xFFFFFFFF;
	unsigned best_n, best_m, baud;
	unsigned baudrate;

#if defined(CONFIG_SYS_NIOS_FIXEDBAUD)
	/* Everything's already setup for fixed-baud PTF assignment */
	baudrate = CONFIG_BAUDRATE;
#else
	baudrate = gd->baudrate;
#endif
	/* compute best N and M couple */
	best_n = YANU_MAX_PRESCALER_N;
	for (n = YANU_MAX_PRESCALER_N; n >= 0; n--) {
		if ((unsigned)CONFIG_SYS_CLK_FREQ / (1 << (n + 4)) >=
		    baudrate) {
			best_n = n;
			break;
		}
	}
	for (k = 0;; k++) {
		if (baudrate <= (max_uns >> (15+n-k)))
			break;
	}
	best_m =
	    (baudrate * (1 << (15 + n - k))) /
	    ((unsigned)CONFIG_SYS_CLK_FREQ >> k);

	baud = best_m + best_n * YANU_BAUDE;
	writel(baud, &uart->baud);

	return;
}

static int oc_serial_init(void)
{
	unsigned action,control;

	/* status register cleanup */
	action =  YANU_ACTION_RRRDY     |
		YANU_ACTION_RTRDY       |
		YANU_ACTION_ROE         |
		YANU_ACTION_RBRK        |
		YANU_ACTION_RFE         |
		YANU_ACTION_RPE         |
	    YANU_ACTION_RFE | YANU_ACTION_RFIFO_CLEAR | YANU_ACTION_TFIFO_CLEAR;

	writel(action, &uart->action);

	/*
	 * control register cleanup
	 * no interrupts enabled
	 * one stop bit
	 * hardware flow control disabled
	 * 8 bits
	 */
	control = (0x7 << YANU_CONTROL_BITS_POS);
	/* enven parity just to be clean */
	control |= YANU_CONTROL_PAREVEN;
	/* we set threshold for fifo */
	control |= YANU_CONTROL_RDYDLY * YANU_RXFIFO_DLY;
	control |= YANU_CONTROL_TXTHR *  YANU_TXFIFO_THR;

	writel(control, &uart->control);

	/* to set baud rate */
	serial_setbrg();

	return (0);
}


/*-----------------------------------------------------------------------
 * YANU CONSOLE
 *---------------------------------------------------------------------*/
static void oc_serial_putc(char c)
{
	int tx_chars;
	unsigned status;

	if (c == '\n')
		serial_putc ('\r');

	while (1) {
		status = readl(&uart->status);
		tx_chars = (status>>YANU_TFIFO_CHARS_POS)
			& ((1<<YANU_TFIFO_CHARS_N)-1);
		if (tx_chars < YANU_TXFIFO_SIZE-1)
			break;
		WATCHDOG_RESET ();
	}

	writel((unsigned char)c, &uart->data);
}

static int oc_serial_tstc(void)
{
	unsigned status ;

	status = readl(&uart->status);
	return (((status >> YANU_RFIFO_CHARS_POS) &
		 ((1 << YANU_RFIFO_CHARS_N) - 1)) > 0);
}

static int oc_serial_getc(void)
{
	while (serial_tstc() == 0)
		WATCHDOG_RESET ();

	/* first we pull the char */
	writel(YANU_ACTION_RFIFO_PULL, &uart->action);

	return(readl(&uart->data) & YANU_DATA_CHAR_MASK);
}

static struct serial_device oc_serial_drv = {
	.name	= "oc_serial",
	.start	= oc_serial_init,
	.stop	= NULL,
	.setbrg	= oc_serial_setbrg,
	.putc	= oc_serial_putc,
	.puts	= default_serial_puts,
	.getc	= oc_serial_getc,
	.tstc	= oc_serial_tstc,
};

void oc_serial_initialize(void)
{
	serial_register(&oc_serial_drv);
}

__weak struct serial_device *default_serial_console(void)
{
	return &oc_serial_drv;
}
