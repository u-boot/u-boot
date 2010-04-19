/*
 * (C) Copyright 2000, 2001, 2002
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
 * Minimal serial functions needed to use one of the SMC ports
 * as serial console interface.
 */

#include <common.h>
#include <mpc8260.h>
#include <asm/cpm_8260.h>

DECLARE_GLOBAL_DATA_PTR;

#if defined(CONFIG_CONS_ON_SMC)

#if CONFIG_CONS_INDEX == 1	/* Console on SMC1 */

#define SMC_INDEX		0
#define PROFF_SMC_BASE		PROFF_SMC1_BASE
#define PROFF_SMC		PROFF_SMC1
#define CPM_CR_SMC_PAGE		CPM_CR_SMC1_PAGE
#define CPM_CR_SMC_SBLOCK	CPM_CR_SMC1_SBLOCK
#define CMXSMR_MASK		(CMXSMR_SMC1|CMXSMR_SMC1CS_MSK)
#define CMXSMR_VALUE		CMXSMR_SMC1CS_BRG7

#elif CONFIG_CONS_INDEX == 2	/* Console on SMC2 */

#define SMC_INDEX		1
#define PROFF_SMC_BASE		PROFF_SMC2_BASE
#define PROFF_SMC		PROFF_SMC2
#define CPM_CR_SMC_PAGE		CPM_CR_SMC2_PAGE
#define CPM_CR_SMC_SBLOCK	CPM_CR_SMC2_SBLOCK
#define CMXSMR_MASK		(CMXSMR_SMC2|CMXSMR_SMC2CS_MSK)
#define CMXSMR_VALUE		CMXSMR_SMC2CS_BRG8

#else

#error "console not correctly defined"

#endif

#if !defined(CONFIG_SYS_SMC_RXBUFLEN)
#define CONFIG_SYS_SMC_RXBUFLEN	1
#define CONFIG_SYS_MAXIDLE	0
#else
#if !defined(CONFIG_SYS_MAXIDLE)
#error "you must define CONFIG_SYS_MAXIDLE"
#endif
#endif

typedef volatile struct serialbuffer {
	cbd_t	rxbd;		/* Rx BD */
	cbd_t	txbd;		/* Tx BD */
	uint	rxindex;	/* index for next character to read */
	volatile uchar	rxbuf[CONFIG_SYS_SMC_RXBUFLEN];/* rx buffers */
	volatile uchar	txbuf;	/* tx buffers */
} serialbuffer_t;

/* map rs_table index to baud rate generator index */
static unsigned char brg_map[] = {
	6,	/* BRG7 for SMC1 */
	7,	/* BRG8 for SMC2 */
	0,	/* BRG1 for SCC1 */
	1,	/* BRG1 for SCC2 */
	2,	/* BRG1 for SCC3 */
	3,	/* BRG1 for SCC4 */
};

