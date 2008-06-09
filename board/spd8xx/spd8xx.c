/*
 * (C) Copyright 2000
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 * Ulrich Lutz, Speech Design GmbH, ulutz@datalab.de.
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

/* ------------------------------------------------------------------------- */

static long int dram_size (long int, long int *, long int);

/* ------------------------------------------------------------------------- */

#define	_NOT_USED_	0xFFFFFFFF

const uint sharc_table[] = {
	/*
	 * Single Read. (Offset 0 in UPM RAM)
	 */
	0x0FF3FC04, 0x0FF3EC00, 0x7FFFEC04, 0xFFFFEC04,
	0xFFFFEC05,		/* last */
	_NOT_USED_, _NOT_USED_, _NOT_USED_,
	/*
	 * Burst Read. (Offset 8 in UPM RAM)
	 */
	/* last */
	_NOT_USED_, _NOT_USED_, _NOT_USED_, _NOT_USED_,
	_NOT_USED_, _NOT_USED_, _NOT_USED_, _NOT_USED_,
	_NOT_USED_, _NOT_USED_, _NOT_USED_, _NOT_USED_,
	_NOT_USED_, _NOT_USED_, _NOT_USED_, _NOT_USED_,
	/*
	 * Single Write. (Offset 18 in UPM RAM)
	 */
	0x0FAFFC04, 0x0FAFEC00, 0x7FFFEC04, 0xFFFFEC04,
	0xFFFFEC05,		/* last */
	_NOT_USED_, _NOT_USED_, _NOT_USED_,
	/*
	 * Burst Write. (Offset 20 in UPM RAM)
	 */
	/* last */
	_NOT_USED_, _NOT_USED_, _NOT_USED_, _NOT_USED_,
	_NOT_USED_, _NOT_USED_, _NOT_USED_, _NOT_USED_,
	_NOT_USED_, _NOT_USED_, _NOT_USED_, _NOT_USED_,
	_NOT_USED_, _NOT_USED_, _NOT_USED_, _NOT_USED_,
	/*
	 * Refresh  (Offset 30 in UPM RAM)
	 */
	/* last */
	_NOT_USED_, _NOT_USED_, _NOT_USED_, _NOT_USED_,
	_NOT_USED_, _NOT_USED_, _NOT_USED_, _NOT_USED_,
	_NOT_USED_, _NOT_USED_, _NOT_USED_, _NOT_USED_,
	/*
	 * Exception. (Offset 3c in UPM RAM)
	 */
	0x7FFFFC07,		/* last */
	_NOT_USED_, _NOT_USED_, _NOT_USED_,
};


const uint sdram_table[] = {
	/*
	 * Single Read. (Offset 0 in UPM RAM)
	 */
	0x1F07FC04, 0xEEAEFC04, 0x11ADFC04, 0xEFBBBC00,
	0x1FF77C47,		/* last */
	/*
	 * SDRAM Initialization (offset 5 in UPM RAM)
	 *
	 * This is no UPM entry point. The following definition uses
	 * the remaining space to establish an initialization
	 * sequence, which is executed by a RUN command.
	 *
	 */
	0x1FF77C35, 0xEFEABC34, 0x1FB57C35,	/* last */
	/*
	 * Burst Read. (Offset 8 in UPM RAM)
	 */
	0x1F07FC04, 0xEEAEFC04, 0x10ADFC04, 0xF0AFFC00,
	0xF0AFFC00, 0xF1AFFC00, 0xEFBBBC00, 0x1FF77C47,	/* last */
	_NOT_USED_, _NOT_USED_, _NOT_USED_, _NOT_USED_,
	_NOT_USED_, _NOT_USED_, _NOT_USED_, _NOT_USED_,
	/*
	 * Single Write. (Offset 18 in UPM RAM)
	 */
	0x1F27FC04, 0xEEAEBC00, 0x01B93C04, 0x1FF77C47,	/* last */
	_NOT_USED_, _NOT_USED_, _NOT_USED_, _NOT_USED_,
	/*
	 * Burst Write. (Offset 20 in UPM RAM)
	 */
	0x1F07FC04, 0xEEAEBC00, 0x10AD7C00, 0xF0AFFC00,
	0xF0AFFC00, 0xE1BBBC04, 0x1FF77C47,	/* last */
	_NOT_USED_,
	_NOT_USED_, _NOT_USED_, _NOT_USED_, _NOT_USED_,
	_NOT_USED_, _NOT_USED_, _NOT_USED_, _NOT_USED_,
	/*
	 * Refresh  (Offset 30 in UPM RAM)
	 */
	0x1FF5FC84, 0xFFFFFC04, 0xFFFFFC04, 0xFFFFFC04,
	0xFFFFFC84, 0xFFFFFC07,	/* last */
	_NOT_USED_, _NOT_USED_,
	_NOT_USED_, _NOT_USED_, _NOT_USED_, _NOT_USED_,
	/*
	 * Exception. (Offset 3c in UPM RAM)
	 */
	0x7FFFFC07,		/* last */
	_NOT_USED_, _NOT_USED_, _NOT_USED_,
};

/* ------------------------------------------------------------------------- */


