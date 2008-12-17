/*
 * (C) Copyright 2001
 * John Clemens <clemens@mclx.com>, Mission Critical Linux, Inc.
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

/*************************************************************************
 * changes for Marvell DB64360 eval board 2003 by Ingo Assmus <ingo.assmus@keymile.com>
 *
  ************************************************************************/

/*
 * mpsc.c - driver for console over the MPSC.
 */


#include <common.h>
#include <config.h>
#include <asm/cache.h>

#include <malloc.h>
#include "mpsc.h"

#include "mv_regs.h"

#include "../include/memory.h"

DECLARE_GLOBAL_DATA_PTR;

/* Define this if you wish to use the MPSC as a register based UART.
 * This will force the serial port to not use the SDMA engine at all.
 */
#undef CONFIG_MPSC_DEBUG_PORT


int (*mpsc_putchar) (char ch) = mpsc_putchar_early;
char (*mpsc_getchar) (void) = mpsc_getchar_debug;
int (*mpsc_test_char) (void) = mpsc_test_char_debug;


static volatile unsigned int *rx_desc_base = NULL;
static unsigned int rx_desc_index = 0;
static volatile unsigned int *tx_desc_base = NULL;
static unsigned int tx_desc_index = 0;

/* local function declarations */
static int galmpsc_connect (int channel, int connect);
static int galmpsc_route_rx_clock (int channel, int brg);
static int galmpsc_route_tx_clock (int channel, int brg);
static int galmpsc_write_config_regs (int mpsc, int mode);
static int galmpsc_config_channel_regs (int mpsc);
static int galmpsc_set_char_length (int mpsc, int value);
static int galmpsc_set_stop_bit_length (int mpsc, int value);
static int galmpsc_set_parity (int mpsc, int value);
static int galmpsc_enter_hunt (int mpsc);
static int galmpsc_set_brkcnt (int mpsc, int value);
static int galmpsc_set_tcschar (int mpsc, int value);
static int galmpsc_set_snoop (int mpsc, int value);
static int galmpsc_shutdown (int mpsc);

static int galsdma_set_RFT (int channel);
static int galsdma_set_SFM (int channel);
static int galsdma_set_rxle (int channel);
static int galsdma_set_txle (int channel);
static int galsdma_set_burstsize (int channel, unsigned int value);
static int galsdma_set_RC (int channel, unsigned int value);

static int galbrg_set_CDV (int channel, int value);
static int galbrg_enable (int channel);
static int galbrg_disable (int channel);
static int galbrg_set_clksrc (int channel, int value);
static int galbrg_set_CUV (int channel, int value);

static void galsdma_enable_rx (void);
static int galsdma_set_mem_space (unsigned int memSpace,
				  unsigned int memSpaceTarget,
				  unsigned int memSpaceAttr,
				  unsigned int baseAddress,
				  unsigned int size);


#define SOFTWARE_CACHE_MANAGEMENT

#ifdef SOFTWARE_CACHE_MANAGEMENT
#define FLUSH_DCACHE(a,b)		 if(dcache_status()){clean_dcache_range((u32)(a),(u32)(b));}
#define FLUSH_AND_INVALIDATE_DCACHE(a,b) if(dcache_status()){flush_dcache_range((u32)(a),(u32)(b));}
#define INVALIDATE_DCACHE(a,b)		 if(dcache_status()){invalidate_dcache_range((u32)(a),(u32)(b));}
#else
#define FLUSH_DCACHE(a,b)
#define FLUSH_AND_INVALIDATE_DCACHE(a,b)
#define INVALIDATE_DCACHE(a,b)
#endif

#ifdef CONFIG_MPSC_DEBUG_PORT
static void mpsc_debug_init (void)
{

	volatile unsigned int temp;

	/* Clear the CFR  (CHR4) */
	/* Write random 'Z' bit (bit 29) of CHR4 to enable debug uart *UNDOCUMENTED FEATURE* */
	temp = GTREGREAD (GALMPSC_CHANNELREG_4 + (CHANNEL * GALMPSC_REG_GAP));
	temp &= 0xffffff00;
	temp |= BIT29;
	GT_REG_WRITE (GALMPSC_CHANNELREG_4 + (CHANNEL * GALMPSC_REG_GAP),
		      temp);

	/* Set the Valid bit 'V' (bit 12) and int generation bit 'INT' (bit 15) */
	temp = GTREGREAD (GALMPSC_CHANNELREG_5 + (CHANNEL * GALMPSC_REG_GAP));
	temp |= (BIT12 | BIT15);
	GT_REG_WRITE (GALMPSC_CHANNELREG_5 + (CHANNEL * GALMPSC_REG_GAP),
		      temp);

	/* Set int mask */
	temp = GTREGREAD (GALMPSC_0_INT_MASK);
	temp |= BIT6;
	GT_REG_WRITE (GALMPSC_0_INT_MASK, temp);
}
#endif

