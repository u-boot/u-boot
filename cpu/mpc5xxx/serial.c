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
 *
 * Martin Krause, 8 Jun 2006
 * Added CONFIG_SERIAL_MULTI support
 */

/*
 * Minimal serial functions needed to use one of the PSC ports
 * as serial console interface.
 */

#include <common.h>
#include <mpc5xxx.h>

#if defined (CONFIG_SERIAL_MULTI)
#include <serial.h>
#endif

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

#if defined(CONFIG_SERIAL_MULTI) && !defined(CONFIG_PSC_CONSOLE2)
#error you must define CONFIG_PSC_CONSOLE2 if CONFIG_SERIAL_MULTI is set
#endif

#if defined(CONFIG_SERIAL_MULTI)
#if CONFIG_PSC_CONSOLE2 == 1
#define PSC_BASE2 MPC5XXX_PSC1
#elif CONFIG_PSC_CONSOLE2 == 2
#define PSC_BASE2 MPC5XXX_PSC2
#elif CONFIG_PSC_CONSOLE2 == 3
#define PSC_BASE2 MPC5XXX_PSC3
#elif defined(CONFIG_MGT5100)
#error CONFIG_PSC_CONSOLE2 must be in 1, 2 or 3
#elif CONFIG_PSC_CONSOLE2 == 4
#define PSC_BASE2 MPC5XXX_PSC4
#elif CONFIG_PSC_CONSOLE2 == 5
#define PSC_BASE2 MPC5XXX_PSC5
#elif CONFIG_PSC_CONSOLE2 == 6
#define PSC_BASE2 MPC5XXX_PSC6
#else
#error CONFIG_PSC_CONSOLE2 must be in 1 ... 6
#endif
#endif /* CONFIG_SERIAL_MULTI */

