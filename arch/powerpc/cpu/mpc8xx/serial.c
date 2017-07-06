/*
 * (C) Copyright 2000
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <commproc.h>
#include <command.h>
#include <serial.h>
#include <watchdog.h>
#include <linux/compiler.h>

DECLARE_GLOBAL_DATA_PTR;

#if !defined(CONFIG_8xx_CONS_NONE)	/* No Console at all */

#if defined(CONFIG_8xx_CONS_SMC1)	/* Console on SMC1 */
#define	SMC_INDEX	0
#define PROFF_SMC	PROFF_SMC1
#define CPM_CR_CH_SMC	CPM_CR_CH_SMC1

#elif defined(CONFIG_8xx_CONS_SMC2)	/* Console on SMC2 */
#define SMC_INDEX	1
#define PROFF_SMC	PROFF_SMC2
#define CPM_CR_CH_SMC	CPM_CR_CH_SMC2

#endif /* CONFIG_8xx_CONS_SMCx */

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

static void serial_setdivisor(volatile cpm8xx_t *cp)
{
	int divisor=(gd->cpu_clk + 8*gd->baudrate)/16/gd->baudrate;

	if(divisor/16>0x1000) {
		/* bad divisor, assume 50MHz clock and 9600 baud */
		divisor=(50*1000*1000 + 8*9600)/16/9600;
	}

#ifdef CONFIG_SYS_BRGCLK_PRESCALE
	divisor /= CONFIG_SYS_BRGCLK_PRESCALE;
#endif

	if(divisor<=0x1000) {
		cp->cp_brgc1=((divisor-1)<<1) | CPM_BRG_EN;
	} else {
		cp->cp_brgc1=((divisor/16-1)<<1) | CPM_BRG_EN | CPM_BRG_DIV16;
	}
}

/*
 * Minimal serial functions needed to use one of the SMC ports
 * as serial console interface.
 */

static void smc_setbrg (void)
{
	volatile immap_t *im = (immap_t *)CONFIG_SYS_IMMR;
	volatile cpm8xx_t *cp = &(im->im_cpm);

	/* Set up the baud rate generator.
	 * See 8xx_io/commproc.c for details.
	 *
	 * Wire BRG1 to SMCx
	 */

	cp->cp_simode = 0x00000000;

	serial_setdivisor(cp);
}

static int smc_init (void)
{
	volatile immap_t *im = (immap_t *)CONFIG_SYS_IMMR;
	volatile smc_t *sp;
	volatile smc_uart_t *up;
	volatile cpm8xx_t *cp = &(im->im_cpm);
	uint	dpaddr;
	volatile serialbuffer_t *rtx;

	/* initialize pointers to SMC */

	sp = (smc_t *) &(cp->cp_smc[SMC_INDEX]);
	up = (smc_uart_t *) &cp->cp_dparam[PROFF_SMC];
	/* Disable relocation */
	up->smc_rpbase = 0;

	/* Disable transmitter/receiver. */
	sp->smc_smcmr &= ~(SMCMR_REN | SMCMR_TEN);

	/* Enable SDMA. */
	im->im_siu_conf.sc_sdcr = 1;

	/* clear error conditions */
#ifdef	CONFIG_SYS_SDSR
	im->im_sdma.sdma_sdsr = CONFIG_SYS_SDSR;
#else
	im->im_sdma.sdma_sdsr = 0x83;
#endif

	/* clear SDMA interrupt mask */
#ifdef	CONFIG_SYS_SDMR
	im->im_sdma.sdma_sdmr = CONFIG_SYS_SDMR;
#else
	im->im_sdma.sdma_sdmr = 0x00;
#endif

#if defined(CONFIG_8xx_CONS_SMC1)
	/* Use Port B for SMC1 instead of other functions. */
	cp->cp_pbpar |=  0x000000c0;
	cp->cp_pbdir &= ~0x000000c0;
	cp->cp_pbodr &= ~0x000000c0;
#else	/* CONFIG_8xx_CONS_SMC2 */
	/* Use Port B for SMC2 instead of other functions.
	 */
	cp->cp_pbpar |=  0x00000c00;
	cp->cp_pbdir &= ~0x00000c00;
	cp->cp_pbodr &= ~0x00000c00;
#endif

	/* Set the physical address of the host memory buffers in
	 * the buffer descriptors.
	 */
	dpaddr = CPM_SERIAL_BASE;

	rtx = (serialbuffer_t *)&cp->cp_dpmem[dpaddr];
	/* Allocate space for two buffer descriptors in the DP ram.
	 * For now, this address seems OK, but it may have to
	 * change with newer versions of the firmware.
	 * damm: allocating space after the two buffers for rx/tx data
	 */

	rtx->rxbd.cbd_bufaddr = (uint) &rtx->rxbuf;
	rtx->rxbd.cbd_sc      = 0;

	rtx->txbd.cbd_bufaddr = (uint) &rtx->txbuf;
	rtx->txbd.cbd_sc      = 0;

	/* Set up the uart parameters in the parameter ram. */
	up->smc_rbase = dpaddr;
	up->smc_tbase = dpaddr+sizeof(cbd_t);
	up->smc_rfcr = SMC_EB;
	up->smc_tfcr = SMC_EB;

	/* Set UART mode, 8 bit, no parity, one stop.
	 * Enable receive and transmit.
	 */
	sp->smc_smcmr = smcr_mk_clen(9) |  SMCMR_SM_UART;

	/* Mask all interrupts and remove anything pending.
	*/
	sp->smc_smcm = 0;
	sp->smc_smce = 0xff;

	/* Set up the baud rate generator */
	smc_setbrg ();

	/* Make the first buffer the only buffer. */
	rtx->txbd.cbd_sc |= BD_SC_WRAP;
	rtx->rxbd.cbd_sc |= BD_SC_EMPTY | BD_SC_WRAP;

	/* single/multi character receive. */
	up->smc_mrblr = CONFIG_SYS_SMC_RXBUFLEN;
	up->smc_maxidl = CONFIG_SYS_MAXIDLE;
	rtx->rxindex = 0;

	/* Initialize Tx/Rx parameters.	*/
	while (cp->cp_cpcr & CPM_CR_FLG)  /* wait if cp is busy */
	  ;

	cp->cp_cpcr = mk_cr_cmd(CPM_CR_CH_SMC, CPM_CR_INIT_TRX) | CPM_CR_FLG;

	while (cp->cp_cpcr & CPM_CR_FLG)  /* wait if cp is busy */
	  ;

	/* Enable transmitter/receiver.	*/
	sp->smc_smcmr |= SMCMR_REN | SMCMR_TEN;

	return (0);
}