char mpsc_getchar_debug (void)
{
	volatile int temp;
	volatile unsigned int cause;

	cause = GTREGREAD (GALMPSC_0_INT_CAUSE);
	while ((cause & BIT6) == 0) {
		cause = GTREGREAD (GALMPSC_0_INT_CAUSE);
	}

	temp = GTREGREAD (GALMPSC_CHANNELREG_10 +
			  (CHANNEL * GALMPSC_REG_GAP));
	/* By writing 1's to the set bits, the register is cleared */
	GT_REG_WRITE (GALMPSC_CHANNELREG_10 + (CHANNEL * GALMPSC_REG_GAP),
		      temp);
	GT_REG_WRITE (GALMPSC_0_INT_CAUSE, cause & ~BIT6);
	return (temp >> 16) & 0xff;
}

/* special function for running out of flash.  doesn't modify any
 * global variables [josh] */
int mpsc_putchar_early (char ch)
{
	int mpsc = CHANNEL;
	int temp =
		GTREGREAD (GALMPSC_CHANNELREG_2 + (mpsc * GALMPSC_REG_GAP));
	galmpsc_set_tcschar (mpsc, ch);
	GT_REG_WRITE (GALMPSC_CHANNELREG_2 + (mpsc * GALMPSC_REG_GAP),
		      temp | 0x200);

#define MAGIC_FACTOR	(10*1000000)

	udelay (MAGIC_FACTOR / gd->baudrate);
	return 0;
}

/* This is used after relocation, see serial.c and mpsc_init2 */
static int mpsc_putchar_sdma (char ch)
{
	volatile unsigned int *p;
	unsigned int temp;


	/* align the descriptor */
	p = tx_desc_base;
	memset ((void *) p, 0, 8 * sizeof (unsigned int));

	/* fill one 64 bit buffer */
	/* word swap, pad with 0 */
	p[4] = 0;		/* x */
	p[5] = (unsigned int) ch;	/* x */

	/* CHANGED completely according to GT64260A dox - NTL */
	p[0] = 0x00010001;	/* 0 */
	p[1] = DESC_OWNER_BIT | DESC_FIRST | DESC_LAST;	/* 4 */
	p[2] = 0;		/* 8 */
	p[3] = (unsigned int) &p[4];	/* c */

#if 0
	p[9] = DESC_FIRST | DESC_LAST;
	p[10] = (unsigned int) &p[0];
	p[11] = (unsigned int) &p[12];
#endif

	FLUSH_DCACHE (&p[0], &p[8]);

	GT_REG_WRITE (GALSDMA_0_CUR_TX_PTR + (CHANNEL * GALSDMA_REG_DIFF),
		      (unsigned int) &p[0]);
	GT_REG_WRITE (GALSDMA_0_FIR_TX_PTR + (CHANNEL * GALSDMA_REG_DIFF),
		      (unsigned int) &p[0]);

	temp = GTREGREAD (GALSDMA_0_COM_REG + (CHANNEL * GALSDMA_REG_DIFF));
	temp |= (TX_DEMAND | TX_STOP);
	GT_REG_WRITE (GALSDMA_0_COM_REG + (CHANNEL * GALSDMA_REG_DIFF), temp);

	INVALIDATE_DCACHE (&p[1], &p[2]);

	while (p[1] & DESC_OWNER_BIT) {
		udelay (100);
		INVALIDATE_DCACHE (&p[1], &p[2]);
	}
	return 0;
}

