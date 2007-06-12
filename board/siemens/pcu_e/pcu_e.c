/*
 * (C) Copyright 2001
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
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

#include <common.h>
#include <mpc8xx.h>
#include <commproc.h>
#include <i2c.h>
#include <command.h>

/* ------------------------------------------------------------------------- */

static long int dram_size (long int, long int *, long int);
static void puma_status (void);
static void puma_set_mode (int mode);
static int puma_init_done (void);
static void puma_load (ulong addr, ulong len);

/* ------------------------------------------------------------------------- */

#define	_NOT_USED_	0xFFFFFFFF

/*
 * 50 MHz SDRAM access using UPM A
 */
const uint sdram_table[] = {
	/*
	 * Single Read. (Offset 0 in UPM RAM)
	 */
	0x1f0dfc04, 0xeeafbc04, 0x11af7c04, 0xefbeec00,
	0x1ffddc47,		/* last */
	/*
	 * SDRAM Initialization (offset 5 in UPM RAM)
	 *
	 * This is no UPM entry point. The following definition uses
	 * the remaining space to establish an initialization
	 * sequence, which is executed by a RUN command.
	 *
	 */
	0x1ffddc35, 0xefceac34, 0x1f3d5c35,	/* last */
	/*
	 * Burst Read. (Offset 8 in UPM RAM)
	 */
	0x1f0dfc04, 0xeeafbc04, 0x10af7c04, 0xf0affc00,
	0xf0affc00, 0xf1affc00, 0xefbeec00, 0x1ffddc47,	/* last */
	_NOT_USED_, _NOT_USED_, _NOT_USED_, _NOT_USED_,
	_NOT_USED_, _NOT_USED_, _NOT_USED_, _NOT_USED_,

	/*
	 * Single Write. (Offset 18 in UPM RAM)
	 */
	0x1f0dfc04, 0xeeafac00, 0x01be4c04, 0x1ffddc47,	/* last */
	_NOT_USED_, _NOT_USED_, _NOT_USED_, _NOT_USED_,
	/*
	 * Burst Write. (Offset 20 in UPM RAM)
	 */
	0x1f0dfc04, 0xeeafac00, 0x10af5c00, 0xf0affc00,
	0xf0affc00, 0xe1beec04, 0x1ffddc47,	/* last */
	_NOT_USED_,
	_NOT_USED_, _NOT_USED_, _NOT_USED_, _NOT_USED_,
	_NOT_USED_, _NOT_USED_, _NOT_USED_, _NOT_USED_,
	/*
	 * Refresh  (Offset 30 in UPM RAM)
	 */
	0x1ffd7c84, 0xfffffc04, 0xfffffc04, 0xfffffc04,
	0xfffffc84, 0xfffffc07,	/* last */
	_NOT_USED_, _NOT_USED_,
	_NOT_USED_, _NOT_USED_, _NOT_USED_, _NOT_USED_,
	/*
	 * Exception. (Offset 3c in UPM RAM)
	 */
	0x7ffffc07,		/* last */
	_NOT_USED_, _NOT_USED_, _NOT_USED_,
};

/* ------------------------------------------------------------------------- */

/*
 * PUMA access using UPM B
 */
const uint puma_table[] = {
	/*
	 * Single Read. (Offset 0 in UPM RAM)
	 */
	_NOT_USED_, _NOT_USED_, _NOT_USED_, _NOT_USED_,
	_NOT_USED_,
	/*
	 * Precharge and MRS
	 */
	_NOT_USED_, _NOT_USED_, _NOT_USED_,
	/*
	 * Burst Read. (Offset 8 in UPM RAM)
	 */
	_NOT_USED_, _NOT_USED_, _NOT_USED_, _NOT_USED_,
	_NOT_USED_, _NOT_USED_, _NOT_USED_, _NOT_USED_,
	_NOT_USED_, _NOT_USED_, _NOT_USED_, _NOT_USED_,
	_NOT_USED_, _NOT_USED_, _NOT_USED_, _NOT_USED_,
	/*
	 * Single Write. (Offset 18 in UPM RAM)
	 */
	0x0ffff804, 0x0ffff400, 0x3ffffc47,	/* last */
	_NOT_USED_,
	_NOT_USED_, _NOT_USED_, _NOT_USED_, _NOT_USED_,
	/*
	 * Burst Write. (Offset 20 in UPM RAM)
	 */
	_NOT_USED_, _NOT_USED_, _NOT_USED_, _NOT_USED_,
	_NOT_USED_, _NOT_USED_, _NOT_USED_, _NOT_USED_,
	_NOT_USED_, _NOT_USED_, _NOT_USED_, _NOT_USED_,
	_NOT_USED_, _NOT_USED_, _NOT_USED_, _NOT_USED_,
	/*
	 * Refresh  (Offset 30 in UPM RAM)
	 */
	_NOT_USED_, _NOT_USED_, _NOT_USED_, _NOT_USED_,
	_NOT_USED_, _NOT_USED_, _NOT_USED_, _NOT_USED_,
	_NOT_USED_, _NOT_USED_, _NOT_USED_, _NOT_USED_,
	/*
	 * Exception. (Offset 3c in UPM RAM)
	 */
	0x7ffffc07,		/* last */
	_NOT_USED_, _NOT_USED_, _NOT_USED_,
};

