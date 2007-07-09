/*
 * genietv/genietv.c
 *
 * The GENIETV is using the following physical memorymap (copied from
 * the FADS configuration):
 *
 * ff020000 -> ff02ffff : pcmcia
 * ff010000 -> ff01ffff : BCSR       connected to CS1, setup by 8xxROM
 * ff000000 -> ff00ffff : IMAP       internal in the cpu
 * 02800000 -> 0287ffff : flash      connected to CS0
 * 00000000 -> nnnnnnnn : sdram      setup by U-Boot
 *
 * CS pins are connected as follows:
 *
 * CS0 -512Kb boot flash
 * CS1 - SDRAM #1
 * CS2 - SDRAM #2
 * CS3 - Flash #1
 * CS4 - Flash #2
 * CS5 - LON (if present)
 * CS6 - PCMCIA #1
 * CS7 - PCMCIA #2
 *
 * Ports are configured as follows:
 *
 * PA7 - SDRAM banks enable
 */

#include <common.h>
#include <mpc8xx.h>

#define CFG_PA7		0x0100

/* ------------------------------------------------------------------------- */

static long int dram_size (long int, long int *, long int);

/* ------------------------------------------------------------------------- */

#define	_NOT_USED_	0xFFFFFFFF

const uint sdram_table[] = {
	/*
	 * Single Read. (Offset 0 in UPMB RAM)
	 */
	0x1F0DFC04, 0xEEAFBC04, 0x11AF7C04, 0xEFBEEC00,
	0x1FFDDC47,		/* last */
	/*
	 * SDRAM Initialization (offset 5 in UPMB RAM)
	 *
	 * This is no UPM entry point. The following definition uses
	 * the remaining space to establish an initialization
	 * sequence, which is executed by a RUN command.
	 *
	 */
	0x1FFDDC34, 0xEFEEAC34, 0x1FBD5C35,	/* last */
	/*
	 * Burst Read. (Offset 8 in UPMB RAM)
	 */
	0x1F0DFC04, 0xEEAFBC04, 0x10AF7C04, 0xF0AFFC00,
	0xF0AFFC00, 0xF1AFFC00, 0xEFBEEC00, 0x1FFDDC47,	/* last */
	_NOT_USED_, _NOT_USED_, _NOT_USED_, _NOT_USED_,
	_NOT_USED_, _NOT_USED_, _NOT_USED_, _NOT_USED_,
	/*
	 * Single Write. (Offset 18 in UPMB RAM)
	 */
	0x1F2DFC04, 0xEEAFAC00, 0x01BE4C04, 0x1FFDDC47,	/* last */
	_NOT_USED_, _NOT_USED_, _NOT_USED_, _NOT_USED_,
	/*
	 * Burst Write. (Offset 20 in UPMB RAM)
	 */
	0x1F0DFC04, 0xEEAFAC00, 0x10AF5C00, 0xF0AFFC00,
	0xF0AFFC00, 0xE1BEEC04, 0x1FFDDC47,	/* last */
	_NOT_USED_,
	_NOT_USED_, _NOT_USED_, _NOT_USED_, _NOT_USED_,
	_NOT_USED_, _NOT_USED_, _NOT_USED_, _NOT_USED_,
	/*
	 * Refresh  (Offset 30 in UPMB RAM)
	 */
	0x1FFD7C84, 0xFFFFFC04, 0xFFFFFC04, 0xFFFFFC04,
	0xFFFFFC84, 0xFFFFFC07,	/* last */
	_NOT_USED_, _NOT_USED_,
	_NOT_USED_, _NOT_USED_, _NOT_USED_, _NOT_USED_,
	/*
	 * Exception. (Offset 3c in UPMB RAM)
	 */
	0x7FFFFC07,		/* last */
	_NOT_USED_, _NOT_USED_, _NOT_USED_,
};

/* ------------------------------------------------------------------------- */


/*
 * Check Board Identity
 */

int checkboard (void)
{
	puts ("Board: GenieTV\n");
	return 0;
}

