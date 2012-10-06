/*
 * (C) Copyright 2004, Freescale, Inc
 * TsiChung Liew, Tsi-Chung.Liew@freescale.com.
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
 */

/*
 * Minimal serial functions needed to use one of the PSC ports
 * as serial console interface.
 */

#include <common.h>
#include <mpc8220.h>
#include <serial.h>
#include <linux/compiler.h>

DECLARE_GLOBAL_DATA_PTR;

#define PSC_BASE   MMAP_PSC1

#if defined(CONFIG_PSC_CONSOLE)
static int mpc8220_serial_init(void)
{
	volatile psc8220_t *psc = (psc8220_t *) PSC_BASE;
	u32 counter;

	/* write to SICR: SIM2 = uart mode,dcd does not affect rx */
	psc->cr = 0;
	psc->ipcr_acr = 0;
	psc->isr_imr = 0;

	/* write to CSR: RX/TX baud rate from timers */
	psc->sr_csr = 0xdd000000;

	psc->mr1_2 = PSC_MR1_BITS_CHAR_8 | PSC_MR1_NO_PARITY | PSC_MR2_STOP_BITS_1;

	/* Setting up BaudRate */
	counter = ((gd->bus_clk / gd->baudrate)) >> 5;
	counter++;

	/* write to CTUR: divide counter upper byte */
	psc->ctur = ((counter & 0xff00) << 16);
	/* write to CTLR: divide counter lower byte */
	psc->ctlr = ((counter & 0x00ff) << 24);

	psc->cr = PSC_CR_RST_RX_CMD;
	psc->cr = PSC_CR_RST_TX_CMD;
	psc->cr = PSC_CR_RST_ERR_STS_CMD;
	psc->cr = PSC_CR_RST_BRK_INT_CMD;
	psc->cr = PSC_CR_RST_MR_PTR_CMD;

	psc->cr = PSC_CR_RX_ENABLE | PSC_CR_TX_ENABLE;
	return (0);
}

static void mpc8220_serial_putc(const char c)
{
	volatile psc8220_t *psc = (psc8220_t *) PSC_BASE;

	if (c == '\n')
		serial_putc ('\r');

	/* Wait for last character to go. */
	while (!(psc->sr_csr & PSC_SR_TXRDY));

	psc->xmitbuf[0] = c;
}

static int mpc8220_serial_getc(void)
{
	volatile psc8220_t *psc = (psc8220_t *) PSC_BASE;

	/* Wait for a character to arrive. */
	while (!(psc->sr_csr & PSC_SR_RXRDY));
	return psc->xmitbuf[2];
}

static int mpc8220_serial_tstc(void)
{
	volatile psc8220_t *psc = (psc8220_t *) PSC_BASE;

	return (psc->sr_csr & PSC_SR_RXRDY);
}

static void mpc8220_serial_setbrg(void)
{
	volatile psc8220_t *psc = (psc8220_t *) PSC_BASE;
	u32 counter;

	counter = ((gd->bus_clk / gd->baudrate)) >> 5;
	counter++;

	/* write to CTUR: divide counter upper byte */
	psc->ctur = ((counter & 0xff00) << 16);
	/* write to CTLR: divide counter lower byte */
	psc->ctlr = ((counter & 0x00ff) << 24);

	psc->cr = PSC_CR_RST_RX_CMD;
	psc->cr = PSC_CR_RST_TX_CMD;

	psc->cr = PSC_CR_RX_ENABLE | PSC_CR_TX_ENABLE;
}

static struct serial_device mpc8220_serial_drv = {
	.name	= "mpc8220_serial",
	.start	= mpc8220_serial_init,
	.stop	= NULL,
	.setbrg	= mpc8220_serial_setbrg,
	.putc	= mpc8220_serial_putc,
	.puts	= default_serial_puts,
	.getc	= mpc8220_serial_getc,
	.tstc	= mpc8220_serial_tstc,
};

void mpc8220_serial_initialize(void)
{
	serial_register(&mpc8220_serial_drv);
}

__weak struct serial_device *default_serial_console(void)
{
	return &mpc8220_serial_drv;
}
#endif /* CONFIG_PSC_CONSOLE */
