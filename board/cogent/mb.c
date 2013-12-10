/*
 * (C) Copyright 2000
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include "dipsw.h"
#include "lcd.h"
#include "rtc.h"
#include "par.h"
#include "pci.h"

/* ------------------------------------------------------------------------- */

#if defined(CONFIG_8260)

#include <ioports.h>

/*
 * I/O Port configuration table
 *
 * if conf is 1, then that port pin will be configured at boot time
 * according to the five values podr/pdir/ppar/psor/pdat for that entry
 */

const iop_conf_t iop_conf_tab[4][32] = {

	/* Port A configuration */
	{			/*            conf ppar psor pdir podr pdat */
	 /* PA31 */ {0, 0, 0, 0, 0, 0},
	 /* PA30 */ {0, 0, 0, 0, 0, 0},
	 /* PA29 */ {0, 0, 0, 0, 0, 0},
	 /* PA28 */ {0, 0, 0, 0, 0, 0},
	 /* PA27 */ {0, 0, 0, 0, 0, 0},
	 /* PA26 */ {0, 0, 0, 0, 0, 0},
	 /* PA25 */ {0, 0, 0, 0, 0, 0},
	 /* PA24 */ {0, 0, 0, 0, 0, 0},
	 /* PA23 */ {0, 0, 0, 0, 0, 0},
	 /* PA22 */ {0, 0, 0, 0, 0, 0},
	 /* PA21 */ {0, 0, 0, 0, 0, 0},
	 /* PA20 */ {0, 0, 0, 0, 0, 0},
	 /* PA19 */ {0, 0, 0, 0, 0, 0},
	 /* PA18 */ {0, 0, 0, 0, 0, 0},
	 /* PA17 */ {0, 0, 0, 0, 0, 0},
	 /* PA16 */ {0, 0, 0, 0, 0, 0},
	 /* PA15 */ {0, 0, 0, 0, 0, 0},
	 /* PA14 */ {0, 0, 0, 0, 0, 0},
	 /* PA13 */ {0, 0, 0, 0, 0, 0},
	 /* PA12 */ {0, 0, 0, 0, 0, 0},
	 /* PA11 */ {0, 0, 0, 0, 0, 0},
	 /* PA10 */ {0, 0, 0, 0, 0, 0},
					/* PA9  */ {1, 1, 0, 1, 0, 0},
					/* SMC2 TXD */
					/* PA8  */ {1, 1, 0, 0, 0, 0},
					/* SMC2 RXD */
	 /* PA7  */ {0, 0, 0, 0, 0, 0},
	 /* PA6  */ {0, 0, 0, 0, 0, 0},
	 /* PA5  */ {0, 0, 0, 0, 0, 0},
	 /* PA4  */ {0, 0, 0, 0, 0, 0},
	 /* PA3  */ {0, 0, 0, 0, 0, 0},
	 /* PA2  */ {0, 0, 0, 0, 0, 0},
	 /* PA1  */ {0, 0, 0, 0, 0, 0},
	 /* PA0  */ {0, 0, 0, 0, 0, 0}
	 },


	{			/*            conf ppar psor pdir podr pdat */
	 /* PB31 */ {0, 0, 0, 0, 0, 0},
	 /* PB30 */ {0, 0, 0, 0, 0, 0},
	 /* PB29 */ {0, 0, 0, 0, 0, 0},
	 /* PB28 */ {0, 0, 0, 0, 0, 0},
	 /* PB27 */ {0, 0, 0, 0, 0, 0},
	 /* PB26 */ {0, 0, 0, 0, 0, 0},
	 /* PB25 */ {0, 0, 0, 0, 0, 0},
	 /* PB24 */ {0, 0, 0, 0, 0, 0},
	 /* PB23 */ {0, 0, 0, 0, 0, 0},
	 /* PB22 */ {0, 0, 0, 0, 0, 0},
	 /* PB21 */ {0, 0, 0, 0, 0, 0},
	 /* PB20 */ {0, 0, 0, 0, 0, 0},
	 /* PB19 */ {0, 0, 0, 0, 0, 0},
	 /* PB18 */ {0, 0, 0, 0, 0, 0},
	 /* PB17 */ {0, 0, 0, 0, 0, 0},
	 /* PB16 */ {0, 0, 0, 0, 0, 0},
	 /* PB15 */ {0, 0, 0, 0, 0, 0},
	 /* PB14 */ {0, 0, 0, 0, 0, 0},
	 /* PB13 */ {0, 0, 0, 0, 0, 0},
	 /* PB12 */ {0, 0, 0, 0, 0, 0},
	 /* PB11 */ {0, 0, 0, 0, 0, 0},
	 /* PB10 */ {0, 0, 0, 0, 0, 0},
	 /* PB9  */ {0, 0, 0, 0, 0, 0},
	 /* PB8  */ {0, 0, 0, 0, 0, 0},
	 /* PB7  */ {0, 0, 0, 0, 0, 0},
	 /* PB6  */ {0, 0, 0, 0, 0, 0},
	 /* PB5  */ {0, 0, 0, 0, 0, 0},
	 /* PB4  */ {0, 0, 0, 0, 0, 0},
					/* PB3  */ {0, 0, 0, 0, 0, 0},
					/* pin doesn't exist */
					/* PB2  */ {0, 0, 0, 0, 0, 0},
					/* pin doesn't exist */
					/* PB1  */ {0, 0, 0, 0, 0, 0},
					/* pin doesn't exist */
					/* PB0  */ {0, 0, 0, 0, 0, 0}
					/* pin doesn't exist */
	 },


	{			/*            conf ppar psor pdir podr pdat */
	 /* PC31 */ {0, 0, 0, 0, 0, 0},
	 /* PC30 */ {0, 0, 0, 0, 0, 0},
	 /* PC29 */ {0, 0, 0, 0, 0, 0},
	 /* PC28 */ {0, 0, 0, 0, 0, 0},
	 /* PC27 */ {0, 0, 0, 0, 0, 0},
	 /* PC26 */ {0, 0, 0, 0, 0, 0},
	 /* PC25 */ {0, 0, 0, 0, 0, 0},
	 /* PC24 */ {0, 0, 0, 0, 0, 0},
	 /* PC23 */ {0, 0, 0, 0, 0, 0},
	 /* PC22 */ {0, 0, 0, 0, 0, 0},
	 /* PC21 */ {0, 0, 0, 0, 0, 0},
	 /* PC20 */ {0, 0, 0, 0, 0, 0},
	 /* PC19 */ {0, 0, 0, 0, 0, 0},
	 /* PC18 */ {0, 0, 0, 0, 0, 0},
	 /* PC17 */ {0, 0, 0, 0, 0, 0},
	 /* PC16 */ {0, 0, 0, 0, 0, 0},
	 /* PC15 */ {0, 0, 0, 0, 0, 0},
	 /* PC14 */ {0, 0, 0, 0, 0, 0},
	 /* PC13 */ {0, 0, 0, 0, 0, 0},
	 /* PC12 */ {0, 0, 0, 0, 0, 0},
	 /* PC11 */ {0, 0, 0, 0, 0, 0},
	 /* PC10 */ {0, 0, 0, 0, 0, 0},
	 /* PC9  */ {0, 0, 0, 0, 0, 0},
	 /* PC8  */ {0, 0, 0, 0, 0, 0},
	 /* PC7  */ {0, 0, 0, 0, 0, 0},
	 /* PC6  */ {0, 0, 0, 0, 0, 0},
	 /* PC5  */ {0, 0, 0, 0, 0, 0},
	 /* PC4  */ {0, 0, 0, 0, 0, 0},
	 /* PC3  */ {0, 0, 0, 0, 0, 0},
	 /* PC2  */ {0, 0, 0, 0, 0, 0},
	 /* PC1  */ {0, 0, 0, 0, 0, 0},
	 /* PC0  */ {0, 0, 0, 0, 0, 0}
	 },


	{			/*            conf ppar psor pdir podr pdat */
	 /* PD31 */ {0, 0, 0, 0, 0, 0},
	 /* PD30 */ {0, 0, 0, 0, 0, 0},
	 /* PD29 */ {0, 0, 0, 0, 0, 0},
	 /* PD28 */ {0, 0, 0, 0, 0, 0},
	 /* PD27 */ {0, 0, 0, 0, 0, 0},
	 /* PD26 */ {0, 0, 0, 0, 0, 0},
	 /* PD25 */ {0, 0, 0, 0, 0, 0},
	 /* PD24 */ {0, 0, 0, 0, 0, 0},
	 /* PD23 */ {0, 0, 0, 0, 0, 0},
	 /* PD22 */ {0, 0, 0, 0, 0, 0},
	 /* PD21 */ {0, 0, 0, 0, 0, 0},
	 /* PD20 */ {0, 0, 0, 0, 0, 0},
	 /* PD19 */ {0, 0, 0, 0, 0, 0},
	 /* PD18 */ {0, 0, 0, 0, 0, 0},
	 /* PD17 */ {0, 0, 0, 0, 0, 0},
	 /* PD16 */ {0, 0, 0, 0, 0, 0},
					/* PD15 */ {1, 1, 1, 0, 0, 0},
					/* I2C SDA */
					/* PD14 */ {1, 1, 1, 0, 0, 0},
					/* I2C SCL */
	 /* PD13 */ {0, 0, 0, 0, 0, 0},
	 /* PD12 */ {0, 0, 0, 0, 0, 0},
	 /* PD11 */ {0, 0, 0, 0, 0, 0},
	 /* PD10 */ {0, 0, 0, 0, 0, 0},
					/* PD9  */ {1, 1, 0, 1, 0, 0},
					/* SMC1 TXD */
					/* PD8  */ {1, 1, 0, 0, 0, 0},
					/* SMC1 RXD */
	 /* PD7  */ {0, 0, 0, 0, 0, 0},
	 /* PD6  */ {0, 0, 0, 0, 0, 0},
	 /* PD5  */ {0, 0, 0, 0, 0, 0},
	 /* PD4  */ {0, 0, 0, 0, 0, 0},
					/* PD3  */ {0, 0, 0, 0, 0, 0},
					/* pin doesn't exist */
					/* PD2  */ {0, 0, 0, 0, 0, 0},
					/* pin doesn't exist */
					/* PD1  */ {0, 0, 0, 0, 0, 0},
					/* pin doesn't exist */
					/* PD0  */ {0, 0, 0, 0, 0, 0}
					/* pin doesn't exist */
	 }
};