#if defined(CONFIG_SERIAL_MULTI)
int serial_init_dev (unsigned long dev_base)
#else
int serial_init (void)
#endif
{
#if defined(CONFIG_SERIAL_MULTI)
	volatile struct mpc5xxx_psc *psc = (struct mpc5xxx_psc *)dev_base;
#else
	volatile struct mpc5xxx_psc *psc = (struct mpc5xxx_psc *)PSC_BASE;
#endif
	unsigned long baseclk;
	int div;

	/* reset PSC */
	psc->command = PSC_SEL_MODE_REG_1;

	/* select clock sources */
#if defined(CONFIG_MGT5100)
	psc->psc_clock_select = 0xdd00;
	baseclk = (CONFIG_SYS_MPC5XXX_CLKIN + 16) / 32;
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

#if defined(CONFIG_SERIAL_MULTI)
void serial_putc_dev (unsigned long dev_base, const char c)
#else
void serial_putc(const char c)
#endif
{
#if defined(CONFIG_SERIAL_MULTI)
	volatile struct mpc5xxx_psc *psc = (struct mpc5xxx_psc *)dev_base;
#else
	volatile struct mpc5xxx_psc *psc = (struct mpc5xxx_psc *)PSC_BASE;
#endif

	if (c == '\n')
#if defined(CONFIG_SERIAL_MULTI)
		serial_putc_dev (dev_base, '\r');
#else
		serial_putc('\r');
#endif

	/* Wait for last character to go. */
	while (!(psc->psc_status & PSC_SR_TXEMP))
		;

	psc->psc_buffer_8 = c;
}

#if defined(CONFIG_SERIAL_MULTI)
void serial_putc_raw_dev(unsigned long dev_base, const char c)
#else
void serial_putc_raw(const char c)
#endif
{
#if defined(CONFIG_SERIAL_MULTI)
	volatile struct mpc5xxx_psc *psc = (struct mpc5xxx_psc *)dev_base;
#else
	volatile struct mpc5xxx_psc *psc = (struct mpc5xxx_psc *)PSC_BASE;
#endif
	/* Wait for last character to go. */
	while (!(psc->psc_status & PSC_SR_TXEMP))
		;

	psc->psc_buffer_8 = c;
}


#if defined(CONFIG_SERIAL_MULTI)
void serial_puts_dev (unsigned long dev_base, const char *s)
#else
void serial_puts (const char *s)
#endif
{
	while (*s) {
#if defined(CONFIG_SERIAL_MULTI)
		serial_putc_dev (dev_base, *s++);
#else
		serial_putc (*s++);
#endif
	}
}

#if defined(CONFIG_SERIAL_MULTI)
int serial_getc_dev (unsigned long dev_base)
#else
int serial_getc(void)
#endif
{
#if defined(CONFIG_SERIAL_MULTI)
	volatile struct mpc5xxx_psc *psc = (struct mpc5xxx_psc *)dev_base;
#else
	volatile struct mpc5xxx_psc *psc = (struct mpc5xxx_psc *)PSC_BASE;
#endif

	/* Wait for a character to arrive. */
	while (!(psc->psc_status & PSC_SR_RXRDY))
		;

	return psc->psc_buffer_8;
}

#if defined(CONFIG_SERIAL_MULTI)
int serial_tstc_dev (unsigned long dev_base)
#else
int serial_tstc(void)
#endif
{
#if defined(CONFIG_SERIAL_MULTI)
	volatile struct mpc5xxx_psc *psc = (struct mpc5xxx_psc *)dev_base;
#else
	volatile struct mpc5xxx_psc *psc = (struct mpc5xxx_psc *)PSC_BASE;
#endif

	return (psc->psc_status & PSC_SR_RXRDY);
}

#if defined(CONFIG_SERIAL_MULTI)
void serial_setbrg_dev (unsigned long dev_base)
#else
void serial_setbrg(void)
#endif
{
#if defined(CONFIG_SERIAL_MULTI)
	volatile struct mpc5xxx_psc *psc = (struct mpc5xxx_psc *)dev_base;
#else
	volatile struct mpc5xxx_psc *psc = (struct mpc5xxx_psc *)PSC_BASE;
#endif
	unsigned long baseclk, div;

#if defined(CONFIG_MGT5100)
	baseclk = (CONFIG_SYS_MPC5XXX_CLKIN + 16) / 32;
#elif defined(CONFIG_MPC5200)
	baseclk = (gd->ipb_clk + 16) / 32;
#endif

	/* set up UART divisor */
	div = (baseclk + (gd->baudrate/2)) / gd->baudrate;
	psc->ctur = (div >> 8) & 0xFF;
	psc->ctlr =  div & 0xff;
}

#if defined(CONFIG_SERIAL_MULTI)
void serial_setrts_dev (unsigned long dev_base, int s)
#else
void serial_setrts(int s)
#endif
{
#if defined(CONFIG_SERIAL_MULTI)
	volatile struct mpc5xxx_psc *psc = (struct mpc5xxx_psc *)dev_base;
#else
	volatile struct mpc5xxx_psc *psc = (struct mpc5xxx_psc *)PSC_BASE;
#endif

	if (s) {
		/* Assert RTS (become LOW) */
		psc->op1 = 0x1;
	}
	else {
		/* Negate RTS (become HIGH) */
		psc->op0 = 0x1;
	}
}

#if defined(CONFIG_SERIAL_MULTI)
int serial_getcts_dev (unsigned long dev_base)
#else
int serial_getcts(void)
#endif
{
#if defined(CONFIG_SERIAL_MULTI)
	volatile struct mpc5xxx_psc *psc = (struct mpc5xxx_psc *)dev_base;
#else
	volatile struct mpc5xxx_psc *psc = (struct mpc5xxx_psc *)PSC_BASE;
#endif

	return (psc->ip & 0x1) ? 0 : 1;
}

#if defined(CONFIG_SERIAL_MULTI)
int serial0_init(void)
{
	return (serial_init_dev(PSC_BASE));
}

int serial1_init(void)
{
	return (serial_init_dev(PSC_BASE2));
}
void serial0_setbrg (void)
{
	serial_setbrg_dev(PSC_BASE);
}
void serial1_setbrg (void)
{
	serial_setbrg_dev(PSC_BASE2);
}

void serial0_putc(const char c)
{
	serial_putc_dev(PSC_BASE,c);
}

void serial1_putc(const char c)
{
	serial_putc_dev(PSC_BASE2, c);
}
void serial0_puts(const char *s)
{
	serial_puts_dev(PSC_BASE, s);
}

void serial1_puts(const char *s)
{
	serial_puts_dev(PSC_BASE2, s);
}

int serial0_getc(void)
{
	return(serial_getc_dev(PSC_BASE));
}

int serial1_getc(void)
{
	return(serial_getc_dev(PSC_BASE2));
}
int serial0_tstc(void)
{
	return (serial_tstc_dev(PSC_BASE));
}

int serial1_tstc(void)
{
	return (serial_tstc_dev(PSC_BASE2));
}

struct serial_device serial0_device =
{
	"serial0",
	"UART0",
	serial0_init,
	serial0_setbrg,
	serial0_getc,
	serial0_tstc,
	serial0_putc,
	serial0_puts,
};

struct serial_device serial1_device =
{
	"serial1",
	"UART1",
	serial1_init,
	serial1_setbrg,
	serial1_getc,
	serial1_tstc,
	serial1_putc,
	serial1_puts,
};
#endif /* CONFIG_SERIAL_MULTI */

#endif /* CONFIG_PSC_CONSOLE */
