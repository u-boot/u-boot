/*
 * Specialty padding for the RCar Gen2 TPL JTAG loading
 *
 * SPDX-License-Identifier:	GPL-2.0
 */

#ifndef __BOOT0_H
#define __BOOT0_H

_start:
	ARM_VECTORS

#ifdef CONFIG_TPL_BUILD
	.word	0x0badc0d3;
	.word	0x0badc0d3;
	.word	0x0badc0d3;
	.word	0x0badc0d3;
	.word	0x0badc0d3;
	.word	0x0badc0d3;
	.word	0x0badc0d3;
	.word	0x0badc0d3;
#endif

#endif /* __BOOT0_H */
