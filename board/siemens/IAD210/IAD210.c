/*
 * (C) Copyright 2001
 * Paul Geerinckx
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
#include "atm.h"
#include <i2c.h>

/* ------------------------------------------------------------------------- */

static long int dram_size (long int, long int *, long int);

/* ------------------------------------------------------------------------- */

/* used PLD registers */
#  define PLD_GCR1_REG (unsigned char *) (0x10000000 + 0)
#  define PLD_EXT_RES  (unsigned char *) (0x10000000 + 10)
#  define PLD_EXT_FETH (unsigned char *) (0x10000000 + 11)
#  define PLD_EXT_LED  (unsigned char *) (0x10000000 + 12)
#  define PLD_EXT_X21  (unsigned char *) (0x10000000 + 13)

#define	_NOT_USED_	0xFFFFFFFF

const uint sdram_table[] = {
	/*
	 * Single Read. (Offset 0 in UPMA RAM)
	 */
	0xFE2DB004, 0xF0AA7004, 0xF0A5F400, 0xF3AFFC47,	/* last */
	_NOT_USED_,
	/*
	 * SDRAM Initialization (offset 5 in UPMA RAM)
	 *
	 * This is no UPM entry point. The following definition uses
	 * the remaining space to establish an initialization
	 * sequence, which is executed by a RUN command.
	 *
	 */
	0xFFFAF834, 0xFFE5B435,	/* last */
	_NOT_USED_,
	/*
	 * Burst Read. (Offset 8 in UPMA RAM)
	 */
	0xFE2DB004, 0xF0AF7404, 0xF0AFFC00, 0xF0AFFC00,
	0xF0AFFC00, 0xF0AAF800, 0xF1A5E447,	/* last */
	_NOT_USED_,
	_NOT_USED_, _NOT_USED_, _NOT_USED_, _NOT_USED_,
	_NOT_USED_, _NOT_USED_, _NOT_USED_, _NOT_USED_,
	/*
	 * Single Write. (Offset 18 in UPMA RAM)
	 */
	0xFE29B300, 0xF1A27304, 0xFFA5F747,	/* last */
	_NOT_USED_,
	_NOT_USED_, _NOT_USED_, _NOT_USED_, _NOT_USED_,
	/*
	 * Burst Write. (Offset 20 in UPMA RAM)
	 */
	0x1F0DFC04, 0xEEABBC00, 0x10A77C00, 0xF0AFFC00,
	0xF1AAF804, 0xFFA5F447,	/* last */
	_NOT_USED_, _NOT_USED_,
	_NOT_USED_, _NOT_USED_, _NOT_USED_, _NOT_USED_,
	_NOT_USED_, _NOT_USED_, _NOT_USED_, _NOT_USED_,
	/*
	 * Refresh  (Offset 30 in UPMA RAM)
	 */
	0xFFAC3884, 0xFFAC3404, 0xFFAFFC04, 0xFFAFFC84,
	0xFFAFFC07,		/* last */
	_NOT_USED_, _NOT_USED_, _NOT_USED_,
	/*
	 * MRS sequence  (Offset 38 in UPMA RAM)
	 */
	0xFFAAB834, 0xFFA57434, 0xFFAFFC05,	/* last */
	_NOT_USED_,
	/*
	 * Exception. (Offset 3c in UPMA RAM)
	 */
	0xFFAFFC04, 0xFFAFFC05,	/* last */
	_NOT_USED_, _NOT_USED_,
};

/* ------------------------------------------------------------------------- */


