/*
 * (C) Copyright 2001
 * Erik Theisen, Wave 7 Optics, etheisen@mindspring.com.
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
#include <command.h>
#include "w7o.h"
#include <asm/processor.h>

#include "vpd.h"
#include "errors.h"
#include <watchdog.h>

unsigned long get_dram_size (void);
void sdram_init(void);

/*
 * Macros to transform values
 * into environment strings.
 */
#define XMK_STR(x)	#x
#define MK_STR(x)	XMK_STR(x)

/* ------------------------------------------------------------------------- */

int board_early_init_f (void)
{
#if defined(CONFIG_W7OLMG)
	/*
	 * Setup GPIO pins - reset devices.
	 */
	out32 (PPC405GP_GPIO0_ODR, 0x10000000);	/* one open drain pin */
	out32 (PPC405GP_GPIO0_OR, 0x3E000000);	/* set output pins to default */
	out32 (PPC405GP_GPIO0_TCR, 0x7f800000);	/* setup for output */

	/*
	 * IRQ 0-15  405GP internally generated; active high; level sensitive
	 * IRQ 16    405GP internally generated; active low; level sensitive
	 * IRQ 17-24 RESERVED
	 * IRQ 25 (EXT IRQ 0) XILINX; active low; level sensitive
	 * IRQ 26 (EXT IRQ 1) PCI INT A; active low; level sensitive
	 * IRQ 27 (EXT IRQ 2) PCI INT B; active low; level sensitive
	 * IRQ 28 (EXT IRQ 3) SAM 2; active low; level sensitive
	 * IRQ 29 (EXT IRQ 4) Battery Bad; active low; level sensitive
	 * IRQ 30 (EXT IRQ 5) Level One PHY; active low; level sensitive
	 * IRQ 31 (EXT IRQ 6) SAM 1; active high; level sensitive
	 */
	mtdcr (UIC0SR, 0xFFFFFFFF);	/* clear all ints */
	mtdcr (UIC0ER, 0x00000000);	/* disable all ints */

	mtdcr (UIC0CR, 0x00000000);	/* set all to be non-critical */
	mtdcr (UIC0PR, 0xFFFFFF80);	/* set int polarities */
	mtdcr (UIC0TR, 0x10000000);	/* set int trigger levels */
	mtdcr (UIC0VCR, 0x00000001);	/* set vect base=0,
					   INT0 highest priority */

	mtdcr (UIC0SR, 0xFFFFFFFF);	/* clear all ints */

#elif defined(CONFIG_W7OLMC)
	/*
	 * Setup GPIO pins
	 */
	out32 (PPC405GP_GPIO0_ODR, 0x01800000);	/* XCV Done Open Drain */
	out32 (PPC405GP_GPIO0_OR, 0x03800000);	/* set out pins to default */
	out32 (PPC405GP_GPIO0_TCR, 0x66C00000);	/* setup for output */

	/*
	 * IRQ 0-15  405GP internally generated; active high; level sensitive
	 * IRQ 16    405GP internally generated; active low; level sensitive
	 * IRQ 17-24 RESERVED
	 * IRQ 25 (EXT IRQ 0) DBE 0; active low; level sensitive
	 * IRQ 26 (EXT IRQ 1) DBE 1; active low; level sensitive
	 * IRQ 27 (EXT IRQ 2) DBE 2; active low; level sensitive
	 * IRQ 28 (EXT IRQ 3) DBE Common; active low; level sensitive
	 * IRQ 29 (EXT IRQ 4) PCI; active low; level sensitive
	 * IRQ 30 (EXT IRQ 5) RCMM Reset; active low; level sensitive
	 * IRQ 31 (EXT IRQ 6) PHY; active high; level sensitive
	 */
	mtdcr (UIC0SR, 0xFFFFFFFF);	/* clear all ints */
	mtdcr (UIC0ER, 0x00000000);	/* disable all ints */

	mtdcr (UIC0CR, 0x00000000);	/* set all to be non-critical */
	mtdcr (UIC0PR, 0xFFFFFF80);	/* set int polarities */
	mtdcr (UIC0TR, 0x10000000);	/* set int trigger levels */
	mtdcr (UIC0VCR, 0x00000001);	/* set vect base=0,
					   INT0 highest priority */

	mtdcr (UIC0SR, 0xFFFFFFFF);	/* clear all ints */

#else  /* Unknown */
#    error "Unknown W7O board configuration"
#endif

	WATCHDOG_RESET ();	/* Reset the watchdog */
	temp_uart_init ();	/* init the uart for debug */
	WATCHDOG_RESET ();	/* Reset the watchdog */
	test_led ();		/* test the LEDs */
	test_sdram (get_dram_size ());	/* test the dram */
	log_stat (ERR_POST1);	/* log status,post1 complete */
	return 0;
}


/* ------------------------------------------------------------------------- */

/*
 * Check Board Identity:
 */
