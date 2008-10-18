/* The dpalloc function used and implemented in this file was derieved
 * from PPCBoot/U-Boot file "cpu/mpc8260/commproc.c".
 */

/* Author: Arun Dharankar <ADharankar@ATTBI.Com>
 * This example is meant to only demonstrate how the IDMA could be used.
 */

/*
 * This file is based on "arch/ppc/8260_io/commproc.c" - here is it's
 * copyright notice:
 *
 * General Purpose functions for the global management of the
 * 8260 Communication Processor Module.
 * Copyright (c) 1999 Dan Malek (dmalek@jlc.net)
 * Copyright (c) 2000 MontaVista Software, Inc (source@mvista.com)
 *  2.3.99 Updates
 *
 * In addition to the individual control of the communication
 * channels, there are a few functions that globally affect the
 * communication processor.
 *
 * Buffer descriptors must be allocated from the dual ported memory
 * space.  The allocator for that is here.  When the communication
 * process is reset, we reclaim the memory available.  There is
 * currently no deallocator for this memory.
 */


#include <common.h>
#include <exports.h>

DECLARE_GLOBAL_DATA_PTR;

#define STANDALONE

#ifndef STANDALONE			/* Linked into/Part of  PPCBoot */
#include <command.h>
#include <watchdog.h>
#else					/* Standalone app of PPCBoot */
#define WATCHDOG_RESET() {						\
			*(ushort *)(CONFIG_SYS_IMMR + 0x1000E) = 0x556c;	\
			*(ushort *)(CONFIG_SYS_IMMR + 0x1000E) = 0xaa39;	\
		}
#endif	/* STANDALONE */

static int debug = 1;

#define DEBUG(fmt, args...)	 {					\
	if(debug != 0) {						\
		printf("[%s %d %s]: ",__FILE__,__LINE__,__FUNCTION__);	\
		printf(fmt, ##args);					\
	}								\
}

#define CPM_CR_IDMA1_SBLOCK  (0x14)
#define CPM_CR_IDMA2_SBLOCK  (0x15)
#define CPM_CR_IDMA3_SBLOCK  (0x16)
#define CPM_CR_IDMA4_SBLOCK  (0x17)
#define CPM_CR_IDMA1_PAGE    (0x07)
#define CPM_CR_IDMA2_PAGE    (0x08)
#define CPM_CR_IDMA3_PAGE    (0x09)
#define CPM_CR_IDMA4_PAGE    (0x0a)
#define PROFF_IDMA1_BASE     ((uint)0x87fe)
#define PROFF_IDMA2_BASE     ((uint)0x88fe)
#define PROFF_IDMA3_BASE     ((uint)0x89fe)
#define PROFF_IDMA4_BASE     ((uint)0x8afe)

#define CPM_CR_INIT_TRX     ((ushort)0x0000)
#define CPM_CR_FLG  ((ushort)0x0001)

#define mk_cr_cmd(PG, SBC, MCN, OP) \
    ((PG << 26) | (SBC << 21) | (MCN << 6) | OP)


#pragma pack(1)
typedef struct ibdbits {
	unsigned b_valid:1;
	unsigned b_resv1:1;
	unsigned b_wrap:1;
	unsigned b_interrupt:1;
	unsigned b_last:1;
	unsigned b_resv2:1;
	unsigned b_cm:1;
	unsigned b_resv3:2;
	unsigned b_sdn:1;
	unsigned b_ddn:1;
	unsigned b_dgbl:1;
	unsigned b_dbo:2;
	unsigned b_resv4:1;
	unsigned b_ddtb:1;
	unsigned b_resv5:2;
	unsigned b_sgbl:1;
	unsigned b_sbo:2;
	unsigned b_resv6:1;
	unsigned b_sdtb:1;
	unsigned b_resv7:9;
} ibdbits_t;

#pragma pack(1)
typedef union ibdbitsu {
	ibdbits_t b;
	uint i;
} ibdbitsu_t;

#pragma pack(1)
typedef struct idma_buf_desc {
	ibdbitsu_t ibd_bits;		/* Status and Control */
	uint ibd_datlen;		/* Data length in buffer */
	uint ibd_sbuf;			/* Source buffer addr in host mem */
	uint ibd_dbuf;			/* Destination buffer addr in host mem */
} ibd_t;