int serial_init (void)
{
	volatile immap_t *im = (immap_t *)CONFIG_SYS_IMMR;
	volatile smc_t *sp;
	volatile smc_uart_t *up;
	volatile cpm8260_t *cp = &(im->im_cpm);
	uint	dpaddr;
	volatile serialbuffer_t *rtx;

	/* initialize pointers to SMC */

	sp = (smc_t *) &(im->im_smc[SMC_INDEX]);
	*(ushort *)(&im->im_dprambase[PROFF_SMC_BASE]) = PROFF_SMC;
	up = (smc_uart_t *)&im->im_dprambase[PROFF_SMC];

	/* Disable transmitter/receiver. */
	sp->smc_smcmr &= ~(SMCMR_REN | SMCMR_TEN);

	/* NOTE: I/O port pins are set up via the iop_conf_tab[] table */

	/* Allocate space for two buffer descriptors in the DP ram.
	 * damm: allocating space after the two buffers for rx/tx data
	 */

	/* allocate size of struct serialbuffer with bd rx/tx,
	 * buffer rx/tx and rx index
	 */
	dpaddr = m8260_cpm_dpalloc((sizeof(serialbuffer_t)), 16);

	rtx = (serialbuffer_t *)&im->im_dprambase[dpaddr];

	/* Set the physical address of the host memory buffers in
	 * the buffer descriptors.
	 */
	rtx->rxbd.cbd_bufaddr = (uint) &rtx->rxbuf;
	rtx->rxbd.cbd_sc      = 0;

	rtx->txbd.cbd_bufaddr = (uint) &rtx->txbuf;
	rtx->txbd.cbd_sc      = 0;

	/* Set up the uart parameters in the parameter ram. */
	up->smc_rbase = dpaddr;
	up->smc_tbase = dpaddr+sizeof(cbd_t);
	up->smc_rfcr = CPMFCR_EB;
	up->smc_tfcr = CPMFCR_EB;
	up->smc_brklen = 0;
	up->smc_brkec = 0;
	up->smc_brkcr = 0;

	/* Set UART mode, 8 bit, no parity, one stop.
	 * Enable receive and transmit.
	 */
	sp->smc_smcmr = smcr_mk_clen(9) |  SMCMR_SM_UART;

	/* Mask all interrupts and remove anything pending. */
	sp->smc_smcm = 0;
	sp->smc_smce = 0xff;

	/* put the SMC channel into NMSI (non multiplexd serial interface)
	 * mode and wire either BRG7 to SMC1 or BRG8 to SMC2 (15-17).
	 */
	im->im_cpmux.cmx_smr = (im->im_cpmux.cmx_smr&~CMXSMR_MASK)|CMXSMR_VALUE;

	/* Set up the baud rate generator. */
	serial_setbrg ();

	/* Make the first buffer the only buffer. */
	rtx->txbd.cbd_sc |= BD_SC_WRAP;
	rtx->rxbd.cbd_sc |= BD_SC_EMPTY | BD_SC_WRAP;

	/* single/multi character receive. */
	up->smc_mrblr = CONFIG_SYS_SMC_RXBUFLEN;
	up->smc_maxidl = CONFIG_SYS_MAXIDLE;
	rtx->rxindex = 0;

	/* Initialize Tx/Rx parameters. */

	while (cp->cp_cpcr & CPM_CR_FLG)  /* wait if cp is busy */
	  ;

	cp->cp_cpcr = mk_cr_cmd(CPM_CR_SMC_PAGE, CPM_CR_SMC_SBLOCK,
					0, CPM_CR_INIT_TRX) | CPM_CR_FLG;

	while (cp->cp_cpcr & CPM_CR_FLG)  /* wait if cp is busy */
	  ;

	/* Enable transmitter/receiver. */
	sp->smc_smcmr |= SMCMR_REN | SMCMR_TEN;

	return (0);
}

void
serial_setbrg (void)
{
#if defined(CONFIG_CONS_USE_EXTC)
	m8260_cpm_extcbrg(brg_map[SMC_INDEX], gd->baudrate,
		CONFIG_CONS_EXTC_RATE, CONFIG_CONS_EXTC_PINSEL);
#else
	m8260_cpm_setbrg(brg_map[SMC_INDEX], gd->baudrate);
#endif
}

void
serial_putc(const char c)
{
	volatile smc_uart_t	*up;
	volatile immap_t	*im = (immap_t *)CONFIG_SYS_IMMR;
	volatile serialbuffer_t	*rtx;

	if (c == '\n')
		serial_putc ('\r');

	up = (smc_uart_t *)&(im->im_dprambase[PROFF_SMC]);

	rtx = (serialbuffer_t *)&im->im_dprambase[up->smc_rbase];

	/* Wait for last character to go. */
	while (rtx->txbd.cbd_sc & BD_SC_READY & BD_SC_READY)
		;
	rtx->txbuf = c;
	rtx->txbd.cbd_datlen = 1;
	rtx->txbd.cbd_sc |= BD_SC_READY;
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
	volatile smc_uart_t	*up;
	volatile immap_t	*im = (immap_t *)CONFIG_SYS_IMMR;
	volatile serialbuffer_t	*rtx;
	unsigned char  c;

	up = (smc_uart_t *)&(im->im_dprambase[PROFF_SMC]);

	rtx = (serialbuffer_t *)&im->im_dprambase[up->smc_rbase];

	/* Wait for character to show up. */
	while (rtx->rxbd.cbd_sc & BD_SC_EMPTY)
		;

	/* the characters are read one by one,
	 * use the rxindex to know the next char to deliver
	 */
	c = *(unsigned char *) (rtx->rxbd.cbd_bufaddr + rtx->rxindex);
	rtx->rxindex++;

	/* check if all char are readout, then make prepare for next receive */
	if (rtx->rxindex >= rtx->rxbd.cbd_datlen) {
		rtx->rxindex = 0;
		rtx->rxbd.cbd_sc |= BD_SC_EMPTY;
	}
	return(c);
}

