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
#include <command.h>

/* ------------------------------------------------------------------------- */

static long int dram_size (long int, long int *, long int);
void can_driver_enable (void);
void can_driver_disable (void);

int fpga_init(void);

/* ------------------------------------------------------------------------- */

#define	_NOT_USED_	0xFFFFFFFF

const uint sdram_table[] =
{
	/*
	 * Single Read. (Offset 0 in UPMA RAM)
	 */
	0x1F0DFC04, 0xEEAFBC04, 0x11AF7C04, 0xEFBAFC00,
	0x1FF5FC47, /* last */
	/*
	 * SDRAM Initialization (offset 5 in UPMA RAM)
	 *
	 * This is no UPM entry point. The following definition uses
	 * the remaining space to establish an initialization
	 * sequence, which is executed by a RUN command.
	 *
	 */
		    0x1FF5FC34, 0xEFEABC34, 0x1FB57C35, /* last */
	/*
	 * Burst Read. (Offset 8 in UPMA RAM)
	 */
	0x1F0DFC04, 0xEEAFBC04, 0x10AF7C04, 0xF0AFFC00,
	0xF0AFFC00, 0xF1AFFC00, 0xEFBAFC00, 0x1FF5FC47, /* last */
	_NOT_USED_, _NOT_USED_, _NOT_USED_, _NOT_USED_,
	_NOT_USED_, _NOT_USED_, _NOT_USED_, _NOT_USED_,
	/*
	 * Single Write. (Offset 18 in UPMA RAM)
	 */
	0x1F0DFC04, 0xEEABBC00, 0x01B27C04, 0x1FF5FC47, /* last */
	_NOT_USED_, _NOT_USED_, _NOT_USED_, _NOT_USED_,
	/*
	 * Burst Write. (Offset 20 in UPMA RAM)
	 */
	0x1F0DFC04, 0xEEABBC00, 0x10A77C00, 0xF0AFFC00,
	0xF0AFFC00, 0xE1BAFC04, 0x1FF5FC47, /* last */
					    _NOT_USED_,
	_NOT_USED_, _NOT_USED_, _NOT_USED_, _NOT_USED_,
	_NOT_USED_, _NOT_USED_, _NOT_USED_, _NOT_USED_,
	/*
	 * Refresh  (Offset 30 in UPMA RAM)
	 */
	0x1FFD7C84, 0xFFFFFC04, 0xFFFFFC04, 0xFFFFFC04,
	0xFFFFFC84, 0xFFFFFC07, /* last */
				_NOT_USED_, _NOT_USED_,
	_NOT_USED_, _NOT_USED_, _NOT_USED_, _NOT_USED_,
	/*
	 * Exception. (Offset 3c in UPMA RAM)
	 */
	0x7FFFFC07, /* last */
		    _NOT_USED_, _NOT_USED_, _NOT_USED_,
};

/* ------------------------------------------------------------------------- */


/*
 * Check Board Identity:
 *
 * Always return 1 (no second DRAM bank since based on TQM8xxL module)
 */

int checkboard (void)
{
    unsigned char *s;
    unsigned char buf[64];

    s = (getenv_f("serial#", (char *)&buf, sizeof(buf)) > 0) ? buf : NULL;

    puts ("Board: Siemens CCM");

    if (s) {
	    puts (" (");

	    for (; *s; ++s) {
		if (*s == ' ')
		    break;
		putc (*s);
	    }
	    putc (')');
    }

    putc ('\n');

    return (0);
}

/* ------------------------------------------------------------------------- */

/*
 * If Power-On-Reset switch off the Red and Green LED: At reset, the
 * data direction registers are cleared and must therefore be restored.
 */
#define RSR_CSRS	0x08000000

int power_on_reset(void)
{
    /* Test Reset Status Register */
    return ((volatile immap_t *)CONFIG_SYS_IMMR)->im_clkrst.car_rsr & RSR_CSRS ? 0:1;
}

#define PB_LED_GREEN	0x10000		/* red LED is on PB.15 */
#define PB_LED_RED	0x20000		/* red LED is on PB.14 */
#define PB_LEDS		(PB_LED_GREEN | PB_LED_RED);

static void init_leds (void)
{
    volatile immap_t *immap  = (immap_t *)CONFIG_SYS_IMMR;

    immap->im_cpm.cp_pbpar &= ~PB_LEDS;
    immap->im_cpm.cp_pbodr &= ~PB_LEDS;
    immap->im_cpm.cp_pbdir |=  PB_LEDS;
    /* Check stop reset status */
    if (power_on_reset()) {
	    immap->im_cpm.cp_pbdat &= ~PB_LEDS;
    }
}

/* ------------------------------------------------------------------------- */