char mpsc_getchar_sdma (void)
{
	static unsigned int done = 0;
	volatile char ch;
	unsigned int len = 0, idx = 0, temp;

	volatile unsigned int *p;


	do {
		p = &rx_desc_base[rx_desc_index * 8];

		INVALIDATE_DCACHE (&p[0], &p[1]);
		/* Wait for character */
		while (p[1] & DESC_OWNER_BIT) {
			udelay (100);
			INVALIDATE_DCACHE (&p[0], &p[1]);
		}

		/* Handle error case */
		if (p[1] & (1 << 15)) {
			printf ("oops, error: %08x\n", p[1]);

			temp = GTREGREAD (GALMPSC_CHANNELREG_2 +
					  (CHANNEL * GALMPSC_REG_GAP));
			temp |= (1 << 23);
			GT_REG_WRITE (GALMPSC_CHANNELREG_2 +
				      (CHANNEL * GALMPSC_REG_GAP), temp);

			/* Can't poll on abort bit, so we just wait. */
			udelay (100);

			galsdma_enable_rx ();
		}

		/* Number of bytes left in this descriptor */
		len = p[0] & 0xffff;

		if (len) {
			/* Where to look */
			idx = 5;
			if (done > 3)
				idx = 4;
			if (done > 7)
				idx = 7;
			if (done > 11)
				idx = 6;

			INVALIDATE_DCACHE (&p[idx], &p[idx + 1]);
			ch = p[idx] & 0xff;
			done++;
		}

		if (done < len) {
			/* this descriptor has more bytes still
			 * shift down the char we just read, and leave the
			 * buffer in place for the next time around
			 */
			p[idx] = p[idx] >> 8;
			FLUSH_DCACHE (&p[idx], &p[idx + 1]);
		}

		if (done == len) {
			/* nothing left in this descriptor.
			 * go to next one
			 */
			p[1] = DESC_OWNER_BIT | DESC_FIRST | DESC_LAST;
			p[0] = 0x00100000;
			FLUSH_DCACHE (&p[0], &p[1]);
			/* Next descriptor */
			rx_desc_index = (rx_desc_index + 1) % RX_DESC;
			done = 0;
		}
	} while (len == 0);	/* galileo bug.. len might be zero */

	return ch;
}


int mpsc_test_char_debug (void)
{
	if ((GTREGREAD (GALMPSC_0_INT_CAUSE) & BIT6) == 0)
		return 0;
	else {
		return 1;
	}
}


int mpsc_test_char_sdma (void)
{
	volatile unsigned int *p = &rx_desc_base[rx_desc_index * 8];

	INVALIDATE_DCACHE (&p[1], &p[2]);

	if (p[1] & DESC_OWNER_BIT)
		return 0;
	else
		return 1;
}

int mpsc_init (int baud)
{
	/* BRG CONFIG */
	galbrg_set_baudrate (CHANNEL, baud);
	galbrg_set_clksrc (CHANNEL, 8);	/* set source=Tclk */
	galbrg_set_CUV (CHANNEL, 0);	/* set up CountUpValue */
	galbrg_enable (CHANNEL);	/* Enable BRG */

	/* Set up clock routing */
	galmpsc_connect (CHANNEL, GALMPSC_CONNECT);	/* connect it */

	galmpsc_route_rx_clock (CHANNEL, CHANNEL);	/* chosse BRG0 for Rx */
	galmpsc_route_tx_clock (CHANNEL, CHANNEL);	/* chose BRG0 for Tx */

	/* reset MPSC state */
	galmpsc_shutdown (CHANNEL);

	/* SDMA CONFIG */
	galsdma_set_burstsize (CHANNEL, L1_CACHE_BYTES / 8);	/* in 64 bit words (8 bytes) */
	galsdma_set_txle (CHANNEL);
	galsdma_set_rxle (CHANNEL);
	galsdma_set_RC (CHANNEL, 0xf);
	galsdma_set_SFM (CHANNEL);
	galsdma_set_RFT (CHANNEL);

	/* MPSC CONFIG */
	galmpsc_write_config_regs (CHANNEL, GALMPSC_UART);
	galmpsc_config_channel_regs (CHANNEL);
	galmpsc_set_char_length (CHANNEL, GALMPSC_CHAR_LENGTH_8);	/* 8 */
	galmpsc_set_parity (CHANNEL, GALMPSC_PARITY_NONE);	/* N */
	galmpsc_set_stop_bit_length (CHANNEL, GALMPSC_STOP_BITS_1);	/* 1 */

#ifdef CONFIG_MPSC_DEBUG_PORT
	mpsc_debug_init ();
#endif

	/* COMM_MPSC CONFIG */
#ifdef SOFTWARE_CACHE_MANAGEMENT
	galmpsc_set_snoop (CHANNEL, 0);	/* disable snoop */
#else
	galmpsc_set_snoop (CHANNEL, 1);	/* enable snoop */
#endif

	return 0;
}


