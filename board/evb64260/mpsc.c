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

/*
 * mpsc.c - driver for console over the MPSC.
 */

#include <common.h>
#include <config.h>
#include <asm/cache.h>

#include <malloc.h>
#include "mpsc.h"

int (*mpsc_putchar)(char ch) = mpsc_putchar_early;

static volatile unsigned int *rx_desc_base=NULL;
static unsigned int rx_desc_index=0;
static volatile unsigned int *tx_desc_base=NULL;
static unsigned int tx_desc_index=0;

/* local function declarations */
static int galmpsc_connect(int channel, int connect);
static int galmpsc_route_serial(int channel, int connect);
static int galmpsc_route_rx_clock(int channel, int brg);
static int galmpsc_route_tx_clock(int channel, int brg);
static int galmpsc_write_config_regs(int mpsc, int mode);
static int galmpsc_config_channel_regs(int mpsc);
static int galmpsc_set_char_length(int mpsc, int value);
static int galmpsc_set_stop_bit_length(int mpsc, int value);
static int galmpsc_set_parity(int mpsc, int value);
static int galmpsc_enter_hunt(int mpsc);
static int galmpsc_set_brkcnt(int mpsc, int value);
static int galmpsc_set_tcschar(int mpsc, int value);
static int galmpsc_set_snoop(int mpsc, int value);
static int galmpsc_shutdown(int mpsc);

static int galsdma_set_RFT(int channel);
static int galsdma_set_SFM(int channel);
static int galsdma_set_rxle(int channel);
static int galsdma_set_txle(int channel);
static int galsdma_set_burstsize(int channel, unsigned int value);
static int galsdma_set_RC(int channel, unsigned int value);

static int galbrg_set_CDV(int channel, int value);
static int galbrg_enable(int channel);
static int galbrg_disable(int channel);
static int galbrg_set_clksrc(int channel, int value);
static int galbrg_set_CUV(int channel, int value);

static void galsdma_enable_rx(void);

/* static int galbrg_reset(int channel); */

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


/* GT64240A errata: cant read MPSC/BRG registers... so make mirrors in ram for read/modify write */
#define MIRROR_HACK ((struct _tag_mirror_hack *)&(gd->mirror_hack))

#define GT_REG_WRITE_MIRROR_G(a,d) {MIRROR_HACK->a ## _M = d; GT_REG_WRITE(a,d);}
#define GTREGREAD_MIRROR_G(a) (MIRROR_HACK->a ## _M)

#define GT_REG_WRITE_MIRROR(a,i,g,d) {MIRROR_HACK->a ## _M[i] = d; GT_REG_WRITE(a + (i*g),d);}
#define GTREGREAD_MIRROR(a,i,g) (MIRROR_HACK->a ## _M[i])

/* make sure this isn't bigger than 16 long words (u-boot.h) */
struct _tag_mirror_hack {
    unsigned GALMPSC_PROTOCONF_REG_M[2];	/* 8008 */
    unsigned GALMPSC_CHANNELREG_1_M[2];		/* 800c */
    unsigned GALMPSC_CHANNELREG_2_M[2];		/* 8010 */
    unsigned GALBRG_0_CONFREG_M[2];		/* b200 */

    unsigned GALMPSC_ROUTING_REGISTER_M;	/* b400 */
    unsigned GALMPSC_RxC_ROUTE_M;		/* b404 */
    unsigned GALMPSC_TxC_ROUTE_M;		/* b408 */

    unsigned int baudrate;			/* current baudrate, for tsc delay calc */
};

/* static struct _tag_mirror_hack *mh = NULL;   */

/* special function for running out of flash.  doesn't modify any
 * global variables [josh] */
int
mpsc_putchar_early(char ch)
{
	DECLARE_GLOBAL_DATA_PTR;
	int mpsc=CHANNEL;
	int temp=GTREGREAD_MIRROR(GALMPSC_CHANNELREG_2,mpsc,GALMPSC_REG_GAP);
	galmpsc_set_tcschar(mpsc,ch);
	GT_REG_WRITE(GALMPSC_CHANNELREG_2+(mpsc*GALMPSC_REG_GAP), temp|0x200);

#define MAGIC_FACTOR	(10*1000000)

	udelay(MAGIC_FACTOR / MIRROR_HACK->baudrate);
	return 0;
}