int
serial_tstc()
{
	volatile smc_uart_t	*up;
	volatile immap_t	*im = (immap_t *)CONFIG_SYS_IMMR;
	volatile serialbuffer_t	*rtx;

	up = (smc_uart_t *)&(im->im_dprambase[PROFF_SMC]);
	rtx = (serialbuffer_t *)&im->im_dprambase[up->smc_rbase];

	return !(rtx->rxbd.cbd_sc & BD_SC_EMPTY);
}

#endif	/* CONFIG_CONS_ON_SMC */

#if defined(CONFIG_KGDB_ON_SMC)

#if defined(CONFIG_CONS_ON_SMC) && CONFIG_KGDB_INDEX == CONFIG_CONS_INDEX
#error Whoops! serial console and kgdb are on the same smc serial port
#endif

#if CONFIG_KGDB_INDEX == 1	/* KGDB Port on SMC1 */

#define KGDB_SMC_INDEX		0
#define KGDB_PROFF_SMC_BASE	PROFF_SMC1_BASE
#define KGDB_PROFF_SMC		PROFF_SMC1
#define KGDB_CPM_CR_SMC_PAGE	CPM_CR_SMC1_PAGE
#define KGDB_CPM_CR_SMC_SBLOCK	CPM_CR_SMC1_SBLOCK
#define KGDB_CMXSMR_MASK	(CMXSMR_SMC1|CMXSMR_SMC1CS_MSK)
#define KGDB_CMXSMR_VALUE	CMXSMR_SMC1CS_BRG7

#elif CONFIG_KGDB_INDEX == 2	/* KGDB Port on SMC2 */

#define KGDB_SMC_INDEX		1
#define KGDB_PROFF_SMC_BASE	PROFF_SMC2_BASE
#define KGDB_PROFF_SMC		PROFF_SMC2
#define KGDB_CPM_CR_SMC_PAGE	CPM_CR_SMC2_PAGE
#define KGDB_CPM_CR_SMC_SBLOCK	CPM_CR_SMC2_SBLOCK
#define KGDB_CMXSMR_MASK	(CMXSMR_SMC2|CMXSMR_SMC2CS_MSK)
#define KGDB_CMXSMR_VALUE	CMXSMR_SMC2CS_BRG8

#else

#error "console not correctly defined"

#endif