void mpsc_sdma_init (void)
{
/* Setup SDMA channel0 SDMA_CONFIG_REG*/
	GT_REG_WRITE (SDMA_CONFIG_REG (0), 0x000020ff);

/*  Enable MPSC-Window0 for DRAM Bank0   */
	if (galsdma_set_mem_space (MV64360_CUNIT_BASE_ADDR_WIN_0_BIT,
				   MV64360_SDMA_DRAM_CS_0_TARGET,
				   0,
				   memoryGetBankBaseAddress
				   (CS_0_LOW_DECODE_ADDRESS),
				   memoryGetBankSize (BANK0)) != true)
		printf ("%s: SDMA_Window0 memory setup failed !!! \n",
			__FUNCTION__);


/*  Disable MPSC-Window1    */
	if (galsdma_set_mem_space (MV64360_CUNIT_BASE_ADDR_WIN_1_BIT,
				   MV64360_SDMA_DRAM_CS_0_TARGET,
				   0,
				   memoryGetBankBaseAddress
				   (CS_1_LOW_DECODE_ADDRESS),
				   memoryGetBankSize (BANK3)) != true)
		printf ("%s: SDMA_Window1 memory setup failed !!! \n",
			__FUNCTION__);


/*  Disable MPSC-Window2 */
	if (galsdma_set_mem_space (MV64360_CUNIT_BASE_ADDR_WIN_2_BIT,
				   MV64360_SDMA_DRAM_CS_0_TARGET,
				   0,
				   memoryGetBankBaseAddress
				   (CS_2_LOW_DECODE_ADDRESS),
				   memoryGetBankSize (BANK3)) != true)
		printf ("%s: SDMA_Window2 memory setup failed !!! \n",
			__FUNCTION__);


/*  Disable MPSC-Window3 */
	if (galsdma_set_mem_space (MV64360_CUNIT_BASE_ADDR_WIN_3_BIT,
				   MV64360_SDMA_DRAM_CS_0_TARGET,
				   0,
				   memoryGetBankBaseAddress
				   (CS_3_LOW_DECODE_ADDRESS),
				   memoryGetBankSize (BANK3)) != true)
		printf ("%s: SDMA_Window3 memory setup failed !!! \n",
			__FUNCTION__);

/*  Setup MPSC0 access mode Window0 full access */
	GT_SET_REG_BITS (MPSC0_ACCESS_PROTECTION_REG,
			 (MV64360_SDMA_WIN_ACCESS_FULL <<
			  (MV64360_CUNIT_BASE_ADDR_WIN_0_BIT * 2)));

/*  Setup MPSC1 access mode Window1 full access */
	GT_SET_REG_BITS (MPSC1_ACCESS_PROTECTION_REG,
			 (MV64360_SDMA_WIN_ACCESS_FULL <<
			  (MV64360_CUNIT_BASE_ADDR_WIN_0_BIT * 2)));

/* Setup MPSC internal address space base address	*/
	GT_REG_WRITE (CUNIT_INTERNAL_SPACE_BASE_ADDR_REG, CONFIG_SYS_GT_REGS);

/* no high address remap*/
	GT_REG_WRITE (CUNIT_HIGH_ADDR_REMAP_REG0, 0x00);
	GT_REG_WRITE (CUNIT_HIGH_ADDR_REMAP_REG1, 0x00);

/* clear interrupt cause register for MPSC (fault register)*/
	GT_REG_WRITE (CUNIT_INTERRUPT_CAUSE_REG, 0x00);
}


void mpsc_init2 (void)
{
	int i;

#ifndef CONFIG_MPSC_DEBUG_PORT
	mpsc_putchar = mpsc_putchar_sdma;
	mpsc_getchar = mpsc_getchar_sdma;
	mpsc_test_char = mpsc_test_char_sdma;
#endif
	/* RX descriptors */
	rx_desc_base = (unsigned int *) malloc (((RX_DESC + 1) * 8) *
						sizeof (unsigned int));

	/* align descriptors */
	rx_desc_base = (unsigned int *)
		(((unsigned int) rx_desc_base + 32) & 0xFFFFFFF0);

	rx_desc_index = 0;

	memset ((void *) rx_desc_base, 0,
		(RX_DESC * 8) * sizeof (unsigned int));

	for (i = 0; i < RX_DESC; i++) {
		rx_desc_base[i * 8 + 3] = (unsigned int) &rx_desc_base[i * 8 + 4];	/* Buffer */
		rx_desc_base[i * 8 + 2] = (unsigned int) &rx_desc_base[(i + 1) * 8];	/* Next descriptor */
		rx_desc_base[i * 8 + 1] = DESC_OWNER_BIT | DESC_FIRST | DESC_LAST;	/* Command & control */
		rx_desc_base[i * 8] = 0x00100000;
	}
	rx_desc_base[(i - 1) * 8 + 2] = (unsigned int) &rx_desc_base[0];

	FLUSH_DCACHE (&rx_desc_base[0], &rx_desc_base[RX_DESC * 8]);
	GT_REG_WRITE (GALSDMA_0_CUR_RX_PTR + (CHANNEL * GALSDMA_REG_DIFF),
		      (unsigned int) &rx_desc_base[0]);

	/* TX descriptors */
	tx_desc_base = (unsigned int *) malloc (((TX_DESC + 1) * 8) *
						sizeof (unsigned int));

	/* align descriptors */
	tx_desc_base = (unsigned int *)
		(((unsigned int) tx_desc_base + 32) & 0xFFFFFFF0);

	tx_desc_index = -1;

	memset ((void *) tx_desc_base, 0,
		(TX_DESC * 8) * sizeof (unsigned int));

	for (i = 0; i < TX_DESC; i++) {
		tx_desc_base[i * 8 + 5] = (unsigned int) 0x23232323;
		tx_desc_base[i * 8 + 4] = (unsigned int) 0x23232323;
		tx_desc_base[i * 8 + 3] =
			(unsigned int) &tx_desc_base[i * 8 + 4];
		tx_desc_base[i * 8 + 2] =
			(unsigned int) &tx_desc_base[(i + 1) * 8];
		tx_desc_base[i * 8 + 1] =
			DESC_OWNER_BIT | DESC_FIRST | DESC_LAST;

		/* set sbytecnt and shadow byte cnt to 1 */
		tx_desc_base[i * 8] = 0x00010001;
	}
	tx_desc_base[(i - 1) * 8 + 2] = (unsigned int) &tx_desc_base[0];

	FLUSH_DCACHE (&tx_desc_base[0], &tx_desc_base[TX_DESC * 8]);

	udelay (100);

	galsdma_enable_rx ();

	return;
}