/* This is used after relocation, see serial.c and mpsc_init2 */
static int
mpsc_putchar_sdma(char ch)
{
	volatile unsigned int *p;
	unsigned int temp;


	/* align the descriptor */
	p = tx_desc_base;
	memset((void *)p, 0, 8 * sizeof(unsigned int));

	/* fill one 64 bit buffer */
	/* word swap, pad with 0 */
	p[4] = 0;						/* x */
	p[5] = (unsigned int)ch;				/* x */

	/* CHANGED completely according to GT64260A dox - NTL */
	p[0] = 0x00010001;					/* 0 */
	p[1] = DESC_OWNER | DESC_FIRST | DESC_LAST;		/* 4 */
	p[2] = 0;						/* 8 */
	p[3] = (unsigned int)&p[4];				/* c */

#if 0
	p[9] = DESC_FIRST | DESC_LAST;
	p[10] = (unsigned int)&p[0];
	p[11] = (unsigned int)&p[12];
#endif

	FLUSH_DCACHE(&p[0], &p[8]);

	GT_REG_WRITE(GALSDMA_0_CUR_TX_PTR+(CHANNEL*GALSDMA_REG_DIFF),
		     (unsigned int)&p[0]);
	GT_REG_WRITE(GALSDMA_0_FIR_TX_PTR+(CHANNEL*GALSDMA_REG_DIFF),
		     (unsigned int)&p[0]);

	temp = GTREGREAD(GALSDMA_0_COM_REG+(CHANNEL*GALSDMA_REG_DIFF));
	temp |= (TX_DEMAND | TX_STOP);
	GT_REG_WRITE(GALSDMA_0_COM_REG+(CHANNEL*GALSDMA_REG_DIFF), temp);

	INVALIDATE_DCACHE(&p[1], &p[2]);

	while(p[1] & DESC_OWNER) {
	    udelay(100);
	    INVALIDATE_DCACHE(&p[1], &p[2]);
	}

	return 0;
}

char
mpsc_getchar(void)
{
    DECLARE_GLOBAL_DATA_PTR;
    static unsigned int done = 0;
    volatile char ch;
    unsigned int len=0, idx=0, temp;

    volatile unsigned int *p;


    do {
	p=&rx_desc_base[rx_desc_index*8];

	INVALIDATE_DCACHE(&p[0], &p[1]);
	/* Wait for character */
	while (p[1] & DESC_OWNER){
	    udelay(100);
	    INVALIDATE_DCACHE(&p[0], &p[1]);
	}

	/* Handle error case */
	if (p[1] & (1<<15)) {
		printf("oops, error: %08x\n", p[1]);

		temp = GTREGREAD_MIRROR(GALMPSC_CHANNELREG_2,CHANNEL,GALMPSC_REG_GAP);
		temp |= (1 << 23);
		GT_REG_WRITE_MIRROR(GALMPSC_CHANNELREG_2, CHANNEL,GALMPSC_REG_GAP, temp);

		/* Can't poll on abort bit, so we just wait. */
		udelay(100);

		galsdma_enable_rx();
	}

	/* Number of bytes left in this descriptor */
	len = p[0] & 0xffff;

	if (len) {
	    /* Where to look */
	    idx = 5;
	    if (done > 3) idx = 4;
	    if (done > 7) idx = 7;
	    if (done > 11) idx = 6;

	    INVALIDATE_DCACHE(&p[idx], &p[idx+1]);
	    ch = p[idx] & 0xff;
	    done++;
	}

	if (done < len) {
		/* this descriptor has more bytes still
		 * shift down the char we just read, and leave the
		 * buffer in place for the next time around
		 */
		p[idx] =  p[idx] >> 8;
		FLUSH_DCACHE(&p[idx], &p[idx+1]);
	}

	if (done == len) {
		/* nothing left in this descriptor.
		 * go to next one
		 */
		p[1] = DESC_OWNER | DESC_FIRST | DESC_LAST;
		p[0] = 0x00100000;
		FLUSH_DCACHE(&p[0], &p[1]);
		/* Next descriptor */
		rx_desc_index = (rx_desc_index + 1) % RX_DESC;
		done = 0;
	}
    } while (len==0);	/* galileo bug.. len might be zero */

    return ch;
}

int
mpsc_test_char(void)
{
	volatile unsigned int *p=&rx_desc_base[rx_desc_index*8];

	INVALIDATE_DCACHE(&p[1], &p[2]);

	if (p[1] & DESC_OWNER) return 0;
	else return 1;
}

