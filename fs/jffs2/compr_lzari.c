/*
 * JFFS2 -- Journalling Flash File System, Version 2.
 *
 * Copyright (C) 2004 Patrik Kluba,
 *                    University of Szeged, Hungary
 *
 * For licensing information, see the file 'LICENCE' in the
 * jffs2 directory.
 *
 * $Id: compr_lzari.c,v 1.3 2004/06/23 16:34:39 havasi Exp $
 *
 */

/*
   Lempel-Ziv-Arithmetic coding compression module for jffs2
   Based on the LZARI source included in LDS (lossless datacompression sources)
*/

/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */

/*
Original copyright follows:

**************************************************************
	LZARI.C -- A Data Compression Program
	(tab = 4 spaces)
**************************************************************
	4/7/1989 Haruhiko Okumura
	Use, distribute, and modify this program freely.
	Please send me your improved versions.
		PC-VAN		SCIENCE
		NIFTY-Serve	PAF01022
		CompuServe	74050,1022
**************************************************************

LZARI.C (c)1989 by Haruyasu Yoshizaki, Haruhiko Okumura, and Kenji Rikitake.
All rights reserved. Permission granted for non-commercial use.

*/

/*

	2004-02-18  pajko <pajko(AT)halom(DOT)u-szeged(DOT)hu>
				Removed unused variables and fixed no return value

	2004-02-16  pajko <pajko(AT)halom(DOT)u-szeged(DOT)hu>
				Initial release

*/


#include <config.h>
#if ((CONFIG_COMMANDS & CFG_CMD_JFFS2) && defined(CONFIG_JFFS2_LZO_LZARI))

#include <linux/stddef.h>
#include <jffs2/jffs2.h>


#define N			4096	/* size of ring buffer */
#define F			60	/* upper limit for match_length */
#define THRESHOLD		2	/* encode string into position and length
					   if match_length is greater than this */
#define NIL			N	/* index for root of binary search trees */

static unsigned char
		text_buf[N + F - 1];	/* ring buffer of size N,
			with extra F-1 bytes to facilitate string comparison */

/********** Arithmetic Compression **********/

/*  If you are not familiar with arithmetic compression, you should read
		I. E. Witten, R. M. Neal, and J. G. Cleary,
			Communications of the ACM, Vol. 30, pp. 520-540 (1987),
	from which much have been borrowed.  */

#define M   15

/*	Q1 (= 2 to the M) must be sufficiently large, but not so
	large as the unsigned long 4 * Q1 * (Q1 - 1) overflows.  */

#define Q1  (1UL << M)
#define Q2  (2 * Q1)
#define Q3  (3 * Q1)
#define Q4  (4 * Q1)
#define MAX_CUM (Q1 - 1)

#define N_CHAR  (256 - THRESHOLD + F)
	/* character code = 0, 1, ..., N_CHAR - 1 */

static unsigned long char_to_sym[N_CHAR], sym_to_char[N_CHAR + 1];
static unsigned long
	sym_freq[N_CHAR + 1],  /* frequency for symbols */
	sym_cum[N_CHAR + 1],   /* cumulative freq for symbols */
	position_cum[N + 1];   /* cumulative freq for positions */

static void StartModel(void)  /* Initialize model */
{
	unsigned long ch, sym, i;

	sym_cum[N_CHAR] = 0;
	for (sym = N_CHAR; sym >= 1; sym--) {
		ch = sym - 1;
		char_to_sym[ch] = sym;  sym_to_char[sym] = ch;
		sym_freq[sym] = 1;
		sym_cum[sym - 1] = sym_cum[sym] + sym_freq[sym];
	}
	sym_freq[0] = 0;  /* sentinel (!= sym_freq[1]) */
	position_cum[N] = 0;
	for (i = N; i >= 1; i--)
		position_cum[i - 1] = position_cum[i] + 10000 / (i + 200);
			/* empirical distribution function (quite tentative) */
			/* Please devise a better mechanism! */
}

static void UpdateModel(unsigned long sym)
{
	unsigned long c, ch_i, ch_sym;
	unsigned long i;
	if (sym_cum[0] >= MAX_CUM) {
		c = 0;
		for (i = N_CHAR; i > 0; i--) {
			sym_cum[i] = c;
			c += (sym_freq[i] = (sym_freq[i] + 1) >> 1);
		}
		sym_cum[0] = c;
	}
	for (i = sym; sym_freq[i] == sym_freq[i - 1]; i--) ;
	if (i < sym) {
		ch_i = sym_to_char[i];    ch_sym = sym_to_char[sym];
		sym_to_char[i] = ch_sym;  sym_to_char[sym] = ch_i;
		char_to_sym[ch_i] = sym;  char_to_sym[ch_sym] = i;
	}
	sym_freq[i]++;
	while (--i > 0) sym_cum[i]++;
	sym_cum[0]++;
}