/* ------------------------------------------------------------------------- */


/*
 * Check Board Identity:
 *
 */

int checkboard (void)
{
	puts ("Board: Siemens PCU E\n");
	return (0);
}

/* ------------------------------------------------------------------------- */

long int initdram (int board_type)
{
	volatile immap_t *immr = (immap_t *) CFG_IMMR;
	volatile memctl8xx_t *memctl = &immr->im_memctl;
	long int size_b0, reg;
	int i;

	/*
	 * Configure UPMA for SDRAM
	 */
	upmconfig (UPMA, (uint *) sdram_table,
		   sizeof (sdram_table) / sizeof (uint));

	memctl->memc_mptpr = CFG_MPTPR;

	/* burst length=4, burst type=sequential, CAS latency=2 */
	memctl->memc_mar = 0x00000088;

	/*
	 * Map controller bank 2 to the SDRAM bank at preliminary address.
	 */
#if PCU_E_WITH_SWAPPED_CS	/* XXX */
	memctl->memc_or5 = CFG_OR5_PRELIM;
	memctl->memc_br5 = CFG_BR5_PRELIM;
#else  /* XXX */
	memctl->memc_or2 = CFG_OR2_PRELIM;
	memctl->memc_br2 = CFG_BR2_PRELIM;
#endif /* XXX */

	/* initialize memory address register */
	memctl->memc_mamr = CFG_MAMR;	/* refresh not enabled yet */

	/* mode initialization (offset 5) */
#if PCU_E_WITH_SWAPPED_CS	/* XXX */
	udelay (200);		/* 0x8000A105 */
	memctl->memc_mcr = MCR_OP_RUN | MCR_MB_CS5 | MCR_MLCF (1) | MCR_MAD (0x05);
#else  /* XXX */
	udelay (200);		/* 0x80004105 */
	memctl->memc_mcr = MCR_OP_RUN | MCR_MB_CS2 | MCR_MLCF (1) | MCR_MAD (0x05);
#endif /* XXX */

	/* run 2 refresh sequence with 4-beat refresh burst (offset 0x30) */
#if PCU_E_WITH_SWAPPED_CS	/* XXX */
	udelay (1);		/* 0x8000A830 */
	memctl->memc_mcr = MCR_OP_RUN | MCR_MB_CS5 | MCR_MLCF (8) | MCR_MAD (0x30);
#else  /* XXX */
	udelay (1);		/* 0x80004830 */
	memctl->memc_mcr = MCR_OP_RUN | MCR_MB_CS2 | MCR_MLCF (8) | MCR_MAD (0x30);
#endif /* XXX */

#if PCU_E_WITH_SWAPPED_CS	/* XXX */
	udelay (1);		/* 0x8000A106 */
	memctl->memc_mcr = MCR_OP_RUN | MCR_MB_CS5 | MCR_MLCF (1) | MCR_MAD (0x06);
#else  /* XXX */
	udelay (1);		/* 0x80004106 */
	memctl->memc_mcr = MCR_OP_RUN | MCR_MB_CS2 | MCR_MLCF (1) | MCR_MAD (0x06);
#endif /* XXX */

	reg = memctl->memc_mamr;
	reg &= ~MAMR_TLFA_MSK;	/* switch timer loop ... */
	reg |= MAMR_TLFA_4X;	/* ... to 4x */
	reg |= MAMR_PTAE;	/* enable refresh */
	memctl->memc_mamr = reg;

	udelay (200);

	/* Need at least 10 DRAM accesses to stabilize */
	for (i = 0; i < 10; ++i) {
#if PCU_E_WITH_SWAPPED_CS	/* XXX */
		volatile unsigned long *addr =
			(volatile unsigned long *) SDRAM_BASE5_PRELIM;
#else  /* XXX */
		volatile unsigned long *addr =
			(volatile unsigned long *) SDRAM_BASE2_PRELIM;
#endif /* XXX */
		unsigned long val;

		val = *(addr + i);
		*(addr + i) = val;
	}

	/*
	 * Check Bank 0 Memory Size for re-configuration
	 */
#if PCU_E_WITH_SWAPPED_CS	/* XXX */
	size_b0 = dram_size (CFG_MAMR, (long *) SDRAM_BASE5_PRELIM, SDRAM_MAX_SIZE);
#else  /* XXX */
	size_b0 = dram_size (CFG_MAMR, (long *) SDRAM_BASE2_PRELIM, SDRAM_MAX_SIZE);
#endif /* XXX */

	memctl->memc_mamr = CFG_MAMR | MAMR_PTAE;

	/*
	 * Final mapping:
	 */

#if PCU_E_WITH_SWAPPED_CS	/* XXX */
	memctl->memc_or5 = ((-size_b0) & 0xFFFF0000) | SDRAM_TIMING;
	memctl->memc_br5 = (CFG_SDRAM_BASE & BR_BA_MSK) | BR_MS_UPMA | BR_V;
#else  /* XXX */
	memctl->memc_or2 = ((-size_b0) & 0xFFFF0000) | SDRAM_TIMING;
	memctl->memc_br2 = (CFG_SDRAM_BASE & BR_BA_MSK) | BR_MS_UPMA | BR_V;
#endif /* XXX */
	udelay (1000);

	/*
	 * Configure UPMB for PUMA
	 */
	upmconfig (UPMB, (uint *) puma_table,
		   sizeof (puma_table) / sizeof (uint));

	return (size_b0);
}

