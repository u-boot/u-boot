/*
 * (C) Copyright 2000
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
 * Hacked for MPC8260 by Murray.Jensen@cmst.csiro.au, 19-Oct-00.
 */

/*
 * Minimal serial functions needed to use one of the SCC ports
 * as serial console interface.
 */

#include <common.h>
#include <mpc8260.h>
#include <asm/cpm_8260.h>
#include <serial.h>
#include <linux/compiler.h>

DECLARE_GLOBAL_DATA_PTR;

#if defined(CONFIG_CONS_ON_SCC)

#if CONFIG_CONS_INDEX == 1	/* Console on SCC1 */

#define SCC_INDEX		0
#define PROFF_SCC		PROFF_SCC1
#define CMXSCR_MASK		(CMXSCR_GR1|CMXSCR_SC1|\
					CMXSCR_RS1CS_MSK|CMXSCR_TS1CS_MSK)
#define CMXSCR_VALUE		(CMXSCR_RS1CS_BRG1|CMXSCR_TS1CS_BRG1)
#define CPM_CR_SCC_PAGE		CPM_CR_SCC1_PAGE
#define CPM_CR_SCC_SBLOCK	CPM_CR_SCC1_SBLOCK

#elif CONFIG_CONS_INDEX == 2	/* Console on SCC2 */

#define SCC_INDEX		1
#define PROFF_SCC		PROFF_SCC2
#define CMXSCR_MASK		(CMXSCR_GR2|CMXSCR_SC2|\
					CMXSCR_RS2CS_MSK|CMXSCR_TS2CS_MSK)
#define CMXSCR_VALUE		(CMXSCR_RS2CS_BRG2|CMXSCR_TS2CS_BRG2)
#define CPM_CR_SCC_PAGE		CPM_CR_SCC2_PAGE
#define CPM_CR_SCC_SBLOCK	CPM_CR_SCC2_SBLOCK

#elif CONFIG_CONS_INDEX == 3	/* Console on SCC3 */

#define SCC_INDEX		2
#define PROFF_SCC		PROFF_SCC3
#define CMXSCR_MASK		(CMXSCR_GR3|CMXSCR_SC3|\
					CMXSCR_RS3CS_MSK|CMXSCR_TS3CS_MSK)
#define CMXSCR_VALUE		(CMXSCR_RS3CS_BRG3|CMXSCR_TS3CS_BRG3)
#define CPM_CR_SCC_PAGE		CPM_CR_SCC3_PAGE
#define CPM_CR_SCC_SBLOCK	CPM_CR_SCC3_SBLOCK

#elif CONFIG_CONS_INDEX == 4	/* Console on SCC4 */

#define SCC_INDEX		3
#define PROFF_SCC		PROFF_SCC4
#define CMXSCR_MASK		(CMXSCR_GR4|CMXSCR_SC4|\
					CMXSCR_RS4CS_MSK|CMXSCR_TS4CS_MSK)
#define CMXSCR_VALUE		(CMXSCR_RS4CS_BRG4|CMXSCR_TS4CS_BRG4)
#define CPM_CR_SCC_PAGE		CPM_CR_SCC4_PAGE
#define CPM_CR_SCC_SBLOCK	CPM_CR_SCC4_SBLOCK

#else

#error "console not correctly defined"

#endif