#pragma pack(1)
typedef struct dcmbits {
	unsigned b_fb:1;
	unsigned b_lp:1;
	unsigned b_resv1:3;
	unsigned b_tc2:1;
	unsigned b_resv2:1;
	unsigned b_wrap:3;
	unsigned b_sinc:1;
	unsigned b_dinc:1;
	unsigned b_erm:1;
	unsigned b_dt:1;
	unsigned b_sd:2;
} dcmbits_t;

#pragma pack(1)
typedef union dcmbitsu {
	dcmbits_t b;
	ushort i;
} dcmbitsu_t;

#pragma pack(1)
typedef struct pram_idma {
	ushort pi_ibase;
	dcmbitsu_t pi_dcmbits;
	ushort pi_ibdptr;
	ushort pi_dprbuf;
	ushort pi_bufinv;		/* internal to CPM */
	ushort pi_ssmax;
	ushort pi_dprinptr;		/* internal to CPM */
	ushort pi_sts;
	ushort pi_dproutptr;		/* internal to CPM */
	ushort pi_seob;
	ushort pi_deob;
	ushort pi_dts;
	ushort pi_retadd;
	ushort pi_resv1;		/* internal to CPM */
	uint pi_bdcnt;
	uint pi_sptr;
	uint pi_dptr;
	uint pi_istate;
} pram_idma_t;


volatile immap_t *immap = (immap_t *) CONFIG_SYS_IMMR;
volatile ibd_t *bdf;
volatile pram_idma_t *piptr;

volatile int dmadone;
volatile int *dmadonep = &dmadone;
void dmadone_handler (void *);

int idma_init (void);
void idma_start (int, int, int, uint, uint, int);
uint dpalloc (uint, uint);


uint dpinit_done = 0;


#ifdef STANDALONE
int ctrlc (void)
{
	if (tstc()) {
		switch (getc ()) {
		case 0x03:		/* ^C - Control C */
			return 1;
		default:
			break;
		}
	}
	return 0;
}
void * memset(void * s,int c,size_t count)
{
	char *xs = (char *) s;
	while (count--)
		*xs++ = c;
	return s;
}
int memcmp(const void * cs,const void * ct,size_t count)
{
	const unsigned char *su1, *su2;
	int res = 0;
	for( su1 = cs, su2 = ct; 0 < count; ++su1, ++su2, count--)
		if ((res = *su1 - *su2) != 0)
			break;
	return res;
}
#endif	/* STANDALONE */

#ifdef STANDALONE
int mem_to_mem_idma2intr (int argc, char *argv[])
#else
int do_idma (bd_t * bd, int argc, char *argv[])
#endif	/* STANDALONE */
{
	int i;

	app_startup(argv);
	dpinit_done = 0;

	idma_init ();

	DEBUG ("Installing dma handler\n");
	install_hdlr (7, dmadone_handler, (void *) bdf);

	memset ((void *) 0x100000, 'a', 512);
	memset ((void *) 0x200000, 'b', 512);

	for (i = 0; i < 32; i++) {
		printf ("Startin IDMA, iteration=%d\n", i);
		idma_start (1, 1, 512, 0x100000, 0x200000, 3);
	}

	DEBUG ("Uninstalling dma handler\n");
	free_hdlr (7);

	return 0;
}

