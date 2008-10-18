/*
 * Copyright (c) 2001 Navin Boppuri / Prashant Patel
 *	<nboppuri@trinetcommunication.com>,
 *	<pmpatel@trinetcommunication.com>
 * Copyright (c) 2001 Gerd Mennchen <Gerd.Mennchen@icn.siemens.de>
 * Copyright (c) 2001-2003 Wolfgang Denk, DENX Software Engineering, <wd@denx.de>.
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

/*
 * MPC8260 CPM SPI interface.
 *
 * Parts of this code are probably not portable and/or specific to
 * the board which I used for the tests. Please send fixes/complaints
 * to wd@denx.de
 *
 */

#include <common.h>
#include <asm/cpm_8260.h>
#include <linux/ctype.h>
#include <malloc.h>
#include <post.h>
#include <net.h>

#if defined(CONFIG_SPI)

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
 * The value 0x2000 makes it far enough from the start of the data
 * area (as well as from the stack pointer).
 * --------------------------------------------------------------- */
#ifndef	CONFIG_SYS_SPI_INIT_OFFSET
#define	CONFIG_SYS_SPI_INIT_OFFSET	0x2000
#endif

#define CPM_SPI_BASE 0x100

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
  (uchar *)&((immap_t *)CONFIG_SYS_IMMR)->im_dprambase
			[CONFIG_SYS_SPI_INIT_OFFSET];
static uchar *txbuf =
  (uchar *)&((immap_t *)CONFIG_SYS_IMMR)->im_dprambase
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
	volatile cpm8260_t *cp;
	volatile cbd_t *tbdf, *rbdf;

	immr = (immap_t *)  CONFIG_SYS_IMMR;
	cp   = (cpm8260_t *) &immr->im_cpm;

	*(ushort *)(&immr->im_dprambase[PROFF_SPI_BASE]) = PROFF_SPI;
	spi  = (spi_t *)&immr->im_dprambase[PROFF_SPI];

/* 1 */
	/* ------------------------------------------------
	 * Initialize Port D SPI pins
	 * (we are only in Master Mode !)
	 * ------------------------------------------------ */

	/* --------------------------------------------
	 * GPIO or per. Function
	 * PPARD[16] = 1 [0x00008000] (SPIMISO)
	 * PPARD[17] = 1 [0x00004000] (SPIMOSI)
	 * PPARD[18] = 1 [0x00002000] (SPICLK)
	 * PPARD[12] = 0 [0x00080000] -> GPIO: (CS for ATC EEPROM)
	 * -------------------------------------------- */
	immr->im_ioport.iop_ppard |=  0x0000E000;	/* set  bits	*/
	immr->im_ioport.iop_ppard &= ~0x00080000;	/* reset bit	*/

	/* ----------------------------------------------
	 * In/Out or per. Function 0/1
	 * PDIRD[16] = 0 [0x00008000] -> PERI1: SPIMISO
	 * PDIRD[17] = 0 [0x00004000] -> PERI1: SPIMOSI
	 * PDIRD[18] = 0 [0x00002000] -> PERI1: SPICLK
	 * PDIRD[12] = 1 [0x00080000] -> GPIO OUT: CS for ATC EEPROM
	 * ---------------------------------------------- */
	immr->im_ioport.iop_pdird &= ~0x0000E000;
	immr->im_ioport.iop_pdird |= 0x00080000;

	/* ----------------------------------------------
	 * special option reg.
	 * PSORD[16] = 1 [0x00008000] -> SPIMISO
	 * PSORD[17] = 1 [0x00004000] -> SPIMOSI
	 * PSORD[18] = 1 [0x00002000] -> SPICLK
	 * ---------------------------------------------- */
	immr->im_ioport.iop_psord |= 0x0000E000;

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
	dpaddr = m8260_cpm_dpalloc (sizeof(cbd_t)*2, 8);
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
	/* Init SPI Tx + Rx Parameters */
	while (cp->cp_cpcr & CPM_CR_FLG)
		;
	cp->cp_cpcr = mk_cr_cmd(CPM_CR_SPI_PAGE, CPM_CR_SPI_SBLOCK,
							0, CPM_CR_INIT_TRX) | CPM_CR_FLG;
	while (cp->cp_cpcr & CPM_CR_FLG)
		;

