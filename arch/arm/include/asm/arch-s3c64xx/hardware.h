/*
 * Originates from Samsung's u-boot 1.1.6 port to S3C6400 / SMDK6400
 *
 * (C) Copyright 2008
 * Guennadi Liakhovetki, DENX Software Engineering, <lg@denx.de>
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

#ifndef _ARCH_HARDWARE_H_
#define _ARCH_HARDWARE_H_

#include <asm/sizes.h>

#ifndef __ASSEMBLY__
#define UData(Data)	((unsigned long) (Data))

#define __REG(x)	(*(vu_long *)(x))
#define __REGl(x)	(*(vu_long *)(x))
#define __REGw(x)	(*(vu_short *)(x))
#define __REGb(x)	(*(vu_char *)(x))
#define __REG2(x, y)	(*(vu_long *)((x) + (y)))
#else
#define UData(Data)	(Data)

#define __REG(x)	(x)
#define __REGl(x)	(x)
#define __REGw(x)	(x)
#define __REGb(x)	(x)
#define __REG2(x, y)	((x) + (y))
#endif

#define Fld(Size, Shft)	(((Size) << 16) + (Shft))

#define FSize(Field)	((Field) >> 16)
#define FShft(Field)	((Field) & 0x0000FFFF)
#define FMsk(Field)	(((UData (1) << FSize (Field)) - 1) << FShft (Field))
#define FAlnMsk(Field)	((UData (1) << FSize (Field)) - 1)
#define F1stBit(Field)	(UData (1) << FShft (Field))

#define FClrBit(Data, Bit)	(Data = (Data & ~(Bit)))
#define FClrFld(Data, Field)	(Data = (Data & ~FMsk(Field)))

#define FInsrt(Value, Field) \
			(UData (Value) << FShft (Field))

#define FExtr(Data, Field) \
			((UData (Data) >> FShft (Field)) & FAlnMsk (Field))

#endif /* _ARCH_HARDWARE_H_ */
