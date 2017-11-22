/*
 * Copyright 2016 Broadcom Corporation.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

/* BOOT0 header information */
_start:
	ARM_VECTORS
	.word	0xbabeface
	.word	_end - _start