/* ------------------------------------------------------------------------- */

/*
 * Check memory range for valid RAM. A simple memory test determines
 * the actually available RAM size between addresses `base' and
 * `base + maxsize'. Some (not all) hardware errors are detected:
 * - short between address lines
 * - short between data lines
 */

static long int dram_size (long int mamr_value, long int *base,
			   long int maxsize)
{
	volatile immap_t *immr = (immap_t *) CFG_IMMR;
	volatile memctl8xx_t *memctl = &immr->im_memctl;

	memctl->memc_mamr = mamr_value;

	return (get_ram_size (base, maxsize));
}

/* ------------------------------------------------------------------------- */

#if PCU_E_WITH_SWAPPED_CS	/* XXX */
#define	ETH_CFG_BITS	(CFG_PB_ETH_CFG1 | CFG_PB_ETH_CFG2  | CFG_PB_ETH_CFG3 )
#else  /* XXX */
#define	ETH_CFG_BITS	(CFG_PB_ETH_MDDIS | CFG_PB_ETH_CFG1 | \
			 CFG_PB_ETH_CFG2  | CFG_PB_ETH_CFG3 )
#endif /* XXX */

#define ETH_ALL_BITS	(ETH_CFG_BITS | CFG_PB_ETH_POWERDOWN | CFG_PB_ETH_RESET)