/* 6 */
	/* Set to big endian. */
	spi->spi_tfcr = CPMFCR_EB;
	spi->spi_rfcr = CPMFCR_EB;

/* 7 */
	/* Set maximum receive size. */
	spi->spi_mrblr = MAX_BUFFER;

/* 8 + 9 */
	/* tx and rx buffer descriptors */
	tbdf = (cbd_t *) & immr->im_dprambase[spi->spi_tbase];
	rbdf = (cbd_t *) & immr->im_dprambase[spi->spi_rbase];

	tbdf->cbd_sc &= ~BD_SC_READY;
	rbdf->cbd_sc &= ~BD_SC_EMPTY;

	/* Set the bd's rx and tx buffer address pointers */
	rbdf->cbd_bufaddr = (ulong) rxbuf;
	tbdf->cbd_bufaddr = (ulong) txbuf;

/* 10 + 11 */
	immr->im_spi.spi_spie = SPI_EMASK;		/* Clear all SPI events	*/
	immr->im_spi.spi_spim = 0x00;			/* Mask  all SPI events */


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
	volatile spi_t *spi;
	volatile immap_t *immr;
	volatile cpm8260_t *cp;
	volatile cbd_t *tbdf, *rbdf;

	immr = (immap_t *)  CONFIG_SYS_IMMR;
	cp   = (cpm8260_t *) &immr->im_cpm;

	spi  = (spi_t *)&immr->im_dprambase[PROFF_SPI];

	/* tx and rx buffer descriptors */
	tbdf = (cbd_t *) & immr->im_dprambase[spi->spi_tbase];
	rbdf = (cbd_t *) & immr->im_dprambase[spi->spi_rbase];

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
	volatile cpm8260_t *cp;
	volatile spi_t *spi;
	cbd_t *tbdf, *rbdf;
	int tm;

	DPRINT (("*** spi_xfer entered ***\n"));

	immr = (immap_t *) CONFIG_SYS_IMMR;
	cp   = (cpm8260_t *) &immr->im_cpm;

	spi  = (spi_t *)&immr->im_dprambase[PROFF_SPI];

	tbdf = (cbd_t *) & immr->im_dprambase[spi->spi_tbase];
	rbdf = (cbd_t *) & immr->im_dprambase[spi->spi_rbase];

	/* Board-specific: Set CS for device (ATC EEPROM) */
	immr->im_ioport.iop_pdatd &= ~0x00080000;

	/* Setting tx bd status and data length */
	tbdf->cbd_sc  = BD_SC_READY | BD_SC_LAST | BD_SC_WRAP;
	tbdf->cbd_datlen = count;

	DPRINT (("*** spi_xfer: Bytes to be xferred: %d ***\n",
							tbdf->cbd_datlen));

	/* Setting rx bd status and data length */
	rbdf->cbd_sc = BD_SC_EMPTY | BD_SC_WRAP;
	rbdf->cbd_datlen = 0;	 /* rx length has no significance */

	immr->im_spi.spi_spmode = SPMODE_REV	|
			SPMODE_MSTR	|
			SPMODE_EN	|
			SPMODE_LEN(8)	|	/* 8 Bits per char */
			SPMODE_PM(0x8) ;	/* medium speed */
	immr->im_spi.spi_spie = SPI_EMASK;		/* Clear all SPI events	*/
	immr->im_spi.spi_spim = 0x00;			/* Mask  all SPI events */

	/* start spi transfer */
	DPRINT (("*** spi_xfer: Performing transfer ...\n"));
	immr->im_spi.spi_spcom |= SPI_STR;		/* Start transmit */

	/* --------------------------------
	 * Wait for SPI transmit to get out
	 * or time out (1 second = 1000 ms)
	 * -------------------------------- */
	for (tm=0; tm<1000; ++tm) {
		if (immr->im_spi.spi_spie & SPI_TXB) {	/* Tx Buffer Empty */
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
	immr->im_ioport.iop_pdatd |= 0x00080000;

	return count;
}
#endif	/* CONFIG_SPI */