void
idma_start (int sinc, int dinc, int sz, uint sbuf, uint dbuf, int ttype)
{
	/* ttype is for M-M, M-P, P-M or P-P: not used for now */

	piptr->pi_istate = 0;	/* manual says: clear it before every START_IDMA */
	piptr->pi_dcmbits.b.b_resv1 = 0;

	if (sinc == 1)
		piptr->pi_dcmbits.b.b_sinc = 1;
	else
		piptr->pi_dcmbits.b.b_sinc = 0;

	if (dinc == 1)
		piptr->pi_dcmbits.b.b_dinc = 1;
	else
		piptr->pi_dcmbits.b.b_dinc = 0;

	piptr->pi_dcmbits.b.b_erm = 0;
	piptr->pi_dcmbits.b.b_sd = 0x00;	/* M-M */

	bdf->ibd_sbuf = sbuf;
	bdf->ibd_dbuf = dbuf;
	bdf->ibd_bits.b.b_cm = 0;
	bdf->ibd_bits.b.b_interrupt = 1;
	bdf->ibd_bits.b.b_wrap = 1;
	bdf->ibd_bits.b.b_last = 1;
	bdf->ibd_bits.b.b_sdn = 0;
	bdf->ibd_bits.b.b_ddn = 0;
	bdf->ibd_bits.b.b_dgbl = 0;
	bdf->ibd_bits.b.b_ddtb = 0;
	bdf->ibd_bits.b.b_sgbl = 0;
	bdf->ibd_bits.b.b_sdtb = 0;
	bdf->ibd_bits.b.b_dbo = 1;
	bdf->ibd_bits.b.b_sbo = 1;
	bdf->ibd_bits.b.b_valid = 1;
	bdf->ibd_datlen = 512;

	*dmadonep = 0;

	immap->im_sdma.sdma_idmr2 = (uchar) 0xf;

	immap->im_cpm.cp_cpcr = mk_cr_cmd (CPM_CR_IDMA2_PAGE,
					   CPM_CR_IDMA2_SBLOCK, 0x0,
					   0x9) | 0x00010000;

	while (*dmadonep != 1) {
		if (ctrlc ()) {
			DEBUG ("\nInterrupted waiting for DMA interrupt.\n");
			goto done;
		}
		printf ("Waiting for DMA interrupt (dmadone=%d b_valid = %d)...\n",
			dmadone, bdf->ibd_bits.b.b_valid);
		udelay (1000000);
	}
	printf ("DMA complete notification received!\n");

  done:
	DEBUG ("memcmp(0x%08x, 0x%08x, 512) = %d\n",
		sbuf, dbuf, memcmp ((void *) sbuf, (void *) dbuf, 512));

	return;
}

#define MAX_INT_BUFSZ	64
#define DCM_WRAP	 0	/* MUST be consistant with MAX_INT_BUFSZ */

int idma_init (void)
{
	uint memaddr;

	immap->im_cpm.cp_rccr &= ~0x00F3FFFF;
	immap->im_cpm.cp_rccr |= 0x00A00A00;

	memaddr = dpalloc (sizeof (pram_idma_t), 64);

	*(volatile ushort *) &immap->im_dprambase[PROFF_IDMA2_BASE] = memaddr;
	piptr = (volatile pram_idma_t *) ((uint) (immap) + memaddr);

	piptr->pi_resv1 = 0;		/* manual says: clear it */
	piptr->pi_dcmbits.b.b_fb = 0;
	piptr->pi_dcmbits.b.b_lp = 1;
	piptr->pi_dcmbits.b.b_erm = 0;
	piptr->pi_dcmbits.b.b_dt = 0;

	memaddr = (uint) dpalloc (sizeof (ibd_t), 64);
	piptr->pi_ibase = piptr->pi_ibdptr = (volatile short) memaddr;
	bdf = (volatile ibd_t *) ((uint) (immap) + memaddr);
	bdf->ibd_bits.b.b_valid = 0;

	memaddr = (uint) dpalloc (64, 64);
	piptr->pi_dprbuf = (volatile ushort) memaddr;
	piptr->pi_dcmbits.b.b_wrap = 4;
	piptr->pi_ssmax = 32;

	piptr->pi_sts = piptr->pi_ssmax;
	piptr->pi_dts = piptr->pi_ssmax;

	return 1;
}

void dmadone_handler (void *arg)
{
	immap->im_sdma.sdma_idmr2 = (uchar) 0x0;

	*dmadonep = 1;

	return;
}


static uint dpbase = 0;

uint dpalloc (uint size, uint align)
{
	volatile immap_t *immr = (immap_t *) CONFIG_SYS_IMMR;
	uint retloc;
	uint align_mask, off;
	uint savebase;

	/* Pointer to initial global data area */

	if (dpinit_done == 0) {
		dpbase = gd->dp_alloc_base;
		dpinit_done = 1;
	}

	align_mask = align - 1;
	savebase = dpbase;

	if ((off = (dpbase & align_mask)) != 0)
		dpbase += (align - off);

	if ((off = size & align_mask) != 0)
		size += align - off;

	if ((dpbase + size) >= gd->dp_alloc_top) {
		dpbase = savebase;
		printf ("dpalloc: ran out of dual port ram!");
		return 0;
	}

	retloc = dpbase;
	dpbase += size;

	memset ((void *) &immr->im_dprambase[retloc], 0, size);

	return (retloc);
}
