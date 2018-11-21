// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2001 Navin Boppuri / Prashant Patel
 *	<nboppuri@trinetcommunication.com>,
 *	<pmpatel@trinetcommunication.com>
 * Copyright (c) 2001 Gerd Mennchen <Gerd.Mennchen@icn.siemens.de>
 * Copyright (c) 2001 Wolfgang Denk, DENX Software Engineering, <wd@denx.de>.
 */

/*
 * MPC8xx CPM SPI interface.
 *
 * Parts of this code are probably not portable and/or specific to
 * the board which I used for the tests. Please send fixes/complaints
 * to wd@denx.de
 *
 */

#include <common.h>
#include <mpc8xx.h>
#include <asm/cpm_8xx.h>
#include <linux/ctype.h>
#include <malloc.h>
#include <post.h>
#include <serial.h>

#define SPI_EEPROM_WREN		0x06
#define SPI_EEPROM_RDSR		0x05
#define SPI_EEPROM_READ		0x03
#define SPI_EEPROM_WRITE	0x02

/* ---------------------------------------------------------------
 * Offset for initial SPI buffers in DPRAM:
 * We need a 520 byte scratch DPRAM area to use at an early stage.
 * It is used between the two initialization calls (spi_init_f()
 * and spi_init_r()).
 * The value 0xb00 makes it far enough from the start of the data
 * area (as well as from the stack pointer).
 * --------------------------------------------------------------- */
#ifndef	CONFIG_SYS_SPI_INIT_OFFSET
#define	CONFIG_SYS_SPI_INIT_OFFSET	0xB00
#endif

#define CPM_SPI_BASE_RX	CPM_SPI_BASE
#define CPM_SPI_BASE_TX	(CPM_SPI_BASE + sizeof(cbd_t))

/* -------------------
 * Function prototypes
 * ------------------- */
ssize_t spi_xfer(size_t);

/* -------------------
 * Variables
 * ------------------- */

#define MAX_BUFFER	0x104

/* ----------------------------------------------------------------------
 * Initially we place the RX and TX buffers at a fixed location in DPRAM!
 * ---------------------------------------------------------------------- */
static uchar *rxbuf =
	(uchar *)&((cpm8xx_t *)&((immap_t *)CONFIG_SYS_IMMR)->im_cpm)->cp_dpmem
			[CONFIG_SYS_SPI_INIT_OFFSET];
static uchar *txbuf =
	(uchar *)&((cpm8xx_t *)&((immap_t *)CONFIG_SYS_IMMR)->im_cpm)->cp_dpmem
			[CONFIG_SYS_SPI_INIT_OFFSET+MAX_BUFFER];

/* **************************************************************************
 *
 *  Function:    spi_init_f
 *
 *  Description: Init SPI-Controller (ROM part)
 *
 *  return:      ---
 *
 * *********************************************************************** */
