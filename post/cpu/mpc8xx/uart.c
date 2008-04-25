/*
 * (C) Copyright 2002
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
 */

#include <common.h>

/*
 * UART test
 *
 * The Serial Management Controllers (SMC) and the Serial Communication
 * Controllers (SCC) listed in ctlr_list array below are tested in
 * the loopback UART mode.
 * The controllers are configured accordingly and several characters
 * are transmitted. The configurable test parameters are:
 *   MIN_PACKET_LENGTH - minimum size of packet to transmit
 *   MAX_PACKET_LENGTH - maximum size of packet to transmit
 *   TEST_NUM - number of tests
 */

#include <post.h>
#if CONFIG_POST & CFG_POST_UART
#if defined(CONFIG_8xx)
#include <commproc.h>
#elif defined(CONFIG_MPC8260)
#include <asm/cpm_8260.h>
#else
#error "Apparently a bad configuration, please fix."
#endif
#include <command.h>
#include <serial.h>

DECLARE_GLOBAL_DATA_PTR;

#define CTLR_SMC 0
#define CTLR_SCC 1

/* The list of controllers to test */
#if defined(CONFIG_MPC823)
static int ctlr_list[][2] =
		{ {CTLR_SMC, 0}, {CTLR_SMC, 1}, {CTLR_SCC, 1} };
#else
static int ctlr_list[][2] = { };
#endif

#define CTRL_LIST_SIZE (sizeof(ctlr_list) / sizeof(ctlr_list[0]))

static struct {
	void (*init) (int index);
	void (*halt) (int index);
	void (*putc) (int index, const char c);
	int (*getc) (int index);
} ctlr_proc[2];

static char *ctlr_name[2] = { "SMC", "SCC" };

static int proff_smc[] = { PROFF_SMC1, PROFF_SMC2 };
static int proff_scc[] =
		{ PROFF_SCC1, PROFF_SCC2, PROFF_SCC3, PROFF_SCC4 };

/*
 * SMC callbacks
 */

static void smc_init (int smc_index)
{
	static int cpm_cr_ch[] = { CPM_CR_CH_SMC1, CPM_CR_CH_SMC2 };

	volatile immap_t *im = (immap_t *) CFG_IMMR;
	volatile smc_t *sp;
	volatile smc_uart_t *up;
	volatile cbd_t *tbdf, *rbdf;
	volatile cpm8xx_t *cp = &(im->im_cpm);
	uint dpaddr;

	/* initialize pointers to SMC */

	sp = (smc_t *) & (cp->cp_smc[smc_index]);
	up = (smc_uart_t *) & cp->cp_dparam[proff_smc[smc_index]];

	/* Disable transmitter/receiver.
	 */
	sp->smc_smcmr &= ~(SMCMR_REN | SMCMR_TEN);

	/* Enable SDMA.
	 */
	im->im_siu_conf.sc_sdcr = 1;

	/* clear error conditions */
#ifdef	CFG_SDSR
	im->im_sdma.sdma_sdsr = CFG_SDSR;
#else
	im->im_sdma.sdma_sdsr = 0x83;
#endif

	/* clear SDMA interrupt mask */
#ifdef	CFG_SDMR
	im->im_sdma.sdma_sdmr = CFG_SDMR;
#else
	im->im_sdma.sdma_sdmr = 0x00;
#endif

#if defined(CONFIG_FADS)
	/* Enable RS232 */
	*((uint *) BCSR1) &=
			~(smc_index == 1 ? BCSR1_RS232EN_1 : BCSR1_RS232EN_2);
#endif

#if defined(CONFIG_RPXLITE) || defined(CONFIG_RPXCLASSIC)
	/* Enable Monitor Port Transceiver */
	*((uchar *) BCSR0) |= BCSR0_ENMONXCVR;
#endif

	/* Set the physical address of the host memory buffers in
	 * the buffer descriptors.
	 */

#ifdef CFG_ALLOC_DPRAM
	dpaddr = dpram_alloc_align (sizeof (cbd_t) * 2 + 2, 8);
#else
	dpaddr = CPM_POST_BASE;
#endif

	/* Allocate space for two buffer descriptors in the DP ram.
	 * For now, this address seems OK, but it may have to
	 * change with newer versions of the firmware.
	 * damm: allocating space after the two buffers for rx/tx data
	 */

	rbdf = (cbd_t *) & cp->cp_dpmem[dpaddr];
	rbdf->cbd_bufaddr = (uint) (rbdf + 2);
	rbdf->cbd_sc = 0;
	tbdf = rbdf + 1;
	tbdf->cbd_bufaddr = ((uint) (rbdf + 2)) + 1;
	tbdf->cbd_sc = 0;

	/* Set up the uart parameters in the parameter ram.
	 */
	up->smc_rbase = dpaddr;
	up->smc_tbase = dpaddr + sizeof (cbd_t);
	up->smc_rfcr = SMC_EB;
	up->smc_tfcr = SMC_EB;

#if defined(CONFIG_MBX)
	board_serial_init ();
#endif

	/* Set UART mode, 8 bit, no parity, one stop.
	 * Enable receive and transmit.
	 * Set local loopback mode.
	 */
	sp->smc_smcmr = smcr_mk_clen (9) | SMCMR_SM_UART | (ushort) 0x0004;

	/* Mask all interrupts and remove anything pending.
	 */
	sp->smc_smcm = 0;
	sp->smc_smce = 0xff;

	/* Set up the baud rate generator.
	 */
	cp->cp_simode = 0x00000000;

	cp->cp_brgc1 =
			(((gd->cpu_clk / 16 / gd->baudrate) -
			  1) << 1) | CPM_BRG_EN;

	/* Make the first buffer the only buffer.
	 */
	tbdf->cbd_sc |= BD_SC_WRAP;
	rbdf->cbd_sc |= BD_SC_EMPTY | BD_SC_WRAP;

	/* Single character receive.
	 */
	up->smc_mrblr = 1;
	up->smc_maxidl = 0;

	/* Initialize Tx/Rx parameters.
	 */

	while (cp->cp_cpcr & CPM_CR_FLG)	/* wait if cp is busy */
		;

	cp->cp_cpcr =
			mk_cr_cmd (cpm_cr_ch[smc_index], CPM_CR_INIT_TRX) | CPM_CR_FLG;

	while (cp->cp_cpcr & CPM_CR_FLG)	/* wait if cp is busy */
		;

	/* Enable transmitter/receiver.
	 */
	sp->smc_smcmr |= SMCMR_REN | SMCMR_TEN;
}