static int mpc8260_scc_serial_init(void)
{
	volatile immap_t *im = (immap_t *)CONFIG_SYS_IMMR;
	volatile scc_t *sp;
	volatile scc_uart_t *up;
	volatile cbd_t *tbdf, *rbdf;
	volatile cpm8260_t *cp = &(im->im_cpm);
	uint	dpaddr;

	/* initialize pointers to SCC */

	sp = (scc_t *) &(im->im_scc[SCC_INDEX]);
	up = (scc_uart_t *)&im->im_dprambase[PROFF_SCC];

	/* Disable transmitter/receiver.
	*/
	sp->scc_gsmrl &= ~(SCC_GSMRL_ENR | SCC_GSMRL_ENT);

	/* put the SCC channel into NMSI (non multiplexd serial interface)
	 * mode and wire the selected SCC Tx and Rx clocks to BRGx (15-15).
	 */
	im->im_cpmux.cmx_scr = (im->im_cpmux.cmx_scr&~CMXSCR_MASK)|CMXSCR_VALUE;

	/* Set up the baud rate generator.
	*/
	serial_setbrg ();

	/* Allocate space for two buffer descriptors in the DP ram.
	 * damm: allocating space after the two buffers for rx/tx data
	 */

	dpaddr = m8260_cpm_dpalloc((2 * sizeof (cbd_t)) + 2, 16);

	/* Set the physical address of the host memory buffers in
	 * the buffer descriptors.
	 */
	rbdf = (cbd_t *)&im->im_dprambase[dpaddr];
	rbdf->cbd_bufaddr = (uint) (rbdf+2);
	rbdf->cbd_sc = BD_SC_EMPTY | BD_SC_WRAP;
	tbdf = rbdf + 1;
	tbdf->cbd_bufaddr = ((uint) (rbdf+2)) + 1;
	tbdf->cbd_sc = BD_SC_WRAP;

	/* Set up the uart parameters in the parameter ram.
	*/
	up->scc_genscc.scc_rbase = dpaddr;
	up->scc_genscc.scc_tbase = dpaddr+sizeof(cbd_t);
	up->scc_genscc.scc_rfcr = CPMFCR_EB;
	up->scc_genscc.scc_tfcr = CPMFCR_EB;
	up->scc_genscc.scc_mrblr = 1;
	up->scc_maxidl = 0;
	up->scc_brkcr = 1;
	up->scc_parec = 0;
	up->scc_frmec = 0;
	up->scc_nosec = 0;
	up->scc_brkec = 0;
	up->scc_uaddr1 = 0;
	up->scc_uaddr2 = 0;
	up->scc_toseq = 0;
	up->scc_char1 = up->scc_char2 = up->scc_char3 = up->scc_char4 = 0x8000;
	up->scc_char5 = up->scc_char6 = up->scc_char7 = up->scc_char8 = 0x8000;
	up->scc_rccm = 0xc0ff;

	/* Mask all interrupts and remove anything pending.
	*/
	sp->scc_sccm = 0;
	sp->scc_scce = 0xffff;

	/* Set 8 bit FIFO, 16 bit oversampling and UART mode.
	*/
	sp->scc_gsmrh = SCC_GSMRH_RFW;	/* 8 bit FIFO */
	sp->scc_gsmrl = \
		SCC_GSMRL_TDCR_16 | SCC_GSMRL_RDCR_16 | SCC_GSMRL_MODE_UART;

	/* Set CTS flow control, 1 stop bit, 8 bit character length,
	 * normal async UART mode, no parity
	 */
	sp->scc_psmr = SCU_PSMR_FLC | SCU_PSMR_CL;

	/* execute the "Init Rx and Tx params" CP command.
	*/

	while (cp->cp_cpcr & CPM_CR_FLG)  /* wait if cp is busy */
	  ;

	cp->cp_cpcr = mk_cr_cmd(CPM_CR_SCC_PAGE, CPM_CR_SCC_SBLOCK,
					0, CPM_CR_INIT_TRX) | CPM_CR_FLG;

	while (cp->cp_cpcr & CPM_CR_FLG)  /* wait if cp is busy */
	  ;

	/* Enable transmitter/receiver.
	*/
	sp->scc_gsmrl |= SCC_GSMRL_ENR | SCC_GSMRL_ENT;

	return (0);
}

static void mpc8260_scc_serial_setbrg(void)
{
#if defined(CONFIG_CONS_USE_EXTC)
	m8260_cpm_extcbrg(SCC_INDEX, gd->baudrate,
		CONFIG_CONS_EXTC_RATE, CONFIG_CONS_EXTC_PINSEL);
#else
	m8260_cpm_setbrg(SCC_INDEX, gd->baudrate);
#endif
}

static void mpc8260_scc_serial_putc(const char c)
{
	volatile scc_uart_t	*up;
	volatile cbd_t		*tbdf;
	volatile immap_t	*im;

	if (c == '\n')
		serial_putc ('\r');

	im = (immap_t *)CONFIG_SYS_IMMR;
	up = (scc_uart_t *)&im->im_dprambase[PROFF_SCC];
	tbdf = (cbd_t *)&im->im_dprambase[up->scc_genscc.scc_tbase];

	/* Wait for last character to go.
	 */
	while (tbdf->cbd_sc & BD_SC_READY)
		;

	/* Load the character into the transmit buffer.
	 */
	*(volatile char *)tbdf->cbd_bufaddr = c;
	tbdf->cbd_datlen = 1;
	tbdf->cbd_sc |= BD_SC_READY;
}

