/*
 * (C) Copyright 2004
 * Klaus Heydeck, Kieback & Peter GmbH & Co KG, heydeck@kieback-peter.de.
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
#include "kup.h"

int misc_init_f (void)
{
	volatile immap_t *immap = (immap_t *) CFG_IMMR;
	volatile sysconf8xx_t *siu = &immap->im_siu_conf;

	while (siu->sc_sipend & 0x20000000) {
		/* printf("waiting for 5V VCC\n"); */
		;
	}

	/* RS232 / RS485 default is RS232 */
	immap->im_ioport.iop_padat &= ~(PA_RS485);
	immap->im_ioport.iop_papar &= ~(PA_RS485);
	immap->im_ioport.iop_paodr &= ~(PA_RS485);
	immap->im_ioport.iop_padir |= (PA_RS485);
	return (0);
}


#ifdef CONFIG_IDE_LED
void ide_led (uchar led, uchar status)
{
	volatile immap_t *immap = (immap_t *) CFG_IMMR;

	/* We have one led for both pcmcia slots */
	if (status) {		/* led on */
		immap->im_ioport.iop_padat &= ~(PA_LED_YELLOW);
	} else {
		immap->im_ioport.iop_padat |= (PA_LED_YELLOW);
	}
}
#endif

void poweron_key (void)
{
	volatile immap_t *immap = (immap_t *) CFG_IMMR;

	immap->im_ioport.iop_pcpar &= ~(PC_SWITCH1);
	immap->im_ioport.iop_pcdir &= ~(PC_SWITCH1);

	if (immap->im_ioport.iop_pcdat & (PC_SWITCH1))
		setenv ("key1", "off");
	else
		setenv ("key1", "on");
}

#ifdef CONFIG_POST
/*
 * Returns 1 if keys pressed to start the power-on long-running tests
 * Called from board_init_f().
 */
int post_hotkeys_pressed (void)
{
	return (0);
}
#endif