#if 0
static void PrintState (void)
{
	volatile immap_t *im = (immap_t *) CFG_IMMR;
	volatile memctl8xx_t *memctl = &im->im_memctl;

	printf ("\n0 - FLASH: B=%08x O=%08x", memctl->memc_br0,
		memctl->memc_or0);
	printf ("\n1 - SDRAM: B=%08x O=%08x", memctl->memc_br1,
		memctl->memc_or1);
	printf ("\n2 - SDRAM: B=%08x O=%08x", memctl->memc_br2,
		memctl->memc_or2);
}
#endif

/* ------------------------------------------------------------------------- */

long int initdram (int board_type)
{
	volatile immap_t *im = (immap_t *) CFG_IMMR;
	volatile memctl8xx_t *memctl = &im->im_memctl;
	long int size_b0, size_b1, size8;

	/* Enable SDRAM */

	/* Configuring PA7 for general purpouse output pin */
	im->im_ioport.iop_papar &= ~CFG_PA7;	/* 0 = general purpouse */
	im->im_ioport.iop_padir |= CFG_PA7;	/* 1 = output */

	/* Enable SDRAM - PA7 = 1 */
	im->im_ioport.iop_padat |= CFG_PA7;	/* value of PA7 */

	/*
	 * Preliminary prescaler for refresh (depends on number of
	 * banks): This value is selected for four cycles every 62.4 us
	 * with two SDRAM banks or four cycles every 31.2 us with one
	 * bank. It will be adjusted after memory sizing.
	 */
	memctl->memc_mptpr = CFG_MPTPR_2BK_4K;

	memctl->memc_mbmr = CFG_MBMR_8COL;

	upmconfig (UPMB, (uint *) sdram_table,
		   sizeof (sdram_table) / sizeof (uint));

	/*
	 * Map controller banks 1 and 2 to the SDRAM banks 1 and 2 at
	 * preliminary addresses - these have to be modified after the
	 * SDRAM size has been determined.
	 */

	memctl->memc_or1 = 0xF0000000 | CFG_OR_TIMING_SDRAM;
	memctl->memc_br1 =
		((SDRAM_BASE1_PRELIM & BR_BA_MSK) | BR_MS_UPMB | BR_V);

	memctl->memc_or2 = 0xF0000000 | CFG_OR_TIMING_SDRAM;
	memctl->memc_br2 =
		((SDRAM_BASE2_PRELIM & BR_BA_MSK) | BR_MS_UPMB | BR_V);

	/* perform SDRAM initialization sequence */
	memctl->memc_mar = 0x00000088;

	memctl->memc_mcr = 0x80802105;	/* SDRAM bank 0 */

	memctl->memc_mcr = 0x80804105;	/* SDRAM bank 1 */

	/* Execute refresh 8 times */
	memctl->memc_mbmr = (CFG_MBMR_8COL & ~MBMR_TLFB_MSK) | MBMR_TLFB_8X;

	memctl->memc_mcr = 0x80802130;	/* SDRAM bank 0 - execute twice */

	memctl->memc_mcr = 0x80804130;	/* SDRAM bank 1 - execute twice */

	/* Execute refresh 4 times */
	memctl->memc_mbmr = CFG_MBMR_8COL;

	/*
	 * Check Bank 0 Memory Size for re-configuration
	 *
	 * try 8 column mode
	 */

#if 0
	PrintState ();
#endif
/*    printf ("\nChecking bank1..."); */
	size8 = dram_size (CFG_MBMR_8COL, (long *) SDRAM_BASE1_PRELIM,
			   SDRAM_MAX_SIZE);

	size_b0 = size8;

/*    printf ("\nChecking bank2..."); */
	size_b1 =
		dram_size (memctl->memc_mbmr, (long *) SDRAM_BASE2_PRELIM,
			   SDRAM_MAX_SIZE);

	/*
	 * Final mapping: map bigger bank first
	 */

	memctl->memc_or1 = ((-size_b0) & 0xFFFF0000) | CFG_OR_TIMING_SDRAM;
	memctl->memc_br1 = (CFG_SDRAM_BASE & BR_BA_MSK) | BR_MS_UPMB | BR_V;

	if (size_b1 > 0) {
		/*
		 * Position Bank 1 immediately above Bank 0
		 */
		memctl->memc_or2 =
			((-size_b1) & 0xFFFF0000) | CFG_OR_TIMING_SDRAM;
		memctl->memc_br2 =
			((CFG_SDRAM_BASE & BR_BA_MSK) | BR_MS_UPMB | BR_V) +
			(size_b0 & BR_BA_MSK);
	} else {
		/*
		 * No bank 1
		 *
		 * invalidate bank
		 */
		memctl->memc_br2 = 0;
		/* adjust refresh rate depending on SDRAM type, one bank */
		memctl->memc_mptpr = CFG_MPTPR_1BK_4K;
	}

	/* If no memory detected, disable SDRAM */
	if ((size_b0 + size_b1) == 0) {
		printf ("disabling SDRAM!\n");
		/* Disable SDRAM - PA7 = 1 */
		im->im_ioport.iop_padat &= ~CFG_PA7;	/* value of PA7 */
	}
/*	else */
/*    printf("done! (%08lx)\n", size_b0 + size_b1); */

#if 0
	PrintState ();
#endif
	return (size_b0 + size_b1);
}

