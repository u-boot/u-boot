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
	write_mmcr_byte (SC520_GPTMRSTA, 0x02);
}

int timer_init(void)
{
	/* Map GP Timer 1 to Master PIC IR0  */
	write_mmcr_byte (SC520_GPTMR1MAP, 0x01);

	/* Disable GP Timers 1 & 2 - Allow configuration writes */
	write_mmcr_word (SC520_GPTMR1CTL, 0x4000);
	write_mmcr_word (SC520_GPTMR2CTL, 0x4000);

	/* Reset GP Timers 1 & 2 */
	write_mmcr_word (SC520_GPTMR1CNT, 0x0000);
	write_mmcr_word (SC520_GPTMR2CNT, 0x0000);

	/* Setup GP Timer 2 as a 100kHz (10us) prescaler */
	write_mmcr_word (SC520_GPTMR2MAXCMPA, 83);
	write_mmcr_word (SC520_GPTMR2CTL, 0xc001);

	/* Setup GP Timer 1 as a 1000 Hz (1ms) interrupt generator */
	write_mmcr_word (SC520_GPTMR1MAXCMPA, 100);
	write_mmcr_word (SC520_GPTMR1CTL, 0xe009);

	/* Clear the GP Timers status register */
	write_mmcr_byte (SC520_GPTMRSTA, 0x07);

	/* Register the SC520 specific timer interrupt handler */
	register_timer_isr (sc520_timer_isr);

	/* Install interrupt handler for GP Timer 1 */
	irq_install_handler (0, timer_isr, NULL);
	unmask_irq (0);

	return 0;
}

void udelay(unsigned long usec)
{
	int m = 0;
	long u;

	read_mmcr_word (SC520_SWTMRMILLI);
	read_mmcr_word (SC520_SWTMRMICRO);

	do {
		m += read_mmcr_word (SC520_SWTMRMILLI);
		u = read_mmcr_word (SC520_SWTMRMICRO) + (m * 1000);
	} while (u < usec);
}
