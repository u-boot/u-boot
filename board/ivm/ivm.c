/*
 * (C) Copyright 2000, 2001
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

#ifdef CONFIG_STATUS_LED
# include <status_led.h>
#endif

/* ------------------------------------------------------------------------- */

static long int dram_size (long int, long int *, long int);

/* ------------------------------------------------------------------------- */

#define	_NOT_USED_	0xFFFFFFFF

/*
 * 50 MHz SHARC access using UPM A
 */
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


/*
 * 50 MHz SDRAM access using UPM B
 */
const uint sdram_table[] = {
	/*
	 * Single Read. (Offset 0 in UPM RAM)
	 */
	0x0E26FC04, 0x11ADFC04, 0xEFBBBC00, 0x1FF77C45,	/* last */
	_NOT_USED_,
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
	0x0E26FC04, 0x10ADFC04, 0xF0AFFC00, 0xF0AFFC00,
	0xF1AFFC00, 0xEFBBBC00, 0x1FF77C45,	/* last */
	_NOT_USED_,
	_NOT_USED_, _NOT_USED_, _NOT_USED_, _NOT_USED_,
	_NOT_USED_, _NOT_USED_, _NOT_USED_, _NOT_USED_,
	/*
	 * Single Write. (Offset 18 in UPM RAM)
	 */
	0x1F27FC04, 0xEEAEBC04, 0x01B93C00, 0x1FF77C45,	/* last */
	_NOT_USED_, _NOT_USED_, _NOT_USED_, _NOT_USED_,
	/*
	 * Burst Write. (Offset 20 in UPM RAM)
	 */
	0x0E26BC00, 0x10AD7C00, 0xF0AFFC00, 0xF0AFFC00,
	0xE1BBBC04, 0x1FF77C45,	/* last */
	_NOT_USED_, _NOT_USED_,
	_NOT_USED_, _NOT_USED_, _NOT_USED_, _NOT_USED_,
	_NOT_USED_, _NOT_USED_, _NOT_USED_, _NOT_USED_,
	/*
	 * Refresh  (Offset 30 in UPM RAM)
	 */
	0x1FF5FC84, 0xFFFFFC04, 0xFFFFFC04, 0xFFFFFC84,
	0xFFFFFC05,		/* last */
	_NOT_USED_, _NOT_USED_, _NOT_USED_,
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
#ifdef CONFIG_IVMS8
	puts ("Board: IVMS8\n");
#endif
#ifdef CONFIG_IVML24
	puts ("Board: IVM-L8/24\n");
#endif
	return (0);
}

/* ------------------------------------------------------------------------- */