phys_size_t initdram (int board_type)
{
    volatile immap_t     *immap  = (immap_t *)CONFIG_SYS_IMMR;
    volatile memctl8xx_t *memctl = &immap->im_memctl;
    long int size8, size9;
    long int size = 0;
    unsigned long reg;

    upmconfig(UPMA, (uint *)sdram_table, sizeof(sdram_table)/sizeof(uint));

    /*
     * Preliminary prescaler for refresh (depends on number of
     * banks): This value is selected for four cycles every 62.4 us
     * with two SDRAM banks or four cycles every 31.2 us with one
     * bank. It will be adjusted after memory sizing.
     */
    memctl->memc_mptpr = CONFIG_SYS_MPTPR_2BK_8K;

    memctl->memc_mar  = 0x00000088;

    /*
     * Map controller banks 2 and 3 to the SDRAM banks 2 and 3 at
     * preliminary addresses - these have to be modified after the
     * SDRAM size has been determined.
     */
    memctl->memc_or2 = CONFIG_SYS_OR2_PRELIM;
    memctl->memc_br2 = CONFIG_SYS_BR2_PRELIM;

    memctl->memc_mamr = CONFIG_SYS_MAMR_8COL & (~(MAMR_PTAE)); /* no refresh yet */

    udelay(200);

    /* perform SDRAM initializsation sequence */

    memctl->memc_mcr  = 0x80004105;	/* SDRAM bank 0 */
    udelay(1);
    memctl->memc_mcr  = 0x80004230;	/* SDRAM bank 0 - execute twice */
    udelay(1);

    memctl->memc_mamr |= MAMR_PTAE;	/* enable refresh */

    udelay (1000);

    /*
     * Check Bank 0 Memory Size for re-configuration
     *
     * try 8 column mode
     */
    size8 = dram_size (CONFIG_SYS_MAMR_8COL, SDRAM_BASE2_PRELIM, SDRAM_MAX_SIZE);

    udelay (1000);

    /*
     * try 9 column mode
     */
    size9 = dram_size (CONFIG_SYS_MAMR_9COL, SDRAM_BASE2_PRELIM, SDRAM_MAX_SIZE);

    if (size8 < size9) {		/* leave configuration at 9 columns	*/
	size = size9;
/*	debug ("SDRAM in 9 column mode: %ld MB\n", size >> 20);	*/
    } else {				/* back to 8 columns			*/
	size = size8;
	memctl->memc_mamr = CONFIG_SYS_MAMR_8COL;
	udelay(500);
/*	debug ("SDRAM in 8 column mode: %ld MB\n", size >> 20);	*/
    }

    udelay (1000);

    /*
     * Adjust refresh rate depending on SDRAM type
     * For types > 128 MBit leave it at the current (fast) rate
     */
    if (size < 0x02000000) {
	/* reduce to 15.6 us (62.4 us / quad) */
	memctl->memc_mptpr = CONFIG_SYS_MPTPR_2BK_4K;
	udelay(1000);
    }

    /*
     * Final mapping
     */

    memctl->memc_or2 = ((-size) & 0xFFFF0000) | CONFIG_SYS_OR_TIMING_SDRAM;
    memctl->memc_br2 = (CONFIG_SYS_SDRAM_BASE & BR_BA_MSK) | BR_MS_UPMA | BR_V;


    /* adjust refresh rate depending on SDRAM type, one bank */
    reg = memctl->memc_mptpr;
    reg >>= 1;	/* reduce to CONFIG_SYS_MPTPR_1BK_8K / _4K */
    memctl->memc_mptpr = reg;

    can_driver_enable ();
    init_leds ();

    udelay(10000);

    return (size);
}

/* ------------------------------------------------------------------------- */

/*
 * Warning - both the PUMA load mode and the CAN driver use UPM B,
 * so make sure only one of both is active.
 */
void can_driver_enable (void)
{
    volatile immap_t     *immap  = (immap_t *)CONFIG_SYS_IMMR;
    volatile memctl8xx_t *memctl = &immap->im_memctl;

    /* Initialize MBMR */
    memctl->memc_mbmr = MBMR_GPL_B4DIS;	/* GPL_B4 ouput line Disable */

    /* Initialize UPMB for CAN: single read */
    memctl->memc_mdr = 0xFFFFC004;
    memctl->memc_mcr = 0x0100 | UPMB;

    memctl->memc_mdr = 0x0FFFD004;
    memctl->memc_mcr = 0x0101 | UPMB;

    memctl->memc_mdr = 0x0FFFC000;
    memctl->memc_mcr = 0x0102 | UPMB;

    memctl->memc_mdr = 0x3FFFC004;
    memctl->memc_mcr = 0x0103 | UPMB;

    memctl->memc_mdr = 0xFFFFDC05;
    memctl->memc_mcr = 0x0104 | UPMB;

    /* Initialize UPMB for CAN: single write */
    memctl->memc_mdr = 0xFFFCC004;
    memctl->memc_mcr = 0x0118 | UPMB;

    memctl->memc_mdr = 0xCFFCD004;
    memctl->memc_mcr = 0x0119 | UPMB;

    memctl->memc_mdr = 0x0FFCC000;
    memctl->memc_mcr = 0x011A | UPMB;

    memctl->memc_mdr = 0x7FFCC004;
    memctl->memc_mcr = 0x011B | UPMB;

    memctl->memc_mdr = 0xFFFDCC05;
    memctl->memc_mcr = 0x011C | UPMB;

    /* Initialize OR3 / BR3 for CAN Bus Controller */
    memctl->memc_or3 = CONFIG_SYS_OR3_CAN;
    memctl->memc_br3 = CONFIG_SYS_BR3_CAN;
}

