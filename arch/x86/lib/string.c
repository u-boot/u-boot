/*
 * Copyright (C) 1991,1992,1993,1997,1998,2003, 2005 Free Software Foundation, Inc.
 * This file is part of the GNU C Library.
 * Copyright (c) 2011 The Chromium OS Authors.
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

/* From glibc-2.14, sysdeps/i386/memset.c */

#include <compiler.h>
#include <asm/string.h>
#include <linux/types.h>

typedef uint32_t op_t;

void *memset(void *dstpp, int c, size_t len)
{
	int d0;
	unsigned long int dstp = (unsigned long int) dstpp;

	/* This explicit register allocation improves code very much indeed. */
	register op_t x asm("ax");

	x = (unsigned char) c;

	/* Clear the direction flag, so filling will move forward.  */
	asm volatile("cld");

	/* This threshold value is optimal.  */
	if (len >= 12) {
		/* Fill X with four copies of the char we want to fill with. */
		x |= (x << 8);
		x |= (x << 16);

		/* Adjust LEN for the bytes handled in the first loop.  */
		len -= (-dstp) % sizeof(op_t);

		/*
		 * There are at least some bytes to set. No need to test for
		 * LEN == 0 in this alignment loop.
		 */

		/* Fill bytes until DSTP is aligned on a longword boundary. */
		asm volatile(
			"rep\n"
			"stosb" /* %0, %2, %3 */ :
			"=D" (dstp), "=c" (d0) :
			"0" (dstp), "1" ((-dstp) % sizeof(op_t)), "a" (x) :
			"memory");

		/* Fill longwords.  */
		asm volatile(
			"rep\n"
			"stosl" /* %0, %2, %3 */ :
			"=D" (dstp), "=c" (d0) :
			"0" (dstp), "1" (len / sizeof(op_t)), "a" (x) :
			"memory");
		len %= sizeof(op_t);
	}

	/* Write the last few bytes. */
	asm volatile(
		"rep\n"
		"stosb" /* %0, %2, %3 */ :
		"=D" (dstp), "=c" (d0) :
		"0" (dstp), "1" (len), "a" (x) :
		"memory");

	return dstpp;
}
