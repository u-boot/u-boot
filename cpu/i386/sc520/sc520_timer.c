/*
 * (C) Copyright 2002
 * Daniel Engström, Omicron Ceti AB <daniel@omicron.se>.
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

/* stuff specific for the sc520, but independent of implementation */

#include <common.h>
#include <asm/interrupt.h>
#include <asm/ic/sc520.h>

void sc520_timer_isr(void)
{
	/* Ack the GP Timer Interrupt */
	sc520_mmcr->gptmrsta = 0x02;
}

int timer_init(void)
{
	/* Register the SC520 specific timer interrupt handler */
	register_timer_isr (sc520_timer_isr);

	/* Install interrupt handler for GP Timer 1 */
	irq_install_handler (0, timer_isr, NULL);

	/* Map GP Timer 1 to Master PIC IR0  */
	sc520_mmcr->gp_tmr_int_map[1] = 0x01;

	/* Disable GP Timers 1 & 2 - Allow configuration writes */
	sc520_mmcr->gptmr1ctl = 0x4000;
	sc520_mmcr->gptmr2ctl = 0x4000;

	/* Reset GP Timers 1 & 2 */
	sc520_mmcr->gptmr1cnt = 0x0000;
	sc520_mmcr->gptmr2cnt = 0x0000;

	/* Setup GP Timer 2 as a 100kHz (10us) prescaler */
	sc520_mmcr->gptmr2maxcmpa = 83;
	sc520_mmcr->gptmr2ctl = 0xc001;

	/* Setup GP Timer 1 as a 1000 Hz (1ms) interrupt generator */
	sc520_mmcr->gptmr1maxcmpa = 100;
	sc520_mmcr->gptmr1ctl = 0xe009;

	unmask_irq (0);

	/* Clear the GP Timer 1 status register to get the show rolling*/
	sc520_mmcr->gptmrsta = 0x02;

	return 0;
}

void __udelay(unsigned long usec)
{
	int m = 0;
	long u;
	long temp;

	temp = sc520_mmcr->swtmrmilli;
	temp = sc520_mmcr->swtmrmicro;

	do {
		m += sc520_mmcr->swtmrmilli;
		u = sc520_mmcr->swtmrmicro + (m * 1000);
	} while (u < usec);
}