static int mpc8260_scc_serial_getc(void)
{
	volatile cbd_t		*rbdf;
	volatile scc_uart_t	*up;
	volatile immap_t	*im;
	unsigned char		c;

	im = (immap_t *)CONFIG_SYS_IMMR;
	up = (scc_uart_t *)&im->im_dprambase[PROFF_SCC];
	rbdf = (cbd_t *)&im->im_dprambase[up->scc_genscc.scc_rbase];

	/* Wait for character to show up.
	 */
	while (rbdf->cbd_sc & BD_SC_EMPTY)
		;

	/* Grab the char and clear the buffer again.
	 */
	c = *(volatile unsigned char *)rbdf->cbd_bufaddr;
	rbdf->cbd_sc |= BD_SC_EMPTY;

	return (c);
}

static int mpc8260_scc_serial_tstc(void)
{
	volatile cbd_t		*rbdf;
	volatile scc_uart_t	*up;
	volatile immap_t	*im;

	im = (immap_t *)CONFIG_SYS_IMMR;
	up = (scc_uart_t *)&im->im_dprambase[PROFF_SCC];
	rbdf = (cbd_t *)&im->im_dprambase[up->scc_genscc.scc_rbase];

	return ((rbdf->cbd_sc & BD_SC_EMPTY) == 0);
}

static struct serial_device mpc8260_scc_serial_drv = {
	.name	= "mpc8260_scc_uart",
	.start	= mpc8260_scc_serial_init,
	.stop	= NULL,
	.setbrg	= mpc8260_scc_serial_setbrg,
	.putc	= mpc8260_scc_serial_putc,
	.puts	= default_serial_puts,
	.getc	= mpc8260_scc_serial_getc,
	.tstc	= mpc8260_scc_serial_tstc,
};

void mpc8260_scc_serial_initialize(void)
{
	serial_register(&mpc8260_scc_serial_drv);
}

__weak struct serial_device *default_serial_console(void)
{
	return &mpc8260_scc_serial_drv;
}
#endif	/* CONFIG_CONS_ON_SCC */

#if defined(CONFIG_KGDB_ON_SCC)

#if defined(CONFIG_CONS_ON_SCC) && CONFIG_KGDB_INDEX == CONFIG_CONS_INDEX
#error Whoops! serial console and kgdb are on the same scc serial port
#endif

#if CONFIG_KGDB_INDEX == 1	/* KGDB Port on SCC1 */

#define KGDB_SCC_INDEX		0
#define KGDB_PROFF_SCC		PROFF_SCC1
#define KGDB_CMXSCR_MASK	(CMXSCR_GR1|CMXSCR_SC1|\
					CMXSCR_RS1CS_MSK|CMXSCR_TS1CS_MSK)
#define KGDB_CMXSCR_VALUE	(CMXSCR_RS1CS_BRG1|CMXSCR_TS1CS_BRG1)
#define KGDB_CPM_CR_SCC_PAGE	CPM_CR_SCC1_PAGE
#define KGDB_CPM_CR_SCC_SBLOCK	CPM_CR_SCC1_SBLOCK

#elif CONFIG_KGDB_INDEX == 2	/* KGDB Port on SCC2 */

#define KGDB_SCC_INDEX		1
#define KGDB_PROFF_SCC		PROFF_SCC2
#define KGDB_CMXSCR_MASK	(CMXSCR_GR2|CMXSCR_SC2|\
					CMXSCR_RS2CS_MSK|CMXSCR_TS2CS_MSK)
#define KGDB_CMXSCR_VALUE	(CMXSCR_RS2CS_BRG2|CMXSCR_TS2CS_BRG2)
#define KGDB_CPM_CR_SCC_PAGE	CPM_CR_SCC2_PAGE
#define KGDB_CPM_CR_SCC_SBLOCK	CPM_CR_SCC2_SBLOCK

