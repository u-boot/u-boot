/*
 * Copyright (c) 2001 Navin Boppuri / Prashant Patel
 *	<nboppuri@trinetcommunication.com>,
 *	<pmpatel@trinetcommunication.com>
 * Copyright (c) 2001 Gerd Mennchen <Gerd.Mennchen@icn.siemens.de>
 * Copyright (c) 2001 Wolfgang Denk, DENX Software Engineering, <wd@denx.de>.
 *
 * SPDX-License-Identifier:	GPL-2.0+
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
#include <commproc.h>
#include <linux/ctype.h>
#include <malloc.h>
#include <post.h>
#include <serial.h>

#if (defined(CONFIG_SPI)) || (CONFIG_POST & CONFIG_SYS_POST_SPI)

/* Warning:
 * You cannot enable DEBUG for early system initalization, i. e. when
 * this driver is used to read environment parameters like "baudrate"
 * from EEPROM which are used to initialize the serial port which is
 * needed to print the debug messages...
 */
#undef	DEBUG

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

#ifdef	DEBUG

#define	DPRINT(a)	printf a;
/* -----------------------------------------------
 * Helper functions to peek into tx and rx buffers
 * ----------------------------------------------- */
static const char * const hex_digit = "0123456789ABCDEF";

static char quickhex (int i)
{
	return hex_digit[i];
}

static void memdump (void *pv, int num)
{
	int i;
	unsigned char *pc = (unsigned char *) pv;

	for (i = 0; i < num; i++)
		printf ("%c%c ", quickhex (pc[i] >> 4), quickhex (pc[i] & 0x0f));
	printf ("\t");
	for (i = 0; i < num; i++)
		printf ("%c", isprint (pc[i]) ? pc[i] : '.');
	printf ("\n");
}
#else	/* !DEBUG */

#define	DPRINT(a)

#endif	/* DEBUG */

/* -------------------
 * Function prototypes
 * ------------------- */
void spi_init (void);

