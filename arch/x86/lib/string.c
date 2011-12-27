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

#define	OP_T_THRES	8
#define OPSIZ	(sizeof(op_t))

#define BYTE_COPY_FWD(dst_bp, src_bp, nbytes)				  \
do {									  \
	int __d0;							  \
	asm volatile(							  \
		/* Clear the direction flag, so copying goes forward.  */ \
		"cld\n"							  \
		/* Copy bytes.  */					  \
		"rep\n"							  \
		"movsb" :						  \
		"=D" (dst_bp), "=S" (src_bp), "=c" (__d0) :		  \
		"0" (dst_bp), "1" (src_bp), "2" (nbytes) :		  \
		"memory");						  \
} while (0)

#define WORD_COPY_FWD(dst_bp, src_bp, nbytes_left, nbytes)		  \
do {									  \
	int __d0;							  \
	asm volatile(							  \
		/* Clear the direction flag, so copying goes forward.  */ \
		"cld\n"							  \
		/* Copy longwords.  */					  \
		"rep\n"							  \
		"movsl" :						  \
		"=D" (dst_bp), "=S" (src_bp), "=c" (__d0) :		  \
		"0" (dst_bp), "1" (src_bp), "2" ((nbytes) / 4) :	  \
		"memory");						  \
	(nbytes_left) = (nbytes) % 4;					  \
} while (0)

void *memcpy(void *dstpp, const void *srcpp, size_t len)
{
	unsigned long int dstp = (long int)dstpp;
	unsigned long int srcp = (long int)srcpp;

	/* Copy from the beginning to the end.  */

	/* If there not too few bytes to copy, use word copy.  */
	if (len >= OP_T_THRES) {
		/* Copy just a few bytes to make DSTP aligned.  */
		len -= (-dstp) % OPSIZ;
		BYTE_COPY_FWD(dstp, srcp, (-dstp) % OPSIZ);

		/* Copy from SRCP to DSTP taking advantage of the known
		 * alignment of DSTP.  Number of bytes remaining is put
		 * in the third argument, i.e. in LEN.  This number may
		 * vary from machine to machine.
		 */
		WORD_COPY_FWD(dstp, srcp, len, len);

		/* Fall out and copy the tail.  */
	}

	/* There are just a few bytes to copy. Use byte memory operations. */
	BYTE_COPY_FWD(dstp, srcp, len);

	return dstpp;
}
