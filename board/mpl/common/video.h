/*
 * (C) Copyright 2001
 * Denis Peter, MPL AG Switzerland
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
 *
 */

#ifdef CONFIG_VIDEO
#ifndef _VIDEO_H
#define _VIDEO_H

int video_init(int busdevfunc);
void video_clear(void);
void video_putc(char ch);
void video_puts(const char *s);
void video_puts_a(const char *s, char attr);
unsigned char video_set_attr(unsigned char attr);

void video_box_area(int x, int y, int w, int h);
void video_dbox_area(int x, int y, int w, int h);
void video_wipe_c_area(unsigned char ch, int x, int y, int w, int h);

void video_wipe_a_area(unsigned char attr, int x, int y, int w, int h);
void video_wipe_ca_area(unsigned char ch, char attr, int x, int y, int w, int h);
void video_puts_axy(const char *s, char attr, int x, int y);
void video_putc_rxy(char ch, int x, int y);
void video_putc_xy(char ch, int x, int y);
unsigned char video_set_attr_xy(unsigned char attr, int x, int y);
void video_copy(unsigned short *buffer);
void video_write(unsigned short *buffer);

#define VGA_ATTR_CLR_RED	0x4
#define VGA_ATTR_CLR_GRN	0x2
#define VGA_ATTR_CLR_BLU	0x1
#define VGA_ATTR_CLR_YEL	(VGA_ATTR_CLR_RED | VGA_ATTR_CLR_GRN)
#define VGA_ATTR_CLR_CYN	(VGA_ATTR_CLR_GRN | VGA_ATTR_CLR_BLU)
#define VGA_ATTR_CLR_MAG	(VGA_ATTR_CLR_BLU | VGA_ATTR_CLR_RED)
#define VGA_ATTR_CLR_BLK	0
#define VGA_ATTR_CLR_WHT	(VGA_ATTR_CLR_RED | VGA_ATTR_CLR_GRN | VGA_ATTR_CLR_BLU)

#define VGA_ATTR_BNK			0x80
#define VGA_ATTR_ITN			0x08

#define VGA_ATTR_BG_MSK  0x70
#define VGA_ATTR_FG_MSK  0x07

#define VGA_ATTR_BG_GET(v) (((v) & VGA_ATTR_BG_MSK)>>4)
#define VGA_ATTR_BG_SET(v, c) (((c) & VGA_ATTR_FG_MSK)<<4) | (v & ~VGA_ATTR_BG_MSK))

#define VGA_ATTR_FG_GET(v) ((v) & VGA_ATTR_FG_MSK)
#define VGA_ATTR_FG_SET(v, c) ((c) & VGA_ATTR_FG_MSK) | (v & ~VGA_ATTR_FG_MSK))

#define VGA_ATTR_FG_BG_SET(v, b, f) (VGA_ATTR_BG_SET(v, b) | VGA_ATTR_FG_SET(v, cf))

#define VGA_ATTR_INVERT(A) ((((A)&0x7)<<4)|(((A)&0x70)>>4) |((A)&0x88))

#endif	/* _VIDEO_H */
#endif	/* CONFIG_VIDEO */
