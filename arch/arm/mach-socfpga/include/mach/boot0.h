/*
 * Specialty padding for the Altera SoCFPGA preloader image
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __BOOT0_H
#define __BOOT0_H

_start:
	ARM_VECTORS

#ifdef CONFIG_SPL_BUILD
	.balignl 64,0xf33db33f;

	.word	0x1337c0d3;	/* SoCFPGA preloader validation word */
	.word	0xc01df00d;	/* Version, flags, length */
	.word	0xcafec0d3;	/* Checksum, zero-pad */
	nop;

	b reset;		/* SoCFPGA jumps here */
	nop;
	nop;
	nop;
#endif

#endif /* __BOOT0_H */