int
mpsc_init(int baud)
{
	DECLARE_GLOBAL_DATA_PTR;

	memset(MIRROR_HACK, 0, sizeof(struct _tag_mirror_hack));
	MIRROR_HACK->GALMPSC_ROUTING_REGISTER_M=0x3fffffff;

	/* BRG CONFIG */
	galbrg_set_baudrate(CHANNEL, baud);
#if defined(CONFIG_ZUMA_V2) || defined(CONFIG_P3G4)
	galbrg_set_clksrc(CHANNEL,0x8);	/* connect TCLK -> BRG */
#else
	galbrg_set_clksrc(CHANNEL,0);
#endif
	galbrg_set_CUV(CHANNEL, 0);
	galbrg_enable(CHANNEL);

	/* Set up clock routing */
	galmpsc_connect(CHANNEL, GALMPSC_CONNECT);
	galmpsc_route_serial(CHANNEL, GALMPSC_CONNECT);
	galmpsc_route_rx_clock(CHANNEL, CHANNEL);
	galmpsc_route_tx_clock(CHANNEL, CHANNEL);

	/* reset MPSC state */
	galmpsc_shutdown(CHANNEL);

	/* SDMA CONFIG */
	galsdma_set_burstsize(CHANNEL, L1_CACHE_BYTES/8);	/* in 64 bit words (8 bytes) */
	galsdma_set_txle(CHANNEL);
	galsdma_set_rxle(CHANNEL);
	galsdma_set_RC(CHANNEL, 0xf);
	galsdma_set_SFM(CHANNEL);
	galsdma_set_RFT(CHANNEL);

	/* MPSC CONFIG */
	galmpsc_write_config_regs(CHANNEL, GALMPSC_UART);
	galmpsc_config_channel_regs(CHANNEL);
	galmpsc_set_char_length(CHANNEL, GALMPSC_CHAR_LENGTH_8);       /* 8 */
	galmpsc_set_parity(CHANNEL, GALMPSC_PARITY_NONE);              /* N */
	galmpsc_set_stop_bit_length(CHANNEL, GALMPSC_STOP_BITS_1);     /* 1 */

	/* COMM_MPSC CONFIG */
#ifdef SOFTWARE_CACHE_MANAGEMENT
	galmpsc_set_snoop(CHANNEL, 0);     				/* disable snoop */
#else
	galmpsc_set_snoop(CHANNEL, 1);     				/* enable snoop */
#endif

	return 0;
}

void
mpsc_init2(void)
{
	int i;

	mpsc_putchar = mpsc_putchar_sdma;

	/* RX descriptors */
	rx_desc_base = (unsigned int *)malloc(((RX_DESC+1)*8) *
		sizeof(unsigned int));

	/* align descriptors */
	rx_desc_base = (unsigned int *)
		(((unsigned int)rx_desc_base+32) & 0xFFFFFFF0);

	rx_desc_index = 0;

	memset((void *)rx_desc_base, 0, (RX_DESC*8)*sizeof(unsigned int));

	for (i = 0; i < RX_DESC; i++) {
		rx_desc_base[i*8 + 3] = (unsigned int)&rx_desc_base[i*8 + 4]; /* Buffer */
		rx_desc_base[i*8 + 2] = (unsigned int)&rx_desc_base[(i+1)*8]; /* Next descriptor */
		rx_desc_base[i*8 + 1] = DESC_OWNER | DESC_FIRST | DESC_LAST;  /* Command & control */
		rx_desc_base[i*8] = 0x00100000;
	}
	rx_desc_base[(i-1)*8 + 2] = (unsigned int)&rx_desc_base[0];

	FLUSH_DCACHE(&rx_desc_base[0], &rx_desc_base[RX_DESC*8]);
	GT_REG_WRITE(GALSDMA_0_CUR_RX_PTR+(CHANNEL*GALSDMA_REG_DIFF),
		     (unsigned int)&rx_desc_base[0]);

	/* TX descriptors */
	tx_desc_base = (unsigned int *)malloc(((TX_DESC+1)*8) *
		sizeof(unsigned int));

	/* align descriptors */
	tx_desc_base = (unsigned int *)
		(((unsigned int)tx_desc_base+32) & 0xFFFFFFF0);

	tx_desc_index = -1;

	memset((void *)tx_desc_base, 0, (TX_DESC*8)*sizeof(unsigned int));

	for (i = 0; i < TX_DESC; i++) {
		tx_desc_base[i*8 + 5] = (unsigned int)0x23232323;
		tx_desc_base[i*8 + 4] = (unsigned int)0x23232323;
		tx_desc_base[i*8 + 3] = (unsigned int)&tx_desc_base[i*8 + 4];
		tx_desc_base[i*8 + 2] = (unsigned int)&tx_desc_base[(i+1)*8];
		tx_desc_base[i*8 + 1] = DESC_OWNER | DESC_FIRST | DESC_LAST;

		/* set sbytecnt and shadow byte cnt to 1 */
		tx_desc_base[i*8] = 0x00010001;
	}
	tx_desc_base[(i-1)*8 + 2] = (unsigned int)&tx_desc_base[0];

	FLUSH_DCACHE(&tx_desc_base[0], &tx_desc_base[TX_DESC*8]);

	udelay(100);

	galsdma_enable_rx();

	return;
}