static unsigned long BinarySearchSym(unsigned long x)
	/* 1      if x >= sym_cum[1],
	   N_CHAR if sym_cum[N_CHAR] > x,
	   i such that sym_cum[i - 1] > x >= sym_cum[i] otherwise */
{
	unsigned long i, j, k;

	i = 1;  j = N_CHAR;
	while (i < j) {
		k = (i + j) / 2;
		if (sym_cum[k] > x) i = k + 1;  else j = k;
	}
	return i;
}

unsigned long BinarySearchPos(unsigned long x)
	/* 0 if x >= position_cum[1],
	   N - 1 if position_cum[N] > x,
	   i such that position_cum[i] > x >= position_cum[i + 1] otherwise */
{
	unsigned long i, j, k;

	i = 1;  j = N;
	while (i < j) {
		k = (i + j) / 2;
		if (position_cum[k] > x) i = k + 1;  else j = k;
	}
	return i - 1;
}

static int Decode(unsigned char *srcbuf, unsigned char *dstbuf, unsigned long srclen,
					unsigned long dstlen)	/* Just the reverse of Encode(). */
{
	unsigned long i, r, j, k, c, range, sym;
	unsigned char *ip, *op;
	unsigned char *srcend = srcbuf + srclen;
	unsigned char *dstend = dstbuf + dstlen;
	unsigned char buffer = 0;
	unsigned char mask = 0;
	unsigned long low = 0;
	unsigned long high = Q4;
	unsigned long value = 0;

	ip = srcbuf;
	op = dstbuf;
	for (i = 0; i < M + 2; i++) {
		value *= 2;
		if ((mask >>= 1) == 0) {
			buffer = (ip >= srcend) ? 0 : *(ip++);
			mask = 128;
		}
		value += ((buffer & mask) != 0);
	}

	StartModel();
	for (i = 0; i < N - F; i++) text_buf[i] = ' ';
	r = N - F;

	while (op < dstend) {
		range = high - low;
		sym = BinarySearchSym((unsigned long)
				(((value - low + 1) * sym_cum[0] - 1) / range));
		high = low + (range * sym_cum[sym - 1]) / sym_cum[0];
		low +=       (range * sym_cum[sym    ]) / sym_cum[0];
		for ( ; ; ) {
			if (low >= Q2) {
				value -= Q2;  low -= Q2;  high -= Q2;
			} else if (low >= Q1 && high <= Q3) {
				value -= Q1;  low -= Q1;  high -= Q1;
			} else if (high > Q2) break;
			low += low;  high += high;
			value *= 2;
			if ((mask >>= 1) == 0) {
				buffer = (ip >= srcend) ? 0 : *(ip++);
				mask = 128;
			}
			value += ((buffer & mask) != 0);
		}
		c = sym_to_char[sym];
		UpdateModel(sym);
		if (c < 256) {
			if (op >= dstend) return -1;
			*(op++) = c;
			text_buf[r++] = c;
			r &= (N - 1);
		} else {
			j = c - 255 + THRESHOLD;
			range = high - low;
			i = BinarySearchPos((unsigned long)
				(((value - low + 1) * position_cum[0] - 1) / range));
			high = low + (range * position_cum[i    ]) / position_cum[0];
			low +=       (range * position_cum[i + 1]) / position_cum[0];
			for ( ; ; ) {
				if (low >= Q2) {
					value -= Q2;  low -= Q2;  high -= Q2;
				} else if (low >= Q1 && high <= Q3) {
					value -= Q1;  low -= Q1;  high -= Q1;
				} else if (high > Q2) break;
				low += low;  high += high;
				value *= 2;
				if ((mask >>= 1) == 0) {
					buffer = (ip >= srcend) ? 0 : *(ip++);
					mask = 128;
				}
				value += ((buffer & mask) != 0);
			}
			i = (r - i - 1) & (N - 1);
			for (k = 0; k < j; k++) {
				c = text_buf[(i + k) & (N - 1)];
				if (op >= dstend) return -1;
				*(op++) = c;
				text_buf[r++] = c;
				r &= (N - 1);
			}
		}
	}
	return 0;
}

int lzari_decompress(unsigned char *data_in, unsigned char *cpage_out,
		      u32 srclen, u32 destlen)
{
    return Decode(data_in, cpage_out, srclen, destlen);
}
#endif /* ((CONFIG_COMMANDS & CFG_CMD_JFFS2) && defined(CONFIG_JFFS2_LZO_LZARI)) */
