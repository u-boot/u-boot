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


static volatile unsigned long system_ticks;
static int timer_init_done =0;

static void timer_isr(void *unused)
{
	system_ticks++;
}

unsigned long get_system_ticks(void)
{
	return system_ticks;
}

#define TIMER0_VALUE 0x04aa /* 1kHz 1.9318MHz / 1000 */
#define TIMER2_VALUE 0x0a8e /* 440Hz */

int timer_init(void)
{
	system_ticks = 0;

	irq_install_handler(0, timer_isr, NULL);

	/* initialize timer 0 and 2
	 *
	 * Timer 0 is used to increment system_tick 1000 times/sec
	 * Timer 1 was used for DRAM refresh in early PC's
	 * Timer 2 is used to drive the speaker
	 * (to stasrt a beep: write 3 to port 0x61,
	 * to stop it again: write 0)
	 */

	outb(PIT_CMD_CTR0|PIT_CMD_BOTH|PIT_CMD_MODE2, PIT_BASE + PIT_COMMAND);
	outb(TIMER0_VALUE&0xff, PIT_BASE + PIT_T0);
	outb(TIMER0_VALUE>>8, PIT_BASE + PIT_T0);

	outb(PIT_CMD_CTR2|PIT_CMD_BOTH|PIT_CMD_MODE3, PIT_BASE + PIT_COMMAND);
	outb(TIMER2_VALUE&0xff, PIT_BASE + PIT_T2);
	outb(TIMER2_VALUE>>8, PIT_BASE + PIT_T2);

	timer_init_done = 1;

	return 0;
}


#ifdef CFG_TIMER_GENERIC

/* the unit for these is CFG_HZ */

/* FixMe: implement these */
void reset_timer (void)
{
	system_ticks = 0;
}

ulong get_timer (ulong base)
{
	return (system_ticks - base);
}

void set_timer (ulong t)
{
	system_ticks = t;
}

static u16 read_pit(void)
{
	u8 low;
	outb(PIT_CMD_LATCH, PIT_BASE + PIT_COMMAND);
	low = inb(PIT_BASE + PIT_T0);
	return ((inb(PIT_BASE + PIT_T0) << 8) | low);
}

/* this is not very exact */
void udelay (unsigned long usec)
{
	int counter;
	int wraps;

	if (!timer_init_done) {
		return;
	}
	counter = read_pit();
	wraps = usec/1000;
	usec = usec%1000;

	usec*=1194;
	usec/=1000;
	usec+=counter;
	if (usec > 1194) {
		usec-=1194;
		wraps++;
	}

	while (1) {
		int new_count = read_pit();

		if (((new_count < usec) && !wraps) || wraps < 0) {
			break;
		}

		if (new_count > counter) {
			wraps--;
		}
		counter = new_count;
	}

}

#if 0
/* this is a version with debug output */
void _udelay (unsigned long usec)
{
	int counter;
	int wraps;

	int usec1, usec2, usec3;
	int wraps1, wraps2, wraps3, wraps4;
	int ctr1, ctr2, ctr3, nct1, nct2;
	int i;
	usec1=usec;
	if (!timer_init_done) {
		return;
	}
	counter = read_pit();
	ctr1 = counter;
	wraps = usec/1000;
	usec = usec%1000;

	usec2 = usec;
	wraps1 = wraps;

	usec*=1194;
	usec/=1000;
	usec+=counter;
	if (usec > 1194) {
		usec-=1194;
		wraps++;
	}

	usec3 = usec;
	wraps2 = wraps;

	ctr2 = wraps3 = nct1 = 4711;
	ctr3 = wraps4 = nct2 = 4711;
	i=0;
	while (1) {
		int new_count = read_pit();
		i++;
		if ((new_count < usec && !wraps) || wraps < 0) {
			break;
		}

		if (new_count > counter) {
			wraps--;
		}
		if (ctr2==4711) {
			ctr2 = counter;
			wraps3 = wraps;
			nct1 = new_count;
		} else {
			ctr3 = counter;
			wraps4 = wraps;
			nct2 = new_count;
		}

		counter = new_count;
	}

	printf("udelay(%d)\n", usec1);
	printf("counter %d\n", ctr1);
	printf("1: wraps %d, usec %d\n", wraps1, usec2);
	printf("2: wraps %d, usec %d\n", wraps2, usec3);
	printf("new_count[0] %d counter %d wraps %d\n", nct1, ctr2, wraps3);
	printf("new_count[%d] %d counter %d wraps %d\n", i, nct2, ctr3, wraps4);

	printf("%d %d %d %d %d\n",
	       read_pit(), read_pit(), read_pit(),
	       read_pit(), read_pit());
}
#endif
#endif
