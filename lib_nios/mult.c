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


#include <common.h>

#if !defined(CONFIG_SYS_NIOS_MULT_HW) && !defined(CONFIG_SYS_NIOS_MULT_MSTEP)

#include "math.h"

USItype __mulsi3 (USItype a, USItype b)
{
	USItype c = 0;

	while (a != 0) {
		if (a & 1)
			c += b;
		a >>= 1;
		b <<= 1;
	}

	return c;
}


UHItype __mulhi3 (UHItype a, UHItype b)
{
	UHItype c = 0;

	while (a != 0) {
		if (a & 1)
			c += b;
		a >>= 1;
		b <<= 1;
	}

	return c;
}

#endif /*!defined(CONFIG_SYS_NIOS_MULT_HW) && !defined(CONFIG_SYS_NIOS_MULT_MSTEP) */
