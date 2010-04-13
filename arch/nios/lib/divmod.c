/*
 * This file is part of GNU CC.
 *
 * GNU CC is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published
 * by the Free Software Foundation; either version 2, or (at your
 * option) any later version.
 *
 * GNU CC is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with GNU CC; see the file COPYING.  If not, write
 * to the Free Software Foundation, 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */


#include "math.h"

USItype udivmodsi4 (USItype num, USItype den, word_type modwanted)
{
	USItype bit = 1;
	USItype res = 0;

	while (den < num && bit && !(den & (1L << 31))) {
		den <<= 1;
		bit <<= 1;
	}
	while (bit) {
		if (num >= den) {
			num -= den;
			res |= bit;
		}
		bit >>= 1;
		den >>= 1;
	}
	if (modwanted)
		return num;
	return res;
}


SItype __divsi3 (SItype a, SItype b)
{
	word_type neg = 0;
	SItype res;

	if (a < 0) {
		a = -a;
		neg = !neg;
	}

	if (b < 0) {
		b = -b;
		neg = !neg;
	}

	res = udivmodsi4 (a, b, 0);

	if (neg)
		res = -res;

	return res;
}


SItype __modsi3 (SItype a, SItype b)
{
	word_type neg = 0;
	SItype res;

	if (a < 0) {
		a = -a;
		neg = 1;
	}

	if (b < 0)
		b = -b;

	res = udivmodsi4 (a, b, 1);

	if (neg)
		res = -res;

	return res;
}


SItype __udivsi3 (SItype a, SItype b)
{
	return udivmodsi4 (a, b, 0);
}


SItype __umodsi3 (SItype a, SItype b)
{
	return udivmodsi4 (a, b, 1);
}
