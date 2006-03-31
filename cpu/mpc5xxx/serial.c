/*
 * (C) Copyright 2000 - 2003
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
 * Hacked for MPC8260 by Murray.Jensen@cmst.csiro.au, 19-Oct-00, with
 * changes based on the file arch/ppc/mbxboot/m8260_tty.c from the
 * Linux/PPC sources (m8260_tty.c had no copyright info in it).
 */

/*
 * Minimal serial functions needed to use one of the PSC ports
 * as serial console interface.
 */

#include <common.h>
#include <mpc5xxx.h>

DECLARE_GLOBAL_DATA_PTR;

#if defined(CONFIG_PSC_CONSOLE)

#if CONFIG_PSC_CONSOLE == 1
#define PSC_BASE MPC5XXX_PSC1
#elif CONFIG_PSC_CONSOLE == 2
#define PSC_BASE MPC5XXX_PSC2
#elif CONFIG_PSC_CONSOLE == 3
#define PSC_BASE MPC5XXX_PSC3
#elif defined(CONFIG_MGT5100)
#error CONFIG_PSC_CONSOLE must be in 1, 2 or 3
#elif CONFIG_PSC_CONSOLE == 4
#define PSC_BASE MPC5XXX_PSC4
#elif CONFIG_PSC_CONSOLE == 5
#define PSC_BASE MPC5XXX_PSC5
#elif CONFIG_PSC_CONSOLE == 6
#define PSC_BASE MPC5XXX_PSC6
#else
#error CONFIG_PSC_CONSOLE must be in 1 ... 6
#endif

int serial_init (void)
{
	volatile struct mpc5xxx_psc *psc = (struct mpc5xxx_psc *)PSC_BASE;
	unsigned long baseclk;
	int div;

	/* reset PSC */
	psc->command = PSC_SEL_MODE_REG_1;

	/* select clock sources */
#if defined(CONFIG_MGT5100)
	psc->psc_clock_select = 0xdd00;
	baseclk = (CFG_MPC5XXX_CLKIN + 16) / 32;
#elif defined(CONFIG_MPC5200)
	psc->psc_clock_select = 0;
	baseclk = (gd->ipb_clk + 16) / 32;
#endif

	/* switch to UART mode */
	psc->sicr = 0;

	/* configure parity, bit length and so on */
#if defined(CONFIG_MGT5100)
	psc->mode = PSC_MODE_ERR | PSC_MODE_8_BITS | PSC_MODE_PARNONE;
#elif defined(CONFIG_MPC5200)
	psc->mode = PSC_MODE_8_BITS | PSC_MODE_PARNONE;
#endif
	psc->mode = PSC_MODE_ONE_STOP;

	/* set up UART divisor */
	div = (baseclk + (gd->baudrate/2)) / gd->baudrate;
	psc->ctur = (div >> 8) & 0xff;
	psc->ctlr = div & 0xff;

	/* disable all interrupts */
	psc->psc_imr = 0;

	/* reset and enable Rx/Tx */
	psc->command = PSC_RST_RX;
	psc->command = PSC_RST_TX;
	psc->command = PSC_RX_ENABLE | PSC_TX_ENABLE;

	return (0);
}

void
serial_putc(const char c)
{
	volatile struct mpc5xxx_psc *psc = (struct mpc5xxx_psc *)PSC_BASE;

	if (c == '\n')
		serial_putc('\r');

	/* Wait for last character to go. */
	while (!(psc->psc_status & PSC_SR_TXEMP))
		;

	psc->psc_buffer_8 = c;
}

void
serial_puts (const char *s)
{
	while (*s) {
		serial_putc (*s++);
	}
}

int
serial_getc(void)
{
	volatile struct mpc5xxx_psc *psc = (struct mpc5xxx_psc *)PSC_BASE;

	/* Wait for a character to arrive. */
	while (!(psc->psc_status & PSC_SR_RXRDY))
		;

	return psc->psc_buffer_8;
}

int
serial_tstc(void)
{
	volatile struct mpc5xxx_psc *psc = (struct mpc5xxx_psc *)PSC_BASE;

	return (psc->psc_status & PSC_SR_RXRDY);
}

void
serial_setbrg(void)
{
	volatile struct mpc5xxx_psc *psc = (struct mpc5xxx_psc *)PSC_BASE;
	unsigned long baseclk, div;

#if defined(CONFIG_MGT5100)
	baseclk = (CFG_MPC5XXX_CLKIN + 16) / 32;
#elif defined(CONFIG_MPC5200)
	baseclk = (gd->ipb_clk + 16) / 32;
#endif

	/* set up UART divisor */
	div = (baseclk + (gd->baudrate/2)) / gd->baudrate;
	psc->ctur = (div >> 8) & 0xFF;
	psc->ctlr =  div & 0xff;
}
#endif /* CONFIG_PSC_CONSOLE */