static void
smc_putc(const char c)
{
	volatile smc_uart_t	*up;
	volatile immap_t	*im = (immap_t *)CONFIG_SYS_IMMR;
	volatile cpm8xx_t	*cpmp = &(im->im_cpm);
	volatile serialbuffer_t	*rtx;

	if (c == '\n')
		smc_putc ('\r');

	up = (smc_uart_t *)&cpmp->cp_dparam[PROFF_SMC];

	rtx = (serialbuffer_t *)&cpmp->cp_dpmem[up->smc_rbase];

	/* Wait for last character to go. */
	rtx->txbuf = c;
	rtx->txbd.cbd_datlen = 1;
	rtx->txbd.cbd_sc |= BD_SC_READY;
	__asm__("eieio");

	while (rtx->txbd.cbd_sc & BD_SC_READY) {
		WATCHDOG_RESET ();
		__asm__("eieio");
	}
}

static void
smc_puts (const char *s)
{
	while (*s) {
		smc_putc (*s++);
	}
}

static int
smc_getc(void)
{
	volatile smc_uart_t	*up;
	volatile immap_t	*im = (immap_t *)CONFIG_SYS_IMMR;
	volatile cpm8xx_t	*cpmp = &(im->im_cpm);
	volatile serialbuffer_t	*rtx;
	unsigned char  c;

	up = (smc_uart_t *)&cpmp->cp_dparam[PROFF_SMC];
	rtx = (serialbuffer_t *)&cpmp->cp_dpmem[up->smc_rbase];

	/* Wait for character to show up. */
	while (rtx->rxbd.cbd_sc & BD_SC_EMPTY)
		WATCHDOG_RESET ();

	/* the characters are read one by one,
	 * use the rxindex to know the next char to deliver
	 */
	c = *(unsigned char *) (rtx->rxbd.cbd_bufaddr+rtx->rxindex);
	rtx->rxindex++;

	/* check if all char are readout, then make prepare for next receive */
	if (rtx->rxindex >= rtx->rxbd.cbd_datlen) {
		rtx->rxindex = 0;
		rtx->rxbd.cbd_sc |= BD_SC_EMPTY;
	}
	return(c);
}

static int
smc_tstc(void)
{
	volatile smc_uart_t	*up;
	volatile immap_t	*im = (immap_t *)CONFIG_SYS_IMMR;
	volatile cpm8xx_t	*cpmp = &(im->im_cpm);
	volatile serialbuffer_t	*rtx;

	up = (smc_uart_t *)&cpmp->cp_dparam[PROFF_SMC];

	rtx = (serialbuffer_t *)&cpmp->cp_dpmem[up->smc_rbase];

	return !(rtx->rxbd.cbd_sc & BD_SC_EMPTY);
}

struct serial_device serial_smc_device =
{
	.name	= "serial_smc",
	.start	= smc_init,
	.stop	= NULL,
	.setbrg	= smc_setbrg,
	.getc	= smc_getc,
	.tstc	= smc_tstc,
	.putc	= smc_putc,
	.puts	= smc_puts,
};

__weak struct serial_device *default_serial_console(void)
{
	return &serial_smc_device;
}

void mpc8xx_serial_initialize(void)
{
	serial_register(&serial_smc_device);
}

#endif	/* CONFIG_8xx_CONS_NONE */
