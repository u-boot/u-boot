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
 * MPC5xx CPM SPI interface.
 *
 * Parts of this code are probably not portable and/or specific to
 * the board which I used for the tests. Please send fixes/complaints
 * to wd@denx.de
 *
 * Ported to MPC5xx
 * Copyright (c) 2003 Denis Peter, MPL AG Switzerland, d.petr@mpl.ch.
 */

#include <common.h>
#include <mpc5xx.h>
#include <asm/5xx_immap.h>
#include <linux/ctype.h>
#include <malloc.h>
#include <post.h>
#include <net.h>

#if defined(CONFIG_SPI)

#undef	DEBUG

#define SPI_EEPROM_WREN		0x06
#define SPI_EEPROM_RDSR		0x05
#define SPI_EEPROM_READ		0x03
#define SPI_EEPROM_WRITE	0x02


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
	int i;

	volatile immap_t *immr;
	volatile qsmcm5xx_t *qsmcm;

	immr = (immap_t *)  CONFIG_SYS_IMMR;
	qsmcm = (qsmcm5xx_t *)&immr->im_qsmcm;

	qsmcm->qsmcm_qsmcr = 0; /* all accesses enabled */
	qsmcm->qsmcm_qspi_il = 0; /* lowest IRQ */

	/* --------------------------------------------
	 * GPIO or per. Function
	 * PQSPAR[00] = 0 reserved
	 * PQSPAR[01] = 1 [0x4000] -> PERI: (SPICS3)
	 * PQSPAR[02] = 0 [0x0000] -> GPIO
	 * PQSPAR[03] = 0 [0x0000] -> GPIO
	 * PQSPAR[04] = 1 [0x0800] -> PERI: (SPICS0)
	 * PQSPAR[05] = 0 reseved
	 * PQSPAR[06] = 1 [0x0200] -> PERI: (SPIMOSI)
	 * PQSPAR[07] = 1 [0x0100] -> PERI: (SPIMISO)
	 * -------------------------------------------- */
	qsmcm->qsmcm_pqspar =  0x3 | (CONFIG_SYS_SPI_CS_USED << 3);

	 /* --------------------------------------------
	 * DDRQS[00] = 0 reserved
	 * DDRQS[01] = 1 [0x0040] -> SPICS3 Output
	 * DDRQS[02] = 0 [0x0000] -> GPIO Output
	 * DDRQS[03] = 0 [0x0000] -> GPIO Output
	 * DDRQS[04] = 1 [0x0008] -> SPICS0 Output
	 * DDRQS[05] = 1 [0x0004] -> SPICLK Output
	 * DDRQS[06] = 1 [0x0002] -> SPIMOSI Output
	 * DDRQS[07] = 0 [0x0001] -> SPIMISO Input
	 * -------------------------------------------- */
	qsmcm->qsmcm_ddrqs = 0x7E;
	 /* --------------------------------------------
	 * Base state for used SPI CS pins, if base = 0 active must be 1
	 * PORTQS[00] = 0 reserved
	 * PORTQS[01] = 0 reserved
	 * PORTQS[02] = 0 reserved
	 * PORTQS[03] = 0 reserved
	 * PORTQS[04] = 0 [0x0000] RxD2
	 * PORTQS[05] = 1 [0x0400] TxD2
	 * PORTQS[06] = 0 [0x0000] RxD1
	 * PORTQS[07] = 1 [0x0100] TxD1
	 * PORTQS[08] = 0 reserved
	 * PORTQS[09] = 0 [0x0000] -> SPICS3 Base Output
	 * PORTQS[10] = 0 [0x0000] -> SPICS2 Base Output
	 * PORTQS[11] = 0 [0x0000] -> SPICS1 Base Output
	 * PORTQS[12] = 0 [0x0000] -> SPICS0 Base Output
	 * PORTQS[13] = 0 [0x0004] -> SPICLK Output
	 * PORTQS[14] = 0 [0x0002] -> SPIMOSI Output
	 * PORTQS[15] = 0 [0x0001] -> SPIMISO Input
	 * -------------------------------------------- */
	qsmcm->qsmcm_portqs |= (CONFIG_SYS_SPI_CS_BASE << 3);
	/* --------------------------------------------
	 * Controll Register 0
	 * SPCR0[00] = 1 (0x8000) Master
	 * SPCR0[01] = 0 (0x0000) Wired-Or
	 * SPCR0[2..5] = (0x2000) Bits per transfer (default 8)
	 * SPCR0[06] = 0 (0x0000) Normal polarity
	 * SPCR0[07] = 0 (0x0000) Normal Clock Phase
	 * SPCR0[08..15] = 14 1.4MHz
	 */
	qsmcm->qsmcm_spcr0=0xA00E;
	/* --------------------------------------------
	 * Controll Register 1
	 * SPCR1[00] = 0 (0x0000) QSPI enabled
	 * SPCR1[1..7] =  (0x7F00) Delay before Transfer
	 * SPCR1[8..15] = (0x0000) Delay After transfer (204.8usec@40MHz)
	 */
	qsmcm->qsmcm_spcr1=0x7F00;
	/* --------------------------------------------
	 * Controll Register 2
	 * SPCR2[00] = 0 (0x0000) SPI IRQs Disabeld
	 * SPCR2[01] = 0 (0x0000) No Wrap around
	 * SPCR2[02] = 0 (0x0000) Wrap to 0
	 * SPCR2[3..7] = (0x0000) End Queue pointer = 0
	 * SPCR2[8..10] = 0 (0x0000) reserved
	 * SPCR2[11..15] = 0 (0x0000) NewQueue Address = 0
	 */
	qsmcm->qsmcm_spcr2=0x0000;
	/* --------------------------------------------
	 * Controll Register 3
	 * SPCR3[00..04] = 0 (0x0000) reserved
	 * SPCR3[05] = 0 (0x0000) Feedback disabled
	 * SPCR3[06] = 0 (0x0000) IRQ on HALTA & MODF disabled
	 * SPCR3[07] = 0 (0x0000) Not halted
	 */
	qsmcm->qsmcm_spcr3=0x00;
	/* --------------------------------------------
	 * SPSR (Controll Register 3) Read only/ reset Flags 08,09,10
	 * SPCR3[08] = 1 (0x80) QSPI finished
	 * SPCR3[09] = 1 (0x40) Mode Fault Flag
	 * SPCR3[10] = 1 (0x20) HALTA
	 * SPCR3[11..15] = 0 (0x0000) Last executed command
	 */
	qsmcm->qsmcm_spsr=0xE0;
	/*-------------------------------------------
	 * Setup RAM
	 */
	for(i=0;i<32;i++) {
		 qsmcm->qsmcm_recram[i]=0x0000;
		 qsmcm->qsmcm_tranram[i]=0x0000;
		 qsmcm->qsmcm_comdram[i]=0x00;
	}
	return;
}