#elif CONFIG_KGDB_INDEX == 3	/* KGDB Port on SCC3 */

#define KGDB_SCC_INDEX		2
#define KGDB_PROFF_SCC		PROFF_SCC3
#define KGDB_CMXSCR_MASK	(CMXSCR_GR3|CMXSCR_SC3|\
					CMXSCR_RS3CS_MSK|CMXSCR_TS3CS_MSK)
#define KGDB_CMXSCR_VALUE	(CMXSCR_RS3CS_BRG3|CMXSCR_TS3CS_BRG3)
#define KGDB_CPM_CR_SCC_PAGE	CPM_CR_SCC3_PAGE
#define KGDB_CPM_CR_SCC_SBLOCK	CPM_CR_SCC3_SBLOCK

#elif CONFIG_KGDB_INDEX == 4	/* KGDB Port on SCC4 */

#define KGDB_SCC_INDEX		3
#define KGDB_PROFF_SCC		PROFF_SCC4
#define KGDB_CMXSCR_MASK	(CMXSCR_GR4|CMXSCR_SC4|\
					CMXSCR_RS4CS_MSK|CMXSCR_TS4CS_MSK)
#define KGDB_CMXSCR_VALUE	(CMXSCR_RS4CS_BRG4|CMXSCR_TS4CS_BRG4)
#define KGDB_CPM_CR_SCC_PAGE	CPM_CR_SCC4_PAGE
#define KGDB_CPM_CR_SCC_SBLOCK	CPM_CR_SCC4_SBLOCK

#else

#error "kgdb serial port not correctly defined"

#endif

void
kgdb_serial_init (void)
{
	volatile immap_t *im = (immap_t *)CONFIG_SYS_IMMR;
	volatile scc_t *sp;
	volatile scc_uart_t *up;
	volatile cbd_t *tbdf, *rbdf;
	volatile cpm8260_t *cp = &(im->im_cpm);
	uint dpaddr, speed = CONFIG_KGDB_BAUDRATE;
	char *s, *e;

	if ((s = getenv("kgdbrate")) != NULL && *s != '\0') {
		ulong rate = simple_strtoul(s, &e, 10);
		if (e > s && *e == '\0')
			speed = rate;
	}

	/* initialize pointers to SCC */

	sp = (scc_t *) &(im->im_scc[KGDB_SCC_INDEX]);
	up = (scc_uart_t *)&im->im_dprambase[KGDB_PROFF_SCC];

	/* Disable transmitter/receiver.
	*/
	sp->scc_gsmrl &= ~(SCC_GSMRL_ENR | SCC_GSMRL_ENT);

	/* put the SCC channel into NMSI (non multiplexd serial interface)
	 * mode and wire the selected SCC Tx and Rx clocks to BRGx (15-15).
	 */
	im->im_cpmux.cmx_scr = \
		(im->im_cpmux.cmx_scr & ~KGDB_CMXSCR_MASK) | KGDB_CMXSCR_VALUE;

	/* Set up the baud rate generator.
	*/
#if defined(CONFIG_KGDB_USE_EXTC)
	m8260_cpm_extcbrg(KGDB_SCC_INDEX, speed,
		CONFIG_KGDB_EXTC_RATE, CONFIG_KGDB_EXTC_PINSEL);
#else
	m8260_cpm_setbrg(KGDB_SCC_INDEX, speed);
#endif

	/* Allocate space for two buffer descriptors in the DP ram.
	 * damm: allocating space after the two buffers for rx/tx data
	 */

	dpaddr = m8260_cpm_dpalloc((2 * sizeof (cbd_t)) + 2, 16);

	/* Set the physical address of the host memory buffers in
	 * the buffer descriptors.
	 */
	rbdf = (cbd_t *)&im->im_dprambase[dpaddr];
	rbdf->cbd_bufaddr = (uint) (rbdf+2);
	rbdf->cbd_sc = BD_SC_EMPTY | BD_SC_WRAP;
	tbdf = rbdf + 1;
	tbdf->cbd_bufaddr = ((uint) (rbdf+2)) + 1;
	tbdf->cbd_sc = BD_SC_WRAP;

	/* Set up the uart parameters in the parameter ram.
	*/
	up->scc_genscc.scc_rbase = dpaddr;
	up->scc_genscc.scc_tbase = dpaddr+sizeof(cbd_t);
	up->scc_genscc.scc_rfcr = CPMFCR_EB;
	up->scc_genscc.scc_tfcr = CPMFCR_EB;
	up->scc_genscc.scc_mrblr = 1;
	up->scc_maxidl = 0;
	up->scc_brkcr = 1;
	up->scc_parec = 0;
	up->scc_frmec = 0;
	up->scc_nosec = 0;
	up->scc_brkec = 0;
	up->scc_uaddr1 = 0;
	up->scc_uaddr2 = 0;
	up->scc_toseq = 0;
	up->scc_char1 = up->scc_char2 = up->scc_char3 = up->scc_char4 = 0x8000;
	up->scc_char5 = up->scc_char6 = up->scc_char7 = up->scc_char8 = 0x8000;
	up->scc_rccm = 0xc0ff;

	/* Mask all interrupts and remove anything pending.
	*/
	sp->scc_sccm = 0;
	sp->scc_scce = 0xffff;

	/* Set 8 bit FIFO, 16 bit oversampling and UART mode.
	*/
	sp->scc_gsmrh = SCC_GSMRH_RFW;	/* 8 bit FIFO */
	sp->scc_gsmrl = \
		SCC_GSMRL_TDCR_16 | SCC_GSMRL_RDCR_16 | SCC_GSMRL_MODE_UART;

	/* Set CTS flow control, 1 stop bit, 8 bit character length,
	 * normal async UART mode, no parity
	 */
	sp->scc_psmr = SCU_PSMR_FLC | SCU_PSMR_CL;

	/* execute the "Init Rx and Tx params" CP command.
	*/

	while (cp->cp_cpcr & CPM_CR_FLG)  /* wait if cp is busy */
	  ;

	cp->cp_cpcr = mk_cr_cmd(KGDB_CPM_CR_SCC_PAGE, KGDB_CPM_CR_SCC_SBLOCK,
					0, CPM_CR_INIT_TRX) | CPM_CR_FLG;

	while (cp->cp_cpcr & CPM_CR_FLG)  /* wait if cp is busy */
	  ;

	/* Enable transmitter/receiver.
	*/
	sp->scc_gsmrl |= SCC_GSMRL_ENR | SCC_GSMRL_ENT;

	printf("SCC%d at %dbps ", CONFIG_KGDB_INDEX, speed);
}