void reset_phy (void)
{
	immap_t *immr = (immap_t *) CFG_IMMR;
	ulong value;

	/* Configure all needed port pins for GPIO */
#if PCU_E_WITH_SWAPPED_CS	/* XXX */
# ifdef CFG_ETH_MDDIS_VALUE
	immr->im_ioport.iop_padat |= CFG_PA_ETH_MDDIS;
# else
	immr->im_ioport.iop_padat &= ~(CFG_PA_ETH_MDDIS);	/* Set low */
# endif
	immr->im_ioport.iop_papar &= ~(CFG_PA_ETH_MDDIS);	/* GPIO */
	immr->im_ioport.iop_paodr &= ~(CFG_PA_ETH_MDDIS);	/* active output */
	immr->im_ioport.iop_padir |= CFG_PA_ETH_MDDIS;	/* output */
#endif /* XXX */
	immr->im_cpm.cp_pbpar &= ~(ETH_ALL_BITS);	/* GPIO */
	immr->im_cpm.cp_pbodr &= ~(ETH_ALL_BITS);	/* active output */

	value = immr->im_cpm.cp_pbdat;

	/* Assert Powerdown and Reset signals */
	value |= CFG_PB_ETH_POWERDOWN;
	value &= ~(CFG_PB_ETH_RESET);

	/* PHY configuration includes MDDIS and CFG1 ... CFG3 */
#if !PCU_E_WITH_SWAPPED_CS
# ifdef CFG_ETH_MDDIS_VALUE
	value |= CFG_PB_ETH_MDDIS;
# else
	value &= ~(CFG_PB_ETH_MDDIS);
# endif
#endif
#ifdef CFG_ETH_CFG1_VALUE
	value |= CFG_PB_ETH_CFG1;
#else
	value &= ~(CFG_PB_ETH_CFG1);
#endif
#ifdef CFG_ETH_CFG2_VALUE
	value |= CFG_PB_ETH_CFG2;
#else
	value &= ~(CFG_PB_ETH_CFG2);
#endif
#ifdef CFG_ETH_CFG3_VALUE
	value |= CFG_PB_ETH_CFG3;
#else
	value &= ~(CFG_PB_ETH_CFG3);
#endif

	/* Drive output signals to initial state */
	immr->im_cpm.cp_pbdat = value;
	immr->im_cpm.cp_pbdir |= ETH_ALL_BITS;
	udelay (10000);

	/* De-assert Ethernet Powerdown */
	immr->im_cpm.cp_pbdat &= ~(CFG_PB_ETH_POWERDOWN);	/* Enable PHY power */
	udelay (10000);

	/* de-assert RESET signal of PHY */
	immr->im_cpm.cp_pbdat |= CFG_PB_ETH_RESET;
	udelay (1000);
}

/*-----------------------------------------------------------------------
 * Board Special Commands: access functions for "PUMA" FPGA
 */
#if (CONFIG_COMMANDS & CFG_CMD_BSP) || defined(CONFIG_CMD_BSP)

#define	PUMA_READ_MODE	0
#define PUMA_LOAD_MODE	1

int do_puma (cmd_tbl_t * cmdtp, int flag, int argc, char *argv[])
{
	ulong addr, len;

	switch (argc) {
	case 2:		/* PUMA reset */
		if (strncmp (argv[1], "stat", 4) == 0) {	/* Reset */
			puma_status ();
			return 0;
		}
		break;
	case 4:		/* PUMA load addr len */
		if (strcmp (argv[1], "load") != 0)
			break;

		addr = simple_strtoul (argv[2], NULL, 16);
		len = simple_strtoul (argv[3], NULL, 16);

		printf ("PUMA load: addr %08lX len %ld (0x%lX):  ",
			addr, len, len);
		puma_load (addr, len);

		return 0;
	default:
		break;
	}
	printf ("Usage:\n%s\n", cmdtp->usage);
	return 1;
}

U_BOOT_CMD (puma, 4, 1, do_puma,
	    "puma    - access PUMA FPGA\n",
	    "status - print PUMA status\n"
	    "puma load addr len - load PUMA configuration data\n");

#endif /* CFG_CMD_BSP */

/* . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . */

static void puma_set_mode (int mode)
{
	volatile immap_t *immr = (immap_t *) CFG_IMMR;
	volatile memctl8xx_t *memctl = &immr->im_memctl;

	/* disable PUMA in memory controller */
#if PCU_E_WITH_SWAPPED_CS	/* XXX */
	memctl->memc_br3 = 0;
#else  /* XXX */
	memctl->memc_br4 = 0;
#endif /* XXX */

	switch (mode) {
	case PUMA_READ_MODE:
#if PCU_E_WITH_SWAPPED_CS	/* XXX */
		memctl->memc_or3 = PUMA_CONF_OR_READ;
		memctl->memc_br3 = PUMA_CONF_BR_READ;
#else  /* XXX */
		memctl->memc_or4 = PUMA_CONF_OR_READ;
		memctl->memc_br4 = PUMA_CONF_BR_READ;
#endif /* XXX */
		break;
	case PUMA_LOAD_MODE:
#if PCU_E_WITH_SWAPPED_CS	/* XXX */
		memctl->memc_or3 = PUMA_CONF_OR_LOAD;
		memctl->memc_br3 = PUMA_CONF_BR_LOAD;
#else  /* XXX */
		memctl->memc_or4 = PUMA_CONF_OR_READ;
		memctl->memc_br4 = PUMA_CONF_BR_READ;
#endif /* XXX */
		break;
	}
}

/* . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . */

#define	PUMA_INIT_TIMEOUT	1000	/* max. 1000 ms = 1 second */

static void puma_load (ulong addr, ulong len)
{
	volatile immap_t *immr = (immap_t *) CFG_IMMR;
	volatile uchar *fpga_addr = (volatile uchar *) PUMA_CONF_BASE;	/* XXX ??? */
	uchar *data = (uchar *) addr;
	int i;

	/* align length */
	if (len & 1)
		++len;

	/* Reset FPGA */
	immr->im_ioport.iop_pcpar &= ~(CFG_PC_PUMA_INIT);	/* make input */
	immr->im_ioport.iop_pcso  &= ~(CFG_PC_PUMA_INIT);
	immr->im_ioport.iop_pcdir &= ~(CFG_PC_PUMA_INIT);

#if PCU_E_WITH_SWAPPED_CS	/* XXX */
	immr->im_cpm.cp_pbpar &= ~(CFG_PB_PUMA_PROG);		/* GPIO */
	immr->im_cpm.cp_pbodr &= ~(CFG_PB_PUMA_PROG);		/* active output */
	immr->im_cpm.cp_pbdat &= ~(CFG_PB_PUMA_PROG);		/* Set low */
	immr->im_cpm.cp_pbdir |=   CFG_PB_PUMA_PROG;		/* output */
#else
	immr->im_ioport.iop_papar &= ~(CFG_PA_PUMA_PROG);	/* GPIO */
	immr->im_ioport.iop_padat &= ~(CFG_PA_PUMA_PROG);	/* Set low */
	immr->im_ioport.iop_paodr &= ~(CFG_PA_PUMA_PROG);	/* active output */
	immr->im_ioport.iop_padir |=   CFG_PA_PUMA_PROG;	/* output */
#endif /* XXX */
	udelay (100);

#if PCU_E_WITH_SWAPPED_CS	/* XXX */
	immr->im_cpm.cp_pbdat |= CFG_PB_PUMA_PROG;	/* release reset */
#else
	immr->im_ioport.iop_padat |= CFG_PA_PUMA_PROG;	/* release reset */
#endif /* XXX */

	/* wait until INIT indicates completion of reset */
	for (i = 0; i < PUMA_INIT_TIMEOUT; ++i) {
		udelay (1000);
		if (immr->im_ioport.iop_pcdat & CFG_PC_PUMA_INIT)
			break;
	}
	if (i == PUMA_INIT_TIMEOUT) {
		printf ("*** PUMA init timeout ***\n");
		return;
	}

	puma_set_mode (PUMA_LOAD_MODE);

	while (len--)
		*fpga_addr = *data++;

	puma_set_mode (PUMA_READ_MODE);

	puma_status ();
}

/* . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . */

static void puma_status (void)
{
	/* Check state */
	printf ("PUMA initialization is %scomplete\n",
		puma_init_done ()? "" : "NOT ");
}

/* . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . */

static int puma_init_done (void)
{
	volatile immap_t *immr = (immap_t *) CFG_IMMR;

	/* make sure pin is GPIO input */
	immr->im_ioport.iop_pcpar &= ~(CFG_PC_PUMA_DONE);
	immr->im_ioport.iop_pcso &= ~(CFG_PC_PUMA_DONE);
	immr->im_ioport.iop_pcdir &= ~(CFG_PC_PUMA_DONE);

	return (immr->im_ioport.iop_pcdat & CFG_PC_PUMA_DONE) ? 1 : 0;
}

/* ------------------------------------------------------------------------- */

int misc_init_r (void)
{
	ulong addr = 0;
	ulong len = 0;
	char *s;

	printf ("PUMA:  ");
	if (puma_init_done ()) {
		printf ("initialized\n");
		return 0;
	}

	if ((s = getenv ("puma_addr")) != NULL)
		addr = simple_strtoul (s, NULL, 16);

	if ((s = getenv ("puma_len")) != NULL)
		len = simple_strtoul (s, NULL, 16);

	if ((!addr) || (!len)) {
		printf ("net list undefined\n");
		return 0;
	}

	printf ("loading... ");

	puma_load (addr, len);
	return (0);
}

/* ------------------------------------------------------------------------- */
