/*
 * Gumstix Pepper and AM335x-based boards information header
 *
 * Copyright (C) 2014, Gumstix, Inc. - http://www.gumstix.com/
 *
 * SPDX-License-Identifier:     GPL-2.0+
 */

#ifndef _BOARD_H_
#define _BOARD_H_

/*
 * We must be able to enable uart0, for initial output. We then have a
 * main pinmux function that can be overridden to enable all other pinmux that
 * is required on the board.
 */
void enable_uart0_pin_mux(void);
void enable_board_pin_mux(void);
#endif
