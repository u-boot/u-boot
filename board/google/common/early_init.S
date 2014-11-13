/*
 * Copyright (c) 2014 Google, Inc
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

.globl early_board_init
early_board_init:
	/* No 32-bit board specific initialisation */
	jmp	early_board_init_ret
