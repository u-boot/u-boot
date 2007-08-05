/*
 * (C) Copyright 2000 - 2007
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
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
 *
 * Based ont the MPC5200 PSC driver.
 * Adapted for MPC512x by Jan Wrobel <wrr@semihalf.com>
 */

/*
 * Minimal serial functions needed to use one of the PSC ports
 * as serial console interface.
 */

#include <common.h>

DECLARE_GLOBAL_DATA_PTR;

#if defined(CONFIG_PSC_CONSOLE)

static void fifo_init (volatile psc512x_t *psc)
{
	volatile immap_t *im = (immap_t *) CFG_IMMR;

	/* reset Rx & Tx fifo slice */
	psc->rfcmd = PSC_FIFO_RESET_SLICE;
	psc->tfcmd = PSC_FIFO_RESET_SLICE;

	/* disable Tx & Rx FIFO interrupts */
	psc->rfintmask = 0;
	psc->tfintmask = 0;

	psc->tfsize = CONSOLE_FIFO_TX_SIZE | (CONSOLE_FIFO_TX_ADDR << 16);
	psc->rfsize = CONSOLE_FIFO_RX_SIZE | (CONSOLE_FIFO_RX_ADDR << 16);

	/* enable Tx & Rx FIFO slice */
	psc->rfcmd = PSC_FIFO_ENABLE_SLICE;
	psc->tfcmd = PSC_FIFO_ENABLE_SLICE;

	im->fifoc.fifoc_cmd = FIFOC_DISABLE_CLOCK_GATE;
	__asm__ volatile ("sync");
}

int serial_init(void)
{
	volatile immap_t *im = (immap_t *) CFG_IMMR;
	volatile psc512x_t *psc = (psc512x_t *) &im->psc[CONFIG_PSC_CONSOLE];
	unsigned long baseclk;
	int div;

	fifo_init (psc);

	/* set MR register to point to MR1 */
	psc->command = PSC_SEL_MODE_REG_1;

	/* disable Tx/Rx */
	psc->command = PSC_TX_DISABLE | PSC_RX_DISABLE;

	/* choose the prescaler	by 16 for the Tx/Rx clock generation */
	psc->psc_clock_select =  0xdd00;

	/* switch to UART mode */
	psc->sicr = 0;

	/* mode register points to mr1 */
	/* configure parity, bit length and so on in mode register 1*/
	psc->mode = PSC_MODE_8_BITS | PSC_MODE_PARNONE;
	/* now, mode register points to mr2 */
	psc->mode = PSC_MODE_1_STOPBIT;

	/* calculate dividor for setting PSC CTUR and CTLR registers */
	baseclk = (gd->ipb_clk + 8) / 16;
	div = (baseclk + (gd->baudrate / 2)) / gd->baudrate;

	psc->ctur = (div >> 8) & 0xff;
	/* set baudrate */
	psc->ctlr = div & 0xff;

	/* disable all interrupts */
	psc->psc_imr = 0;

	/* reset and enable Rx/Tx */
	psc->command = PSC_RST_RX;
	psc->command = PSC_RST_TX;
	psc->command = PSC_RX_ENABLE | PSC_TX_ENABLE;

	return 0;
}

void serial_putc (const char c)
{
	volatile immap_t *im = (immap_t *)CFG_IMMR;
	volatile psc512x_t *psc = (psc512x_t *) &im->psc[CONFIG_PSC_CONSOLE];

	if (c == '\n')
		serial_putc ('\r');

	/* Wait for last character to go. */
	while (!(psc->psc_status & PSC_SR_TXEMP))
		;

	psc->tfdata_8 = c;
}

void serial_putc_raw (const char c)
{
	volatile immap_t *im = (immap_t *) CFG_IMMR;
	volatile psc512x_t *psc = (psc512x_t *) &im->psc[CONFIG_PSC_CONSOLE];

	/* Wait for last character to go. */
	while (!(psc->psc_status & PSC_SR_TXEMP))
		;

	psc->tfdata_8 = c;
}


void serial_puts (const char *s)
{
	while (*s) {
		serial_putc (*s++);
	}
}

int serial_getc (void)
{
	volatile immap_t *im = (immap_t *) CFG_IMMR;
	volatile psc512x_t *psc = (psc512x_t *) &im->psc[CONFIG_PSC_CONSOLE];

	/* Wait for a character to arrive. */
	while (psc->rfstat & PSC_FIFO_EMPTY)
		;

	return psc->rfdata_8;
}

int serial_tstc (void)
{
	volatile immap_t *im = (immap_t *) CFG_IMMR;
	volatile psc512x_t *psc = (psc512x_t *) &im->psc[CONFIG_PSC_CONSOLE];

	return !(psc->rfstat & PSC_FIFO_EMPTY);
}

void serial_setbrg (void)
{
	volatile immap_t *im = (immap_t *) CFG_IMMR;
	volatile psc512x_t *psc = (psc512x_t *) &im->psc[CONFIG_PSC_CONSOLE];
	unsigned long baseclk, div;

	baseclk = (gd->csb_clk + 8) / 16;
	div = (baseclk + (gd->baudrate / 2)) / gd->baudrate;

	psc->ctur = (div >> 8) & 0xFF;
	psc->ctlr =  div & 0xff; /* set baudrate */
}

void serial_setrts(int s)
{
	volatile immap_t *im = (immap_t *) CFG_IMMR;
	volatile psc512x_t *psc = (psc512x_t *) &im->psc[CONFIG_PSC_CONSOLE];

	if (s) {
		/* Assert RTS (become LOW) */
		psc->op1 = 0x1;
	}
	else {
		/* Negate RTS (become HIGH) */
		psc->op0 = 0x1;
	}
}

int serial_getcts(void)
{
	volatile immap_t *im = (immap_t *) CFG_IMMR;
	volatile psc512x_t *psc = (psc512x_t *) &im->psc[CONFIG_PSC_CONSOLE];

	return (psc->ip & 0x1) ? 0 : 1;
}
#endif /* CONFIG_PSC_CONSOLE */
