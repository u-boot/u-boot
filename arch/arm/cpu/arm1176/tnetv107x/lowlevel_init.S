/*
 * TNETV107X: Low-level pre-relocation initialization
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

.globl lowlevel_init
lowlevel_init:
	/* nothing for now, maybe needed for more exotic boot modes */
	mov	pc, lr
