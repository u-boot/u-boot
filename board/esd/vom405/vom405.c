/*
 * (C) Copyright 2001-2004
 * Stefan Roese, esd gmbh germany, stefan.roese@esd-electronics.com
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
#include <asm/processor.h>
#include <asm/io.h>
#include <command.h>
#include <malloc.h>
#include <sja1000.h>

DECLARE_GLOBAL_DATA_PTR;

extern void lxt971_no_sleep(void);

/*
 * generate a short spike on the CAN tx line
 * to bring the couplers in sync
 */
void init_coupler(u32 addr)
{
	struct sja1000_basic_s *ctrl = (struct sja1000_basic_s *)addr;

	/* reset */
	out_8(&ctrl->cr, CR_RR);

	/* dominant */
	out_8(&ctrl->btr0, 0x00); /* btr setup is required */
	out_8(&ctrl->btr1, 0x14); /* we use 1Mbit/s */
	out_8(&ctrl->oc, OC_TP1 | OC_TN1 | OC_POL1 |
	      OC_TP0 | OC_TN0 | OC_POL0 | OC_MODE1);
	out_8(&ctrl->cr, 0x00);

	/* delay */
	in_8(&ctrl->cr);
	in_8(&ctrl->cr);
	in_8(&ctrl->cr);
	in_8(&ctrl->cr);

	/* reset */
	out_8(&ctrl->cr, CR_RR);
}

int board_early_init_f (void)
{
	/*
	 * IRQ 0-15  405GP internally generated; active high; level sensitive
	 * IRQ 16    405GP internally generated; active low; level sensitive
	 * IRQ 17-24 RESERVED
	 * IRQ 25 (EXT IRQ 0) CAN0; active low; level sensitive
	 * IRQ 26 (EXT IRQ 1) SER0 ; active low; level sensitive
	 * IRQ 27 (EXT IRQ 2) SER1; active low; level sensitive
	 * IRQ 28 (EXT IRQ 3) FPGA 0; active low; level sensitive
	 * IRQ 29 (EXT IRQ 4) FPGA 1; active low; level sensitive
	 * IRQ 30 (EXT IRQ 5) PCI INTA; active low; level sensitive
	 * IRQ 31 (EXT IRQ 6) COMPACT FLASH; active high; level sensitive
	 */
	mtdcr(UIC0SR, 0xFFFFFFFF);       /* clear all ints */
	mtdcr(UIC0ER, 0x00000000);       /* disable all ints */
	mtdcr(UIC0CR, 0x00000000);       /* set all to be non-critical*/
	mtdcr(UIC0PR, 0xFFFFFF80);       /* set int polarities */
	mtdcr(UIC0TR, 0x10000000);       /* set int trigger levels */
	mtdcr(UIC0VCR, 0x00000001);      /* set vect base=0,INT0 highest priority*/
	mtdcr(UIC0SR, 0xFFFFFFFF);       /* clear all ints */

	/*
	 * EBC Configuration Register: set ready timeout to 512 ebc-clks -> ca. 15 us
	 */
	mtebc (EBC0_CFG, 0xa8400000); /* ebc always driven */

	/*
	 * Reset CPLD via GPIO12 (CS3) pin
	 */
	out_be32((void *)GPIO0_OR,
		 in_be32((void *)GPIO0_OR) & ~(0x80000000 >> 12));
	udelay(1000); /* wait 1ms */
	out_be32((void *)GPIO0_OR,
		 in_be32((void *)GPIO0_OR) | (0x80000000 >> 12));
	udelay(1000); /* wait 1ms */

	return 0;
}

int misc_init_r (void)
{
	/* adjust flash start and offset */
	gd->bd->bi_flashstart = 0 - gd->bd->bi_flashsize;
	gd->bd->bi_flashoffset = 0;

	/*
	 * Init magnetic coupler
	 */
	if (!getenv("noinitcoupler"))
		init_coupler(CAN_BA);

	return (0);
}

/*
 * Check Board Identity:
 */
int checkboard (void)
{
	char str[64];
	int i = getenv_r ("serial#", str, sizeof(str));
	int flashcnt;
	int delay;
	u8 *led_reg = (u8 *)(CAN_BA + 0x1000);

	puts ("Board: ");

	if (i == -1) {
		puts ("### No HW ID - assuming VOM405");
	} else {
		puts(str);
	}

	printf(" (PLD-Version=%02d)\n", in_8(led_reg));

	/*
	 * Flash LEDs
	 */
	for (flashcnt = 0; flashcnt < 3; flashcnt++) {
		out_8(led_reg, 0x40);        /* LED_B..D off */
		for (delay = 0; delay < 100; delay++)
			udelay(1000);
		out_8(led_reg, 0x47);        /* LED_B..D on */
		for (delay = 0; delay < 50; delay++)
			udelay(1000);
	}
	out_8(led_reg, 0x40);

	return 0;
}

void reset_phy(void)
{
#ifdef CONFIG_LXT971_NO_SLEEP

	/*
	 * Disable sleep mode in LXT971
	 */
	lxt971_no_sleep();
#endif
}
