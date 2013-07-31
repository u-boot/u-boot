/*
 * (C) Copyright 2001
 * Stefan Roese, esd gmbh germany, stefan.roese@esd-electronics.com
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include "ocrtc.h"
#include <asm/processor.h>
#include <i2c.h>
#include <command.h>


extern void lxt971_no_sleep(void);


int board_early_init_f (void)
{
	/*
	 * IRQ 0-15  405GP internally generated; active high; level sensitive
	 * IRQ 16    405GP internally generated; active low; level sensitive
	 * IRQ 17-24 RESERVED
	 * IRQ 25 (EXT IRQ 0) CAN0; active low; level sensitive
	 * IRQ 26 (EXT IRQ 1) CAN1; active low; level sensitive
	 * IRQ 27 (EXT IRQ 2) PCI SLOT 0; active low; level sensitive
	 * IRQ 28 (EXT IRQ 3) PCI SLOT 1; active low; level sensitive
	 * IRQ 29 (EXT IRQ 4) PCI SLOT 2; active low; level sensitive
	 * IRQ 30 (EXT IRQ 5) PCI SLOT 3; active low; level sensitive
	 * IRQ 31 (EXT IRQ 6) COMPACT FLASH; active high; level sensitive
	 */
	mtdcr (UIC0SR, 0xFFFFFFFF);	/* clear all ints */
	mtdcr (UIC0ER, 0x00000000);	/* disable all ints */
	mtdcr (UIC0CR, 0x00000000);	/* set all to be non-critical */
	mtdcr (UIC0PR, 0xFFFFFF81);	/* set int polarities */
	mtdcr (UIC0TR, 0x10000000);	/* set int trigger levels */
	mtdcr (UIC0VCR, 0x00000001);	/* set vect base=0,INT0 highest priority */
	mtdcr (UIC0SR, 0xFFFFFFFF);	/* clear all ints */

	/*
	 * EBC Configuration Register: clear EBTC -> high-Z ebc signals between
	 * transfers, set device-paced timeout to 256 cycles
	 */
	mtebc (EBC0_CFG, 0x20400000);

	return 0;
}

/*
 * Check Board Identity:
 */
int checkboard (void)
{
	char str[64];
	int i = getenv_f("serial#", str, sizeof (str));

	puts ("Board: ");

	if (i == -1) {
#ifdef CONFIG_OCRTC
		puts ("### No HW ID - assuming OCRTC");
#endif
#ifdef CONFIG_ORSG
		puts ("### No HW ID - assuming ORSG");
#endif
	} else {
		puts (str);
	}

	putc ('\n');

	/*
	 * Disable sleep mode in LXT971
	 */
	lxt971_no_sleep();

	return (0);
}