int galbrg_set_baudrate (int channel, int rate)
{
	int clock;

	galbrg_disable (channel);	/*ok */

#ifdef ZUMA_NTL
	/* from tclk */
	clock = (CONFIG_SYS_TCLK / (16 * rate)) - 1;
#else
	clock = (CONFIG_SYS_TCLK / (16 * rate)) - 1;
#endif

	galbrg_set_CDV (channel, clock);	/* set timer Reg. for BRG */

	galbrg_enable (channel);

	gd->baudrate = rate;

	return 0;
}

/* ------------------------------------------------------------------ */

/* Below are all the private functions that no one else needs */

static int galbrg_set_CDV (int channel, int value)
{
	unsigned int temp;

	temp = GTREGREAD (GALBRG_0_CONFREG + (channel * GALBRG_REG_GAP));
	temp &= 0xFFFF0000;
	temp |= (value & 0x0000FFFF);
	GT_REG_WRITE (GALBRG_0_CONFREG + (channel * GALBRG_REG_GAP), temp);

	return 0;
}

static int galbrg_enable (int channel)
{
	unsigned int temp;

	temp = GTREGREAD (GALBRG_0_CONFREG + (channel * GALBRG_REG_GAP));
	temp |= 0x00010000;
	GT_REG_WRITE (GALBRG_0_CONFREG + (channel * GALBRG_REG_GAP), temp);

	return 0;
}

static int galbrg_disable (int channel)
{
	unsigned int temp;

	temp = GTREGREAD (GALBRG_0_CONFREG + (channel * GALBRG_REG_GAP));
	temp &= 0xFFFEFFFF;
	GT_REG_WRITE (GALBRG_0_CONFREG + (channel * GALBRG_REG_GAP), temp);

	return 0;
}

static int galbrg_set_clksrc (int channel, int value)
{
	unsigned int temp;

	temp = GTREGREAD (GALBRG_0_CONFREG + (channel * GALBRG_REG_GAP));
	temp &= 0xFFC3FFFF;	/* Bit 18 - 21 (MV 64260 18-22) */
	temp |= (value << 18);
	GT_REG_WRITE (GALBRG_0_CONFREG + (channel * GALBRG_REG_GAP), temp);
	return 0;
}

static int galbrg_set_CUV (int channel, int value)
{
	/* set CountUpValue */
	GT_REG_WRITE (GALBRG_0_BTREG + (channel * GALBRG_REG_GAP), value);

	return 0;
}

#if 0
static int galbrg_reset (int channel)
{
	unsigned int temp;

	temp = GTREGREAD (GALBRG_0_CONFREG + (channel * GALBRG_REG_GAP));
	temp |= 0x20000;
	GT_REG_WRITE (GALBRG_0_CONFREG + (channel * GALBRG_REG_GAP), temp);

	return 0;
}
#endif

static int galsdma_set_RFT (int channel)
{
	unsigned int temp;

	temp = GTREGREAD (GALSDMA_0_CONF_REG + (channel * GALSDMA_REG_DIFF));
	temp |= 0x00000001;
	GT_REG_WRITE (GALSDMA_0_CONF_REG + (channel * GALSDMA_REG_DIFF),
		      temp);

	return 0;
}

static int galsdma_set_SFM (int channel)
{
	unsigned int temp;

	temp = GTREGREAD (GALSDMA_0_CONF_REG + (channel * GALSDMA_REG_DIFF));
	temp |= 0x00000002;
	GT_REG_WRITE (GALSDMA_0_CONF_REG + (channel * GALSDMA_REG_DIFF),
		      temp);

	return 0;
}

