/*
 * (C) Copyright 2002
 * Keith Outwater, keith_outwater@mvis.com
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
#include <asm/8xx_immap.h>
#include <linux/ctype.h>

/*
 * Basic beeper support for the GEN860T board.  The GEN860T includes
 * an audio sounder driven by a Phillips TDA8551 amplifier.  The
 * TDA8551 features a digital volume control which uses a "trinary"
 * input (high/high-Z/low) to set volume.  The 860's SPKROUT pin
 * drives the amplifier input.
 */

/*
 * Initialize beeper-related hardware. Initialize timer 1 for use with
 * the beeper. Use 66 MHz internal clock with prescale of 33 to get
 * 1 uS period per count.
 * FIXME: we should really compute the prescale based on the reported
 * core clock frequency.
 */
void init_beeper (void)
{
	volatile immap_t *immap = (immap_t *) CONFIG_SYS_IMMR;

	immap->im_cpmtimer.cpmt_tgcr &= ~TGCR_RST1 | TGCR_STP1;
	immap->im_cpmtimer.cpmt_tmr1 = ((33 << TMR_PS_SHIFT) & TMR_PS_MSK)
		| TMR_OM | TMR_FRR | TMR_ICLK_IN_GEN;
	immap->im_cpmtimer.cpmt_tcn1 = 0;
	immap->im_cpmtimer.cpmt_ter1 = 0xffff;
	immap->im_cpmtimer.cpmt_tgcr |= TGCR_RST1;
}

/*
 * Set beeper frequency.  Max allowed frequency is 2.5 KHz.  This limit
 * is mostly arbitrary, but the beeper isn't really much good beyond this
 * frequency.
 */
void set_beeper_frequency (uint frequency)
{
#define FREQ_LIMIT	2500

	volatile immap_t *immap = (immap_t *) CONFIG_SYS_IMMR;

	/*
	 * Compute timer ticks given desired frequency.  The timer is set up
	 * to count 0.5 uS per tick and it takes two ticks per cycle (Hz).
	 */
	if (frequency > FREQ_LIMIT)
		frequency = FREQ_LIMIT;
	frequency = 1000000 / frequency;
	immap->im_cpmtimer.cpmt_trr1 = (ushort) frequency;
}

/*
 * Turn the beeper on
 */
void beeper_on (void)
{
	volatile immap_t *immap = (immap_t *) CONFIG_SYS_IMMR;

	immap->im_cpmtimer.cpmt_tgcr &= ~TGCR_STP1;
}

/*
 * Turn the beeper off
 */
void beeper_off (void)
{
	volatile immap_t *immap = (immap_t *) CONFIG_SYS_IMMR;

	immap->im_cpmtimer.cpmt_tgcr |= TGCR_STP1;
}

/*
 * Increase or decrease the beeper volume.  Volume can be set
 * from off to full in 64 steps.  To increase volume, the output
 * pin is actively driven high, then returned to tristate.
 * To decrease volume, output a low on the port pin (no need to
 * change pin mode to tristate) then output a high to go back to
 * tristate.
 */
void set_beeper_volume (int steps)
{
	volatile immap_t *immap = (immap_t *) CONFIG_SYS_IMMR;
	int i;

	if (steps >= 0) {
		for (i = 0; i < (steps >= 64 ? 64 : steps); i++) {
			immap->im_cpm.cp_pbodr &= ~(0x80000000 >> 19);
			udelay (1);
			immap->im_cpm.cp_pbodr |= (0x80000000 >> 19);
			udelay (1);
		}
	} else {
		for (i = 0; i > (steps <= -64 ? -64 : steps); i--) {
			immap->im_cpm.cp_pbdat &= ~(0x80000000 >> 19);
			udelay (1);
			immap->im_cpm.cp_pbdat |= (0x80000000 >> 19);
			udelay (1);
		}
	}
}

/*
 * Check the environment to see if the beeper needs beeping.
 * Controlled by a sequence of the form:
 * freq/delta volume/on time/off time;... where:
 * freq			= frequency in Hz (0 - 2500)
 * delta volume = volume steps up or down (-64 <= vol <= 64)
 * on time		= time in mS
 * off time		= time in mS
 *
 * Return 1 on success, 0 on failure
 */
int do_beeper (char *sequence)
{
#define DELIMITER	';'

	int args[4];
	int i;
	int val;
	char *p = sequence;
	char *tp;

	/*
	 * Parse the control sequence.  This is a really simple parser
	 * without any real error checking.  You can probably blow it
	 * up really easily.
	 */
	if (*p == '\0' || !isdigit (*p)) {
		printf ("%s:%d: null or invalid string (%s)\n",
			__FILE__, __LINE__, p);
		return 0;
	}

	i = 0;
	while (*p != '\0') {
		while (*p != DELIMITER) {
			if (i > 3)
				i = 0;
			val = (int) simple_strtol (p, &tp, 0);
			if (tp == p) {
				printf ("%s:%d: no digits or bad format\n",
					__FILE__, __LINE__);
				return 0;
			} else {
				args[i] = val;
			}

			i++;
			if (*tp == DELIMITER)
				p = tp;
			else
				p = ++tp;
		}
		p++;

		/*
		 * Well, we got something that has a chance of being correct
		 */
#if 0
		for (i = 0; i < 4; i++) {
			printf ("%s:%d:arg %d = %d\n", __FILE__, __LINE__, i,
				args[i]);
		}
		printf ("\n");
#endif
		set_beeper_frequency (args[0]);
		set_beeper_volume (args[1]);
		beeper_on ();
		udelay (1000 * args[2]);
		beeper_off ();
		udelay (1000 * args[3]);
	}
	return 1;
}