void spi_init_f(void)
{
	immap_t __iomem *immr = (immap_t __iomem *)CONFIG_SYS_IMMR;
	cpm8xx_t __iomem *cp = &immr->im_cpm;
	spi_t __iomem *spi = (spi_t __iomem *)&cp->cp_dparam[PROFF_SPI];
	cbd_t __iomem *tbdf, *rbdf;

	/* Disable relocation */
	out_be16(&spi->spi_rpbase, 0);

/* 1 */
	/* ------------------------------------------------
	 * Initialize Port B SPI pins -> page 34-8 MPC860UM
	 * (we are only in Master Mode !)
	 * ------------------------------------------------ */

	/* --------------------------------------------
	 * GPIO or per. Function
	 * PBPAR[28] = 1 [0x00000008] -> PERI: (SPIMISO)
	 * PBPAR[29] = 1 [0x00000004] -> PERI: (SPIMOSI)
	 * PBPAR[30] = 1 [0x00000002] -> PERI: (SPICLK)
	 * PBPAR[31] = 0 [0x00000001] -> GPIO: (CS for PCUE/CCM-EEPROM)
	 * -------------------------------------------- */
	clrsetbits_be32(&cp->cp_pbpar, 0x00000001, 0x0000000E);	/* set  bits */

	/* ----------------------------------------------
	 * In/Out or per. Function 0/1
	 * PBDIR[28] = 1 [0x00000008] -> PERI1: SPIMISO
	 * PBDIR[29] = 1 [0x00000004] -> PERI1: SPIMOSI
	 * PBDIR[30] = 1 [0x00000002] -> PERI1: SPICLK
	 * PBDIR[31] = 1 [0x00000001] -> GPIO OUT: CS for PCUE/CCM-EEPROM
	 * ---------------------------------------------- */
	setbits_be32(&cp->cp_pbdir, 0x0000000F);

	/* ----------------------------------------------
	 * open drain or active output
	 * PBODR[28] = 1 [0x00000008] -> open drain: SPIMISO
	 * PBODR[29] = 0 [0x00000004] -> active output SPIMOSI
	 * PBODR[30] = 0 [0x00000002] -> active output: SPICLK
	 * PBODR[31] = 0 [0x00000001] -> active output GPIO OUT: CS for PCUE/CCM
	 * ---------------------------------------------- */

	clrsetbits_be16(&cp->cp_pbodr, 0x00000007, 0x00000008);

	/* Initialize the parameter ram.
	 * We need to make sure many things are initialized to zero
	 */
	out_be32(&spi->spi_rstate, 0);
	out_be32(&spi->spi_rdp, 0);
	out_be16(&spi->spi_rbptr, 0);
	out_be16(&spi->spi_rbc, 0);
	out_be32(&spi->spi_rxtmp, 0);
	out_be32(&spi->spi_tstate, 0);
	out_be32(&spi->spi_tdp, 0);
	out_be16(&spi->spi_tbptr, 0);
	out_be16(&spi->spi_tbc, 0);
	out_be32(&spi->spi_txtmp, 0);

/* 3 */
	/* Set up the SPI parameters in the parameter ram */
	out_be16(&spi->spi_rbase, CPM_SPI_BASE_RX);
	out_be16(&spi->spi_tbase, CPM_SPI_BASE_TX);

	/***********IMPORTANT******************/

	/*
	 * Setting transmit and receive buffer descriptor pointers
	 * initially to rbase and tbase. Only the microcode patches
	 * documentation talks about initializing this pointer. This
	 * is missing from the sample I2C driver. If you dont
	 * initialize these pointers, the kernel hangs.
	 */
	out_be16(&spi->spi_rbptr, CPM_SPI_BASE_RX);
	out_be16(&spi->spi_tbptr, CPM_SPI_BASE_TX);

/* 4 */
	/* Init SPI Tx + Rx Parameters */
	while (in_be16(&cp->cp_cpcr) & CPM_CR_FLG)
		;

	out_be16(&cp->cp_cpcr, mk_cr_cmd(CPM_CR_CH_SPI, CPM_CR_INIT_TRX) |
			       CPM_CR_FLG);
	while (in_be16(&cp->cp_cpcr) & CPM_CR_FLG)
		;

/* 5 */
	/* Set SDMA configuration register */
	out_be32(&immr->im_siu_conf.sc_sdcr, 0x0001);

/* 6 */
	/* Set to big endian. */
	out_8(&spi->spi_tfcr, SMC_EB);
	out_8(&spi->spi_rfcr, SMC_EB);

/* 7 */
	/* Set maximum receive size. */
	out_be16(&spi->spi_mrblr, MAX_BUFFER);

/* 8 + 9 */
	/* tx and rx buffer descriptors */
	tbdf = (cbd_t __iomem *)&cp->cp_dpmem[CPM_SPI_BASE_TX];
	rbdf = (cbd_t __iomem *)&cp->cp_dpmem[CPM_SPI_BASE_RX];

	clrbits_be16(&tbdf->cbd_sc, BD_SC_READY);
	clrbits_be16(&rbdf->cbd_sc, BD_SC_EMPTY);

	/* Set the bd's rx and tx buffer address pointers */
	out_be32(&rbdf->cbd_bufaddr, (ulong)rxbuf);
	out_be32(&tbdf->cbd_bufaddr, (ulong)txbuf);

/* 10 + 11 */
	out_8(&cp->cp_spim, 0);			/* Mask  all SPI events */
	out_8(&cp->cp_spie, SPI_EMASK);		/* Clear all SPI events	*/

	return;
}

/* **************************************************************************
 *
 *  Function:    spi_init_r
 *
 *  Description: Init SPI-Controller (RAM part) -
 *		 The malloc engine is ready and we can move our buffers to
 *		 normal RAM
 *
 *  return:      ---
 *
 * *********************************************************************** */
void spi_init_r(void)
{
	immap_t __iomem *immr = (immap_t __iomem *)CONFIG_SYS_IMMR;
	cpm8xx_t __iomem *cp = &immr->im_cpm;
	spi_t __iomem *spi = (spi_t __iomem *)&cp->cp_dparam[PROFF_SPI];
	cbd_t __iomem *tbdf, *rbdf;

	/* Disable relocation */
	out_be16(&spi->spi_rpbase, 0);

	/* tx and rx buffer descriptors */
	tbdf = (cbd_t __iomem *)&cp->cp_dpmem[CPM_SPI_BASE_TX];
	rbdf = (cbd_t __iomem *)&cp->cp_dpmem[CPM_SPI_BASE_RX];

	/* Allocate memory for RX and TX buffers */
	rxbuf = (uchar *)malloc(MAX_BUFFER);
	txbuf = (uchar *)malloc(MAX_BUFFER);

	out_be32(&rbdf->cbd_bufaddr, (ulong)rxbuf);
	out_be32(&tbdf->cbd_bufaddr, (ulong)txbuf);

	return;
}

