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
#include <serial.h>

DECLARE_GLOBAL_DATA_PTR;

#if defined(CONFIG_PSC_CONSOLE) || defined(CONFIG_SERIAL_MULTI)

static void fifo_init (volatile psc512x_t *psc)
{
	volatile immap_t *im = (immap_t *) CONFIG_SYS_IMMR;
	u32 tfsize, rfsize;

	/* reset Rx & Tx fifo slice */
	out_be32(&psc->rfcmd, PSC_FIFO_RESET_SLICE);
	out_be32(&psc->tfcmd, PSC_FIFO_RESET_SLICE);

	/* disable Tx & Rx FIFO interrupts */
	out_be32(&psc->rfintmask, 0);
	out_be32(&psc->tfintmask, 0);

#if defined(CONFIG_SERIAL_MULTI)
	switch (((u32)psc & 0xf00) >> 8) {
	case 0:
		tfsize = FIFOC_PSC0_TX_SIZE | (FIFOC_PSC0_TX_ADDR << 16);
		rfsize = FIFOC_PSC0_RX_SIZE | (FIFOC_PSC0_RX_ADDR << 16);
		break;
	case 1:
		tfsize = FIFOC_PSC1_TX_SIZE | (FIFOC_PSC1_TX_ADDR << 16);
		rfsize = FIFOC_PSC1_RX_SIZE | (FIFOC_PSC1_RX_ADDR << 16);
		break;
	case 2:
		tfsize = FIFOC_PSC2_TX_SIZE | (FIFOC_PSC2_TX_ADDR << 16);
		rfsize = FIFOC_PSC2_RX_SIZE | (FIFOC_PSC2_RX_ADDR << 16);
		break;
	case 3:
		tfsize = FIFOC_PSC3_TX_SIZE | (FIFOC_PSC3_TX_ADDR << 16);
		rfsize = FIFOC_PSC3_RX_SIZE | (FIFOC_PSC3_RX_ADDR << 16);
		break;
	case 4:
		tfsize = FIFOC_PSC4_TX_SIZE | (FIFOC_PSC4_TX_ADDR << 16);
		rfsize = FIFOC_PSC4_RX_SIZE | (FIFOC_PSC4_RX_ADDR << 16);
		break;
	case 5:
		tfsize = FIFOC_PSC5_TX_SIZE | (FIFOC_PSC5_TX_ADDR << 16);
		rfsize = FIFOC_PSC5_RX_SIZE | (FIFOC_PSC5_RX_ADDR << 16);
		break;
	case 6:
		tfsize = FIFOC_PSC6_TX_SIZE | (FIFOC_PSC6_TX_ADDR << 16);
		rfsize = FIFOC_PSC6_RX_SIZE | (FIFOC_PSC6_RX_ADDR << 16);
		break;
	case 7:
		tfsize = FIFOC_PSC7_TX_SIZE | (FIFOC_PSC7_TX_ADDR << 16);
		rfsize = FIFOC_PSC7_RX_SIZE | (FIFOC_PSC7_RX_ADDR << 16);
		break;
	case 8:
		tfsize = FIFOC_PSC8_TX_SIZE | (FIFOC_PSC8_TX_ADDR << 16);
		rfsize = FIFOC_PSC8_RX_SIZE | (FIFOC_PSC8_RX_ADDR << 16);
		break;
	case 9:
		tfsize = FIFOC_PSC9_TX_SIZE | (FIFOC_PSC9_TX_ADDR << 16);
		rfsize = FIFOC_PSC9_RX_SIZE | (FIFOC_PSC9_RX_ADDR << 16);
		break;
	case 10:
		tfsize = FIFOC_PSC10_TX_SIZE | (FIFOC_PSC10_TX_ADDR << 16);
		rfsize = FIFOC_PSC10_RX_SIZE | (FIFOC_PSC10_RX_ADDR << 16);
		break;
	case 11:
		tfsize = FIFOC_PSC11_TX_SIZE | (FIFOC_PSC11_TX_ADDR << 16);
		rfsize = FIFOC_PSC11_RX_SIZE | (FIFOC_PSC11_RX_ADDR << 16);
		break;
	default:
		return;
	}
#else
	tfsize = CONSOLE_FIFO_TX_SIZE | (CONSOLE_FIFO_TX_ADDR << 16);
	rfsize = CONSOLE_FIFO_RX_SIZE | (CONSOLE_FIFO_RX_ADDR << 16);
#endif
	out_be32(&psc->tfsize, tfsize);
	out_be32(&psc->rfsize, rfsize);

	/* enable Tx & Rx FIFO slice */
	out_be32(&psc->rfcmd, PSC_FIFO_ENABLE_SLICE);
	out_be32(&psc->tfcmd, PSC_FIFO_ENABLE_SLICE);

	out_be32(&im->fifoc.fifoc_cmd, FIFOC_DISABLE_CLOCK_GATE);
	__asm__ volatile ("sync");
}