static void smc_halt(int smc_index)
{
}

static void smc_putc (int smc_index, const char c)
{
	volatile cbd_t *tbdf;
	volatile char *buf;
	volatile smc_uart_t *up;
	volatile immap_t *im = (immap_t *) CFG_IMMR;
	volatile cpm8xx_t *cpmp = &(im->im_cpm);

	up = (smc_uart_t *) & cpmp->cp_dparam[proff_smc[smc_index]];

	tbdf = (cbd_t *) & cpmp->cp_dpmem[up->smc_tbase];

	/* Wait for last character to go.
	 */

	buf = (char *) tbdf->cbd_bufaddr;
#if 0
	__asm__ ("eieio");
	while (tbdf->cbd_sc & BD_SC_READY)
		__asm__ ("eieio");
#endif

	*buf = c;
	tbdf->cbd_datlen = 1;
	tbdf->cbd_sc |= BD_SC_READY;
	__asm__ ("eieio");
#if 1
	while (tbdf->cbd_sc & BD_SC_READY)
		__asm__ ("eieio");
#endif
}

static int smc_getc (int smc_index)
{
	volatile cbd_t *rbdf;
	volatile unsigned char *buf;
	volatile smc_uart_t *up;
	volatile immap_t *im = (immap_t *) CFG_IMMR;
	volatile cpm8xx_t *cpmp = &(im->im_cpm);
	unsigned char c;
	int i;

	up = (smc_uart_t *) & cpmp->cp_dparam[proff_smc[smc_index]];

	rbdf = (cbd_t *) & cpmp->cp_dpmem[up->smc_rbase];

	/* Wait for character to show up.
	 */
	buf = (unsigned char *) rbdf->cbd_bufaddr;
#if 0
	while (rbdf->cbd_sc & BD_SC_EMPTY);
#else
	for (i = 100; i > 0; i--) {
		if (!(rbdf->cbd_sc & BD_SC_EMPTY))
			break;
		udelay (1000);
	}

	if (i == 0)
		return -1;
#endif
	c = *buf;
	rbdf->cbd_sc |= BD_SC_EMPTY;

	return (c);
}

  /*
   * SCC callbacks
   */

