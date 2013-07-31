/*
 * File: memcpy.S
 *
 * Copyright 2004-2007 Analog Devices Inc.
 * Enter bugs at http://blackfin.uclinux.org/
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

.align 2

.globl _memcpy_ASM;
.type _memcpy_ASM, STT_FUNC;
_memcpy_ASM:
	CC = R2 <=  0;	/* length not positive?*/
	IF CC JUMP  .L_P1L2147483647;	/* Nothing to do */

	P0 = R0 ;	/* dst*/
	P1 = R1 ;	/* src*/
	P2 = R2 ;	/* length */

	/* check for overlapping data */
	CC = R1 < R0;	/* src < dst */
	IF !CC JUMP .Lno_overlap;
	R3 = R1 + R2;
	CC = R0 < R3;	/* and dst < src+len */
	IF CC JUMP .Lhas_overlap;

.Lno_overlap:
	/* Check for aligned data.*/

	R3 = R1 | R0;
	R0 = 0x3;
	R3 = R3 & R0;
	CC = R3;	/* low bits set on either address? */
	IF CC JUMP .Lnot_aligned;

	/* Both addresses are word-aligned, so we can copy
	at least part of the data using word copies.*/
	P2 = P2 >> 2;
	CC = P2 <= 2;
	IF !CC JUMP .Lmore_than_seven;
	/* less than eight bytes... */
	P2 = R2;
	LSETUP(.Lthree_start, .Lthree_end) LC0=P2;
	R0 = R1;	/* setup src address for return */
.Lthree_start:
	R3 = B[P1++] (X);
.Lthree_end:
	B[P0++] = R3;

	RTS;

.Lmore_than_seven:
	/* There's at least eight bytes to copy. */
	P2 += -1;	/* because we unroll one iteration */
	LSETUP(.Lword_loop, .Lword_loop) LC0=P2;
	R0 = R1;
	I1 = P1;
	R3 = [I1++];
.Lword_loop:
	MNOP || [P0++] = R3 || R3 = [I1++];

	[P0++] = R3;
	/* Any remaining bytes to copy? */
	R3 = 0x3;
	R3 = R2 & R3;
	CC = R3 == 0;
	P1 = I1;	/* in case there's something left, */
	IF !CC JUMP .Lbytes_left;
	RTS;
.Lbytes_left:	P2 = R3;
.Lnot_aligned:
	/* From here, we're copying byte-by-byte. */
	LSETUP (.Lbyte_start , .Lbyte_end) LC0=P2;
	R0 = R1;	/* Save src address for return */
.Lbyte_start:
	R1 = B[P1++] (X);
.Lbyte_end:
	B[P0++] = R1;

.L_P1L2147483647:
	RTS;

.Lhas_overlap:
/* Need to reverse the copying, because the
 * dst would clobber the src.
 * Don't bother to work out alignment for
 * the reverse case.
 */
	R0 = R1;	/* save src for later. */
	P0 = P0 + P2;
	P0 += -1;
	P1 = P1 + P2;
	P1 += -1;
	LSETUP(.Lover_start, .Lover_end) LC0=P2;
.Lover_start:
	R1 = B[P1--] (X);
.Lover_end:
	B[P0--] = R1;

	RTS;

.size _memcpy_ASM, .-_memcpy_ASM
