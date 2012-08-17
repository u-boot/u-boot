/*
 * (C) Copyright 2010
 * Texas Instruments, <www.ti.com>
 * Aneesh V <aneesh@ti.com>
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */
#ifndef _UTILS_H_
#define _UTILS_H_

static inline s32 log_2_n_round_up(u32 n)
{
	s32 log2n = -1;
	u32 temp = n;

	while (temp) {
		log2n++;
		temp >>= 1;
	}

	if (n & (n - 1))
		return log2n + 1; /* not power of 2 - round up */
	else
		return log2n; /* power of 2 */
}

static inline s32 log_2_n_round_down(u32 n)
{
	s32 log2n = -1;
	u32 temp = n;

	while (temp) {
		log2n++;
		temp >>= 1;
	}

	return log2n;
}

#endif
