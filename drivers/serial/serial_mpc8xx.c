/*
 * (C) Copyright 2000
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <command.h>
#include <serial.h>
#include <watchdog.h>
#include <asm/cpm_8xx.h>
#include <linux/compiler.h>

DECLARE_GLOBAL_DATA_PTR;

#if defined(CONFIG_8xx_CONS_SMC1)	/* Console on SMC1 */
#define	SMC_INDEX	0
#define PROFF_SMC	PROFF_SMC1
#define CPM_CR_CH_SMC	CPM_CR_CH_SMC1
#define IOPINS		0xc0

#elif defined(CONFIG_8xx_CONS_SMC2)	/* Console on SMC2 */
#define SMC_INDEX	1
#define PROFF_SMC	PROFF_SMC2
#define CPM_CR_CH_SMC	CPM_CR_CH_SMC2
#define IOPINS		0xc00

#endif /* CONFIG_8xx_CONS_SMCx */

struct serialbuffer {
	cbd_t	rxbd;		/* Rx BD */
	cbd_t	txbd;		/* Tx BD */
	uint	rxindex;	/* index for next character to read */
	uchar	rxbuf[CONFIG_SYS_SMC_RXBUFLEN];/* rx buffers */
	uchar	txbuf;	/* tx buffers */
};

static void serial_setdivisor(cpm8xx_t __iomem *cp)
{
	int divisor = (gd->cpu_clk + 8 * gd->baudrate) / 16 / gd->baudrate;

	if (divisor / 16 > 0x1000) {
		/* bad divisor, assume 50MHz clock and 9600 baud */
		divisor = (50 * 1000 * 1000 + 8 * 9600) / 16 / 9600;
	}

	divisor /= CONFIG_SYS_BRGCLK_PRESCALE;

	if (divisor <= 0x1000)
		out_be32(&cp->cp_brgc1, ((divisor - 1) << 1) | CPM_BRG_EN);
	else
		out_be32(&cp->cp_brgc1, ((divisor / 16 - 1) << 1) | CPM_BRG_EN |
			 CPM_BRG_DIV16);
}

/*
 * Minimal serial functions needed to use one of the SMC ports
 * as serial console interface.
 */

static void smc_setbrg(void)
{
	immap_t __iomem *im = (immap_t __iomem *)CONFIG_SYS_IMMR;
	cpm8xx_t __iomem *cp = &(im->im_cpm);

	/* Set up the baud rate generator.
	 * See 8xx_io/commproc.c for details.
	 *
	 * Wire BRG1 to SMCx
	 */

	out_be32(&cp->cp_simode, 0);

	serial_setdivisor(cp);
}