void
kgdb_serial_init (void)
{
	volatile immap_t *im = (immap_t *)CONFIG_SYS_IMMR;
	volatile smc_t *sp;
	volatile smc_uart_t *up;
	volatile cbd_t *tbdf, *rbdf;
	volatile cpm8260_t *cp = &(im->im_cpm);
	uint dpaddr, speed = CONFIG_KGDB_BAUDRATE;
	char *s, *e;

	if ((s = getenv("kgdbrate")) != NULL && *s != '\0') {
		ulong rate = simple_strtoul(s, &e, 10);
		if (e > s && *e == '\0')
			speed = rate;
	}

	/* initialize pointers to SMC */

	sp = (smc_t *) &(im->im_smc[KGDB_SMC_INDEX]);
	*(ushort *)(&im->im_dprambase[KGDB_PROFF_SMC_BASE]) = KGDB_PROFF_SMC;
	up = (smc_uart_t *)&im->im_dprambase[KGDB_PROFF_SMC];

	/* Disable transmitter/receiver. */
	sp->smc_smcmr &= ~(SMCMR_REN | SMCMR_TEN);

	/* NOTE: I/O port pins are set up via the iop_conf_tab[] table */

	/* Allocate space for two buffer descriptors in the DP ram.
	 * damm: allocating space after the two buffers for rx/tx data
	 */

	dpaddr = m8260_cpm_dpalloc((2 * sizeof (cbd_t)) + 2, 16);

	/* Set the physical address of the host memory buffers in
	 * the buffer descriptors.
	 */
	rbdf = (cbd_t *)&im->im_dprambase[dpaddr];
	rbdf->cbd_bufaddr = (uint) (rbdf+2);
	rbdf->cbd_sc = 0;
	tbdf = rbdf + 1;
	tbdf->cbd_bufaddr = ((uint) (rbdf+2)) + 1;
	tbdf->cbd_sc = 0;

	/* Set up the uart parameters in the parameter ram. */
	up->smc_rbase = dpaddr;
	up->smc_tbase = dpaddr+sizeof(cbd_t);
	up->smc_rfcr = CPMFCR_EB;
	up->smc_tfcr = CPMFCR_EB;
	up->smc_brklen = 0;
	up->smc_brkec = 0;
	up->smc_brkcr = 0;

	/* Set UART mode, 8 bit, no parity, one stop.
	 * Enable receive and transmit.
	 */
	sp->smc_smcmr = smcr_mk_clen(9) |  SMCMR_SM_UART;

	/* Mask all interrupts and remove anything pending. */
	sp->smc_smcm = 0;
	sp->smc_smce = 0xff;

	/* put the SMC channel into NMSI (non multiplexd serial interface)
	 * mode and wire either BRG7 to SMC1 or BRG8 to SMC2 (15-17).
	 */
	im->im_cpmux.cmx_smr =
		(im->im_cpmux.cmx_smr & ~KGDB_CMXSMR_MASK) | KGDB_CMXSMR_VALUE;

	/* Set up the baud rate generator. */
#if defined(CONFIG_KGDB_USE_EXTC)
	m8260_cpm_extcbrg(brg_map[KGDB_SMC_INDEX], speed,
		CONFIG_KGDB_EXTC_RATE, CONFIG_KGDB_EXTC_PINSEL);
#else
	m8260_cpm_setbrg(brg_map[KGDB_SMC_INDEX], speed);
#endif

	/* Make the first buffer the only buffer. */
	tbdf->cbd_sc |= BD_SC_WRAP;
	rbdf->cbd_sc |= BD_SC_EMPTY | BD_SC_WRAP;

	/* Single character receive. */
	up->smc_mrblr = 1;
	up->smc_maxidl = 0;

	/* Initialize Tx/Rx parameters. */

	while (cp->cp_cpcr & CPM_CR_FLG)  /* wait if cp is busy */
	  ;

	cp->cp_cpcr = mk_cr_cmd(KGDB_CPM_CR_SMC_PAGE, KGDB_CPM_CR_SMC_SBLOCK,
					0, CPM_CR_INIT_TRX) | CPM_CR_FLG;

	while (cp->cp_cpcr & CPM_CR_FLG)  /* wait if cp is busy */
	  ;

	/* Enable transmitter/receiver.	*/
	sp->smc_smcmr |= SMCMR_REN | SMCMR_TEN;

	printf("SMC%d at %dbps ", CONFIG_KGDB_INDEX, speed);
}

void
putDebugChar(const char c)
{
	volatile cbd_t		*tbdf;
	volatile char		*buf;
	volatile smc_uart_t	*up;
	volatile immap_t	*im = (immap_t *)CONFIG_SYS_IMMR;

	if (c == '\n')
		putDebugChar ('\r');

	up = (smc_uart_t *)&(im->im_dprambase[KGDB_PROFF_SMC]);

	tbdf = (cbd_t *)&im->im_dprambase[up->smc_tbase];

	/* Wait for last character to go. */
	buf = (char *)tbdf->cbd_bufaddr;
	while (tbdf->cbd_sc & BD_SC_READY)
		;

	*buf = c;
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
	volatile unsigned char	*buf;
	volatile smc_uart_t	*up;
	volatile immap_t	*im = (immap_t *)CONFIG_SYS_IMMR;
	unsigned char		c;

	up = (smc_uart_t *)&(im->im_dprambase[KGDB_PROFF_SMC]);

	rbdf = (cbd_t *)&im->im_dprambase[up->smc_rbase];

	/* Wait for character to show up. */
	buf = (unsigned char *)rbdf->cbd_bufaddr;
	while (rbdf->cbd_sc & BD_SC_EMPTY)
		;
	c = *buf;
	rbdf->cbd_sc |= BD_SC_EMPTY;

	return(c);
}

void
kgdb_interruptible(int yes)
{
	return;
}

#endif	/* CONFIG_KGDB_ON_SMC */
