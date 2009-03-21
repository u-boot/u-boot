/*
 * (C) Copyright 2002
 * Daniel Engström, Omicron Ceti AB, daniel@omicron.se.
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
#include <asm/io.h>
#include <asm/i8254.h>
#include <asm/ibmpc.h>

#define TIMER0_VALUE 0x04aa /* 1kHz 1.9318MHz / 1000 */
#define TIMER2_VALUE 0x0a8e /* 440Hz */

int timer_init(void)
{
	/* initialize timer 0 and 2
	 *
	 * Timer 0 is used to increment system_tick 1000 times/sec
	 * Timer 1 was used for DRAM refresh in early PC's
	 * Timer 2 is used to drive the speaker
	 * (to stasrt a beep: write 3 to port 0x61,
	 * to stop it again: write 0)
	 */
	outb (PIT_CMD_CTR0 | PIT_CMD_BOTH | PIT_CMD_MODE2,
	      PIT_BASE + PIT_COMMAND);
	outb (TIMER0_VALUE & 0xff, PIT_BASE + PIT_T0);
	outb (TIMER0_VALUE >> 8, PIT_BASE + PIT_T0);

	outb (PIT_CMD_CTR2 | PIT_CMD_BOTH | PIT_CMD_MODE3,
	      PIT_BASE + PIT_COMMAND);
	outb (TIMER2_VALUE & 0xff, PIT_BASE + PIT_T2);
	outb (TIMER2_VALUE >> 8, PIT_BASE + PIT_T2);

	irq_install_handler (0, timer_isr, NULL);
	unmask_irq (0);

	return 0;
}

static u16 read_pit(void)
{
	u8 low;

	outb (PIT_CMD_LATCH, PIT_BASE + PIT_COMMAND);
	low = inb (PIT_BASE + PIT_T0);

	return ((inb (PIT_BASE + PIT_T0) << 8) | low);
}

/* this is not very exact */
void udelay (unsigned long usec)
{
	int counter;
	int wraps;

	if (timer_init_done)
	{
		counter = read_pit ();
		wraps = usec / 1000;
		usec = usec % 1000;

		usec *= 1194;
		usec /= 1000;
		usec += counter;

		while (usec > 1194) {
			usec -= 1194;
			wraps++;
		}

		while (1) {
			int new_count = read_pit ();

			if (((new_count < usec) && !wraps) || wraps < 0)
				break;

			if (new_count > counter)
				wraps--;

			counter = new_count;
		}
	}

}