ssize_t spi_read (uchar *, int, uchar *, int);
ssize_t spi_write (uchar *, int, uchar *, int);
ssize_t spi_xfer (size_t);

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
void spi_init_f (void)
{
	unsigned int dpaddr;

	volatile spi_t *spi;
	volatile immap_t *immr;
	volatile cpm8xx_t *cp;
	volatile cbd_t *tbdf, *rbdf;

	immr = (immap_t *)  CONFIG_SYS_IMMR;
	cp   = (cpm8xx_t *) &immr->im_cpm;

#ifdef CONFIG_SYS_SPI_UCODE_PATCH
	spi  = (spi_t *)&cp->cp_dpmem[spi->spi_rpbase];
#else
	spi  = (spi_t *)&cp->cp_dparam[PROFF_SPI];
	/* Disable relocation */
	spi->spi_rpbase = 0;
#endif

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
	cp->cp_pbpar |=  0x0000000E;	/* set  bits	*/
	cp->cp_pbpar &= ~0x00000001;	/* reset bit	*/

	/* ----------------------------------------------
	 * In/Out or per. Function 0/1
	 * PBDIR[28] = 1 [0x00000008] -> PERI1: SPIMISO
	 * PBDIR[29] = 1 [0x00000004] -> PERI1: SPIMOSI
	 * PBDIR[30] = 1 [0x00000002] -> PERI1: SPICLK
	 * PBDIR[31] = 1 [0x00000001] -> GPIO OUT: CS for PCUE/CCM-EEPROM
	 * ---------------------------------------------- */
	cp->cp_pbdir |= 0x0000000F;

	/* ----------------------------------------------
	 * open drain or active output
	 * PBODR[28] = 1 [0x00000008] -> open drain: SPIMISO
	 * PBODR[29] = 0 [0x00000004] -> active output SPIMOSI
	 * PBODR[30] = 0 [0x00000002] -> active output: SPICLK
	 * PBODR[31] = 0 [0x00000001] -> active output: GPIO OUT: CS for PCUE/CCM
	 * ---------------------------------------------- */

	cp->cp_pbodr |=  0x00000008;
	cp->cp_pbodr &= ~0x00000007;

	/* Initialize the parameter ram.
	 * We need to make sure many things are initialized to zero
	 */
	spi->spi_rstate	= 0;
	spi->spi_rdp	= 0;
	spi->spi_rbptr	= 0;
	spi->spi_rbc	= 0;
	spi->spi_rxtmp	= 0;
	spi->spi_tstate	= 0;
	spi->spi_tdp	= 0;
	spi->spi_tbptr	= 0;
	spi->spi_tbc	= 0;
	spi->spi_txtmp	= 0;

	/* Allocate space for one transmit and one receive buffer
	 * descriptor in the DP ram
	 */
#ifdef CONFIG_SYS_ALLOC_DPRAM
	dpaddr = dpram_alloc_align (sizeof(cbd_t)*2, 8);
#else
	dpaddr = CPM_SPI_BASE;
#endif

/* 3 */
	/* Set up the SPI parameters in the parameter ram */
	spi->spi_rbase = dpaddr;
	spi->spi_tbase = dpaddr + sizeof (cbd_t);

	/***********IMPORTANT******************/

	/*
	 * Setting transmit and receive buffer descriptor pointers
	 * initially to rbase and tbase. Only the microcode patches
	 * documentation talks about initializing this pointer. This
	 * is missing from the sample I2C driver. If you dont
	 * initialize these pointers, the kernel hangs.
	 */
	spi->spi_rbptr = spi->spi_rbase;
	spi->spi_tbptr = spi->spi_tbase;

/* 4 */
#ifdef CONFIG_SYS_SPI_UCODE_PATCH
	/*
	 *  Initialize required parameters if using microcode patch.
	 */
	spi->spi_rstate = 0;
	spi->spi_tstate = 0;
#else
	/* Init SPI Tx + Rx Parameters */
	while (cp->cp_cpcr & CPM_CR_FLG)
		;
	cp->cp_cpcr = mk_cr_cmd(CPM_CR_CH_SPI, CPM_CR_INIT_TRX) | CPM_CR_FLG;
	while (cp->cp_cpcr & CPM_CR_FLG)
		;
#endif	/* CONFIG_SYS_SPI_UCODE_PATCH */

/* 5 */
	/* Set SDMA configuration register */
	immr->im_siu_conf.sc_sdcr = 0x0001;

/* 6 */
	/* Set to big endian. */
	spi->spi_tfcr = SMC_EB;
	spi->spi_rfcr = SMC_EB;

/* 7 */
	/* Set maximum receive size. */
	spi->spi_mrblr = MAX_BUFFER;

/* 8 + 9 */
	/* tx and rx buffer descriptors */
	tbdf = (cbd_t *) & cp->cp_dpmem[spi->spi_tbase];
	rbdf = (cbd_t *) & cp->cp_dpmem[spi->spi_rbase];

	tbdf->cbd_sc &= ~BD_SC_READY;
	rbdf->cbd_sc &= ~BD_SC_EMPTY;

	/* Set the bd's rx and tx buffer address pointers */
	rbdf->cbd_bufaddr = (ulong) rxbuf;
	tbdf->cbd_bufaddr = (ulong) txbuf;

/* 10 + 11 */
	cp->cp_spim = 0;			/* Mask  all SPI events */
	cp->cp_spie = SPI_EMASK;		/* Clear all SPI events	*/

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
void spi_init_r (void)
{
	volatile cpm8xx_t *cp;
	volatile spi_t *spi;
	volatile immap_t *immr;
	volatile cbd_t *tbdf, *rbdf;

	immr = (immap_t *)  CONFIG_SYS_IMMR;
	cp   = (cpm8xx_t *) &immr->im_cpm;

#ifdef CONFIG_SYS_SPI_UCODE_PATCH
	spi  = (spi_t *)&cp->cp_dpmem[spi->spi_rpbase];
#else
	spi  = (spi_t *)&cp->cp_dparam[PROFF_SPI];
	/* Disable relocation */
	spi->spi_rpbase = 0;
#endif

	/* tx and rx buffer descriptors */
	tbdf = (cbd_t *) & cp->cp_dpmem[spi->spi_tbase];
	rbdf = (cbd_t *) & cp->cp_dpmem[spi->spi_rbase];

	/* Allocate memory for RX and TX buffers */
	rxbuf = (uchar *) malloc (MAX_BUFFER);
	txbuf = (uchar *) malloc (MAX_BUFFER);

	rbdf->cbd_bufaddr = (ulong) rxbuf;
	tbdf->cbd_bufaddr = (ulong) txbuf;

	return;
}

/****************************************************************************
 *  Function:    spi_write
 **************************************************************************** */
ssize_t spi_write (uchar *addr, int alen, uchar *buffer, int len)
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
		if (!(rxbuf[1] & 1)) {
			break;
		}
		udelay(1000);
	}
	if (i >= 1000) {
		printf ("*** spi_write: Time out while writing!\n");
	}

	return len;
}

/****************************************************************************
 *  Function:    spi_read
 **************************************************************************** */