void
putDebugChar(const char c)
{
	volatile scc_uart_t	*up;
	volatile cbd_t		*tbdf;
	volatile immap_t	*im;

	if (c == '\n')
		putDebugChar ('\r');

	im = (immap_t *)CONFIG_SYS_IMMR;
	up = (scc_uart_t *)&im->im_dprambase[KGDB_PROFF_SCC];
	tbdf = (cbd_t *)&im->im_dprambase[up->scc_genscc.scc_tbase];

	/* Wait for last character to go.
	 */
	while (tbdf->cbd_sc & BD_SC_READY)
		;

	/* Load the character into the transmit buffer.
	 */
	*(volatile char *)tbdf->cbd_bufaddr = c;
	tbdf->cbd_datlen = 1;
	tbdf->cbd_sc |= BD_SC_READY;
}

void
putDebugStr (const char *s)
{
	while (*s) {
		putDebugChar (*s++);
	}
}

int
getDebugChar(void)
{
	volatile cbd_t		*rbdf;
	volatile scc_uart_t	*up;
	volatile immap_t	*im;
	unsigned char		c;

	im = (immap_t *)CONFIG_SYS_IMMR;
	up = (scc_uart_t *)&im->im_dprambase[KGDB_PROFF_SCC];
	rbdf = (cbd_t *)&im->im_dprambase[up->scc_genscc.scc_rbase];

	/* Wait for character to show up.
	 */
	while (rbdf->cbd_sc & BD_SC_EMPTY)
		;

	/* Grab the char and clear the buffer again.
	 */
	c = *(volatile unsigned char *)rbdf->cbd_bufaddr;
	rbdf->cbd_sc |= BD_SC_EMPTY;

	return (c);
}

void
kgdb_interruptible(int yes)
{
	return;
}

#endif	/* CONFIG_KGDB_ON_SCC */