int
galbrg_set_baudrate(int channel, int rate)
{
	DECLARE_GLOBAL_DATA_PTR;
	int clock;

	galbrg_disable(channel);

#if defined(CONFIG_ZUMA_V2) || defined(CONFIG_P3G4)
	/* from tclk */
	clock = (CFG_BUS_HZ/(16*rate)) - 1;
#else
	clock = (3686400/(16*rate)) - 1;
#endif

	galbrg_set_CDV(channel, clock);

	galbrg_enable(channel);

	MIRROR_HACK->baudrate = rate;

	return 0;
}

/* ------------------------------------------------------------------ */

/* Below are all the private functions that no one else needs */

static int
galbrg_set_CDV(int channel, int value)
{
	DECLARE_GLOBAL_DATA_PTR;
	unsigned int temp;

	temp = GTREGREAD_MIRROR(GALBRG_0_CONFREG, channel, GALBRG_REG_GAP);
	temp &= 0xFFFF0000;
	temp |= (value & 0x0000FFFF);
	GT_REG_WRITE_MIRROR(GALBRG_0_CONFREG,channel,GALBRG_REG_GAP, temp);

	return 0;
}

static int
galbrg_enable(int channel)
{
	DECLARE_GLOBAL_DATA_PTR;
	unsigned int temp;

	temp = GTREGREAD_MIRROR(GALBRG_0_CONFREG, channel, GALBRG_REG_GAP);
	temp |= 0x00010000;
	GT_REG_WRITE_MIRROR(GALBRG_0_CONFREG, channel, GALBRG_REG_GAP,temp);

	return 0;
}

static int
galbrg_disable(int channel)
{
	DECLARE_GLOBAL_DATA_PTR;
	unsigned int temp;

	temp = GTREGREAD_MIRROR(GALBRG_0_CONFREG, channel, GALBRG_REG_GAP);
	temp &= 0xFFFEFFFF;
	GT_REG_WRITE_MIRROR(GALBRG_0_CONFREG, channel, GALBRG_REG_GAP,temp);

	return 0;
}

static int
galbrg_set_clksrc(int channel, int value)
{
	DECLARE_GLOBAL_DATA_PTR;
	unsigned int temp;

	temp = GTREGREAD_MIRROR(GALBRG_0_CONFREG,channel, GALBRG_REG_GAP);
	temp &= 0xFF83FFFF;
	temp |= (value << 18);
	GT_REG_WRITE_MIRROR(GALBRG_0_CONFREG,channel, GALBRG_REG_GAP,temp);

	return 0;
}

static int
galbrg_set_CUV(int channel, int value)
{
	GT_REG_WRITE(GALBRG_0_BTREG + (channel * GALBRG_REG_GAP), value);

	return 0;
}

#if 0
static int
galbrg_reset(int channel)
{
	unsigned int temp;

	temp = GTREGREAD(GALBRG_0_CONFREG + (channel * GALBRG_REG_GAP));
	temp |= 0x20000;
	GT_REG_WRITE(GALBRG_0_CONFREG + (channel * GALBRG_REG_GAP), temp);

	return 0;
}
#endif

static int
galsdma_set_RFT(int channel)
{
	unsigned int temp;

	temp = GTREGREAD(GALSDMA_0_CONF_REG+(channel*GALSDMA_REG_DIFF));
	temp |= 0x00000001;
	GT_REG_WRITE(GALSDMA_0_CONF_REG+(channel*GALSDMA_REG_DIFF), temp);

	return 0;
}

