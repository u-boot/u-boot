/*
 * (C) Copyright 2005-2010
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * (C) Copyright 2010
 * Sergei Poselenov, Emcraft Systems, sposelenov@emcraft.com.
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */
#ifndef _led_display_h_
#define _led_display_h_

/* Display Commands */
#define DISPLAY_CLEAR	0x1 /* Clear the display */
#define DISPLAY_HOME	0x2 /* Set cursor at home position */

void display_set(int cmd);
int display_putc(char c);
#endif
