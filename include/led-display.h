/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2005-2010
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * (C) Copyright 2010
 * Sergei Poselenov, Emcraft Systems, sposelenov@emcraft.com.
 */
#ifndef _led_display_h_
#define _led_display_h_

/* Display Commands */
#define DISPLAY_CLEAR	0x1 /* Clear the display */
#define DISPLAY_HOME	0x2 /* Set cursor at home position */

void display_set(int cmd);
int display_putc(char c);
#endif