static int
galsdma_set_SFM(int channel)
{
	unsigned int temp;

	temp = GTREGREAD(GALSDMA_0_CONF_REG+(channel*GALSDMA_REG_DIFF));
	temp |= 0x00000002;
	GT_REG_WRITE(GALSDMA_0_CONF_REG+(channel*GALSDMA_REG_DIFF), temp);

	return 0;
}

static int
galsdma_set_rxle(int channel)
{
	unsigned int temp;

	temp = GTREGREAD(GALSDMA_0_CONF_REG+(channel*GALSDMA_REG_DIFF));
	temp |= 0x00000040;
	GT_REG_WRITE(GALSDMA_0_CONF_REG+(channel*GALSDMA_REG_DIFF), temp);

	return 0;
}

static int
galsdma_set_txle(int channel)
{
	unsigned int temp;

	temp = GTREGREAD(GALSDMA_0_CONF_REG+(channel*GALSDMA_REG_DIFF));
	temp |= 0x00000080;
	GT_REG_WRITE(GALSDMA_0_CONF_REG+(channel*GALSDMA_REG_DIFF), temp);

	return 0;
}

static int
galsdma_set_RC(int channel, unsigned int value)
{
	unsigned int temp;

	temp = GTREGREAD(GALSDMA_0_CONF_REG+(channel*GALSDMA_REG_DIFF));
	temp &= ~0x0000003c;
	temp |= (value << 2);
	GT_REG_WRITE(GALSDMA_0_CONF_REG+(channel*GALSDMA_REG_DIFF), temp);

	return 0;
}

static int
galsdma_set_burstsize(int channel, unsigned int value)
{
	unsigned int temp;

	temp = GTREGREAD(GALSDMA_0_CONF_REG+(channel*GALSDMA_REG_DIFF));
	temp &= 0xFFFFCFFF;
	switch (value) {
	 case 8:
		GT_REG_WRITE(GALSDMA_0_CONF_REG+(channel*GALSDMA_REG_DIFF),
			     (temp | (0x3 << 12)));
		break;

	 case 4:
		GT_REG_WRITE(GALSDMA_0_CONF_REG+(channel*GALSDMA_REG_DIFF),
			     (temp | (0x2 << 12)));
		break;

	 case 2:
		GT_REG_WRITE(GALSDMA_0_CONF_REG+(channel*GALSDMA_REG_DIFF),
			     (temp | (0x1 << 12)));
		break;

	 case 1:
		GT_REG_WRITE(GALSDMA_0_CONF_REG+(channel*GALSDMA_REG_DIFF),
			     (temp | (0x0 << 12)));
		break;

	 default:
		return -1;
		break;
	}

	return 0;
}

static int
galmpsc_connect(int channel, int connect)
{
	DECLARE_GLOBAL_DATA_PTR;
	unsigned int temp;

	temp = GTREGREAD_MIRROR_G(GALMPSC_ROUTING_REGISTER);

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

	GT_REG_WRITE_MIRROR_G(GALMPSC_ROUTING_REGISTER, temp);

	return 0;
}

static int
galmpsc_route_serial(int channel, int connect)
{
	unsigned int temp;

	temp = GTREGREAD(GALMPSC_SERIAL_MULTIPLEX);

	if ((channel == 0) && connect)
		temp |= 0x00000100;
	else if ((channel == 1) && connect)
		temp |= 0x00001000;
	else if ((channel == 0) && !connect)
		temp &= ~0x00000100;
	else
		temp &= ~0x00001000;

	GT_REG_WRITE(GALMPSC_SERIAL_MULTIPLEX,temp);

	return 0;
}

static int
galmpsc_route_rx_clock(int channel, int brg)
{
	DECLARE_GLOBAL_DATA_PTR;
	unsigned int temp;

	temp = GTREGREAD_MIRROR_G(GALMPSC_RxC_ROUTE);

	if (channel == 0)
		temp |= brg;
	else
		temp |= (brg << 8);

	GT_REG_WRITE_MIRROR_G(GALMPSC_RxC_ROUTE,temp);

	return 0;
}

static int
galmpsc_route_tx_clock(int channel, int brg)
{
	DECLARE_GLOBAL_DATA_PTR;
	unsigned int temp;

	temp = GTREGREAD_MIRROR_G(GALMPSC_TxC_ROUTE);

	if (channel == 0)
		temp |= brg;
	else
		temp |= (brg << 8);

	GT_REG_WRITE_MIRROR_G(GALMPSC_TxC_ROUTE,temp);

	return 0;
}

