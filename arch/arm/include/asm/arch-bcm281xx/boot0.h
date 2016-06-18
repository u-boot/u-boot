/*
 * Copyright 2016 Broadcom Corporation.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __BOOT0_H
#define __BOOT0_H

/* BOOT0 header information */
#define ARM_SOC_BOOT0_HOOK	\
	.word	0xbabeface;	\
	.word	_end - _start

#endif /* __BOOT0_H */
