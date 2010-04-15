/*
 * (C) Copyright 2000 - 2010
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
#include <asm/io.h>
#include <asm/processor.h>

DECLARE_GLOBAL_DATA_PTR;

#if defined(CONFIG_PSC_CONSOLE)

static void fifo_init (volatile psc512x_t *psc)
{
	volatile immap_t *im = (immap_t *) CONFIG_SYS_IMMR;

	/* reset Rx & Tx fifo slice */
	out_be32(&psc->rfcmd, PSC_FIFO_RESET_SLICE);
	out_be32(&psc->tfcmd, PSC_FIFO_RESET_SLICE);

	/* disable Tx & Rx FIFO interrupts */
	out_be32(&psc->rfintmask, 0);
	out_be32(&psc->tfintmask, 0);

	out_be32(&psc->tfsize, CONSOLE_FIFO_TX_SIZE | (CONSOLE_FIFO_TX_ADDR << 16));
	out_be32(&psc->rfsize, CONSOLE_FIFO_RX_SIZE | (CONSOLE_FIFO_RX_ADDR << 16));

	/* enable Tx & Rx FIFO slice */
	out_be32(&psc->rfcmd, PSC_FIFO_ENABLE_SLICE);
	out_be32(&psc->tfcmd, PSC_FIFO_ENABLE_SLICE);

	out_be32(&im->fifoc.fifoc_cmd, FIFOC_DISABLE_CLOCK_GATE);
	__asm__ volatile ("sync");
}

void serial_setbrg(void)
{
	volatile immap_t *im = (immap_t *) CONFIG_SYS_IMMR;
	volatile psc512x_t *psc = (psc512x_t *) &im->psc[CONFIG_PSC_CONSOLE];
	unsigned long baseclk, div;

	/* calculate dividor for setting PSC CTUR and CTLR registers */
	baseclk = (gd->ips_clk + 8) / 16;
	div = (baseclk + (gd->baudrate / 2)) / gd->baudrate;

	out_8(&psc->ctur, (div >> 8) & 0xff);
	out_8(&psc->ctlr,  div & 0xff); /* set baudrate */
}

int serial_init(void)
{
	volatile immap_t *im = (immap_t *) CONFIG_SYS_IMMR;
	volatile psc512x_t *psc = (psc512x_t *) &im->psc[CONFIG_PSC_CONSOLE];

	fifo_init (psc);

	/* set MR register to point to MR1 */
	out_8(&psc->command, PSC_SEL_MODE_REG_1);

	/* disable Tx/Rx */
	out_8(&psc->command, PSC_TX_DISABLE | PSC_RX_DISABLE);

	/* choose the prescaler	by 16 for the Tx/Rx clock generation */
	out_be16(&psc->psc_clock_select, 0xdd00);

	/* switch to UART mode */
	out_be32(&psc->sicr, 0);

	/* mode register points to mr1 */
	/* configure parity, bit length and so on in mode register 1*/
	out_8(&psc->mode, PSC_MODE_8_BITS | PSC_MODE_PARNONE);
	/* now, mode register points to mr2 */
	out_8(&psc->mode, PSC_MODE_1_STOPBIT);

	/* set baudrate */
	serial_setbrg();

	/* disable all interrupts */
	out_be16(&psc->psc_imr, 0);

	/* reset and enable Rx/Tx */
	out_8(&psc->command, PSC_RST_RX);
	out_8(&psc->command, PSC_RST_TX);
	out_8(&psc->command, PSC_RX_ENABLE | PSC_TX_ENABLE);

	return 0;
}

void serial_putc (const char c)
{
	volatile immap_t *im = (immap_t *)CONFIG_SYS_IMMR;
	volatile psc512x_t *psc = (psc512x_t *) &im->psc[CONFIG_PSC_CONSOLE];

	if (c == '\n')
		serial_putc ('\r');

	/* Wait for last character to go. */
	while (!(in_be16(&psc->psc_status) & PSC_SR_TXEMP))
		;

	out_8(&psc->tfdata_8, c);
}

void serial_putc_raw (const char c)
{
	volatile immap_t *im = (immap_t *) CONFIG_SYS_IMMR;
	volatile psc512x_t *psc = (psc512x_t *) &im->psc[CONFIG_PSC_CONSOLE];

	/* Wait for last character to go. */
	while (!(in_be16(&psc->psc_status) & PSC_SR_TXEMP))
		;

	out_8(&psc->tfdata_8, c);
}


void serial_puts (const char *s)
{
	while (*s) {
		serial_putc (*s++);
	}
}

int serial_getc (void)
{
	volatile immap_t *im = (immap_t *) CONFIG_SYS_IMMR;
	volatile psc512x_t *psc = (psc512x_t *) &im->psc[CONFIG_PSC_CONSOLE];

	/* Wait for a character to arrive. */
	while (in_be32(&psc->rfstat) & PSC_FIFO_EMPTY)
		;

	return in_8(&psc->rfdata_8);
}

int serial_tstc (void)
{
	volatile immap_t *im = (immap_t *) CONFIG_SYS_IMMR;
	volatile psc512x_t *psc = (psc512x_t *) &im->psc[CONFIG_PSC_CONSOLE];

	return !(in_be32(&psc->rfstat) & PSC_FIFO_EMPTY);
}

void serial_setrts(int s)
{
	volatile immap_t *im = (immap_t *) CONFIG_SYS_IMMR;
	volatile psc512x_t *psc = (psc512x_t *) &im->psc[CONFIG_PSC_CONSOLE];

	if (s) {
		/* Assert RTS (become LOW) */
		out_8(&psc->op1, 0x1);
	}
	else {
		/* Negate RTS (become HIGH) */
		out_8(&psc->op0, 0x1);
	}
}

int serial_getcts(void)
{
	volatile immap_t *im = (immap_t *) CONFIG_SYS_IMMR;
	volatile psc512x_t *psc = (psc512x_t *) &im->psc[CONFIG_PSC_CONSOLE];

	return (in_8(&psc->ip) & 0x1) ? 0 : 1;
}
#endif /* CONFIG_PSC_CONSOLE */