/* **************************************************************************
 *
 *  Function:    spi_init_r
 *  Dummy, all initializations have been done in spi_init_r
 * *********************************************************************** */
void spi_init_r (void)
{
	return;

}

/****************************************************************************
 *  Function:    spi_write
 **************************************************************************** */
ssize_t short_spi_write (uchar *addr, int alen, uchar *buffer, int len)
{
	int i,dlen;
	volatile immap_t *immr;
	volatile qsmcm5xx_t *qsmcm;

	immr = (immap_t *)  CONFIG_SYS_IMMR;
	qsmcm = (qsmcm5xx_t *)&immr->im_qsmcm;
	for(i=0;i<32;i++) {
		 qsmcm->qsmcm_recram[i]=0x0000;
		 qsmcm->qsmcm_tranram[i]=0x0000;
		 qsmcm->qsmcm_comdram[i]=0x00;
	}
	qsmcm->qsmcm_tranram[0] =  SPI_EEPROM_WREN; /* write enable */
	spi_xfer(1);
	i=0;
	qsmcm->qsmcm_tranram[i++] =  SPI_EEPROM_WRITE; /* WRITE memory array */
	qsmcm->qsmcm_tranram[i++] =  addr[0];
	qsmcm->qsmcm_tranram[i++] =  addr[1];

	for(dlen=0;dlen<len;dlen++) {
		qsmcm->qsmcm_tranram[i+dlen] = buffer[dlen]; /* WRITE memory array */
	}
	/* transmit it */
	spi_xfer(i+dlen);
	/* ignore received data	*/
	for (i = 0; i < 1000; i++) {
		qsmcm->qsmcm_tranram[0] =  SPI_EEPROM_RDSR; /* read status */
		qsmcm->qsmcm_tranram[1] = 0;
		spi_xfer(2);
		if (!(qsmcm->qsmcm_recram[1] & 1)) {
			break;
		}
		udelay(1000);
	}
	if (i >= 1000) {
		printf ("*** spi_write: Time out while writing!\n");
	}
	return len;
}

#define TRANSFER_LEN 16

ssize_t spi_write (uchar *addr, int alen, uchar *buffer, int len)
{
	int index,i,newlen;
	uchar newaddr[2];
	int curraddr;

	curraddr=(addr[alen-2]<<8)+addr[alen-1];
	i=len;
	index=0;
	do {
		newaddr[1]=(curraddr & 0xff);
		newaddr[0]=((curraddr>>8) & 0xff);
		if(i>TRANSFER_LEN) {
			newlen=TRANSFER_LEN;
			i-=TRANSFER_LEN;
		}
		else {
			newlen=i;
			i=0;
		}
		short_spi_write (newaddr, 2, &buffer[index], newlen);
		index+=newlen;
		curraddr+=newlen;
	}while(i);
	return (len);
}