phys_size_t initdram (int board_type)
{
	volatile immap_t *immr = (immap_t *) CFG_IMMR;
	volatile memctl8xx_t *memctl = &immr->im_memctl;
	long int size_b0;

	/* enable SDRAM clock ("switch on" SDRAM) */
	immr->im_cpm.cp_pbpar &= ~(CFG_PB_SDRAM_CLKE);	/* GPIO */
	immr->im_cpm.cp_pbodr &= ~(CFG_PB_SDRAM_CLKE);	/* active output */
	immr->im_cpm.cp_pbdir |= CFG_PB_SDRAM_CLKE;	/* output */
	immr->im_cpm.cp_pbdat |= CFG_PB_SDRAM_CLKE;	/* assert SDRAM CLKE */
	udelay (1);

	/*
	 * Map controller bank 1 for ELIC SACCO
	 */
	memctl->memc_or1 = CFG_OR1;
	memctl->memc_br1 = CFG_BR1;

	/*
	 * Map controller bank 2 for ELIC EPIC
	 */
	memctl->memc_or2 = CFG_OR2;
	memctl->memc_br2 = CFG_BR2;

	/*
	 * Configure UPMA for SHARC
	 */
	upmconfig (UPMA, (uint *) sharc_table,
		   sizeof (sharc_table) / sizeof (uint));

#if defined(CONFIG_IVML24)
	/*
	 * Map controller bank 4 for HDLC Address space
	 */
	memctl->memc_or4 = CFG_OR4;
	memctl->memc_br4 = CFG_BR4;
#endif

	/*
	 * Map controller bank 5 for SHARC
	 */
	memctl->memc_or5 = CFG_OR5;
	memctl->memc_br5 = CFG_BR5;

	memctl->memc_mamr = 0x00001000;

	/*
	 * Configure UPMB for SDRAM
	 */
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
	memctl->memc_mcr = 0x80806105;	/* precharge */
	udelay (1);
	memctl->memc_mcr = 0x80806106;	/* load mode register */
	udelay (1);
	memctl->memc_mcr = 0x80806130;	/* autorefresh */
	udelay (1);
	memctl->memc_mcr = 0x80806130;	/* autorefresh */
	udelay (1);
	memctl->memc_mcr = 0x80806130;	/* autorefresh */
	udelay (1);
	memctl->memc_mcr = 0x80806130;	/* autorefresh */
	udelay (1);
	memctl->memc_mcr = 0x80806130;	/* autorefresh */
	udelay (1);
	memctl->memc_mcr = 0x80806130;	/* autorefresh */
	udelay (1);
	memctl->memc_mcr = 0x80806130;	/* autorefresh */
	udelay (1);
	memctl->memc_mcr = 0x80806130;	/* autorefresh */

	memctl->memc_mbmr |= MBMR_PTBE;	/* refresh enabled */

	/*
	 * Check Bank 0 Memory Size for re-configuration
	 */
	size_b0 =
		dram_size (CFG_MBMR_8COL, (long *) SDRAM_BASE3_PRELIM,
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
	volatile immap_t *immr = (immap_t *) CFG_IMMR;
	volatile memctl8xx_t *memctl = &immr->im_memctl;

	memctl->memc_mbmr = mamr_value;

	return (get_ram_size (base, maxsize));
}

/* ------------------------------------------------------------------------- */

void reset_phy (void)
{
	immap_t *immr = (immap_t *) CFG_IMMR;

	/* De-assert Ethernet Powerdown */
	immr->im_cpm.cp_pbpar &= ~(CFG_PB_ETH_POWERDOWN);	/* GPIO */
	immr->im_cpm.cp_pbodr &= ~(CFG_PB_ETH_POWERDOWN);	/* active output */
	immr->im_cpm.cp_pbdir |= CFG_PB_ETH_POWERDOWN;	/* output */
	immr->im_cpm.cp_pbdat &= ~(CFG_PB_ETH_POWERDOWN);	/* Enable PHY power */
	udelay (1000);

	/*
	 * RESET is implemented by a positive pulse of at least 1 us
	 * at the reset pin.
	 *
	 * Configure RESET pins for NS DP83843 PHY, and RESET chip.
	 *
	 * Note: The RESET pin is high active, but there is an
	 *       inverter on the SPD823TS board...
	 */
	immr->im_ioport.iop_pcpar &= ~(CFG_PC_ETH_RESET);
	immr->im_ioport.iop_pcdir |= CFG_PC_ETH_RESET;
	/* assert RESET signal of PHY */
	immr->im_ioport.iop_pcdat &= ~(CFG_PC_ETH_RESET);
	udelay (10);
	/* de-assert RESET signal of PHY */
	immr->im_ioport.iop_pcdat |= CFG_PC_ETH_RESET;
	udelay (10);
}

/* ------------------------------------------------------------------------- */

void show_boot_progress (int status)
{
#if defined(CONFIG_STATUS_LED)
# if defined(STATUS_LED_YELLOW)
	status_led_set (STATUS_LED_YELLOW,
			(status < 0) ? STATUS_LED_ON : STATUS_LED_OFF);
# endif	/* STATUS_LED_YELLOW */
# if defined(STATUS_LED_BOOT)
	if (status == 6)
		status_led_set (STATUS_LED_BOOT, STATUS_LED_OFF);
# endif	/* STATUS_LED_BOOT */
#endif /* CONFIG_STATUS_LED */
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