static int galsdma_set_rxle (int channel)
{
	unsigned int temp;

	temp = GTREGREAD (GALSDMA_0_CONF_REG + (channel * GALSDMA_REG_DIFF));
	temp |= 0x00000040;
	GT_REG_WRITE (GALSDMA_0_CONF_REG + (channel * GALSDMA_REG_DIFF),
		      temp);

	return 0;
}

static int galsdma_set_txle (int channel)
{
	unsigned int temp;

	temp = GTREGREAD (GALSDMA_0_CONF_REG + (channel * GALSDMA_REG_DIFF));
	temp |= 0x00000080;
	GT_REG_WRITE (GALSDMA_0_CONF_REG + (channel * GALSDMA_REG_DIFF),
		      temp);

	return 0;
}

static int galsdma_set_RC (int channel, unsigned int value)
{
	unsigned int temp;

	temp = GTREGREAD (GALSDMA_0_CONF_REG + (channel * GALSDMA_REG_DIFF));
	temp &= ~0x0000003c;
	temp |= (value << 2);
	GT_REG_WRITE (GALSDMA_0_CONF_REG + (channel * GALSDMA_REG_DIFF),
		      temp);

	return 0;
}

static int galsdma_set_burstsize (int channel, unsigned int value)
{
	unsigned int temp;

	temp = GTREGREAD (GALSDMA_0_CONF_REG + (channel * GALSDMA_REG_DIFF));
	temp &= 0xFFFFCFFF;
	switch (value) {
	case 8:
		GT_REG_WRITE (GALSDMA_0_CONF_REG +
			      (channel * GALSDMA_REG_DIFF),
			      (temp | (0x3 << 12)));
		break;

	case 4:
		GT_REG_WRITE (GALSDMA_0_CONF_REG +
			      (channel * GALSDMA_REG_DIFF),
			      (temp | (0x2 << 12)));
		break;

	case 2:
		GT_REG_WRITE (GALSDMA_0_CONF_REG +
			      (channel * GALSDMA_REG_DIFF),
			      (temp | (0x1 << 12)));
		break;

	case 1:
		GT_REG_WRITE (GALSDMA_0_CONF_REG +
			      (channel * GALSDMA_REG_DIFF),
			      (temp | (0x0 << 12)));
		break;

	default:
		return -1;
		break;
	}

	return 0;
}

static int galmpsc_connect (int channel, int connect)
{
	unsigned int temp;

	temp = GTREGREAD (GALMPSC_ROUTING_REGISTER);

	if ((channel == 0) && connect)
		temp &= ~0x00000007;
	else if ((channel == 1) && connect)
		temp &= ~(0x00000007 << 6);
	else if ((channel == 0) && !connect)
		temp |= 0x00000007;
	else
		temp |= (0x00000007 << 6);

	/* Just in case... */
	temp &= 0x3fffffff;

	GT_REG_WRITE (GALMPSC_ROUTING_REGISTER, temp);

	return 0;
}

static int galmpsc_route_rx_clock (int channel, int brg)
{
	unsigned int temp;

	temp = GTREGREAD (GALMPSC_RxC_ROUTE);

	if (channel == 0) {
		temp &= ~0x0000000F;
		temp |= brg;
	} else {
		temp &= ~0x00000F00;
		temp |= (brg << 8);
	}

	GT_REG_WRITE (GALMPSC_RxC_ROUTE, temp);

	return 0;
}

static int galmpsc_route_tx_clock (int channel, int brg)
{
	unsigned int temp;

	temp = GTREGREAD (GALMPSC_TxC_ROUTE);

	if (channel == 0) {
		temp &= ~0x0000000F;
		temp |= brg;
	} else {
		temp &= ~0x00000F00;
		temp |= (brg << 8);
	}

	GT_REG_WRITE (GALMPSC_TxC_ROUTE, temp);

	return 0;
}

static int galmpsc_write_config_regs (int mpsc, int mode)
{
	if (mode == GALMPSC_UART) {
		/* Main config reg Low (Null modem, Enable Tx/Rx, UART mode) */
		GT_REG_WRITE (GALMPSC_MCONF_LOW + (mpsc * GALMPSC_REG_GAP),
			      0x000004c4);

		/* Main config reg High (32x Rx/Tx clock mode, width=8bits */
		GT_REG_WRITE (GALMPSC_MCONF_HIGH + (mpsc * GALMPSC_REG_GAP),
			      0x024003f8);
		/*        22 2222 1111 */
		/*        54 3210 9876 */
		/* 0000 0010 0000 0000 */
		/*       1 */
		/*       098 7654 3210 */
		/* 0000 0011 1111 1000 */
	} else
		return -1;

	return 0;
}