/****************************************************************************
 *  Function:    spi_read
 **************************************************************************** */
ssize_t short_spi_read (uchar *addr, int alen, uchar *buffer, int len)
{
	int i;
	volatile immap_t *immr;
	volatile qsmcm5xx_t *qsmcm;

	immr = (immap_t *)  CONFIG_SYS_IMMR;
	qsmcm = (qsmcm5xx_t *)&immr->im_qsmcm;

	for(i=0;i<32;i++) {
		 qsmcm->qsmcm_recram[i]=0x0000;
		 qsmcm->qsmcm_tranram[i]=0x0000;
		 qsmcm->qsmcm_comdram[i]=0x00;
	}
	i=0;
	qsmcm->qsmcm_tranram[i++] = (SPI_EEPROM_READ); /* READ memory array */
	qsmcm->qsmcm_tranram[i++] = addr[0] & 0xff;
	qsmcm->qsmcm_tranram[i++] = addr[1] & 0xff;
	spi_xfer(3 + len);
	for(i=0;i<len;i++) {
		*buffer++=(char)qsmcm->qsmcm_recram[i+3];
	}
	return len;
}

ssize_t spi_read (uchar *addr, int alen, uchar *buffer, int len)
{
	int index,i,newlen;
	uchar newaddr[2];
	int curraddr;

	curraddr=(addr[alen-2]<<8)+addr[alen-1];
	i=len;
	index=0;
	do {
		newaddr[1]=(curraddr & 0xff);
		newaddr[0]=((curraddr>>8) & 0xff);
		if(i>TRANSFER_LEN) {
			newlen=TRANSFER_LEN;
			i-=TRANSFER_LEN;
		}
		else {
			newlen=i;
			i=0;
		}
		short_spi_read (newaddr, 2, &buffer[index], newlen);
		index+=newlen;
		curraddr+=newlen;
	}while(i);
	return (len);
}

/****************************************************************************
 *  Function:    spi_xfer
 **************************************************************************** */
ssize_t spi_xfer (size_t count)
{
	volatile immap_t *immr;
	volatile qsmcm5xx_t *qsmcm;
	int i;
	int tm;
	ushort status;
	immr = (immap_t *)  CONFIG_SYS_IMMR;
	qsmcm = (qsmcm5xx_t *)&immr->im_qsmcm;
	DPRINT (("*** spi_xfer entered count %d***\n",count));

	/* Set CS for device */
	for(i=0;i<(count-1);i++)
		qsmcm->qsmcm_comdram[i] = 0x80 | CONFIG_SYS_SPI_CS_ACT;  /* CS3 is connected to the SPI EEPROM */

	qsmcm->qsmcm_comdram[i] = CONFIG_SYS_SPI_CS_ACT; /* CS3 is connected to the SPI EEPROM */
	qsmcm->qsmcm_spcr2=((count-1)&0x1F)<<8;

	DPRINT (("*** spi_xfer: Bytes to be xferred: %d ***\n", count));

	qsmcm->qsmcm_spsr=0xE0; /* clear all flags */

	/* start spi transfer */
	DPRINT (("*** spi_xfer: Performing transfer ...\n"));
	qsmcm->qsmcm_spcr1 |= 0x8000;		/* Start transmit */

	/* --------------------------------
	 * Wait for SPI transmit to get out
	 * or time out (1 second = 1000 ms)
	 * -------------------------------- */
	for (tm=0; tm<1000; ++tm) {
		status=qsmcm->qsmcm_spcr1;
		if((status & 0x8000)==0)
			break;
		udelay (1000);
	}
	if (tm >= 1000) {
		printf ("*** spi_xfer: Time out while xferring to/from SPI!\n");
	}
#ifdef	DEBUG
	printf ("\nspi_xfer: txbuf after xfer\n");
	memdump ((void *) qsmcm->qsmcm_tranram, 32);	/* dump of txbuf before transmit */
	printf ("spi_xfer: rxbuf after xfer\n");
	memdump ((void *) qsmcm->qsmcm_recram, 32);	/* dump of rxbuf after transmit */
	printf ("\nspi_xfer: commbuf after xfer\n");
	memdump ((void *) qsmcm->qsmcm_comdram, 32);	/* dump of txbuf before transmit */
	printf ("\n");
#endif

	return count;
}

#endif	/* CONFIG_SPI  */
