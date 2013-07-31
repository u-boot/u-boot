/*
 * (C) Copyright 2000
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

/* Modified by Udi Finkelstein
 *
 * This file includes communication routines for SMC1 that can run even if
 * SMC2 have already been initialized.
 */

#include <common.h>
#include <watchdog.h>
#include <commproc.h>
#include <stdio_dev.h>
#include <lcd.h>

DECLARE_GLOBAL_DATA_PTR;

#define	SMC_INDEX	0
#define PROFF_SMC	PROFF_SMC1
#define CPM_CR_CH_SMC	CPM_CR_CH_SMC1

#define RBC823_KBD_BAUDRATE	38400
#define CPM_KEYBOARD_BASE	0x1000
/*
 * Minimal serial functions needed to use one of the SMC ports
 * as serial console interface.
 */

void smc1_setbrg (void)
{
	volatile immap_t *im = (immap_t *)CONFIG_SYS_IMMR;
	volatile cpm8xx_t *cp = &(im->im_cpm);

	/* Set up the baud rate generator.
	 * See 8xx_io/commproc.c for details.
	 *
	 * Wire BRG2 to SMC1, BRG1 to SMC2
	 */

	cp->cp_simode = 0x00001000;

	cp->cp_brgc2 =
		(((gd->cpu_clk / 16 / RBC823_KBD_BAUDRATE)-1) << 1) | CPM_BRG_EN;
}

int smc1_init (void)
{
	volatile immap_t *im = (immap_t *)CONFIG_SYS_IMMR;
	volatile smc_t *sp;
	volatile smc_uart_t *up;
	volatile cbd_t *tbdf, *rbdf;
	volatile cpm8xx_t *cp = &(im->im_cpm);
	uint	dpaddr;

	/* initialize pointers to SMC */

	sp = (smc_t *) &(cp->cp_smc[SMC_INDEX]);
	up = (smc_uart_t *) &cp->cp_dparam[PROFF_SMC];

	/* Disable transmitter/receiver.
	*/
	sp->smc_smcmr &= ~(SMCMR_REN | SMCMR_TEN);

	/* Enable SDMA.
	*/
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

	/* Use Port B for SMC1 instead of other functions.
	*/
	cp->cp_pbpar |=  0x000000c0;
	cp->cp_pbdir &= ~0x000000c0;
	cp->cp_pbodr &= ~0x000000c0;

	/* Set the physical address of the host memory buffers in
	 * the buffer descriptors.
	 */

#ifdef CONFIG_SYS_ALLOC_DPRAM
	dpaddr = dpram_alloc_align (sizeof(cbd_t)*2 + 2, 8) ;
#else
	dpaddr = CPM_KEYBOARD_BASE ;
#endif

	/* Allocate space for two buffer descriptors in the DP ram.
	 * For now, this address seems OK, but it may have to
	 * change with newer versions of the firmware.
	 * damm: allocating space after the two buffers for rx/tx data
	 */

	rbdf = (cbd_t *)&cp->cp_dpmem[dpaddr];
	rbdf->cbd_bufaddr = (uint) (rbdf+2);
	rbdf->cbd_sc = 0;
	tbdf = rbdf + 1;
	tbdf->cbd_bufaddr = ((uint) (rbdf+2)) + 1;
	tbdf->cbd_sc = 0;

	/* Set up the uart parameters in the parameter ram.
	*/
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

	/* Set up the baud rate generator.
	*/
	smc1_setbrg ();

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

	while (cp->cp_cpcr & CPM_CR_FLG)  /* wait if cp is busy */
	  ;

	cp->cp_cpcr = mk_cr_cmd(CPM_CR_CH_SMC, CPM_CR_INIT_TRX) | CPM_CR_FLG;

	while (cp->cp_cpcr & CPM_CR_FLG)  /* wait if cp is busy */
	  ;

	/* Enable transmitter/receiver.
	*/
	sp->smc_smcmr |= SMCMR_REN | SMCMR_TEN;

	return (0);
}

void smc1_putc(const char c)
{
	volatile cbd_t		*tbdf;
	volatile char		*buf;
	volatile smc_uart_t	*up;
	volatile immap_t	*im = (immap_t *)CONFIG_SYS_IMMR;
	volatile cpm8xx_t	*cpmp = &(im->im_cpm);

	up = (smc_uart_t *)&cpmp->cp_dparam[PROFF_SMC];

	tbdf = (cbd_t *)&cpmp->cp_dpmem[up->smc_tbase];

	/* Wait for last character to go.
	*/

	buf = (char *)tbdf->cbd_bufaddr;

	*buf = c;
	tbdf->cbd_datlen = 1;
	tbdf->cbd_sc |= BD_SC_READY;
	__asm__("eieio");

	while (tbdf->cbd_sc & BD_SC_READY) {
		WATCHDOG_RESET ();
		__asm__("eieio");
	}
}

int smc1_getc(void)
{
	volatile cbd_t		*rbdf;
	volatile unsigned char	*buf;
	volatile smc_uart_t	*up;
	volatile immap_t	*im = (immap_t *)CONFIG_SYS_IMMR;
	volatile cpm8xx_t	*cpmp = &(im->im_cpm);
	unsigned char		c;

	up = (smc_uart_t *)&cpmp->cp_dparam[PROFF_SMC];

	rbdf = (cbd_t *)&cpmp->cp_dpmem[up->smc_rbase];

	/* Wait for character to show up.
	*/
	buf = (unsigned char *)rbdf->cbd_bufaddr;

	while (rbdf->cbd_sc & BD_SC_EMPTY)
		WATCHDOG_RESET ();

	c = *buf;
	rbdf->cbd_sc |= BD_SC_EMPTY;

	return(c);
}

int smc1_tstc(void)
{
	volatile cbd_t		*rbdf;
	volatile smc_uart_t	*up;
	volatile immap_t	*im = (immap_t *)CONFIG_SYS_IMMR;
	volatile cpm8xx_t	*cpmp = &(im->im_cpm);

	up = (smc_uart_t *)&cpmp->cp_dparam[PROFF_SMC];

	rbdf = (cbd_t *)&cpmp->cp_dpmem[up->smc_rbase];

	return(!(rbdf->cbd_sc & BD_SC_EMPTY));
}

/* search for keyboard and register it if found */
int drv_keyboard_init(void)
{
	int error = 0;
	struct stdio_dev kbd_dev;

	if (0) {
		/* register the keyboard */
		memset (&kbd_dev, 0, sizeof(struct stdio_dev));
		strcpy(kbd_dev.name, "kbd");
		kbd_dev.flags =  DEV_FLAGS_INPUT | DEV_FLAGS_SYSTEM;
		kbd_dev.putc = NULL;
		kbd_dev.puts = NULL;
		kbd_dev.getc = smc1_getc;
		kbd_dev.tstc = smc1_tstc;
		error = stdio_register (&kbd_dev);
	} else {
		lcd_is_enabled = 0;
		lcd_disable();
	}
	return error;
}
