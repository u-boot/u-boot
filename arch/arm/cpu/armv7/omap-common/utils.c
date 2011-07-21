/*
 * Copyright 2011 Linaro Limited
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
#include <common.h>
static void do_cancel_out(u32 *num, u32 *den, u32 factor)
{
	while (1) {
		if (((*num)/factor*factor == (*num)) &&
		   ((*den)/factor*factor == (*den))) {
			(*num) /= factor;
			(*den) /= factor;
		} else
			break;
	}
}

/*
 * Cancel out the denominator and numerator of a fraction
 * to get smaller numerator and denominator.
 */
void cancel_out(u32 *num, u32 *den, u32 den_limit)
{
	do_cancel_out(num, den, 2);
	do_cancel_out(num, den, 3);
	do_cancel_out(num, den, 5);
	do_cancel_out(num, den, 7);
	do_cancel_out(num, den, 11);
	do_cancel_out(num, den, 13);
	do_cancel_out(num, den, 17);
	while ((*den) > den_limit) {
		*num /= 2;
		/*
		 * Round up the denominator so that the final fraction
		 * (num/den) is always <= the desired value
		 */
		*den = (*den + 1) / 2;
	}
}