static void scc_init (int scc_index)
{
	static int cpm_cr_ch[] = {
		CPM_CR_CH_SCC1,
		CPM_CR_CH_SCC2,
		CPM_CR_CH_SCC3,
		CPM_CR_CH_SCC4,
	};

	volatile immap_t *im = (immap_t *) CFG_IMMR;
	volatile scc_t *sp;
	volatile scc_uart_t *up;
	volatile cbd_t *tbdf, *rbdf;
	volatile cpm8xx_t *cp = &(im->im_cpm);
	uint dpaddr;

	/* initialize pointers to SCC */

	sp = (scc_t *) & (cp->cp_scc[scc_index]);
	up = (scc_uart_t *) & cp->cp_dparam[proff_scc[scc_index]];

	/* Disable transmitter/receiver.
	 */
	sp->scc_gsmrl &= ~(SCC_GSMRL_ENR | SCC_GSMRL_ENT);


	/* Allocate space for two buffer descriptors in the DP ram.
	 */

#ifdef CFG_ALLOC_DPRAM
	dpaddr = dpram_alloc_align (sizeof (cbd_t) * 2 + 2, 8);
#else
	dpaddr = CPM_POST_BASE;
#endif

	/* Enable SDMA.
	 */
	im->im_siu_conf.sc_sdcr = 0x0001;

	/* Set the physical address of the host memory buffers in
	 * the buffer descriptors.
	 */

	rbdf = (cbd_t *) & cp->cp_dpmem[dpaddr];
	rbdf->cbd_bufaddr = (uint) (rbdf + 2);
	rbdf->cbd_sc = 0;
	tbdf = rbdf + 1;
	tbdf->cbd_bufaddr = ((uint) (rbdf + 2)) + 1;
	tbdf->cbd_sc = 0;

	/* Set up the baud rate generator.
	 */
	cp->cp_sicr &= ~(0x000000FF << (8 * scc_index));
	/* no |= needed, since BRG1 is 000 */

	cp->cp_brgc1 =
			(((gd->cpu_clk / 16 / gd->baudrate) -
			  1) << 1) | CPM_BRG_EN;

	/* Set up the uart parameters in the parameter ram.
	 */
	up->scc_genscc.scc_rbase = dpaddr;
	up->scc_genscc.scc_tbase = dpaddr + sizeof (cbd_t);

	/* Initialize Tx/Rx parameters.
	 */
	while (cp->cp_cpcr & CPM_CR_FLG)	/* wait if cp is busy */
		;
	cp->cp_cpcr =
			mk_cr_cmd (cpm_cr_ch[scc_index], CPM_CR_INIT_TRX) | CPM_CR_FLG;

	while (cp->cp_cpcr & CPM_CR_FLG)	/* wait if cp is busy */
		;

	up->scc_genscc.scc_rfcr = SCC_EB | 0x05;
	up->scc_genscc.scc_tfcr = SCC_EB | 0x05;

	up->scc_genscc.scc_mrblr = 1;	/* Single character receive */
	up->scc_maxidl = 0;		/* disable max idle */
	up->scc_brkcr = 1;		/* send one break character on stop TX */
	up->scc_parec = 0;
	up->scc_frmec = 0;
	up->scc_nosec = 0;
	up->scc_brkec = 0;
	up->scc_uaddr1 = 0;
	up->scc_uaddr2 = 0;
	up->scc_toseq = 0;
	up->scc_char1 = 0x8000;
	up->scc_char2 = 0x8000;
	up->scc_char3 = 0x8000;
	up->scc_char4 = 0x8000;
	up->scc_char5 = 0x8000;
	up->scc_char6 = 0x8000;
	up->scc_char7 = 0x8000;
	up->scc_char8 = 0x8000;
	up->scc_rccm = 0xc0ff;

	/* Set low latency / small fifo.
	 */
	sp->scc_gsmrh = SCC_GSMRH_RFW;

	/* Set UART mode
	 */
	sp->scc_gsmrl &= ~0xF;
	sp->scc_gsmrl |= SCC_GSMRL_MODE_UART;

	/* Set local loopback mode.
	 */
	sp->scc_gsmrl &= ~SCC_GSMRL_DIAG_LE;
	sp->scc_gsmrl |= SCC_GSMRL_DIAG_LOOP;

	/* Set clock divider 16 on Tx and Rx
	 */
	sp->scc_gsmrl |= (SCC_GSMRL_TDCR_16 | SCC_GSMRL_RDCR_16);

	sp->scc_psmr |= SCU_PSMR_CL;

	/* Mask all interrupts and remove anything pending.
	 */
	sp->scc_sccm = 0;
	sp->scc_scce = 0xffff;
	sp->scc_dsr = 0x7e7e;
	sp->scc_psmr = 0x3000;

	/* Make the first buffer the only buffer.
	 */
	tbdf->cbd_sc |= BD_SC_WRAP;
	rbdf->cbd_sc |= BD_SC_EMPTY | BD_SC_WRAP;

	/* Enable transmitter/receiver.
	 */
	sp->scc_gsmrl |= (SCC_GSMRL_ENR | SCC_GSMRL_ENT);
}

