/*
 * (C) Copyright 2005-2009
 * Jens Scharsig @ BuS Elektronik GmbH & Co. KG, <esw@bus-elektronik.de>
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

#ifndef __BUS_VCXK_H_
#define __BUS_VCXK_H_

extern int vcxk_init(unsigned long width, unsigned long height);
extern void vcxk_setpixel(int x, int y, unsigned long color);
extern int vcxk_acknowledge_wait(void);
extern int vcxk_request(void);
extern void vcxk_loadimage(ulong source);
extern int vcxk_display_bitmap(ulong addr, int x, int y);
extern void vcxk_setbrightness(unsigned int side, short brightness);
extern int video_display_bitmap(ulong addr, int x, int y);

#endif