/* ------------------------------------------------------------------------- */

/*
 * Check memory range for valid RAM. A simple memory test determines
 * the actually available RAM size between addresses `base' and
 * `base + maxsize'. Some (not all) hardware errors are detected:
 * - short between address lines
 * - short between data lines
 */

static long int dram_size (long int mbmr_value, long int *base,
			   long int maxsize)
{
	long size;

	/*memctl->memc_mbmr = mbmr_value; */

	size = get_ram_size (base, maxsize);

	if (size) {
/*      printf("(%08lx)", size); */
	} else {
		printf ("(0)");
	}

	return (size);
}

#if defined(CONFIG_CMD_PCMCIA)

#ifdef	CFG_PCMCIA_MEM_ADDR
volatile unsigned char *pcmcia_mem = (unsigned char *) CFG_PCMCIA_MEM_ADDR;
#endif

int pcmcia_init (void)
{
	volatile pcmconf8xx_t *pcmp;
	uint v, slota, slotb;

	/*
	 ** Enable the PCMCIA for a Flash card.
	 */
	pcmp = (pcmconf8xx_t *) (&(((immap_t *) CFG_IMMR)->im_pcmcia));

#if 0
	pcmp->pcmc_pbr0 = CFG_PCMCIA_MEM_ADDR;
	pcmp->pcmc_por0 = 0xc00ff05d;
#endif

	/* Set all slots to zero by default. */
	pcmp->pcmc_pgcra = 0;
	pcmp->pcmc_pgcrb = 0;
#ifdef PCMCIA_SLOT_A
	pcmp->pcmc_pgcra = 0x40;
#endif
#ifdef PCMCIA_SLOT_B
	pcmp->pcmc_pgcrb = 0x40;
#endif

	/* Check if any PCMCIA card is luged in. */
	slota = (pcmp->pcmc_pipr & 0x18000000) == 0;
	slotb = (pcmp->pcmc_pipr & 0x00001800) == 0;

	if (!(slota || slotb)) {
		printf ("No card present\n");
#ifdef PCMCIA_SLOT_A
		pcmp->pcmc_pgcra = 0;
#endif
#ifdef PCMCIA_SLOT_B
		pcmp->pcmc_pgcrb = 0;
#endif
		return -1;
	} else
		printf ("Unknown card (");

	v = 0;

	switch ((pcmp->pcmc_pipr >> 14) & 3) {
	case 0x00:
		printf ("5V");
		v = 5;
		break;
	case 0x01:
		printf ("5V and 3V");
		v = 3;
		break;
	case 0x03:
		printf ("5V, 3V and x.xV");
		v = 3;
		break;
	}

	switch (v) {
	case 3:
		printf ("; using 3V");
		/* Enable 3 volt Vcc. */

		break;

	default:
		printf ("; unknown voltage");
		return -1;
	}
	printf (")\n");
	/* disable pcmcia reset after a while */

	udelay (20);

	pcmp->pcmc_pgcrb = 0;

	/* If you using a real hd you should give a short
	 * spin-up time. */
#ifdef CONFIG_DISK_SPINUP_TIME
	udelay (CONFIG_DISK_SPINUP_TIME);
#endif

	return 0;
}
#endif /* CFG_CMD_PCMCIA */