#endif /* CONFIG_8260 */

/* ------------------------------------------------------------------------- */

/*
 * Check Board Identity:
 */

int checkboard (void)
{
	puts ("Board: Cogent " COGENT_MOTHERBOARD " motherboard with a "
	      COGENT_CPU_MODULE " CPU Module\n");
	return (0);
}

/* ------------------------------------------------------------------------- */

/*
 * Miscelaneous platform dependent initialisations while still
 * running in flash
 */

int misc_init_f (void)
{
	printf ("DIPSW: ");
	dipsw_init ();
	return (0);
}

/* ------------------------------------------------------------------------- */

phys_size_t initdram (int board_type)
{
#ifdef CONFIG_CMA111
	return (32L * 1024L * 1024L);
#else
	unsigned char dipsw_val;
	int dual, size0, size1;
	long int memsize;

	dipsw_val = dipsw_cooked ();

	dual = dipsw_val & 0x01;
	size0 = (dipsw_val & 0x08) >> 3;
	size1 = (dipsw_val & 0x04) >> 2;

	if (size0)
		if (size1)
			memsize = 16L * 1024L * 1024L;
		else
			memsize = 1L * 1024L * 1024L;
	else if (size1)
		memsize = 4L * 1024L * 1024L;
	else {
		printf ("[Illegal dip switch settings - assuming 16Mbyte SIMMs] ");
		memsize = 16L * 1024L * 1024L;	/* shouldn't happen - guess 16M */
	}

	if (dual)
		memsize *= 2L;

	return (memsize);
#endif
}

/* ------------------------------------------------------------------------- */

/*
 * Miscelaneous platform dependent initialisations after monitor
 * has been relocated into ram
 */

int misc_init_r (void)
{
	printf ("LCD:   ");
	lcd_init ();

#if 0
	printf ("RTC:   ");
	rtc_init ();

	printf ("PAR:   ");
	par_init ();

	printf ("KBM:   ");
	kbm_init ();

	printf ("PCI:   ");
	pci_init ();
#endif
	return (0);
}
