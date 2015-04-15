/*
 * Board specific setup info
 *
 * (C) Copyright 2003, ARM Ltd.
 * Philippe Robin, <philippe.robin@arm.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <config.h>

/* Set up the platform, once the cpu has been initialized */
.globl lowlevel_init
lowlevel_init:

	/* All done by Versatile's boot monitor! */
	mov pc, lr