static int smc_init(void)
{
	immap_t __iomem *im = (immap_t __iomem *)CONFIG_SYS_IMMR;
	smc_t __iomem *sp;
	smc_uart_t __iomem *up;
	cpm8xx_t __iomem *cp = &(im->im_cpm);
	struct serialbuffer __iomem *rtx;

	/* initialize pointers to SMC */

	sp = cp->cp_smc + SMC_INDEX;
	up = (smc_uart_t __iomem *)&cp->cp_dparam[PROFF_SMC];
	/* Disable relocation */
	out_be16(&up->smc_rpbase, 0);

	/* Disable transmitter/receiver. */
	clrbits_be16(&sp->smc_smcmr, SMCMR_REN | SMCMR_TEN);

	/* Enable SDMA. */
	out_be32(&im->im_siu_conf.sc_sdcr, 1);

	/* clear error conditions */
	out_8(&im->im_sdma.sdma_sdsr, CONFIG_SYS_SDSR);

	/* clear SDMA interrupt mask */
	out_8(&im->im_sdma.sdma_sdmr, CONFIG_SYS_SDMR);

	/* Use Port B for SMCx instead of other functions. */
	setbits_be32(&cp->cp_pbpar, IOPINS);
	clrbits_be32(&cp->cp_pbdir, IOPINS);
	clrbits_be16(&cp->cp_pbodr, IOPINS);

	/* Set the physical address of the host memory buffers in
	 * the buffer descriptors.
	 */
	rtx = (struct serialbuffer __iomem *)&cp->cp_dpmem[CPM_SERIAL_BASE];
	/* Allocate space for two buffer descriptors in the DP ram.
	 * For now, this address seems OK, but it may have to
	 * change with newer versions of the firmware.
	 * damm: allocating space after the two buffers for rx/tx data
	 */

	out_be32(&rtx->rxbd.cbd_bufaddr, (__force uint)&rtx->rxbuf);
	out_be16(&rtx->rxbd.cbd_sc, 0);

	out_be32(&rtx->txbd.cbd_bufaddr, (__force uint)&rtx->txbuf);
	out_be16(&rtx->txbd.cbd_sc, 0);

	/* Set up the uart parameters in the parameter ram. */
	out_be16(&up->smc_rbase, CPM_SERIAL_BASE);
	out_be16(&up->smc_tbase, CPM_SERIAL_BASE + sizeof(cbd_t));
	out_8(&up->smc_rfcr, SMC_EB);
	out_8(&up->smc_tfcr, SMC_EB);

	/* Set UART mode, 8 bit, no parity, one stop.
	 * Enable receive and transmit.
	 */
	out_be16(&sp->smc_smcmr, smcr_mk_clen(9) | SMCMR_SM_UART);

	/* Mask all interrupts and remove anything pending.
	*/
	out_8(&sp->smc_smcm, 0);
	out_8(&sp->smc_smce, 0xff);

	/* Set up the baud rate generator */
	smc_setbrg();

	/* Make the first buffer the only buffer. */
	setbits_be16(&rtx->txbd.cbd_sc, BD_SC_WRAP);
	setbits_be16(&rtx->rxbd.cbd_sc, BD_SC_EMPTY | BD_SC_WRAP);

	/* single/multi character receive. */
	out_be16(&up->smc_mrblr, CONFIG_SYS_SMC_RXBUFLEN);
	out_be16(&up->smc_maxidl, CONFIG_SYS_MAXIDLE);
	out_be32(&rtx->rxindex, 0);

	/* Initialize Tx/Rx parameters.	*/
	while (in_be16(&cp->cp_cpcr) & CPM_CR_FLG)	/* wait if cp is busy */
		;

	out_be16(&cp->cp_cpcr,
		 mk_cr_cmd(CPM_CR_CH_SMC, CPM_CR_INIT_TRX) | CPM_CR_FLG);

	while (in_be16(&cp->cp_cpcr) & CPM_CR_FLG)	/* wait if cp is busy */
		;

	/* Enable transmitter/receiver.	*/
	setbits_be16(&sp->smc_smcmr, SMCMR_REN | SMCMR_TEN);

	return 0;
}

static void smc_putc(const char c)
{
	immap_t	__iomem *im = (immap_t __iomem *)CONFIG_SYS_IMMR;
	cpm8xx_t	__iomem *cpmp = &(im->im_cpm);
	struct serialbuffer	__iomem *rtx;

	if (c == '\n')
		smc_putc('\r');

	rtx = (struct serialbuffer __iomem *)&cpmp->cp_dpmem[CPM_SERIAL_BASE];

	/* Wait for last character to go. */
	out_8(&rtx->txbuf, c);
	out_be16(&rtx->txbd.cbd_datlen, 1);
	setbits_be16(&rtx->txbd.cbd_sc, BD_SC_READY);

	while (in_be16(&rtx->txbd.cbd_sc) & BD_SC_READY)
		WATCHDOG_RESET();
}

static void smc_puts(const char *s)
{
	while (*s)
		smc_putc(*s++);
}

static int smc_getc(void)
{
	immap_t	__iomem *im = (immap_t __iomem *)CONFIG_SYS_IMMR;
	cpm8xx_t	__iomem *cpmp = &(im->im_cpm);
	struct serialbuffer	__iomem *rtx;
	unsigned char  c;
	uint rxindex;

	rtx = (struct serialbuffer __iomem *)&cpmp->cp_dpmem[CPM_SERIAL_BASE];

	/* Wait for character to show up. */
	while (in_be16(&rtx->rxbd.cbd_sc) & BD_SC_EMPTY)
		WATCHDOG_RESET();

	/* the characters are read one by one,
	 * use the rxindex to know the next char to deliver
	 */
	rxindex = in_be32(&rtx->rxindex);
	c = in_8(rtx->rxbuf + rxindex);
	rxindex++;

	/* check if all char are readout, then make prepare for next receive */
	if (rxindex >= in_be16(&rtx->rxbd.cbd_datlen)) {
		rxindex = 0;
		setbits_be16(&rtx->rxbd.cbd_sc, BD_SC_EMPTY);
	}
	out_be32(&rtx->rxindex, rxindex);
	return c;
}

static int smc_tstc(void)
{
	immap_t	__iomem *im = (immap_t __iomem *)CONFIG_SYS_IMMR;
	cpm8xx_t	__iomem *cpmp = &(im->im_cpm);
	struct serialbuffer	__iomem *rtx;

	rtx = (struct serialbuffer __iomem *)&cpmp->cp_dpmem[CPM_SERIAL_BASE];

	return !(in_be16(&rtx->rxbd.cbd_sc) & BD_SC_EMPTY);
}

struct serial_device serial_smc_device = {
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
