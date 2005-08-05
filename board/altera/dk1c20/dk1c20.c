/*
 * (C) Copyright 2003, Psyent Corporation <www.psyent.com>
 * Scott McNutt <smcnutt@psyent.com>
 *
 * CompactFlash/IDE:
 * (C) Copyright 2004, Shlomo Kut <skut@vyyo.com>
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
#include <nios-io.h>
#if	defined(CONFIG_SEVENSEG)
#include "../common/sevenseg.h"
#endif

void _default_hdlr (void)
{
	printf ("default_hdlr\n");
}

int board_early_init_f (void)
{
#if	defined(CONFIG_SEVENSEG)
	/* init seven segment led display and switch off */
	sevenseg_set(SEVENSEG_OFF);
#endif
	return 0;
}

int checkboard (void)
{
	puts ("Board: Altera Nios 1C20 Development Kit\n");
	return 0;
}

long int initdram (int board_type)
{
	return (0);
}

#if (CONFIG_COMMANDS & CFG_CMD_IDE)
int ide_preinit (void)
{
	nios_pio_t *present = (nios_pio_t *) CFG_CF_PRESENT;
	nios_pio_t *power = (nios_pio_t *) CFG_CF_POWER;
	nios_pio_t *atasel = (nios_pio_t *) CFG_CF_ATASEL;

	/* setup data direction registers */
	present->direction = NIOS_PIO_IN;
	power->direction = NIOS_PIO_OUT;
	atasel->direction = NIOS_PIO_OUT;

	/* Check for presence of card */
	if (present->data)
		return 1;
	printf ("Ok\n");

	/* Finish setup */
	power->data = 1;	/* Turn on power FET */
	atasel->data = 0;	/* Put in ATA mode */

	return 0;
}
#endif /* CONFIG_COMMANDS & CFG_CMD_IDE */