long int initdram (int board_type)
{
	volatile immap_t *immap = (immap_t *) CFG_IMMR;
	volatile memctl8xx_t *memctl = &immap->im_memctl;
	volatile iop8xx_t *iop = &immap->im_ioport;
	volatile fec_t *fecp = &immap->im_cpm.cp_fec;
	long int size;

	upmconfig (UPMA, (uint *) sdram_table,
		   sizeof (sdram_table) / sizeof (uint));

	/*
	 * Preliminary prescaler for refresh (depends on number of
	 * banks): This value is selected for four cycles every 62.4 us
	 * with two SDRAM banks or four cycles every 31.2 us with one
	 * bank. It will be adjusted after memory sizing.
	 */
	memctl->memc_mptpr = CFG_MPTPR;

	memctl->memc_mar = 0x00000088;

	/*
	 * Map controller banks 2 and 3 to the SDRAM banks 2 and 3 at
	 * preliminary addresses - these have to be modified after the
	 * SDRAM size has been determined.
	 */
	memctl->memc_or2 = CFG_OR2_PRELIM;
	memctl->memc_br2 = CFG_BR2_PRELIM;

	memctl->memc_mamr = CFG_MAMR & (~(MAMR_PTAE));	/* no refresh yet */

	udelay (200);

	/* perform SDRAM initializsation sequence */

	memctl->memc_mcr = 0x80004105;	/* SDRAM bank 0 */
	udelay (1);
	memctl->memc_mcr = 0x80004230;	/* SDRAM bank 0 - execute twice */
	udelay (1);

	memctl->memc_mcr = 0x80004105;	/* SDRAM precharge */
	udelay (1);
	memctl->memc_mcr = 0x80004030;	/* SDRAM 16x autorefresh */
	udelay (1);
	memctl->memc_mcr = 0x80004138;	/* SDRAM upload parameters */
	udelay (1);

	memctl->memc_mamr |= MAMR_PTAE;	/* enable refresh */

	udelay (1000);

	/*
	 * Check Bank 0 Memory Size for re-configuration
	 *
	 */
	size = dram_size (CFG_MAMR, (long *) SDRAM_BASE_PRELIM,
			  SDRAM_MAX_SIZE);

	udelay (1000);


	memctl->memc_mamr = CFG_MAMR;
	udelay (1000);

	/*
	 * Final mapping
	 */
	memctl->memc_or2 = ((-size) & 0xFFFF0000) | CFG_OR2_PRELIM;
	memctl->memc_br2 = ((CFG_SDRAM_BASE & BR_BA_MSK) | BR_MS_UPMA | BR_V);

	udelay (10000);

	/* prepare pin multiplexing for fast ethernet */

	atmLoad ();
	fecp->fec_ecntrl = 0x00000004;	/* rev D3 pinmux SET */
	iop->iop_pdpar |= 0x0080;	/* set pin as MII_clock */


	return (size);
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

	memctl->memc_mamr = mamr_value;

	return (get_ram_size (base, maxsize));
}

/*
 * Check Board Identity:
 */

int checkboard (void)
{
	return (0);
}

void board_serial_init (void)
{
	;			/* nothing to do here */
}

void board_ether_init (void)
{
	volatile immap_t *immap = (immap_t *) CFG_IMMR;
	volatile iop8xx_t *iop = &immap->im_ioport;
	volatile fec_t *fecp = &immap->im_cpm.cp_fec;

	atmLoad ();
	fecp->fec_ecntrl = 0x00000004;	/* rev D3 pinmux SET */
	iop->iop_pdpar |= 0x0080;	/* set pin as MII_clock */
}

int board_early_init_f (void)
{
	volatile immap_t *immap = (immap_t *) CFG_IMMR;
	volatile cpmtimer8xx_t *timers = &immap->im_cpmtimer;
	volatile memctl8xx_t *memctl = &immap->im_memctl;
	volatile iop8xx_t *iop = &immap->im_ioport;

	/* configure the LED timing output pins - port A pin 4 */
	iop->iop_papar = 0x0800;
	iop->iop_padir = 0x0800;

	/* start timer 2 for the 4hz LED blink rate */
	timers->cpmt_tmr2 = 0xff2c;	/* 4hz for 64mhz */
	timers->cpmt_trr2 = 0x000003d0;	/* clk/16 , prescale=256 */
	timers->cpmt_tgcr = 0x00000810;	/* run timer 2 */

	/* chip select for PLD access */
	memctl->memc_br6 = 0x10000401;
	memctl->memc_or6 = 0xFC000908;

	/* PLD initial values ( set LEDs, remove reset on LXT) */

	*PLD_GCR1_REG = 0x06;
	*PLD_EXT_RES = 0xC0;
	*PLD_EXT_FETH = 0x40;
	*PLD_EXT_LED = 0xFF;
	*PLD_EXT_X21 = 0x04;
	return 0;
}

void board_get_enetaddr (uchar * addr)
{
	int i;
	volatile immap_t *immap = (immap_t *) CFG_IMMR;
	volatile cpm8xx_t *cpm = &immap->im_cpm;
	unsigned int rccrtmp;

	char default_mac_addr[] = { 0x00, 0x08, 0x01, 0x02, 0x03, 0x04 };

	for (i = 0; i < 6; i++)
		addr[i] = default_mac_addr[i];

	printf ("There is an error in the i2c driver .. /n");
	printf ("You need to fix it first....../n");

	rccrtmp = cpm->cp_rccr;
	cpm->cp_rccr |= 0x0020;

	i2c_reg_read (0xa0, 0);
	printf ("seep = '-%c-%c-%c-%c-%c-%c-'\n",
		i2c_reg_read (0xa0, 0), i2c_reg_read (0xa0, 0),
		i2c_reg_read (0xa0, 0), i2c_reg_read (0xa0, 0),
		i2c_reg_read (0xa0, 0), i2c_reg_read (0xa0, 0));

	cpm->cp_rccr = rccrtmp;
}
