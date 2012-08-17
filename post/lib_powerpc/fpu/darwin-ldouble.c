/*
 * Borrowed from GCC 4.2.2 (which still was GPL v2+)
 */
/* 128-bit long double support routines for Darwin.
   Copyright (C) 1993, 2003, 2004, 2005, 2006, 2007
   Free Software Foundation, Inc.

This file is part of GCC.

GCC is free software; you can redistribute it and/or modify it under
the terms of the GNU General Public License as published by the Free
Software Foundation; either version 2, or (at your option) any later
version.

In addition to the permissions in the GNU General Public License, the
Free Software Foundation gives you unlimited permission to link the
compiled version of this file into combinations with other programs,
and to distribute those combinations without any restriction coming
from the use of this file.  (The General Public License restrictions
do apply in other respects; for example, they cover modification of
the file, and distribution when not linked into a combine
executable.)

GCC is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or
FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
for more details.

You should have received a copy of the GNU General Public License
along with GCC; see the file COPYING.  If not, write to the Free
Software Foundation, 51 Franklin Street, Fifth Floor, Boston, MA
02110-1301, USA.  */

/*
 * Implementations of floating-point long double basic arithmetic
 * functions called by the IBM C compiler when generating code for
 * PowerPC platforms.  In particular, the following functions are
 * implemented: __gcc_qadd, __gcc_qsub, __gcc_qmul, and __gcc_qdiv.
 * Double-double algorithms are based on the paper "Doubled-Precision
 * IEEE Standard 754 Floating-Point Arithmetic" by W. Kahan, February 26,
 * 1987.  An alternative published reference is "Software for
 * Doubled-Precision Floating-Point Computations", by Seppo Linnainmaa,
 * ACM TOMS vol 7 no 3, September 1981, pages 272-283.
 */

/*
 * Each long double is made up of two IEEE doubles.  The value of the
 * long double is the sum of the values of the two parts.  The most
 * significant part is required to be the value of the long double
 * rounded to the nearest double, as specified by IEEE.  For Inf
 * values, the least significant part is required to be one of +0.0 or
 * -0.0.  No other requirements are made; so, for example, 1.0 may be
 * represented as (1.0, +0.0) or (1.0, -0.0), and the low part of a
 * NaN is don't-care.
 *
 * This code currently assumes big-endian.
 */

#define fabs(x) __builtin_fabs(x)
#define isless(x, y) __builtin_isless(x, y)
#define inf() __builtin_inf()
#define unlikely(x) __builtin_expect((x), 0)
#define nonfinite(a) unlikely(!isless(fabs(a), inf()))

typedef union {
	long double ldval;
	double dval[2];
} longDblUnion;

/* Add two 'long double' values and return the result.	*/
long double __gcc_qadd(double a, double aa, double c, double cc)
{
	longDblUnion x;
	double z, q, zz, xh;

	z = a + c;

	if (nonfinite(z)) {
		z = cc + aa + c + a;
		if (nonfinite(z))
			return z;
		x.dval[0] = z;	/* Will always be DBL_MAX.  */
		zz = aa + cc;
		if (fabs(a) > fabs(c))
			x.dval[1] = a - z + c + zz;
		else
			x.dval[1] = c - z + a + zz;
	} else {
		q = a - z;
		zz = q + c + (a - (q + z)) + aa + cc;

		/* Keep -0 result.  */
		if (zz == 0.0)
			return z;

		xh = z + zz;
		if (nonfinite(xh))
			return xh;

		x.dval[0] = xh;
		x.dval[1] = z - xh + zz;
	}
	return x.ldval;
}

long double __gcc_qsub(double a, double b, double c, double d)
{
	return __gcc_qadd(a, b, -c, -d);
}

long double __gcc_qmul(double a, double b, double c, double d)
{
	longDblUnion z;
	double t, tau, u, v, w;

	t = a * c;		/* Highest order double term.  */

	if (unlikely(t == 0)	/* Preserve -0.  */
	    || nonfinite(t))
		return t;

	/* Sum terms of two highest orders. */

	/* Use fused multiply-add to get low part of a * c.  */
#ifndef __NO_FPRS__
	asm("fmsub %0,%1,%2,%3" : "=f"(tau) : "f"(a), "f"(c), "f"(t));
#else
	tau = fmsub(a, c, t);
#endif
	v = a * d;
	w = b * c;
	tau += v + w;		/* Add in other second-order terms.  */
	u = t + tau;

	/* Construct long double result.  */
	if (nonfinite(u))
		return u;
	z.dval[0] = u;
	z.dval[1] = (t - u) + tau;
	return z.ldval;
}
