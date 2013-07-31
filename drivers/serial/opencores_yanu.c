/*
 * Copyright 2010, Renato Andreola <renato.andreola@imagos.it>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <watchdog.h>
#include <asm/io.h>
#include <nios2-yanu.h>

DECLARE_GLOBAL_DATA_PTR;

/*-----------------------------------------------------------------*/
/* YANU Imagos serial port */
/*-----------------------------------------------------------------*/

static yanu_uart_t *uart = (yanu_uart_t *)CONFIG_SYS_NIOS_CONSOLE;

#if defined(CONFIG_SYS_NIOS_FIXEDBAUD)

/* Everything's already setup for fixed-baud PTF assignment*/

static void oc_serial_setbrg(void)
{
	int n, k;
	const unsigned max_uns = 0xFFFFFFFF;
	unsigned best_n, best_m, baud;

	/* compute best N and M couple */
	best_n = YANU_MAX_PRESCALER_N;
	for (n = YANU_MAX_PRESCALER_N; n >= 0; n--) {
		if ((unsigned)CONFIG_SYS_CLK_FREQ / (1 << (n + 4)) >=
		    (unsigned)CONFIG_BAUDRATE) {
			best_n = n;
			break;
		}
	}
	for (k = 0;; k++) {
		if ((unsigned)CONFIG_BAUDRATE <= (max_uns >> (15+n-k)))
			break;
	}
	best_m =
	    ((unsigned)CONFIG_BAUDRATE * (1 << (15 + n - k))) /
	    ((unsigned)CONFIG_SYS_CLK_FREQ >> k);

	baud = best_m + best_n * YANU_BAUDE;
	writel(baud, &uart->baud);

	return;
}

#else

static void oc_serial_setbrg(void)
{
	int n, k;
	const unsigned max_uns = 0xFFFFFFFF;
	unsigned best_n, best_m, baud;

	/* compute best N and M couple */
	best_n = YANU_MAX_PRESCALER_N;
	for (n = YANU_MAX_PRESCALER_N; n >= 0; n--) {
		if ((unsigned)CONFIG_SYS_CLK_FREQ / (1 << (n + 4)) >=
		    gd->baudrate) {
			best_n = n;
			break;
		}
	}
	for (k = 0;; k++) {
		if (gd->baudrate <= (max_uns >> (15+n-k)))
			break;
	}
	best_m =
	    (gd->baudrate * (1 << (15 + n - k))) /
	    ((unsigned)CONFIG_SYS_CLK_FREQ >> k);

	baud = best_m + best_n * YANU_BAUDE;
	writel(baud, &uart->baud);

	return;
}


#endif /* CONFIG_SYS_NIOS_FIXEDBAUD */

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

statoc int oc_serial_getc(void)
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