static int galmpsc_config_channel_regs (int mpsc)
{
	GT_REG_WRITE (GALMPSC_CHANNELREG_1 + (mpsc * GALMPSC_REG_GAP), 0);
	GT_REG_WRITE (GALMPSC_CHANNELREG_2 + (mpsc * GALMPSC_REG_GAP), 0);
	GT_REG_WRITE (GALMPSC_CHANNELREG_3 + (mpsc * GALMPSC_REG_GAP), 1);
	GT_REG_WRITE (GALMPSC_CHANNELREG_4 + (mpsc * GALMPSC_REG_GAP), 0);
	GT_REG_WRITE (GALMPSC_CHANNELREG_5 + (mpsc * GALMPSC_REG_GAP), 0);
	GT_REG_WRITE (GALMPSC_CHANNELREG_6 + (mpsc * GALMPSC_REG_GAP), 0);
	GT_REG_WRITE (GALMPSC_CHANNELREG_7 + (mpsc * GALMPSC_REG_GAP), 0);
	GT_REG_WRITE (GALMPSC_CHANNELREG_8 + (mpsc * GALMPSC_REG_GAP), 0);
	GT_REG_WRITE (GALMPSC_CHANNELREG_9 + (mpsc * GALMPSC_REG_GAP), 0);
	GT_REG_WRITE (GALMPSC_CHANNELREG_10 + (mpsc * GALMPSC_REG_GAP), 0);

	galmpsc_set_brkcnt (mpsc, 0x3);
	galmpsc_set_tcschar (mpsc, 0xab);

	return 0;
}

static int galmpsc_set_brkcnt (int mpsc, int value)
{
	unsigned int temp;

	temp = GTREGREAD (GALMPSC_CHANNELREG_1 + (mpsc * GALMPSC_REG_GAP));
	temp &= 0x0000FFFF;
	temp |= (value << 16);
	GT_REG_WRITE (GALMPSC_CHANNELREG_1 + (mpsc * GALMPSC_REG_GAP), temp);

	return 0;
}

static int galmpsc_set_tcschar (int mpsc, int value)
{
	unsigned int temp;

	temp = GTREGREAD (GALMPSC_CHANNELREG_1 + (mpsc * GALMPSC_REG_GAP));
	temp &= 0xFFFF0000;
	temp |= value;
	GT_REG_WRITE (GALMPSC_CHANNELREG_1 + (mpsc * GALMPSC_REG_GAP), temp);

	return 0;
}

static int galmpsc_set_char_length (int mpsc, int value)
{
	unsigned int temp;

	temp = GTREGREAD (GALMPSC_PROTOCONF_REG + (mpsc * GALMPSC_REG_GAP));
	temp &= 0xFFFFCFFF;
	temp |= (value << 12);
	GT_REG_WRITE (GALMPSC_PROTOCONF_REG + (mpsc * GALMPSC_REG_GAP), temp);

	return 0;
}

static int galmpsc_set_stop_bit_length (int mpsc, int value)
{
	unsigned int temp;

	temp = GTREGREAD (GALMPSC_PROTOCONF_REG + (mpsc * GALMPSC_REG_GAP));
	temp &= 0xFFFFBFFF;
	temp |= (value << 14);
	GT_REG_WRITE (GALMPSC_PROTOCONF_REG + (mpsc * GALMPSC_REG_GAP), temp);

	return 0;
}

static int galmpsc_set_parity (int mpsc, int value)
{
	unsigned int temp;

	temp = GTREGREAD (GALMPSC_CHANNELREG_2 + (mpsc * GALMPSC_REG_GAP));
	if (value != -1) {
		temp &= 0xFFF3FFF3;
		temp |= ((value << 18) | (value << 2));
		temp |= ((value << 17) | (value << 1));
	} else {
		temp &= 0xFFF1FFF1;
	}

	GT_REG_WRITE (GALMPSC_CHANNELREG_2 + (mpsc * GALMPSC_REG_GAP), temp);

	return 0;
}

static int galmpsc_enter_hunt (int mpsc)
{
	int temp;

	temp = GTREGREAD (GALMPSC_CHANNELREG_2 + (mpsc * GALMPSC_REG_GAP));
	temp |= 0x80000000;
	GT_REG_WRITE (GALMPSC_CHANNELREG_2 + (mpsc * GALMPSC_REG_GAP), temp);

	while (GTREGREAD (GALMPSC_CHANNELREG_2 + (mpsc * GALMPSC_REG_GAP)) &
	       MPSC_ENTER_HUNT) {
		udelay (1);
	}
	return 0;
}