/****************************************************************************
 *  Function:    spi_write
 **************************************************************************** */
ssize_t spi_write(uchar *addr, int alen, uchar *buffer, int len)
{
	int i;

	memset(rxbuf, 0, MAX_BUFFER);
	memset(txbuf, 0, MAX_BUFFER);
	*txbuf = SPI_EEPROM_WREN;		/* write enable		*/
	spi_xfer(1);
	memcpy(txbuf, addr, alen);
	*txbuf = SPI_EEPROM_WRITE;		/* WRITE memory array	*/
	memcpy(alen + txbuf, buffer, len);
	spi_xfer(alen + len);
						/* ignore received data	*/
	for (i = 0; i < 1000; i++) {
		*txbuf = SPI_EEPROM_RDSR;	/* read status		*/
		txbuf[1] = 0;
		spi_xfer(2);
		if (!(rxbuf[1] & 1))
			break;
		udelay(1000);
	}
	if (i >= 1000)
		printf("*** spi_write: Time out while writing!\n");

	return len;
}

/****************************************************************************
 *  Function:    spi_read
 **************************************************************************** */
ssize_t spi_read(uchar *addr, int alen, uchar *buffer, int len)
{
	memset(rxbuf, 0, MAX_BUFFER);
	memset(txbuf, 0, MAX_BUFFER);
	memcpy(txbuf, addr, alen);
	*txbuf = SPI_EEPROM_READ;		/* READ memory array	*/

	/*
	 * There is a bug in 860T (?) that cuts the last byte of input
	 * if we're reading into DPRAM. The solution we choose here is
	 * to always read len+1 bytes (we have one extra byte at the
	 * end of the buffer).
	 */
	spi_xfer(alen + len + 1);
	memcpy(buffer, alen + rxbuf, len);

	return len;
}

/****************************************************************************
 *  Function:    spi_xfer
 **************************************************************************** */
ssize_t spi_xfer(size_t count)
{
	immap_t __iomem *immr = (immap_t __iomem *)CONFIG_SYS_IMMR;
	cpm8xx_t __iomem *cp = &immr->im_cpm;
	spi_t __iomem *spi = (spi_t __iomem *)&cp->cp_dparam[PROFF_SPI];
	cbd_t __iomem *tbdf, *rbdf;
	int tm;

	/* Disable relocation */
	out_be16(&spi->spi_rpbase, 0);

	tbdf = (cbd_t __iomem *)&cp->cp_dpmem[CPM_SPI_BASE_TX];
	rbdf = (cbd_t __iomem *)&cp->cp_dpmem[CPM_SPI_BASE_RX];

	/* Set CS for device */
	clrbits_be32(&cp->cp_pbdat, 0x0001);

	/* Setting tx bd status and data length */
	out_be16(&tbdf->cbd_sc, BD_SC_READY | BD_SC_LAST | BD_SC_WRAP);
	out_be16(&tbdf->cbd_datlen, count);

	/* Setting rx bd status and data length */
	out_be16(&rbdf->cbd_sc, BD_SC_EMPTY | BD_SC_WRAP);
	out_be16(&rbdf->cbd_datlen, 0);	 /* rx length has no significance */

	clrsetbits_be16(&cp->cp_spmode, ~SPMODE_LOOP, SPMODE_REV | SPMODE_MSTR |
			SPMODE_EN | SPMODE_LEN(8) | SPMODE_PM(0x8));
	out_8(&cp->cp_spim, 0);		/* Mask  all SPI events */
	out_8(&cp->cp_spie, SPI_EMASK);	/* Clear all SPI events	*/

	/* start spi transfer */
	setbits_8(&cp->cp_spcom, SPI_STR);		/* Start transmit */

	/* --------------------------------
	 * Wait for SPI transmit to get out
	 * or time out (1 second = 1000 ms)
	 * -------------------------------- */
	for (tm = 0; tm < 1000; ++tm) {
		if (in_8(&cp->cp_spie) & SPI_TXB)	/* Tx Buffer Empty */
			break;
		if ((in_be16(&tbdf->cbd_sc) & BD_SC_READY) == 0)
			break;
		udelay(1000);
	}
	if (tm >= 1000)
		printf("*** spi_xfer: Time out while xferring to/from SPI!\n");

	/* Clear CS for device */
	setbits_be32(&cp->cp_pbdat, 0x0001);

	return count;
}
