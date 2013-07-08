/*
 * Copyright (C) 2007 Sergey Kubushyn <ksi@koi8.net>
 *
 * Board-specific low level initialization code. Called at the very end
 * of arch/arm/cpu/arm926ejs/davinci/lowlevel_init.S. Just returns if there is no
 * initialization required.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <config.h>

.globl	dv_board_init
dv_board_init:

	mov	pc, lr