static int galmpsc_shutdown (int mpsc)
{
	unsigned int temp;

	/* cause RX abort (clears RX) */
	temp = GTREGREAD (GALMPSC_CHANNELREG_2 + (mpsc * GALMPSC_REG_GAP));
	temp |= MPSC_RX_ABORT | MPSC_TX_ABORT;
	temp &= ~MPSC_ENTER_HUNT;
	GT_REG_WRITE (GALMPSC_CHANNELREG_2 + (mpsc * GALMPSC_REG_GAP), temp);

	GT_REG_WRITE (GALSDMA_0_COM_REG, 0);
	GT_REG_WRITE (GALSDMA_0_COM_REG, SDMA_TX_ABORT | SDMA_RX_ABORT);

	/* shut down the MPSC */
	GT_REG_WRITE (GALMPSC_MCONF_LOW, 0);
	GT_REG_WRITE (GALMPSC_MCONF_HIGH, 0);
	GT_REG_WRITE (GALMPSC_PROTOCONF_REG + (mpsc * GALMPSC_REG_GAP), 0);

	udelay (100);

	/* shut down the sdma engines. */
	/* reset config to default */
	GT_REG_WRITE (GALSDMA_0_CONF_REG, 0x000000fc);

	udelay (100);

	/* clear the SDMA current and first TX and RX pointers */
	GT_REG_WRITE (GALSDMA_0_CUR_RX_PTR, 0);
	GT_REG_WRITE (GALSDMA_0_CUR_TX_PTR, 0);
	GT_REG_WRITE (GALSDMA_0_FIR_TX_PTR, 0);

	udelay (100);

	return 0;
}

static void galsdma_enable_rx (void)
{
	int temp;

	/* Enable RX processing */
	temp = GTREGREAD (GALSDMA_0_COM_REG + (CHANNEL * GALSDMA_REG_DIFF));
	temp |= RX_ENABLE;
	GT_REG_WRITE (GALSDMA_0_COM_REG + (CHANNEL * GALSDMA_REG_DIFF), temp);

	galmpsc_enter_hunt (CHANNEL);
}

static int galmpsc_set_snoop (int mpsc, int value)
{
	int reg =
		mpsc ? MPSC_1_ADDRESS_CONTROL_LOW :
		MPSC_0_ADDRESS_CONTROL_LOW;
	int temp = GTREGREAD (reg);

	if (value)
		temp |= (1 << 6) | (1 << 14) | (1 << 22) | (1 << 30);
	else
		temp &= ~((1 << 6) | (1 << 14) | (1 << 22) | (1 << 30));
	GT_REG_WRITE (reg, temp);
	return 0;
}

/*******************************************************************************
* galsdma_set_mem_space - Set MV64360 IDMA memory decoding map.
*
* DESCRIPTION:
*       the MV64360 SDMA has its own address decoding map that is de-coupled
*       from the CPU interface address decoding windows. The SDMA channels
*       share four address windows. Each region can be individually configured
*       by this function by associating it to a target interface and setting
*       base and size values.
*
*      NOTE!!!
*       The size must be in 64Kbyte granularity.
*       The base address must be aligned to the size.
*       The size must be a series of 1s followed by a series of zeros
*
* OUTPUT:
*       None.
*
* RETURN:
*       True for success, false otherwise.
*
*******************************************************************************/

static int galsdma_set_mem_space (unsigned int memSpace,
				  unsigned int memSpaceTarget,
				  unsigned int memSpaceAttr,
				  unsigned int baseAddress, unsigned int size)
{
	unsigned int temp;

	if (size == 0) {
		GT_RESET_REG_BITS (MV64360_CUNIT_BASE_ADDR_ENABLE_REG,
				   1 << memSpace);
		return true;
	}

	/* The base address must be aligned to the size.  */
	if (baseAddress % size != 0) {
		return false;
	}
	if (size < 0x10000) {
		return false;
	}

	/* Align size and base to 64K */
	baseAddress &= 0xffff0000;
	size &= 0xffff0000;
	temp = size >> 16;

	/* Checking that the size is a sequence of '1' followed by a
	   sequence of '0' starting from LSB to MSB. */
	while ((temp > 0) && (temp & 0x1)) {
		temp = temp >> 1;
	}

	if (temp != 0) {
		GT_REG_WRITE (MV64360_CUNIT_BASE_ADDR_REG0 + memSpace * 8,
			      (baseAddress | memSpaceTarget | memSpaceAttr));
		GT_REG_WRITE ((MV64360_CUNIT_SIZE0 + memSpace * 8),
			      (size - 1) & 0xffff0000);
		GT_RESET_REG_BITS (MV64360_CUNIT_BASE_ADDR_ENABLE_REG,
				   1 << memSpace);
	} else {
		/* An invalid size was specified */
		return false;
	}
	return true;
}