ssize_t spi_read (uchar *addr, int alen, uchar *buffer, int len)
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
ssize_t spi_xfer (size_t count)
{
	volatile immap_t *immr;
	volatile cpm8xx_t *cp;
	volatile spi_t *spi;
	cbd_t *tbdf, *rbdf;
	ushort loop;
	int tm;

	DPRINT (("*** spi_xfer entered ***\n"));

	immr = (immap_t *) CONFIG_SYS_IMMR;
	cp   = (cpm8xx_t *) &immr->im_cpm;

#ifdef CONFIG_SYS_SPI_UCODE_PATCH
	spi  = (spi_t *)&cp->cp_dpmem[spi->spi_rpbase];
#else
	spi  = (spi_t *)&cp->cp_dparam[PROFF_SPI];
	/* Disable relocation */
	spi->spi_rpbase = 0;
#endif

	tbdf = (cbd_t *) & cp->cp_dpmem[spi->spi_tbase];
	rbdf = (cbd_t *) & cp->cp_dpmem[spi->spi_rbase];

	/* Set CS for device */
	cp->cp_pbdat &= ~0x0001;

	/* Setting tx bd status and data length */
	tbdf->cbd_sc  = BD_SC_READY | BD_SC_LAST | BD_SC_WRAP;
	tbdf->cbd_datlen = count;

	DPRINT (("*** spi_xfer: Bytes to be xferred: %d ***\n",
							tbdf->cbd_datlen));

	/* Setting rx bd status and data length */
	rbdf->cbd_sc = BD_SC_EMPTY | BD_SC_WRAP;
	rbdf->cbd_datlen = 0;	 /* rx length has no significance */

	loop = cp->cp_spmode & SPMODE_LOOP;
	cp->cp_spmode = /*SPMODE_DIV16	|*/	/* BRG/16 mode not used here */
			loop		|
			SPMODE_REV	|
			SPMODE_MSTR	|
			SPMODE_EN	|
			SPMODE_LEN(8)	|	/* 8 Bits per char */
			SPMODE_PM(0x8) ;	/* medium speed */
	cp->cp_spim = 0;			/* Mask  all SPI events */
	cp->cp_spie = SPI_EMASK;		/* Clear all SPI events	*/

	/* start spi transfer */
	DPRINT (("*** spi_xfer: Performing transfer ...\n"));
	cp->cp_spcom |= SPI_STR;		/* Start transmit */

	/* --------------------------------
	 * Wait for SPI transmit to get out
	 * or time out (1 second = 1000 ms)
	 * -------------------------------- */
	for (tm=0; tm<1000; ++tm) {
		if (cp->cp_spie & SPI_TXB) {	/* Tx Buffer Empty */
			DPRINT (("*** spi_xfer: Tx buffer empty\n"));
			break;
		}
		if ((tbdf->cbd_sc & BD_SC_READY) == 0) {
			DPRINT (("*** spi_xfer: Tx BD done\n"));
			break;
		}
		udelay (1000);
	}
	if (tm >= 1000) {
		printf ("*** spi_xfer: Time out while xferring to/from SPI!\n");
	}
	DPRINT (("*** spi_xfer: ... transfer ended\n"));

#ifdef	DEBUG
	printf ("\nspi_xfer: txbuf after xfer\n");
	memdump ((void *) txbuf, 16);	/* dump of txbuf before transmit */
	printf ("spi_xfer: rxbuf after xfer\n");
	memdump ((void *) rxbuf, 16);	/* dump of rxbuf after transmit */
	printf ("\n");
#endif

	/* Clear CS for device */
	cp->cp_pbdat |= 0x0001;

	return count;
}
#endif	/* CONFIG_SPI || (CONFIG_POST & CONFIG_SYS_POST_SPI) */

/*
 * SPI test
 *
 * The Serial Peripheral Interface (SPI) is tested in the local loopback mode.
 * The interface is configured accordingly and several packets
 * are transfered. The configurable test parameters are:
 *   TEST_MIN_LENGTH - minimum size of packet to transfer
 *   TEST_MAX_LENGTH - maximum size of packet to transfer
 *   TEST_NUM - number of tests
 */

#if CONFIG_POST & CONFIG_SYS_POST_SPI

#define TEST_MIN_LENGTH		1
#define TEST_MAX_LENGTH		MAX_BUFFER
#define TEST_NUM		1

static void packet_fill (char * packet, int length)
{
	char c = (char) length;
	int i;

	for (i = 0; i < length; i++)
	{
	    packet[i] = c++;
	}
}

static int packet_check (char * packet, int length)
{
	char c = (char) length;
	int i;

	for (i = 0; i < length; i++) {
	    if (packet[i] != c++) return -1;
	}

	return 0;
}

int spi_post_test (int flags)
{
	int res = -1;
	volatile immap_t *immr = (immap_t *) CONFIG_SYS_IMMR;
	volatile cpm8xx_t *cp = (cpm8xx_t *) & immr->im_cpm;
	int i;
	int l;

	spi_init_f ();
	spi_init_r ();

	cp->cp_spmode |= SPMODE_LOOP;

	for (i = 0; i < TEST_NUM; i++) {
		for (l = TEST_MIN_LENGTH; l <= TEST_MAX_LENGTH; l += 8) {
			packet_fill ((char *)txbuf, l);

			spi_xfer (l);

			if (packet_check ((char *)rxbuf, l) < 0) {
				goto Done;
			}
		}
	}

	res = 0;

      Done:

	cp->cp_spmode &= ~SPMODE_LOOP;

	/*
	 * SCC2 parameter RAM space overlaps
	 * the SPI parameter RAM space. So we need to restore
	 * the SCC2 configuration if it is used by UART.
	 */

#if !defined(CONFIG_8xx_CONS_NONE)
	serial_reinit_all ();
#endif

	if (res != 0) {
		post_log ("SPI test failed\n");
	}

	return res;
}
#endif	/* CONFIG_POST & CONFIG_SYS_POST_SPI */
