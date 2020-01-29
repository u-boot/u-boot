/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2019 Intel Corporation <www.intel.com>
 */

/* board early initialization */
.globl early_board_init
early_board_init:
	jmp	early_board_init_ret