static int
galmpsc_write_config_regs(int mpsc, int mode)
{
	if (mode == GALMPSC_UART) {
		/* Main config reg Low (Null modem, Enable Tx/Rx, UART mode) */
		GT_REG_WRITE(GALMPSC_MCONF_LOW + (mpsc*GALMPSC_REG_GAP),
			     0x000004c4);

		/* Main config reg High (32x Rx/Tx clock mode, width=8bits */
		GT_REG_WRITE(GALMPSC_MCONF_HIGH +(mpsc*GALMPSC_REG_GAP),
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

static int
galmpsc_config_channel_regs(int mpsc)
{
	DECLARE_GLOBAL_DATA_PTR;
	GT_REG_WRITE_MIRROR(GALMPSC_CHANNELREG_1,mpsc,GALMPSC_REG_GAP, 0);
	GT_REG_WRITE_MIRROR(GALMPSC_CHANNELREG_2,mpsc,GALMPSC_REG_GAP, 0);
	GT_REG_WRITE(GALMPSC_CHANNELREG_3+(mpsc*GALMPSC_REG_GAP), 1);
	GT_REG_WRITE(GALMPSC_CHANNELREG_4+(mpsc*GALMPSC_REG_GAP), 0);
	GT_REG_WRITE(GALMPSC_CHANNELREG_5+(mpsc*GALMPSC_REG_GAP), 0);
	GT_REG_WRITE(GALMPSC_CHANNELREG_6+(mpsc*GALMPSC_REG_GAP), 0);
	GT_REG_WRITE(GALMPSC_CHANNELREG_7+(mpsc*GALMPSC_REG_GAP), 0);
	GT_REG_WRITE(GALMPSC_CHANNELREG_8+(mpsc*GALMPSC_REG_GAP), 0);
	GT_REG_WRITE(GALMPSC_CHANNELREG_9+(mpsc*GALMPSC_REG_GAP), 0);
	GT_REG_WRITE(GALMPSC_CHANNELREG_10+(mpsc*GALMPSC_REG_GAP), 0);

	galmpsc_set_brkcnt(mpsc, 0x3);
	galmpsc_set_tcschar(mpsc, 0xab);

	return 0;
}

static int
galmpsc_set_brkcnt(int mpsc, int value)
{
	DECLARE_GLOBAL_DATA_PTR;
	unsigned int temp;

	temp = GTREGREAD_MIRROR(GALMPSC_CHANNELREG_1,mpsc,GALMPSC_REG_GAP);
	temp &= 0x0000FFFF;
	temp |= (value << 16);
	GT_REG_WRITE_MIRROR(GALMPSC_CHANNELREG_1,mpsc,GALMPSC_REG_GAP, temp);

	return 0;
}

static int
galmpsc_set_tcschar(int mpsc, int value)
{
	DECLARE_GLOBAL_DATA_PTR;
	unsigned int temp;

	temp = GTREGREAD_MIRROR(GALMPSC_CHANNELREG_1,mpsc,GALMPSC_REG_GAP);
	temp &= 0xFFFF0000;
	temp |= value;
	GT_REG_WRITE_MIRROR(GALMPSC_CHANNELREG_1,mpsc,GALMPSC_REG_GAP, temp);

	return 0;
}

static int
galmpsc_set_char_length(int mpsc, int value)
{
	DECLARE_GLOBAL_DATA_PTR;
	unsigned int temp;

	temp = GTREGREAD_MIRROR(GALMPSC_PROTOCONF_REG,mpsc,GALMPSC_REG_GAP);
	temp &= 0xFFFFCFFF;
	temp |= (value << 12);
	GT_REG_WRITE_MIRROR(GALMPSC_PROTOCONF_REG,mpsc,GALMPSC_REG_GAP, temp);

	return 0;
}

static int
galmpsc_set_stop_bit_length(int mpsc, int value)
{
	DECLARE_GLOBAL_DATA_PTR;
	unsigned int temp;

	temp = GTREGREAD_MIRROR(GALMPSC_PROTOCONF_REG,mpsc,GALMPSC_REG_GAP);
	temp |= (value << 14);
	GT_REG_WRITE_MIRROR(GALMPSC_PROTOCONF_REG,mpsc,GALMPSC_REG_GAP,temp);

	return 0;
}

static int
galmpsc_set_parity(int mpsc, int value)
{
	DECLARE_GLOBAL_DATA_PTR;
	unsigned int temp;

	temp = GTREGREAD_MIRROR(GALMPSC_CHANNELREG_2,mpsc,GALMPSC_REG_GAP);
	if (value != -1) {
		temp &= 0xFFF3FFF3;
		temp |= ((value << 18) | (value << 2));
		temp |= ((value << 17) | (value << 1));
	} else {
		temp &= 0xFFF1FFF1;
	}

	GT_REG_WRITE_MIRROR(GALMPSC_CHANNELREG_2,mpsc,GALMPSC_REG_GAP, temp);

	return 0;
}

static int
galmpsc_enter_hunt(int mpsc)
{
	DECLARE_GLOBAL_DATA_PTR;
	int temp;

	temp = GTREGREAD_MIRROR(GALMPSC_CHANNELREG_2,mpsc,GALMPSC_REG_GAP);
	temp |= 0x80000000;
	GT_REG_WRITE_MIRROR(GALMPSC_CHANNELREG_2,mpsc,GALMPSC_REG_GAP, temp);

	/* Should Poll on Enter Hunt bit, but the register is write-only */
	/* errata suggests pausing 100 system cycles */
	udelay(100);

	return 0;
}


static int
galmpsc_shutdown(int mpsc)
{
	DECLARE_GLOBAL_DATA_PTR;
#if 0
	unsigned int temp;

	/* cause RX abort (clears RX) */
	temp = GTREGREAD_MIRROR(GALMPSC_CHANNELREG_2,mpsc,GALMPSC_REG_GAP);
	temp |= MPSC_RX_ABORT | MPSC_TX_ABORT;
	temp &= ~MPSC_ENTER_HUNT;
	GT_REG_WRITE_MIRROR(GALMPSC_CHANNELREG_2,mpsc,GALMPSC_REG_GAP,temp);
#endif

	GT_REG_WRITE(GALSDMA_0_COM_REG + CHANNEL * GALSDMA_REG_DIFF, 0);
	GT_REG_WRITE(GALSDMA_0_COM_REG + CHANNEL * GALSDMA_REG_DIFF,
	             SDMA_TX_ABORT | SDMA_RX_ABORT);

	/* shut down the MPSC */
	GT_REG_WRITE(GALMPSC_MCONF_LOW, 0);
	GT_REG_WRITE(GALMPSC_MCONF_HIGH, 0);
	GT_REG_WRITE_MIRROR(GALMPSC_PROTOCONF_REG, mpsc, GALMPSC_REG_GAP,0);

	udelay(100);

	/* shut down the sdma engines. */
	/* reset config to default */
	GT_REG_WRITE(GALSDMA_0_CONF_REG + CHANNEL * GALSDMA_REG_DIFF,
	             0x000000fc);

	udelay(100);

	/* clear the SDMA current and first TX and RX pointers */
	GT_REG_WRITE(GALSDMA_0_CUR_RX_PTR + CHANNEL * GALSDMA_REG_DIFF, 0);
	GT_REG_WRITE(GALSDMA_0_CUR_TX_PTR + CHANNEL * GALSDMA_REG_DIFF, 0);
	GT_REG_WRITE(GALSDMA_0_FIR_TX_PTR + CHANNEL * GALSDMA_REG_DIFF, 0);

	udelay(100);

	return 0;
}

static void
galsdma_enable_rx(void)
{
	int temp;

	/* Enable RX processing */
	temp = GTREGREAD(GALSDMA_0_COM_REG+(CHANNEL*GALSDMA_REG_DIFF));
	temp |= RX_ENABLE;
	GT_REG_WRITE(GALSDMA_0_COM_REG+(CHANNEL*GALSDMA_REG_DIFF), temp);

	galmpsc_enter_hunt(CHANNEL);
}

static int
galmpsc_set_snoop(int mpsc, int value)
{
	int reg = mpsc ? MPSC_1_ADDRESS_CONTROL_LOW : MPSC_0_ADDRESS_CONTROL_LOW;
	int temp=GTREGREAD(reg);
	if(value)
	    temp |= (1<< 6) | (1<<14) | (1<<22) | (1<<30);
	else
	    temp &= ~((1<< 6) | (1<<14) | (1<<22) | (1<<30));
	GT_REG_WRITE(reg, temp);
	return 0;
}