int checkboard (void)
{
	VPD vpd;

	puts ("Board: ");

	/* VPD data present in I2C EEPROM */
	if (vpd_get_data (CONFIG_SYS_DEF_EEPROM_ADDR, &vpd) == 0) {
		/*
		 * Known board type.
		 */
		if (vpd.productId[0] &&
		    ((strncmp (vpd.productId, "GMM", 3) == 0) ||
		     (strncmp (vpd.productId, "CMM", 3) == 0))) {

			/* Output board information on startup */
			printf ("\"%s\", revision '%c', serial# %ld, manufacturer %u\n", vpd.productId, vpd.revisionId, vpd.serialNum, vpd.manuID);
			return (0);
		}
	}

	puts ("### Unknown HW ID - assuming NOTHING\n");
	return (0);
}

/* ------------------------------------------------------------------------- */

phys_size_t initdram (int board_type)
{
	/*
	 * ToDo: Move the asm init routine sdram_init() to this C file,
	 * or even better use some common ppc4xx code available
	 * in arch/powerpc/cpu/ppc4xx
	 */
	sdram_init();

	return get_dram_size ();
}

unsigned long get_dram_size (void)
{
	int tmp, i, regs[4];
	int size = 0;

	/* Get bank Size registers */
	mtdcr (SDRAM0_CFGADDR, SDRAM0_B0CR);	/* get bank 0 config reg */
	regs[0] = mfdcr (SDRAM0_CFGDATA);

	mtdcr (SDRAM0_CFGADDR, SDRAM0_B1CR);	/* get bank 1 config reg */
	regs[1] = mfdcr (SDRAM0_CFGDATA);

	mtdcr (SDRAM0_CFGADDR, SDRAM0_B2CR);	/* get bank 2 config reg */
	regs[2] = mfdcr (SDRAM0_CFGDATA);

	mtdcr (SDRAM0_CFGADDR, SDRAM0_B3CR);	/* get bank 3 config reg */
	regs[3] = mfdcr (SDRAM0_CFGDATA);

	/* compute the size, add each bank if enabled */
	for (i = 0; i < 4; i++) {
		if (regs[i] & 0x0001) {	/* if enabled, */
			tmp = ((regs[i] >> (31 - 14)) & 0x7);	/* get size bits */
			tmp = 0x400000 << tmp;	/* Size bits X 4MB = size */
			size += tmp;
		}
	}

	return size;
}

int misc_init_f (void)
{
	return 0;
}

static void w7o_env_init (VPD * vpd)
{
	/*
	 * Read VPD
	 */
	if (vpd_get_data (CONFIG_SYS_DEF_EEPROM_ADDR, vpd) != 0)
		return;

	/*
	 * Known board type.
	 */
	if (vpd->productId[0] &&
	    ((strncmp (vpd->productId, "GMM", 3) == 0) ||
	     (strncmp (vpd->productId, "CMM", 3) == 0))) {
		char buf[30];
		char *eth;
		char *serial = getenv ("serial#");
		char *ethaddr = getenv ("ethaddr");

		/* Set 'serial#' envvar if serial# isn't set */
		if (!serial) {
			sprintf (buf, "%s-%ld", vpd->productId,
				 vpd->serialNum);
			setenv ("serial#", buf);
		}

		/* Set 'ethaddr' envvar if 'ethaddr' envvar is the default */
		eth = (char *)(vpd->ethAddrs[0]);
		if (ethaddr
		    && (strcmp (ethaddr, MK_STR (CONFIG_ETHADDR)) == 0)) {
			/* Now setup ethaddr */
			sprintf (buf, "%02x:%02x:%02x:%02x:%02x:%02x",
				 eth[0], eth[1], eth[2], eth[3], eth[4],
				 eth[5]);
			setenv ("ethaddr", buf);
		}
	}
}				/* w7o_env_init() */


int misc_init_r (void)
{
	VPD vpd;		/* VPD information */

#if defined(CONFIG_W7OLMG)
	unsigned long greg;	/* GPIO Register */

	greg = in32 (PPC405GP_GPIO0_OR);

	/*
	 * XXX - Unreset devices - this should be moved into VxWorks driver code
	 */
	greg |= 0x41800000L;	/* SAM, PHY, Galileo */

	out32 (PPC405GP_GPIO0_OR, greg);	/* set output pins to default */
#endif /* CONFIG_W7OLMG */

	/*
	 * Initialize W7O environment variables
	 */
	w7o_env_init (&vpd);

	/*
	 * Initialize the FPGA(s).
	 */
	if (init_fpga () == 0)
		test_fpga ((unsigned short *) CONFIG_FPGAS_BASE);

	/* More POST testing. */
	post2 ();

	/* Done with hardware initialization and POST. */
	log_stat (ERR_POSTOK);

	/* Call silly, fail safe boot init routine */
	init_fsboot ();

	return (0);
}
