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

void reset_timer(void)
{
	write_mmcr_word(SC520_GPTMR0CNT, 0);
	write_mmcr_word(SC520_GPTMR0CTL, 0x6001);

}

ulong get_timer(ulong base)
{
	/* fixme: 30 or 33 */
	return	read_mmcr_word(SC520_GPTMR0CNT) / 33;
}

void set_timer(ulong t)
{
	/* FixMe: use two cascade coupled timers */
	write_mmcr_word(SC520_GPTMR0CTL, 0x4001);
	write_mmcr_word(SC520_GPTMR0CNT, t*33);
	write_mmcr_word(SC520_GPTMR0CTL, 0x6001);
}


void udelay(unsigned long usec)
{
	int m=0;
	long u;

	read_mmcr_word(SC520_SWTMRMILLI);
	read_mmcr_word(SC520_SWTMRMICRO);

#if 0
	/* do not enable this line, udelay is used in the serial driver -> recursion */
	printf("udelay: %ld m.u %d.%d  tm.tu %d.%d\n", usec, m, u, tm, tu);
#endif
	while (1) {

		m += read_mmcr_word(SC520_SWTMRMILLI);
		u = read_mmcr_word(SC520_SWTMRMICRO) + (m * 1000);

		if (usec <= u) {
			break;
		}
	}
}