void can_driver_disable (void)
{
    volatile immap_t     *immap  = (immap_t *)CONFIG_SYS_IMMR;
    volatile memctl8xx_t *memctl = &immap->im_memctl;

    /* Reset OR3 / BR3 to disable  CAN Bus Controller */
    memctl->memc_br3 = 0;
    memctl->memc_or3 = 0;

    memctl->memc_mbmr = 0;
}


/* ------------------------------------------------------------------------- */

/*
 * Check memory range for valid RAM. A simple memory test determines
 * the actually available RAM size between addresses `base' and
 * `base + maxsize'. Some (not all) hardware errors are detected:
 * - short between address lines
 * - short between data lines
 */

static long int dram_size (long int mamr_value, long int *base, long int maxsize)
{
    volatile immap_t     *immap  = (immap_t *)CONFIG_SYS_IMMR;
    volatile memctl8xx_t *memctl = &immap->im_memctl;

    memctl->memc_mamr = mamr_value;

    return (get_ram_size(base, maxsize));
}

/* ------------------------------------------------------------------------- */

#define	ETH_CFG_BITS	(CONFIG_SYS_PB_ETH_CFG1 | CONFIG_SYS_PB_ETH_CFG2  | CONFIG_SYS_PB_ETH_CFG3 )

#define ETH_ALL_BITS	(ETH_CFG_BITS | CONFIG_SYS_PB_ETH_POWERDOWN)

void	reset_phy(void)
{
	immap_t *immr = (immap_t *)CONFIG_SYS_IMMR;
	ulong value;

	/* Configure all needed port pins for GPIO */
#ifdef CONFIG_SYS_ETH_MDDIS_VALUE
	immr->im_ioport.iop_padat |=   CONFIG_SYS_PA_ETH_MDDIS;
#else
	immr->im_ioport.iop_padat &= ~(CONFIG_SYS_PA_ETH_MDDIS | CONFIG_SYS_PA_ETH_RESET);	/* Set low */
#endif
	immr->im_ioport.iop_papar &= ~(CONFIG_SYS_PA_ETH_MDDIS | CONFIG_SYS_PA_ETH_RESET);	/* GPIO */
	immr->im_ioport.iop_paodr &= ~(CONFIG_SYS_PA_ETH_MDDIS | CONFIG_SYS_PA_ETH_RESET);	/* active output */
	immr->im_ioport.iop_padir |=   CONFIG_SYS_PA_ETH_MDDIS | CONFIG_SYS_PA_ETH_RESET;	/* output */

	immr->im_cpm.cp_pbpar &= ~(ETH_ALL_BITS);	/* GPIO */
	immr->im_cpm.cp_pbodr &= ~(ETH_ALL_BITS);	/* active output */

	value  = immr->im_cpm.cp_pbdat;

	/* Assert Powerdown and Reset signals */
	value |=  CONFIG_SYS_PB_ETH_POWERDOWN;

	/* PHY configuration includes MDDIS and CFG1 ... CFG3 */
#ifdef CONFIG_SYS_ETH_CFG1_VALUE
	value |=   CONFIG_SYS_PB_ETH_CFG1;
#else
	value &= ~(CONFIG_SYS_PB_ETH_CFG1);
#endif
#ifdef CONFIG_SYS_ETH_CFG2_VALUE
	value |=   CONFIG_SYS_PB_ETH_CFG2;
#else
	value &= ~(CONFIG_SYS_PB_ETH_CFG2);
#endif
#ifdef CONFIG_SYS_ETH_CFG3_VALUE
	value |=   CONFIG_SYS_PB_ETH_CFG3;
#else
	value &= ~(CONFIG_SYS_PB_ETH_CFG3);
#endif

	/* Drive output signals to initial state */
	immr->im_cpm.cp_pbdat  = value;
	immr->im_cpm.cp_pbdir |= ETH_ALL_BITS;
	udelay (10000);

	/* De-assert Ethernet Powerdown */
	immr->im_cpm.cp_pbdat &= ~(CONFIG_SYS_PB_ETH_POWERDOWN); /* Enable PHY power */
	udelay (10000);

	/* de-assert RESET signal of PHY */
	immr->im_ioport.iop_padat |= CONFIG_SYS_PA_ETH_RESET;
	udelay (1000);
}


int misc_init_r (void)
{
	fpga_init();
	return (0);
}
/* ------------------------------------------------------------------------- */