void serial_setbrg_dev(unsigned int idx)
{
	volatile immap_t *im = (immap_t *) CONFIG_SYS_IMMR;
	volatile psc512x_t *psc = (psc512x_t *) &im->psc[idx];
	unsigned long baseclk, div;
	unsigned long baudrate;
	char buf[16];
	char *br_env;

	baudrate = gd->baudrate;
	if (idx != CONFIG_PSC_CONSOLE) {
		/* Allows setting baudrate for other serial devices
		 * on PSCx using environment. If not specified, use
		 * the same baudrate as for console.
		 */
		sprintf(buf, "psc%d_baudrate", idx);
		br_env = getenv(buf);
		if (br_env)
			baudrate = simple_strtoul(br_env, NULL, 10);

		debug("%s: idx %d, baudrate %d\n", __func__, idx, baudrate);
	}

	/* calculate divisor for setting PSC CTUR and CTLR registers */
	baseclk = (gd->ips_clk + 8) / 16;
	div = (baseclk + (baudrate / 2)) / baudrate;

	out_8(&psc->ctur, (div >> 8) & 0xff);
	out_8(&psc->ctlr,  div & 0xff); /* set baudrate */
}

int serial_init_dev(unsigned int idx)
{
	volatile immap_t *im = (immap_t *) CONFIG_SYS_IMMR;
	volatile psc512x_t *psc = (psc512x_t *) &im->psc[idx];
#if defined(CONFIG_SERIAL_MULTI)
	u32 reg;

	reg = in_be32(&im->clk.sccr[0]);
	out_be32(&im->clk.sccr[0], reg | CLOCK_SCCR1_PSC_EN(idx));
#endif

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
	serial_setbrg_dev(idx);

	/* disable all interrupts */
	out_be16(&psc->psc_imr, 0);

	/* reset and enable Rx/Tx */
	out_8(&psc->command, PSC_RST_RX);
	out_8(&psc->command, PSC_RST_TX);
	out_8(&psc->command, PSC_RX_ENABLE | PSC_TX_ENABLE);

	return 0;
}

int serial_uninit_dev(unsigned int idx)
{
	volatile immap_t *im = (immap_t *) CONFIG_SYS_IMMR;
	volatile psc512x_t *psc = (psc512x_t *) &im->psc[idx];
	u32 reg;

	out_8(&psc->command, PSC_RX_DISABLE | PSC_TX_DISABLE);
	reg = in_be32(&im->clk.sccr[0]);
	reg &= ~CLOCK_SCCR1_PSC_EN(idx);
	out_be32(&im->clk.sccr[0], reg);

	return 0;
}

void serial_putc_dev(unsigned int idx, const char c)
{
	volatile immap_t *im = (immap_t *)CONFIG_SYS_IMMR;
	volatile psc512x_t *psc = (psc512x_t *) &im->psc[idx];

	if (c == '\n')
		serial_putc_dev(idx, '\r');

	/* Wait for last character to go. */
	while (!(in_be16(&psc->psc_status) & PSC_SR_TXEMP))
		;

	out_8(&psc->tfdata_8, c);
}

void serial_putc_raw_dev(unsigned int idx, const char c)
{
	volatile immap_t *im = (immap_t *) CONFIG_SYS_IMMR;
	volatile psc512x_t *psc = (psc512x_t *) &im->psc[idx];

	/* Wait for last character to go. */
	while (!(in_be16(&psc->psc_status) & PSC_SR_TXEMP))
		;

	out_8(&psc->tfdata_8, c);
}

void serial_puts_dev(unsigned int idx, const char *s)
{
	while (*s)
		serial_putc_dev(idx, *s++);
}

int serial_getc_dev(unsigned int idx)
{
	volatile immap_t *im = (immap_t *) CONFIG_SYS_IMMR;
	volatile psc512x_t *psc = (psc512x_t *) &im->psc[idx];

	/* Wait for a character to arrive. */
	while (in_be32(&psc->rfstat) & PSC_FIFO_EMPTY)
		;

	return in_8(&psc->rfdata_8);
}

int serial_tstc_dev(unsigned int idx)
{
	volatile immap_t *im = (immap_t *) CONFIG_SYS_IMMR;
	volatile psc512x_t *psc = (psc512x_t *) &im->psc[idx];

	return !(in_be32(&psc->rfstat) & PSC_FIFO_EMPTY);
}

void serial_setrts_dev(unsigned int idx, int s)
{
	volatile immap_t *im = (immap_t *) CONFIG_SYS_IMMR;
	volatile psc512x_t *psc = (psc512x_t *) &im->psc[idx];

	if (s) {
		/* Assert RTS (become LOW) */
		out_8(&psc->op1, 0x1);
	}
	else {
		/* Negate RTS (become HIGH) */
		out_8(&psc->op0, 0x1);
	}
}

int serial_getcts_dev(unsigned int idx)
{
	volatile immap_t *im = (immap_t *) CONFIG_SYS_IMMR;
	volatile psc512x_t *psc = (psc512x_t *) &im->psc[idx];

	return (in_8(&psc->ip) & 0x1) ? 0 : 1;
}
#endif /* CONFIG_PSC_CONSOLE || CONFIG_SERIAL_MULTI */