static void scc_halt(int scc_index)
{
	volatile immap_t *im = (immap_t *) CFG_IMMR;
	volatile cpm8xx_t *cp = &(im->im_cpm);
	volatile scc_t *sp = (scc_t *) & (cp->cp_scc[scc_index]);

	sp->scc_gsmrl &= ~(SCC_GSMRL_ENR | SCC_GSMRL_ENT | SCC_GSMRL_DIAG_LE);
}

static void scc_putc (int scc_index, const char c)
{
	volatile cbd_t *tbdf;
	volatile char *buf;
	volatile scc_uart_t *up;
	volatile immap_t *im = (immap_t *) CFG_IMMR;
	volatile cpm8xx_t *cpmp = &(im->im_cpm);

	up = (scc_uart_t *) & cpmp->cp_dparam[proff_scc[scc_index]];

	tbdf = (cbd_t *) & cpmp->cp_dpmem[up->scc_genscc.scc_tbase];

	/* Wait for last character to go.
	 */

	buf = (char *) tbdf->cbd_bufaddr;
#if 0
	__asm__ ("eieio");
	while (tbdf->cbd_sc & BD_SC_READY)
		__asm__ ("eieio");
#endif

	*buf = c;
	tbdf->cbd_datlen = 1;
	tbdf->cbd_sc |= BD_SC_READY;
	__asm__ ("eieio");
#if 1
	while (tbdf->cbd_sc & BD_SC_READY)
		__asm__ ("eieio");
#endif
}

static int scc_getc (int scc_index)
{
	volatile cbd_t *rbdf;
	volatile unsigned char *buf;
	volatile scc_uart_t *up;
	volatile immap_t *im = (immap_t *) CFG_IMMR;
	volatile cpm8xx_t *cpmp = &(im->im_cpm);
	unsigned char c;
	int i;

	up = (scc_uart_t *) & cpmp->cp_dparam[proff_scc[scc_index]];

	rbdf = (cbd_t *) & cpmp->cp_dpmem[up->scc_genscc.scc_rbase];

	/* Wait for character to show up.
	 */
	buf = (unsigned char *) rbdf->cbd_bufaddr;
#if 0
	while (rbdf->cbd_sc & BD_SC_EMPTY);
#else
	for (i = 100; i > 0; i--) {
		if (!(rbdf->cbd_sc & BD_SC_EMPTY))
			break;
		udelay (1000);
	}

	if (i == 0)
		return -1;
#endif
	c = *buf;
	rbdf->cbd_sc |= BD_SC_EMPTY;

	return (c);
}

  /*
   * Test routines
   */

static int test_ctlr (int ctlr, int index)
{
	int res = -1;
	char test_str[] = "*** UART Test String ***\r\n";
	int i;

	ctlr_proc[ctlr].init (index);

	for (i = 0; i < sizeof (test_str) - 1; i++) {
		ctlr_proc[ctlr].putc (index, test_str[i]);
		if (ctlr_proc[ctlr].getc (index) != test_str[i])
			goto Done;
	}

	res = 0;

Done:
	ctlr_proc[ctlr].halt (index);

	if (res != 0) {
		post_log ("uart %s%d test failed\n",
				ctlr_name[ctlr], index + 1);
	}

	return res;
}

int uart_post_test (int flags)
{
	int res = 0;
	int i;

	ctlr_proc[CTLR_SMC].init = smc_init;
	ctlr_proc[CTLR_SMC].halt = smc_halt;
	ctlr_proc[CTLR_SMC].putc = smc_putc;
	ctlr_proc[CTLR_SMC].getc = smc_getc;

	ctlr_proc[CTLR_SCC].init = scc_init;
	ctlr_proc[CTLR_SCC].halt = scc_halt;
	ctlr_proc[CTLR_SCC].putc = scc_putc;
	ctlr_proc[CTLR_SCC].getc = scc_getc;

	for (i = 0; i < CTRL_LIST_SIZE; i++) {
		if (test_ctlr (ctlr_list[i][0], ctlr_list[i][1]) != 0) {
			res = -1;
		}
	}

#if !defined(CONFIG_8xx_CONS_NONE)
	serial_reinit_all ();
#endif

	return res;
}

#endif /* CONFIG_POST & CFG_POST_UART */