/*
 * Check Board Identity:
 *
 */

int checkboard (void)
{
	puts ("Board: SPD823TS\n");
	return (0);
}

/* ------------------------------------------------------------------------- */

phys_size_t initdram (int board_type)
{
	volatile immap_t *immap = (immap_t *) CFG_IMMR;
	volatile memctl8xx_t *memctl = &immap->im_memctl;
	long int size_b0;

#if 0
	/*
	 * Map controller bank 2 to the SRAM bank at preliminary address.
	 */
	memctl->memc_or2 = CFG_OR2;
	memctl->memc_br2 = CFG_BR2;
#endif

	/*
	 * Map controller bank 4 to the PER8 bank.
	 */
	memctl->memc_or4 = CFG_OR4;
	memctl->memc_br4 = CFG_BR4;

#if 0
	/* Configure SHARC at UMA */
	upmconfig (UPMA, (uint *) sharc_table,
		   sizeof (sharc_table) / sizeof (uint));
	/* Map controller bank 5 to the SHARC */
	memctl->memc_or5 = CFG_OR5;
	memctl->memc_br5 = CFG_BR5;
#endif

	memctl->memc_mamr = 0x00001000;

	/* Configure SDRAM at UMB */
	upmconfig (UPMB, (uint *) sdram_table,
		   sizeof (sdram_table) / sizeof (uint));

	memctl->memc_mptpr = CFG_MPTPR_1BK_8K;

	memctl->memc_mar = 0x00000088;

	/*
	 * Map controller bank 3 to the SDRAM bank at preliminary address.
	 */
	memctl->memc_or3 = CFG_OR3_PRELIM;
	memctl->memc_br3 = CFG_BR3_PRELIM;

	memctl->memc_mbmr = CFG_MBMR_8COL;	/* refresh not enabled yet */

	udelay (200);
	memctl->memc_mcr = 0x80806105;
	udelay (1);
	memctl->memc_mcr = 0x80806130;
	udelay (1);
	memctl->memc_mcr = 0x80806130;
	udelay (1);
	memctl->memc_mcr = 0x80806106;

	memctl->memc_mbmr |= MBMR_PTBE;	/* refresh enabled */

	/*
	 * Check Bank 0 Memory Size for re-configuration
	 */
	size_b0 =
		dram_size (CFG_MBMR_8COL, SDRAM_BASE3_PRELIM,
			   SDRAM_MAX_SIZE);

	memctl->memc_mbmr = CFG_MBMR_8COL | MBMR_PTBE;

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
	volatile immap_t *immap = (immap_t *) CFG_IMMR;
	volatile memctl8xx_t *memctl = &immap->im_memctl;

	memctl->memc_mbmr = mamr_value;

	return (get_ram_size (base, maxsize));
}

/* ------------------------------------------------------------------------- */

void reset_phy (void)
{
	immap_t *immr = (immap_t *) CFG_IMMR;
	ushort sreg;

	/* Configure extra port pins for NS DP83843 PHY */
	immr->im_ioport.iop_papar &= ~(PA_ENET_MDC | PA_ENET_MDIO);

	sreg = immr->im_ioport.iop_padir;
	sreg |= PA_ENET_MDC;	/* Mgmt. Data Clock is Output */
	sreg &= ~(PA_ENET_MDIO);	/* Mgmt. Data I/O is bidirect. => Input */
	immr->im_ioport.iop_padir = sreg;

	immr->im_ioport.iop_padat &= ~(PA_ENET_MDC);	/* set MDC = 0 */

	/*
	 * RESET in implemented by a positive pulse of at least 1 us
	 * at the reset pin.
	 *
	 * Configure RESET pins for NS DP83843 PHY, and RESET chip.
	 *
	 * Note: The RESET pin is high active, but there is an
	 *       inverter on the SPD823TS board...
	 */
	immr->im_ioport.iop_pcpar &= ~(PC_ENET_RESET);
	immr->im_ioport.iop_pcdir |= PC_ENET_RESET;
	/* assert RESET signal of PHY */
	immr->im_ioport.iop_pcdat &= ~(PC_ENET_RESET);
	udelay (10);
	/* de-assert RESET signal of PHY */
	immr->im_ioport.iop_pcdat |= PC_ENET_RESET;
	udelay (10);
}

/* ------------------------------------------------------------------------- */

void ide_set_reset (int on)
{
	volatile immap_t *immr = (immap_t *) CFG_IMMR;

	/*
	 * Configure PC for IDE Reset Pin
	 */
	if (on) {		/* assert RESET */
		immr->im_ioport.iop_pcdat &= ~(CFG_PC_IDE_RESET);
	} else {		/* release RESET */
		immr->im_ioport.iop_pcdat |= CFG_PC_IDE_RESET;
	}

	/* program port pin as GPIO output */
	immr->im_ioport.iop_pcpar &= ~(CFG_PC_IDE_RESET);
	immr->im_ioport.iop_pcso &= ~(CFG_PC_IDE_RESET);
	immr->im_ioport.iop_pcdir |= CFG_PC_IDE_RESET;
}

/* ------------------------------------------------------------------------- */