#if defined(CONFIG_SERIAL_MULTI)

#define DECLARE_PSC_SERIAL_FUNCTIONS(port) \
	int serial##port##_init(void) \
	{ \
		return serial_init_dev(port); \
	} \
	int serial##port##_uninit(void) \
	{ \
		return serial_uninit_dev(port); \
	} \
	void serial##port##_setbrg(void) \
	{ \
		serial_setbrg_dev(port); \
	} \
	int serial##port##_getc(void) \
	{ \
		return serial_getc_dev(port); \
	} \
	int serial##port##_tstc(void) \
	{ \
		return serial_tstc_dev(port); \
	} \
	void serial##port##_putc(const char c) \
	{ \
		serial_putc_dev(port, c); \
	} \
	void serial##port##_puts(const char *s) \
	{ \
		serial_puts_dev(port, s); \
	}

#define INIT_PSC_SERIAL_STRUCTURE(port, name, bus) { \
	name, \
	bus, \
	serial##port##_init, \
	serial##port##_uninit, \
	serial##port##_setbrg, \
	serial##port##_getc, \
	serial##port##_tstc, \
	serial##port##_putc, \
	serial##port##_puts, \
}

#if defined(CONFIG_SYS_PSC1)
DECLARE_PSC_SERIAL_FUNCTIONS(1);
struct serial_device serial1_device =
INIT_PSC_SERIAL_STRUCTURE(1, "psc1", "UART1");
#endif

#if defined(CONFIG_SYS_PSC3)
DECLARE_PSC_SERIAL_FUNCTIONS(3);
struct serial_device serial3_device =
INIT_PSC_SERIAL_STRUCTURE(3, "psc3", "UART3");
#endif

#if defined(CONFIG_SYS_PSC4)
DECLARE_PSC_SERIAL_FUNCTIONS(4);
struct serial_device serial4_device =
INIT_PSC_SERIAL_STRUCTURE(4, "psc4", "UART4");
#endif

#if defined(CONFIG_SYS_PSC6)
DECLARE_PSC_SERIAL_FUNCTIONS(6);
struct serial_device serial6_device =
INIT_PSC_SERIAL_STRUCTURE(6, "psc6", "UART6");
#endif

#else

void serial_setbrg(void)
{
	serial_setbrg_dev(CONFIG_PSC_CONSOLE);
}

int serial_init(void)
{
	return serial_init_dev(CONFIG_PSC_CONSOLE);
}

void serial_putc(const char c)
{
	serial_putc_dev(CONFIG_PSC_CONSOLE, c);
}

void serial_putc_raw(const char c)
{
	serial_putc_raw_dev(CONFIG_PSC_CONSOLE, c);
}

void serial_puts(const char *s)
{
	serial_puts_dev(CONFIG_PSC_CONSOLE, s);
}

int serial_getc(void)
{
	return serial_getc_dev(CONFIG_PSC_CONSOLE);
}

int serial_tstc(void)
{
	return serial_tstc_dev(CONFIG_PSC_CONSOLE);
}

void serial_setrts(int s)
{
	return serial_setrts_dev(CONFIG_PSC_CONSOLE, s);
}

int serial_getcts(void)
{
	return serial_getcts_dev(CONFIG_PSC_CONSOLE);
}
#endif /* CONFIG_PSC_CONSOLE */

#if defined(CONFIG_SERIAL_MULTI)
#include <stdio_dev.h>
/*
 * Routines for communication with serial devices over PSC
 */
/* Bitfield for initialized PSCs */
static unsigned int initialized;

struct stdio_dev *open_port(int num, int baudrate)
{
	struct stdio_dev *port;
	char env_var[16];
	char env_val[10];
	char name[7];

	if (num < 0 || num > 11)
		return NULL;

	sprintf(name, "psc%d", num);
	port = stdio_get_by_name(name);
	if (!port)
		return NULL;

	if (!test_bit(num, &initialized)) {
		sprintf(env_var, "psc%d_baudrate", num);
		sprintf(env_val, "%d", baudrate);
		setenv(env_var, env_val);

		if (port->start())
			return NULL;

		set_bit(num, &initialized);
	}

	return port;
}

int close_port(int num)
{
	struct stdio_dev *port;
	int ret;
	char name[7];

	if (num < 0 || num > 11)
		return -1;

	sprintf(name, "psc%d", num);
	port = stdio_get_by_name(name);
	if (!port)
		return -1;

	ret = port->stop();
	clear_bit(num, &initialized);

	return ret;
}

int write_port(struct stdio_dev *port, char *buf)
{
	if (!port || !buf)
		return -1;

	port->puts(buf);

	return 0;
}

int read_port(struct stdio_dev *port, char *buf, int size)
{
	int cnt = 0;

	if (!port || !buf)
		return -1;

	if (!size)
		return 0;

	while (port->tstc()) {
		buf[cnt++] = port->getc();
		if (cnt > size)
			break;
	}

	return cnt;
}
#endif /* CONFIG_SERIAL_MULTI */
